#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>
#include "client.h"
#include "dotenv.h"

using std::cout;
using std::cerr;
using std::endl;
using std::cin;

namespace {
    std::string API_KEY;
    std::string SECRET_KEY;
    std::vector<std::string> SYMBOLS;
}

std::mutex mtx;
std::condition_variable cv;
bool connected_flag = false;
bool authenticated_flag = false;
bool subscribed_flag = false;

void onConnect() {
    std::lock_guard<std::mutex> lock(mtx);
    connected_flag = true;
    cv.notify_one();
}

void onAuthenticate() {
    std::lock_guard<std::mutex> lock(mtx);
    authenticated_flag = true;
    cv.notify_one();
}

void onSubscribe() {
    std::lock_guard<std::mutex> lock(mtx);
    subscribed_flag = true;
    cv.notify_one();
}

int main() {
    dotenv::EnvSingleton& env = dotenv::EnvSingleton::get_instance();
    if (!env.load_env("../.env")) {
        cerr << "Failed to load .env file!" << endl;
        return EXIT_FAILURE;
    }

    try {
        API_KEY = std::get<std::string>(*env.get("API_KEY"));
        SECRET_KEY = std::get<std::string>(*env.get("API_SECRET_KEY"));
        SYMBOLS = std::get<std::vector<std::string>>(*env.get("SYMBOL_LIST"));
    } catch (const std::bad_variant_access& e) {
        cerr << "Type mismatch when accessing .env variables: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    try {
        WebClient clientObject(API_KEY, SECRET_KEY);

        clientObject.setOnConnect(onConnect);
        clientObject.setOnAuthenticate(onAuthenticate);
        clientObject.setOnSubscribe(onSubscribe);

        string uri      = "wss://stream.data.alpaca.markets/v1beta3/crypto/us";
        string hostname = "stream.data.alpaca.markets"; 

        clientObject.connect(uri, hostname);

        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, []{ return connected_flag; });
        }

        clientObject.authenticate();

        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, []{ return authenticated_flag; });
        }

        clientObject.subscribeBars(SYMBOLS);

        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, []{ return subscribed_flag; });
        }

        cout << "Streaming bars. Press Enter to exit..." << endl;
        cin.get();

        clientObject.unsubscribeBars(SYMBOLS);
        
        clientObject.disconnect();
        clientObject.executeOrders();

    } catch (const std::exception& e) {
        cerr << "Error: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
