#include "client.h"
#include "bar.h"

using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::thread;

using json = nlohmann::json;

WebClient::WebClient() : connected(false) {
    c.init_asio();
}

WebClient::WebClient(string& api_key, string& api_secret_key) 
    : ALPACA_API_KEY(api_key), ALPACA_API_SECRET_KEY(api_secret_key), connected(false), processingPool(std::thread::hardware_concurrency()) {
    c.init_asio();
}

WebClient::~WebClient() {
    if (thread.joinable()) {
        thread.join();
    }
    processingPool.join();
}

void WebClient::setOnConnect(function<void()> callback) {
    onConnectCallback = callback;
}

void WebClient::setOnAuthenticate(function<void()> callback) {
    onAuthenticateCallback = callback;
}

void WebClient::setOnSubscribe(function<void()> callback) {
    onSubscribeCallback = callback;
}

void WebClient::connect(const string& uri, const string& hostname) {
    c.clear_access_channels(websocketpp::log::alevel::all);

    c.set_message_handler(bind(&WebClient::onMessage, this, _1, _2));
    c.set_tls_init_handler(bind(&WebClient::onTLS, this, hostname.c_str(), _1));
    c.set_open_handler(bind(&WebClient::onOpen, this, _1));
    c.set_close_handler(bind(&WebClient::onClose, this, _1));
    c.set_fail_handler(bind(&WebClient::onFail, this, _1));

    error_code ec;
    client::connection_ptr con = c.get_connection(uri, ec);
    if(ec){
        throw std::runtime_error("Could not create connection: " + ec.message());
    }

    c.connect(con);

    thread = std::thread(&client::run, &c);
}

void WebClient::disconnect() {
    if(connected){
        c.close(hdl, websocketpp::close::status::normal, "Client disconnecting");
        connected = false;
    }
}

void WebClient::authenticate() {
    if (connected) {
        json j;
        j["action"] = "auth";
        j["key"]    = ALPACA_API_KEY;
        j["secret"] = ALPACA_API_SECRET_KEY;

        string message = j.dump();
        c.send(hdl, message, websocketpp::frame::opcode::text);
        cout << "Sent authentication message to Alpaca." << endl;
    } else {
        cerr << "Cannot authenticate; not connected to WebSocket server." << endl;
    }
}

void WebClient::subscribeBars(const vector<string>& symbols) {
    if (connected) {
        json j;
        j["action"] = "subscribe";
        j["bars"]   = symbols;

        string message = j.dump();
        c.send(hdl, message, websocketpp::frame::opcode::text);
        cout << "Sent subscription message: " << message << endl;
    } else {
        cerr << "Cannot subscribe; not connected to WebSocket server." << endl;
    }
}


void WebClient::unsubscribeBars(const vector<string>& symbols) {
    if (connected) {
        json j;
        j["action"] = "unsubscribe";
        j["bars"]   = symbols;

        string message = j.dump();
        c.send(hdl, message, websocketpp::frame::opcode::text);

        std::lock_guard<std::mutex> lock(subMutex);
        for (auto &sym : symbols) {
            subscriptions.erase(sym);
        }

        cout << "Unsubscribed from symbols: ";
        for (auto &sym : symbols) cout << sym << " ";
        cout << endl;
    } else {
        cerr << "Cannot unsubscribe; not connected to WebSocket server." << endl;
    }
}

void WebClient::run() {
    c.run();
}

void WebClient::onOpen(connection_hdl hdl) {
    this->hdl = hdl;
    connected = true;
    cout << "Connection opened." << endl;

    if (onConnectCallback)
        onConnectCallback();
}

void WebClient::onClose(connection_hdl hdl) {
    connected = false;
    cout << "Connection closed." << endl;
}

void WebClient::onFail(connection_hdl hdl) {
    cerr << "Connection failed." << endl;
}

void WebClient::onMessage(connection_hdl hdl, message_ptr msg) {
    boost::asio::post(this->processingPool, [this, msg]() {
            if (msg->get_opcode() != websocketpp::frame::opcode::text) {
                cout << "Received non-text message. (Opcode=" << msg->get_opcode() << ") Ignoring." << endl;
                return;
            }

            std::string payload = msg->get_payload();
            if (payload.empty()) {
                cout << "Received empty text message. Ignoring." << endl;
                return;
            }

            try {
                auto response = json::parse(payload);

                if (response.is_array()) {
                    for (const auto& elem : response) {
                        if (elem.is_object()) {
                            if (elem.value("T", "") == "success" && elem.value("msg", "") == "authenticated") {
                                cout << "Authentication successful." << endl;
                                if (onAuthenticateCallback)
                                    onAuthenticateCallback();
                                return;
                            }

                            if(elem.value("T", "") == "subscription") {
                                cout << "Subscription successful for symbols: " << elem.dump(4) << endl;
                                if(onSubscribeCallback)
                                    onSubscribeCallback();
                                return;
                            }

                            if(elem.value("T", "") == "b"){
                                Bar bar(elem);
                                {
                                    std::lock_guard<std::mutex> lock(orderMutex);
                                    createOrder(bar);
                                    cout << orders.size() << ". Bar received:"
                                            << " symbol=" << bar.S
                                            << " open=" << bar.o
                                            << " close=" << bar.c
                                            << " time=" << bar.t << endl;
                                }
                            } else {
                                cout << "Unknown JSON object in array: " << elem.dump(4) << endl;
                            }
                        } else {
                            cout << "Non-object JSON in array: " << elem.dump() << endl;
                        }
                    }
                }
                else if (response.is_object()) {
                    if (response.value("T", "") == "b") {
                        Bar bar(response);
                        cout << "Single bar received: symbol=" << bar.S
                                  << " open=" << bar.o
                                  << " close=" << bar.c
                                  << " time=" << bar.t << endl;
                    } else if (response.value("T", "") == "subscription") {
                        cout << "Subscription successful for symbols: " << response.dump(4) << endl;
                        if (onSubscribeCallback) {
                            onSubscribeCallback();
                        }
                    } else {
                        cout << "Received object: " << response.dump(4) << endl;
                    }
                }
                else {
                    cout << "Received non-object, non-array JSON: " << response.dump() << endl;
                }
            } catch (const std::exception& e) {
                cerr << "Failed to parse JSON message: " << e.what()
                          << "\nPayload was: " << payload << endl;
            }
        });
}


