cmake_minimum_required(VERSION 2.8.12)  # version provided by Ubuntu 14.04

# These must come before the project directive, won't work otherwise

if((NOT CMAKE_BUILD_TYPE) OR (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    set(install_prefix_suffix "-debug")
else()
    set(install_prefix_suffix "")
endif()
set(
    CMAKE_INSTALL_PREFIX
        "${CMAKE_CURRENT_BINARY_DIR}/../bin-httpclient-curl${install_prefix_suffix}"
    CACHE PATH "destination for make install"
)

if(NOT ("${CMAKE_VERSION}" VERSION_LESS "3.1"))
    set(CMAKE_INSTALL_MESSAGE LAZY
        CACHE STRING "Show messages during install? Lazy means only changed.")
    set_property(CACHE CMAKE_INSTALL_MESSAGE
        PROPERTY STRINGS "ALWAYS" "LAZY" "NEVER")
endif()

project(httpclient-curl)

set(_bin_libkullo "${CMAKE_CURRENT_SOURCE_DIR}/../bin-libkullo${install_prefix_suffix}")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${_bin_libkullo}/cmake")
include(CompilerSettings)
include(KulloCommon)

find_package(libkullo REQUIRED
    NO_MODULE
    PATHS ${_bin_libkullo}
    NO_DEFAULT_PATH
)

if(WIN32 AND ((NOT CMAKE_BUILD_TYPE) OR (CMAKE_BUILD_TYPE STREQUAL "Debug")))
    find_package(curl REQUIRED
        NO_MODULE
        PATHS "${CMAKE_CURRENT_SOURCE_DIR}/../build-curl-debug"
        NO_DEFAULT_PATH
    )
else()
    find_package(curl REQUIRED
        NO_MODULE
        PATHS "${CMAKE_CURRENT_SOURCE_DIR}/../build-curl"
        NO_DEFAULT_PATH
    )
endif()

if(UNIX AND NOT APPLE)
    find_package(openssl REQUIRED
        NO_MODULE
        PATHS "${CMAKE_CURRENT_SOURCE_DIR}/../build-openssl"
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
