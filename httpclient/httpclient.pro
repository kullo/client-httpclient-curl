include(../common_pre.pri)

TARGET = httpclient
TEMPLATE = lib
CONFIG += staticlib
CONFIG -= qt

SOURCES += \
    cabundle.cpp \
    httpclientfactoryimpl.cpp \
    httpclientimpl.cpp

HEADERS += \
    cabundle.h \
    httpclientfactoryimpl.h \
    httpclientimpl.h

# BEGIN curl
windowsDebug(): INCLUDEPATH += $$PWD/../../build-curl-debug/include
else:           INCLUDEPATH += $$PWD/../../build-curl/include

win32: QMAKE_CXX_FLAGS += -DCURL_STATICLIB
# END curl

include(../common_post.pri)
