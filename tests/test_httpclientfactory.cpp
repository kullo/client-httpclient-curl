/*
 * Copyright 2015–2019 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include <iostream>
#include <gmock/gmock.h>

#include "httpclient/httpclientfactoryimpl.h"

using namespace testing;

TEST(HttpClientFactory, versionsWorks)
{
    auto versions = HttpClient::HttpClientFactoryImpl().versions();
    EXPECT_THAT(versions, Contains(Key("cURL")));
}
