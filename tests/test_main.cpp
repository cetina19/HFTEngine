#include <cppunit/ui/text/TestRunner.h>
#include "TestAuthenticate.h"
#include "TestConnect.h"

int main(int argc, char* argv[]) {
    CppUnit::TextUi::TestRunner runner;

    runner.addTest(TestAuthenticate::suite());
    runner.addTest(TestConnect::suite());

    bool wasSuccessful = runner.run("", false);
    return wasSuccessful ? 0 : 1;
}
