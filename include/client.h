#ifndef CLIENT_H
#define CLIENT_H

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>
#include <websocketpp/common/functional.hpp>
#include <websocketpp/close.hpp>
#include <websocketpp/connection.hpp>

#include <string>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <map>
#include <set>
#include <mutex>
#include <queue>
#include <functional>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

#include "bar.h"
#include "order.h"

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef client::connection_ptr connection_ptr;
typedef websocketpp::connection_hdl connection_hdl;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;
typedef websocketpp::lib::error_code error_code;
typedef client::message_ptr message_ptr;
typedef std::string string;

using std::vector;
using std::function;
using std::mutex;
using std::set;
using std::cout;
using std::cin;
using std::cerr;
using std::endl;

class WebClient {
public:
    WebClient();
    WebClient(string& apiKey, string& apiSecretKey);
    ~WebClient();

    void connect(const string& uri, const string& hostname);
    void disconnect();

    void authenticate();

    void subscribeBars(const vector<string>& symbols);

    void unsubscribeBars(const vector<string>& symbols);

    void placeOrder(Order& order);

    void createOrder(Bar& bar);

    void executeOrders();

    void setOnConnect(function<void()> callback);
    void setOnAuthenticate(function<void()> callback);
    void setOnSubscribe(function<void()> callback);

private:
    void run();

    void onOpen(connection_hdl hdl);
    void onClose(connection_hdl hdl);
    void onFail(connection_hdl hdl);
    void onMessage(connection_hdl hdl, message_ptr msg);
    context_ptr onTLS(const char* hostname, connection_hdl);

    client c;
    websocketpp::lib::thread thread;
    connection_hdl hdl;
    bool connected;

    mutex subMutex;
    set<string> subscriptions;

    string ALPACA_API_KEY;
    string ALPACA_API_SECRET_KEY;

    function<void()> onConnectCallback;
    function<void()> onAuthenticateCallback;
    function<void()> onSubscribeCallback;

    mutex callbackMutex;
    bool authReceived = false;
    bool subscribeReceived = false;

    vector<Order> orders;
    std::mutex orderMutex;
    std::condition_variable orderCV;
    bool stopOrderThread = false;
    boost::asio::thread_pool processingPool;
};

#endif