/* Copyright 2015–2017 Kullo GmbH. All rights reserved. */
#ifdef _MSC_VER
    // Stop MSVC from polluting the namespace with min and max macros. These
    // collide with std::numeric_limits<T>::max().
    #define NOMINMAX
#endif

#include "httpclient/httpclientimpl.h"

#include <cstring>
#include <limits>
#include <regex>
#include <sstream>

#include <kulloclient/http/Request.h>
#include <kulloclient/http/RequestListener.h>
#include <kulloclient/http/Response.h>
#include <kulloclient/http/ResponseError.h>
#include <kulloclient/http/ResponseListener.h>
#include <kulloclient/http/TransferProgress.h>
#include <kulloclient/util/assert.h>
#include <kulloclient/util/librarylogger.h>
#include <kulloclient/util/strings.h>

#include "httpclient/cabundle.h"
#include "httpclient/utils.h"

using namespace Kullo;
using curl::curl_easy;
using curl::curl_easy_exception;
using curl::curl_header;
using curl::curl_pair;

namespace HttpClient {

namespace {

int xferInfoCallback(
        void *function,
        curl_off_t dltotal,
        curl_off_t dlnow,
        curl_off_t ultotal,
        curl_off_t ulnow)
{
    // get bound progress function from ResponseListener
    kulloAssert(function);
    auto progressFunction = static_cast<ProgressFunctionType*>(function);

    // returning non-zero cancels the request
    Http::TransferProgress progress = {ulnow, ultotal, dlnow, dltotal};
    auto result = (*progressFunction)(progress);
    return result == Http::ProgressResult::Cancel ? 1 : 0;
}

size_t readCallback(char *buffer, size_t size, size_t nmemb, void *function)
{
    // get bound read function from RequestListener
    kulloAssert(function);
    auto readFunction = static_cast<ReadFunctionType*>(function);

    // get data from listener
    auto result = (*readFunction)(size * nmemb);
    kulloAssert(result.size() <= size * nmemb);

    // pass data to curl
    memcpy(buffer, result.data(), result.size());
    return result.size();
}

size_t headerCallback(char *buffer, size_t size, size_t nmemb, void *headers)
{
    // get headers vector from Result
    kulloAssert(headers);
    auto headersVec = static_cast<std::vector<Http::HttpHeader>*>(headers);

    // get header line from curl
    std::string line(buffer, size * nmemb);

    const auto keyValuePair = Utils::parseHeader(line);
    if (keyValuePair)
    {
        headersVec->emplace_back(keyValuePair->first, keyValuePair->second);
    }

    return line.size();
}

size_t writeCallback(char *buffer, size_t size, size_t nmemb, void *function)
{
    // get bound dataReceived function from ResponseListener
    kulloAssert(function);
    auto dataReceivedFunction = static_cast<WriteFunctionType*>(function);

    // wrap data from curl in vector
    std::vector<uint8_t> data(size * nmemb);
    memcpy(data.data(), buffer, size * nmemb);

    // pass data to listener
    (*dataReceivedFunction)(data);
    return data.size();
}

size_t noopWriteCallback(char *buffer, size_t size, size_t nmemb, void *userdata)
{
    (void)buffer;
    (void)userdata;
    return size * nmemb;
}

std::string formattedTraceback(const curl_easy_exception &exception)
{
    std::ostringstream out;
    bool first = true;
    for (const auto &pair : exception.get_traceback())
    {
        if (!first) out << "\n";
        first = false;
        out << "'" << pair.first << "' in " << pair.second;
    }
    return out.str();
}

}

HttpClientImpl::HttpClientImpl(const boost::optional<std::string> &acceptLanguage)
    : acceptLanguage_(acceptLanguage),
      curlEasy_(new curl_easy),
      requestState_(new RequestState)
{
}

HttpClientImpl::~HttpClientImpl()
{
}

Http::Response HttpClientImpl::sendRequest(const Http::Request &request,
        int32_t timeoutMs,
        const std::shared_ptr<Http::RequestListener> &requestListener,
        const std::shared_ptr<Http::ResponseListener> &responseListener)
{
    kulloAssert(timeoutMs >= 0);

    Http::Response result(
                boost::optional<Http::ResponseError>(),
                0,
                std::vector<Http::HttpHeader>());

    // Disable OCSP checking to prevents Kullo from going down when the CA's
    // infrastructure does
    curlEasy_->add<CURLOPT_SSL_OPTIONS>(CURLSSLOPT_NO_REVOKE);

#ifdef _WIN32
    // Disable ALPN which is not supported on Windows < 8.1
    // https://msdn.microsoft.com/en-us/library/windows/desktop/aa379340%28v=vs.85%29.aspx
    curlEasy_->add<CURLOPT_SSL_ENABLE_ALPN>(0);
#endif

    // CA bundle
    if (CaBundle::available())
    {
        curlEasy_->add<CURLOPT_CAINFO>(CaBundle::path().c_str());
    }

    // timeout
    if (timeoutMs > 0)
    {
        curlEasy_->add<CURLOPT_TIMEOUT_MS>(timeoutMs);
    }

    // URL
    curlEasy_->add<CURLOPT_URL>(request.url.c_str());

    // request headers
    addHeaders(request.headers);

    // register callbacks
    setMethod(request.method, requestListener.get());
    addResponseHeaderCallback(result.headers);

    if (responseListener)
    {
        addCancelCallback(responseListener.get());
        addResponseBodyCallback(responseListener.get());
    }
    else
    {
        // disable printing responses to stdout
        curlEasy_->add<CURLOPT_WRITEFUNCTION>(&noopWriteCallback);
    }

    // request more specific error messages from cURL
    curlErrorBuffer_[0] = '\0';
    curlEasy_->add<CURLOPT_ERRORBUFFER>(curlErrorBuffer_);

    try {
        curlEasy_->perform();
        result.statusCode = *curlEasy_->get_info<long>(CURLINFO_RESPONSE_CODE);
    }
    catch (curl_easy_exception &ex)
    {
        switch (ex.get_code())
        {
        case CURLE_ABORTED_BY_CALLBACK:
            result.error = Http::ResponseError::Canceled;
            break;

        case CURLE_OPERATION_TIMEDOUT:
            result.error = Http::ResponseError::Timeout;
            Log.e() << "HTTP exception: " << formattedTraceback(ex) << "\n"
                    << "Details: " << curlErrorBuffer_;
            break;

        default:
            result.error = Http::ResponseError::NetworkError;

            Log.e() << "HTTP exception: " << formattedTraceback(ex) << "\n"
                    << "Details: " << curlErrorBuffer_;
        }
    }

    curlEasy_->reset();
    requestState_.reset(new RequestState);
    return result;
}

void HttpClientImpl::addHeaders(const std::vector<Http::HttpHeader> &headers)
{
    bool acceptLanguageSet = false;
    requestState_->reqHeaders.reset(new curl_header());
    for (const auto &hdr : headers)
    {
        requestState_->reqHeaders->add(hdr.key + ": " + hdr.value);
        auto lcaseKey = Util::Strings::toLower(hdr.key);
        if (lcaseKey == "content-length")
        {
            curlEasy_->add<CURLOPT_INFILESIZE>(std::stol(hdr.value));
        }
        else if (lcaseKey == "accept-language")
        {
            acceptLanguageSet = true;
        }
    }
    if (!acceptLanguageSet && acceptLanguage_)
    {
        requestState_->reqHeaders->add("Accept-Language: " + *acceptLanguage_);
    }
    curlEasy_->add(curl_pair<CURLoption, curl_header>(
                       CURLOPT_HTTPHEADER, *requestState_->reqHeaders));
}

void HttpClientImpl::addCancelCallback(Http::ResponseListener *respL)
{
    kulloAssert(respL);

    requestState_->progressFunction.reset(
                new ProgressFunctionType(
                    std::bind(
                        &Http::ResponseListener::progressed,
                        respL,
                        std::placeholders::_1
                        )));

    curlEasy_->add<CURLOPT_NOPROGRESS>(0);
    curlEasy_->add<CURLOPT_XFERINFOFUNCTION>(&xferInfoCallback);
    curlEasy_->add<CURLOPT_XFERINFODATA>(requestState_->progressFunction.get());
}

void HttpClientImpl::addRequestBodyCallback(Http::RequestListener *reqL)
{
    kulloAssert(reqL);

    requestState_->reqBodyReadFunction.reset(
                new ReadFunctionType(
                    std::bind(
                        &Http::RequestListener::read,
                        reqL,
                        std::placeholders::_1
                        )));

    curlEasy_->add<CURLOPT_READFUNCTION>(&readCallback);
    curlEasy_->add<CURLOPT_READDATA>(requestState_->reqBodyReadFunction.get());
}

void HttpClientImpl::setMethod(
        Http::HttpMethod method, Http::RequestListener *reqL)
{
    switch (method)
    {
    case Http::HttpMethod::Get:
        curlEasy_->add<CURLOPT_HTTPGET>(1);
        break;

    case Http::HttpMethod::Put:
        curlEasy_->add<CURLOPT_UPLOAD>(1);
        addRequestBodyCallback(reqL);
        break;

    case Http::HttpMethod::Post:
        curlEasy_->add<CURLOPT_UPLOAD>(1);
        curlEasy_->add<CURLOPT_CUSTOMREQUEST>("POST");
        addRequestBodyCallback(reqL);
        break;

    case Http::HttpMethod::Patch:
        curlEasy_->add<CURLOPT_UPLOAD>(1);
        curlEasy_->add<CURLOPT_CUSTOMREQUEST>("PATCH");
        addRequestBodyCallback(reqL);
        break;

    case Http::HttpMethod::Delete:
        curlEasy_->add<CURLOPT_HTTPGET>(1);
        curlEasy_->add<CURLOPT_CUSTOMREQUEST>("DELETE");
        break;
    }
}

void HttpClientImpl::addResponseHeaderCallback(
        std::vector<Http::HttpHeader> &headers)
{
    curlEasy_->add<CURLOPT_HEADERFUNCTION>(&headerCallback);
    curlEasy_->add<CURLOPT_HEADERDATA>(&headers);
}

void HttpClientImpl::addResponseBodyCallback(Http::ResponseListener *respL)
{
    kulloAssert(respL);

    requestState_->respBodyWriteFunction.reset(
                new WriteFunctionType(
                    std::bind(
                        &Http::ResponseListener::dataReceived,
                        respL,
                        std::placeholders::_1
                        )));

    curlEasy_->add<CURLOPT_WRITEFUNCTION>(&writeCallback);
    curlEasy_->add<CURLOPT_WRITEDATA>(requestState_->respBodyWriteFunction.get());
}

}
