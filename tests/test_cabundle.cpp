/*
 * Copyright 2015–2019 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include <fstream>
#include <gmock/gmock.h>

#include "httpclient/cabundle.h"

using namespace testing;

#ifdef __linux__
    static const bool IS_LINUX = true;
#else
    static const bool IS_LINUX = false;
#endif

TEST(CaBundle, availableWorks)
{
    EXPECT_THAT(HttpClient::CaBundle::available(), Eq(IS_LINUX));
}

TEST(CaBundle, pathWorks)
{
    if (HttpClient::CaBundle::available())
    {
        auto path = HttpClient::CaBundle::path();

        // check whether file exists and is readable
        auto readable = std::ifstream(path).good();
        EXPECT_THAT(readable, Eq(true));

        // must produce the same result when queried again
        // (tested to prevent errors in the lazy loading code)
        EXPECT_THAT(HttpClient::CaBundle::path(), StrEq(path));
    }
}
