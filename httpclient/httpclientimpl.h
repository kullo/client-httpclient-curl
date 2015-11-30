#pragma once

#include <functional>
#include <memory>
#include <vector>

#include <curl/curl.h>
#include <curl_easy.h>
#include <curl_header.h>

#include <kulloclient/http/HttpClient.h>
#include <kulloclient/http/ProgressResult.h>

namespace HttpClient {

// represents ResponseListener::progress
using ProgressFunctionType = std::function<
    Kullo::Http::ProgressResult(int64_t, int64_t, int64_t, int64_t)
>;

// represents RequestListener::read
using ReadFunctionType = std::function<std::vector<uint8_t>(int64_t)>;

// represents ResponseListener::dataReceived
using WriteFunctionType = std::function<void(std::vector<uint8_t>)>;


class HttpClientImpl : public Kullo::Http::HttpClient
{
public:
    HttpClientImpl();
    virtual ~HttpClientImpl() override;

    virtual Kullo::Http::Response sendRequest(
            const Kullo::Http::Request &request,
            int64_t timeout,
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

    // must live longer than curlEasy_, so declare it before curlEasy_
    char curlErrorBuffer_[CURL_ERROR_SIZE];
    std::unique_ptr<curl::curl_easy> curlEasy_;
    std::unique_ptr<RequestState> requestState_;
};

}
