#include <iostream>
#include <gmock/gmock.h>

#include "httpclient/httpclientfactoryimpl.h"

using namespace testing;

TEST(HttpClientFactory, versionsWorks)
{
    auto versions = HttpClient::HttpClientFactoryImpl().versions();
    EXPECT_THAT(versions, Contains(Key("cURL")));
}

