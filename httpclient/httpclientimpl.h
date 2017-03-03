/* Copyright 2015–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <functional>
#include <memory>
#include <vector>

#include <boost/optional.hpp>
#include <curl/curl.h>
#include <curl_easy.h>
#include <curl_header.h>

#include <kulloclient/http/HttpClient.h>
#include <kulloclient/http/HttpHeader.h>
#include <kulloclient/http/HttpMethod.h>
#include <kulloclient/http/ProgressResult.h>
#include <kulloclient/http/TransferProgress.h>

namespace HttpClient {

// represents ResponseListener::progressed
using ProgressFunctionType = std::function<
    Kullo::Http::ProgressResult(const Kullo::Http::TransferProgress &)
>;

// represents RequestListener::read
using ReadFunctionType = std::function<std::vector<uint8_t>(int64_t)>;

// represents ResponseListener::dataReceived
using WriteFunctionType = std::function<void(std::vector<uint8_t>)>;


class HttpClientImpl : public Kullo::Http::HttpClient
{
public:
    explicit HttpClientImpl(const boost::optional<std::string> &acceptLanguage);
    virtual ~HttpClientImpl() override;

    virtual Kullo::Http::Response sendRequest(const Kullo::Http::Request &request,
            int32_t timeoutMs,
            const std::shared_ptr<Kullo::Http::RequestListener> &requestListener,
            const std::shared_ptr<Kullo::Http::ResponseListener> &responseListener
            ) override;

private:
    struct RequestState
    {
        std::unique_ptr<curl_header> reqHeaders;
        std::unique_ptr<ProgressFunctionType> progressFunction;
        std::unique_ptr<ReadFunctionType> reqBodyReadFunction;
        std::unique_ptr<WriteFunctionType> respBodyWriteFunction;
    };

    void addHeaders(const std::vector<Kullo::Http::HttpHeader> &headers);

    void addCancelCallback(Kullo::Http::ResponseListener *respL);

    void addRequestBodyCallback(Kullo::Http::RequestListener *reqL);

    void setMethod(
            Kullo::Http::HttpMethod method,
            Kullo::Http::RequestListener *reqL);

    void addResponseHeaderCallback(
            std::vector<Kullo::Http::HttpHeader> &headers);

    void addResponseBodyCallback(Kullo::Http::ResponseListener *respL);

    boost::optional<std::string> acceptLanguage_;

    // must live longer than curlEasy_, so declare it before curlEasy_
    char curlErrorBuffer_[CURL_ERROR_SIZE];
    std::unique_ptr<curl::curl_easy> curlEasy_;
    std::unique_ptr<RequestState> requestState_;
};

}
