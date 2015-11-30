include(../common_pre.pri)

TARGET = tests
TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    test_cabundle.cpp \
    test_httpclient.cpp \
    test_httpclientfactory.cpp

HEADERS += \
    mock_requestlistener.h \
    mock_responselistener.h

linux() {
    copyOpenSslCommand = @echo Copying OpenSSL ...
    copyOpenSslCommand += ; rsync -pgo --update \"$$OUT_PWD/../../build-openssl/lib/libcrypto.so.1.0.0\" \"$$OUT_PWD/../../build-openssl/lib/libssl.so.1.0.0\" \"$$OUT_PWD/\"
    copyOpenSsl.commands = $$copyOpenSslCommand
    QMAKE_EXTRA_TARGETS += copyOpenSsl
    all.depends += copyOpenSsl
}

# BEGIN httpclient
win32:CONFIG(release, debug|release):     LIBS += -L$$OUT_PWD/../httpclient/release/ -lhttpclient
else:win32:CONFIG(debug, debug|release):  LIBS += -L$$OUT_PWD/../httpclient/debug/ -lhttpclient
else:unix:                                LIBS += -L$$OUT_PWD/../httpclient/ -lhttpclient

DEPENDPATH += $$PWD/../httpclient

win32:CONFIG(release, debug|release):    PRE_TARGETDEPS += $$OUT_PWD/../httpclient/release/httpclient.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../httpclient/debug/httpclient.lib
else:unix:                               PRE_TARGETDEPS += $$OUT_PWD/../httpclient/libhttpclient.a
# END httpclient

# BEGIN libkullo
LIBS += -L$$LIBKULLO_BIN_DIR/lib -lkulloclient -lboost_regex -lgmock_main
DEPENDPATH += $$LIBKULLO_BIN_DIR/include
win32 {
    TD_PREFIX = $${LIBKULLO_BIN_DIR}/lib/
    TD_SUFFIX = .lib
} else {
    TD_PREFIX = $${LIBKULLO_BIN_DIR}/lib/lib
    TD_SUFFIX = .a
}
PRE_TARGETDEPS += \
    $${TD_PREFIX}kulloclient$${TD_SUFFIX} \
    $${TD_PREFIX}boost_regex$${TD_SUFFIX} \
    $${TD_PREFIX}gmock_main$${TD_SUFFIX}
# END libkullo

# BEGIN curl
windowsDebug(): INCLUDEPATH += $$PWD/../../build-curl-debug/include
else:           INCLUDEPATH += $$PWD/../../build-curl/include

windowsDebug(): TD_BUILD_DIR = $$PWD/../../build-curl-debug
else:           TD_BUILD_DIR = $$PWD/../../build-curl
win32 {
    TD_FILE_CURL = libcurl.lib
    TD_FILE_CURLCPP = curlcpp.lib
} else {
    TD_FILE_CURL = libcurl.a
    TD_FILE_CURLCPP = libcurlcpp.a
}
PRE_TARGETDEPS += \
    $${TD_BUILD_DIR}/lib/$${TD_FILE_CURL} \
    $${TD_BUILD_DIR}/lib/$${TD_FILE_CURLCPP}

win32: QMAKE_CXX_FLAGS += -DCURL_STATICLIB

win32:CONFIG(release, debug|release):    LIBS += -L$$OUT_PWD/../../build-curl/lib -lcurlcpp -llibcurl
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../build-curl-debug/lib -lcurlcpp -llibcurl
else:macx:                               LIBS += -L$$OUT_PWD/../../build-curl/lib -lcurlcpp -lcurl -framework Security -framework Foundation -lz
else:                                    LIBS += -L$$OUT_PWD/../../build-curl/lib -lcurlcpp -lcurl -lz

linux():                                 LIBS += -L$$OUT_PWD/../../build-openssl/lib -lssl -lcrypto
# END curl

QMAKE_EXTRA_TARGETS += all

include(../common_post.pri)
