cmake_minimum_required(VERSION 2.8.12)  # version provided by Ubuntu 14.04

# These must come before the project directive, won't work otherwise

set(
    CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/../bin-httpclient-curl"
    CACHE PATH "destination for make install"
)

if(NOT ("${CMAKE_VERSION}" VERSION_LESS "3.1"))
    set(CMAKE_INSTALL_MESSAGE LAZY
        CACHE STRING "Show messages during install? Lazy means only changed.")
    set_property(CACHE CMAKE_INSTALL_MESSAGE
        PROPERTY STRINGS "ALWAYS" "LAZY" "NEVER")
endif()

project(httpclient-curl)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_CURRENT_LIST_DIR}/../compilescripts/cmake-modules")
include(CompilerSettings)
include(KulloCommon)

find_package(botan REQUIRED
    NO_MODULE
    PATHS "${CMAKE_CURRENT_BINARY_DIR}/../bin-botan"
    NO_DEFAULT_PATH
)
find_package(libkullo REQUIRED
    NO_MODULE
    PATHS "${CMAKE_CURRENT_BINARY_DIR}/../bin-libkullo"
    NO_DEFAULT_PATH
)

if(WIN32 AND ((NOT CMAKE_BUILD_TYPE) OR (CMAKE_BUILD_TYPE STREQUAL "Debug")))
    find_package(curl REQUIRED
        NO_MODULE
        PATHS "${CMAKE_CURRENT_BINARY_DIR}/../../build-curl-debug"
        NO_DEFAULT_PATH
    )
else()
    find_package(curl REQUIRED
        NO_MODULE
        PATHS "${CMAKE_CURRENT_BINARY_DIR}/../../build-curl"
        NO_DEFAULT_PATH
    )
endif()

if(UNIX AND NOT APPLE)
    find_package(openssl REQUIRED
        NO_MODULE
        PATHS "${CMAKE_CURRENT_BINARY_DIR}/../../build-openssl"
        NO_DEFAULT_PATH
    )
endif()

### BEGIN internal libs

include_directories(.)

add_subdirectory("httpclient")
add_subdirectory("tests")

### END internal libs


enable_testing()
add_test("httpclient-curl" "tests/httpclient_tests")

install(DIRECTORY cmake/ DESTINATION cmake)
