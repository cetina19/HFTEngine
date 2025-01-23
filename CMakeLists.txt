cmake_minimum_required(VERSION 3.10)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

project(HFTEngine)

enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

find_path(CPPUNIT_INCLUDE_DIR
    NAMES cppunit/TestCase.h
    PATHS /usr/include /usr/local/include
)

find_library(CPPUNIT_LIBRARY
    NAMES cppunit
    PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib
)

if(NOT CPPUNIT_INCLUDE_DIR OR NOT CPPUNIT_LIBRARY)
    message(FATAL_ERROR "CppUnit not found. Please install libcppunit-dev.")
endif()

add_library(CppUnit::CppUnit UNKNOWN IMPORTED)
set_target_properties(CppUnit::CppUnit PROPERTIES
    IMPORTED_LOCATION "${CPPUNIT_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${CPPUNIT_INCLUDE_DIR}"
)

find_package(CURL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(Threads REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(
    include
)

file(GLOB LIB_SOURCES src/*.cpp)
add_library(HFTEngineLib ${LIB_SOURCES})

target_include_directories(HFTEngineLib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(HFTEngineLib
    PUBLIC
        CURL::libcurl
        Boost::system
        Boost::thread
        Threads::Threads
        nlohmann_json::nlohmann_json
        OpenSSL::SSL
        OpenSSL::Crypto
)

file(GLOB SOURCES src/*.cpp)

add_executable(HFTEngine ${SOURCES})

target_link_libraries(HFTEngine
    PRIVATE
        HFTEngineLib
)

file(GLOB TEST_SOURCES tests/*.cpp)
add_executable(HFTEngineTests ${TEST_SOURCES})

target_link_libraries(
    HFTEngineTests
    PRIVATE
        CppUnit::CppUnit
        HFTEngineLib
)

add_test(NAME HFTEngineTests COMMAND HFTEngineTests)
