/* Copyright 2015–2016 Kullo GmbH. All rights reserved. */
#include <gmock/gmock.h>

#include <kulloclient/protocol/debug.h>

#include "httpclient/httpclientimpl.h"
#include "tests/mock_requestlistener.h"
#include "tests/mock_responselistener.h"

using namespace testing;
using namespace Kullo;

class HttpClientImpl : public Test
{
protected:
    HttpClientImpl()
        : req(new Http::Request(
                  Http::HttpMethod::Get,
                  "http://httpbin.org/get",
                  std::vector<Http::HttpHeader>()
                  ))
        , reqL(std::make_shared<MockRequestListener>())
        , respL(std::make_shared<MockResponseListener>())
    {
    }

    HttpClient::HttpClientImpl uut;
    std::unique_ptr<Http::Request> req;
    std::shared_ptr<MockRequestListener> reqL;
    std::shared_ptr<MockResponseListener> respL;
};

#if 0
TEST_F(HttpClientImpl, successOnBadAuth)
{
    EXPECT_CALL(*respL, progress(_))
            .WillRepeatedly(Return(Http::ProgressResult::Continue));
    req->url = "http://httpbin.org/basic-auth/user/passwd";
    auto resp = uut.sendRequest(*req, 0, nullptr, respL);
    EXPECT_THAT(resp.error, Eq(boost::none));
    EXPECT_THAT(resp.statusCode, Eq(401));
}
#endif

TEST_F(HttpClientImpl, connectionFailure)
{
    EXPECT_CALL(*respL, progressed(_))
            .WillRepeatedly(Return(Http::ProgressResult::Continue));

    req->url = "http://localhost:1/";
    auto resp = uut.sendRequest(*req, 0, nullptr, respL);
    EXPECT_THAT(resp.error, Eq(Http::ResponseError::NetworkError));
    EXPECT_THAT(resp.statusCode, Eq(0));
}

TEST_F(HttpClientImpl, failsOnNegativeTimeout)
{
    EXPECT_THROW(uut.sendRequest(*req, -1, nullptr, respL), std::exception);
}
