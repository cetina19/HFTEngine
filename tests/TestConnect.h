#ifndef TESTCONNECT_H
#define TESTCONNECT_H

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <thread>
#include <iostream>
#include "client.h"

#define MOCK_WEBSOCKET_URI "wss://mock.websocket.server"
#define MOCK_WEBSOCKET_HOSTNAME "mock.websocket.server"

typedef websocketpp::server<websocketpp::config::asio> MockServer;


class TestConnect : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TestConnect);
    CPPUNIT_TEST(testConnectSuccess);
    CPPUNIT_TEST(testConnectFailure);
    CPPUNIT_TEST_SUITE_END();

private:
    WebClient *client;
    bool connected_flag;
    std::thread* mockServerThread;

public:
    void setUp() override;
    void tearDown() override;

    void testConnectSuccess();
    void testConnectFailure();
};

#endif
