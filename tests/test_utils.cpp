/* Copyright 2015–2016 Kullo GmbH. All rights reserved. */
#include <gmock/gmock.h>

#include "httpclient/utils.h"

using namespace testing;
using namespace HttpClient;

TEST(Utils, parseHeaderEmpty)
{
    EXPECT_THAT(Utils::parseHeader(""), Eq(boost::none));
    EXPECT_THAT(Utils::parseHeader(" "), Eq(boost::none));
    EXPECT_THAT(Utils::parseHeader("\r\n"), Eq(boost::none));
}

TEST(Utils, parseHeaderSimpleKeyValue)
{
    {
        auto out = Utils::parseHeader("Server: nginx\r\n");
        ASSERT_THAT(out, Ne(boost::none));
        EXPECT_THAT(*out, Pair(StrEq("Server"), StrEq("nginx")));
    }
    {
        auto out = Utils::parseHeader("Server: Apache2\r\n");
        ASSERT_THAT(out, Ne(boost::none));
        EXPECT_THAT(*out, Pair(StrEq("Server"), StrEq("Apache2")));
    }
    {
        auto out = Utils::parseHeader("Content-Type: application/json\r\n");
        ASSERT_THAT(out, Ne(boost::none));
        EXPECT_THAT(*out, Pair(StrEq("Content-Type"), StrEq("application/json")));
    }
    {
        auto out = Utils::parseHeader("Date: Mon, 10 Oct 2016 18:34:43 GMT\r\n");
        ASSERT_THAT(out, Ne(boost::none));
        EXPECT_THAT(*out, Pair(StrEq("Date"), StrEq("Mon, 10 Oct 2016 18:34:43 GMT")));
    }
}

TEST(Utils, parseHeaderSpacing)
{
    {
        // no spaces
        auto out = Utils::parseHeader("Server:Apache2\r\n");
        ASSERT_THAT(out, Ne(boost::none));
        EXPECT_THAT(*out, Pair(StrEq("Server"), StrEq("Apache2")));
    }
    {
        // more spaces
        auto out = Utils::parseHeader("Server:      Apache2\r\n");
        ASSERT_THAT(out, Ne(boost::none));
        EXPECT_THAT(*out, Pair(StrEq("Server"), StrEq("Apache2")));
    }
    {
        // spaces and tabs
        auto out = Utils::parseHeader("Server:  \t \t    Apache2\r\n");
        ASSERT_THAT(out, Ne(boost::none));
        EXPECT_THAT(*out, Pair(StrEq("Server"), StrEq("Apache2")));
    }
    {
        // trailing whitespace
        auto out = Utils::parseHeader("Server: Apache2   \t  \r\n");
        ASSERT_THAT(out, Ne(boost::none));
        EXPECT_THAT(*out, Pair(StrEq("Server"), StrEq("Apache2")));
    }
}

TEST(Utils, parseHeaderInvalid)
{
    // no key
    EXPECT_THAT(Utils::parseHeader(": bar"), Eq(boost::none));

    // no value
    EXPECT_THAT(Utils::parseHeader("foo: "), Eq(boost::none));

    // no colon
    EXPECT_THAT(Utils::parseHeader("foo bar"), Eq(boost::none));
}
