#include "TestConnect.h"
#include <cppunit/TestAssert.h>


void startMockServer() {
    MockServer server;

    server.init_asio();
    server.set_open_handler([](websocketpp::connection_hdl hdl) {
        std::cout << "Mock server connection opened." << std::endl;
    });
    server.set_close_handler([](websocketpp::connection_hdl hdl) {
        std::cout << "Mock server connection closed." << std::endl;
    });

    server.listen(9002);
    server.start_accept();
    server.run();
}

void TestConnect::setUp() {
    client = new WebClient();
    connected_flag = false;

    client->setOnConnect([this]() {
        connected_flag = true;
        std::cout << "Connection callback triggered." << std::endl;
    });

    mockServerThread = new std::thread(startMockServer);
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void TestConnect::tearDown() {
    delete client;
    client = nullptr;

    if (mockServerThread && mockServerThread->joinable()) {
        mockServerThread->detach();
        delete mockServerThread;
        mockServerThread = nullptr;
    }
}

void TestConnect::testConnectSuccess() {
    try {
        client->connect("ws://localhost:9002", "localhost");
        std::this_thread::sleep_for(std::chrono::seconds(1));

        CPPUNIT_ASSERT_MESSAGE("Connection should be successful", connected_flag);
    } catch (const std::exception &e) {
        CPPUNIT_FAIL(std::string("Unexpected exception: ") + e.what());
    }
}

void TestConnect::testConnectFailure() {
    try {
        client->connect("wss://invalid.websocket.server", "invalid.websocket.server");
        CPPUNIT_FAIL("Connection should throw an exception on failure");
    } catch (const std::runtime_error &e) {
        CPPUNIT_ASSERT_MESSAGE("Expected exception for connection failure", true);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestConnect);
