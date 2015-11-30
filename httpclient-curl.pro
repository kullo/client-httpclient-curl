TEMPLATE = subdirs

SUBDIRS += \
    httpclient \
    tests

tests.depends += httpclient

include(common_pre.pri)
include(common_post.pri)
