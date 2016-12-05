add_executable(httpclient_tests
    mock_requestlistener.h
    mock_responselistener.h

    test_cabundle.cpp
    test_httpclient.cpp
    test_httpclientfactory.cpp
    test_utils.cpp
)

target_link_libraries(httpclient_tests
    PUBLIC
        httpclient
        gmock_main
)

## configure_file updates the file when it has changed, even when httpclient hasn't changed
#foreach(_filename libcrypto.so.1.0.0 libssl.so.1.0.0)
#    configure_file(
#        ../../build-openssl/lib/${_filename}
#        ${CMAKE_CURRENT_BINARY_DIR}
#        COPYONLY
#    )
#endforeach()
#set(_filename)
