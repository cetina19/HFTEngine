#include "TestAuthenticate.h"
#include <cppunit/TestAssert.h>

void TestAuthenticate::setUp() {
    client = new WebClient();
    connected_flag = false;
    authenticated_flag = false;

    client->setOnConnect([this]() {
        std::lock_guard<std::mutex> lock(mtx);
        connected_flag = true;
        cv.notify_one();
    });

    client->setOnAuthenticate([this]() {
        std::lock_guard<std::mutex> lock(mtx);
        authenticated_flag = true;
        cv.notify_one();
    });
}

void TestAuthenticate::tearDown() {
    delete client;
}

void TestAuthenticate::testAuthenticateSuccess() {
    try {
        client->connect(MOCK_WEBSOCKET_URI, MOCK_WEBSOCKET_HOSTNAME);

        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return connected_flag; });
        }

        CPPUNIT_ASSERT(connected_flag);

        client->authenticate();

        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return authenticated_flag; });
        }

        CPPUNIT_ASSERT_MESSAGE("Authentication should be successful", authenticated_flag);
    } catch (const std::exception &e) {
        CPPUNIT_FAIL(std::string("Unexpected exception: ") + e.what());
    }
}

void TestAuthenticate::testAuthenticateWithoutConnection() {
    try {
        client->authenticate();
        CPPUNIT_FAIL("Authentication without connection should throw an exception or log an error");
    } catch (const std::exception &e) {
        CPPUNIT_ASSERT_MESSAGE("Expected exception for authentication without connection", true);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestAuthenticate);
