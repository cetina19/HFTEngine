#ifndef TESTAUTHENTICATE_H
#define TESTAUTHENTICATE_H

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <thread>
#include <condition_variable>
#include <stdexcept>
#include "client.h"

#define MOCK_WEBSOCKET_URI "wss://mock.websocket.server"
#define MOCK_WEBSOCKET_HOSTNAME "mock.websocket.server"

class TestAuthenticate : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TestAuthenticate);
    CPPUNIT_TEST(testAuthenticateSuccess);
    CPPUNIT_TEST(testAuthenticateWithoutConnection);
    CPPUNIT_TEST_SUITE_END();

private:
    WebClient *client;
    std::mutex mtx;
    std::condition_variable cv;
    bool connected_flag;
    bool authenticated_flag;

public:
    void setUp() override;
    void tearDown() override;

    void testAuthenticateSuccess();
    void testAuthenticateWithoutConnection();
};

#endif