void WebClient::placeOrder(Order& order)
{
    try {
        nlohmann::json orderBody;
        orderBody["symbol"]        = order.symbol;
        orderBody["qty"]           = order.qty;
        orderBody["side"]          = order.side;
        orderBody["type"]          = order.type;
        orderBody["time_in_force"] = order.time_in_force;

        string jsonBody = orderBody.dump();

        boost::asio::io_context ioc;
        boost::asio::ssl::context sslCtx(boost::asio::ssl::context::tlsv12_client);
        sslCtx.set_verify_mode(boost::asio::ssl::verify_none);

        const string host = "paper-api.alpaca.markets";
        const string port = "443";
        boost::asio::ip::tcp::resolver resolver(ioc);
        auto const results = resolver.resolve(host, port);

        boost::beast::tcp_stream plain_stream(ioc);
        plain_stream.connect(results);

        boost::beast::ssl_stream<boost::beast::tcp_stream> sslStream(std::move(plain_stream), sslCtx);

        ::SSL_set_tlsext_host_name(sslStream.native_handle(), host.c_str());

        sslStream.handshake(boost::asio::ssl::stream_base::client);

        boost::beast::http::request<boost::beast::http::string_body> req{
            boost::beast::http::verb::post, "/v2/orders", 11
        };
        req.set(boost::beast::http::field::host, host);
        req.set("APCA-API-KEY-ID", ALPACA_API_KEY);
        req.set("APCA-API-SECRET-KEY", ALPACA_API_SECRET_KEY);
        req.set(boost::beast::http::field::content_type, "application/json");
        req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.body() = jsonBody;
        req.prepare_payload();

        boost::beast::http::write(sslStream, req);

        boost::beast::flat_buffer buffer;
        boost::beast::http::response<boost::beast::http::string_body> res;
        boost::beast::http::read(sslStream, buffer, res);
        cout << "Order response code: " << res.result_int() << endl;
        if (res.result_int() >= 200 && res.result_int() < 300) {
            cout << "Order placed successfully!" << endl;
        } else {
            cerr << "Order failed or partially successful." << endl;
        }

        boost::system::error_code ec;
        sslStream.shutdown(ec);

    } catch (const std::exception &e) {
        cerr << "Exception in placeOrder: " << e.what() << endl;
    }
}

void WebClient::createOrder(Bar& bar){
    string symbolBuffer = bar.S;
    symbolBuffer.erase(symbolBuffer.begin() + 3);
    const string symbol = symbolBuffer;
    const string qty = "0.001";
    const string side = "sell";
    const string type = "market";
    const string time_in_force = "gtc";
    orders.push_back(Order(symbol, qty, side, type, time_in_force));
}

void WebClient::executeOrders(){
    unsigned int numThreads = std::thread::hardware_concurrency();
    if(numThreads == 0) numThreads = 4;

    boost::asio::thread_pool pool(numThreads);

    std::mutex coutMutex;

    auto start = std::chrono::high_resolution_clock::now();

    for(auto& order : orders){
        boost::asio::post(pool, [this, &order, &coutMutex]() {
            try{
                {
                    std::lock_guard<std::mutex> lock(coutMutex);
                    cout << "Placing order for symbol: " << order.symbol << endl;
                }
                placeOrder(order);
            }
            catch(const std::exception& e){
                std::lock_guard<std::mutex> lock(coutMutex);
                cerr << "Exception while placing order for " << order.symbol 
                          << ": " << e.what() << endl;
            }
        });
    }

    pool.join();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    {
        std::lock_guard<std::mutex> lock(coutMutex);
        cout << "Executed " << orders.size() << " orders concurrently in " 
                  << elapsed.count() << " seconds." << endl;
    }

    orders.clear();
}


context_ptr WebClient::onTLS(const char* hostname, connection_hdl) {
    context_ptr ctx = websocketpp::lib::make_shared<websocketpp::lib::asio::ssl::context>(
        websocketpp::lib::asio::ssl::context::tlsv12
    );

    ctx->set_options(websocketpp::lib::asio::ssl::context::default_workarounds |
                     websocketpp::lib::asio::ssl::context::no_sslv2 |
                     websocketpp::lib::asio::ssl::context::no_sslv3 |
                     websocketpp::lib::asio::ssl::context::single_dh_use);

    ctx->set_verify_mode(boost::asio::ssl::verify_none);

    return ctx;
}