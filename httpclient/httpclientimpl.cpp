#ifdef _MSC_VER
    // Stop MSVC from polluting the namespace with min and max macros. These
    // collide with std::numeric_limits<T>::max().
    #define NOMINMAX
#endif

#include "httpclient/httpclientimpl.h"

#include <cstring>
#include <limits>
#include <sstream>

#include <boost/regex.hpp>
#include <kulloclient/http/Request.h>
#include <kulloclient/http/RequestListener.h>
#include <kulloclient/http/Response.h>
#include <kulloclient/http/ResponseError.h>
#include <kulloclient/http/ResponseListener.h>
#include <kulloclient/util/assert.h>
#include <kulloclient/util/librarylogger.h>

#include "httpclient/cabundle.h"

using namespace Kullo;
using curl::curl_easy;
using curl::curl_easy_exception;
using curl::curl_header;
using curl::curl_pair;

namespace HttpClient {

static int xferInfoCallback(
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
    auto result = (*progressFunction)(ulnow, ultotal, dlnow, dltotal);
    return result == Http::ProgressResult::Cancel ? 1 : 0;
}

static size_t readCallback(
        char *buffer, size_t size, size_t nmemb, void *function)
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

static const boost::regex HEADER_REGEX(
        "\\A"          // start of string
            "([^:]*)"  // header name
            ":[ \t]*"  // colon and optional spaces or tabs
            "(.*)"     // header value
        "\\z"          // end of string
        );

static size_t headerCallback(
        char *buffer, size_t size, size_t nmemb, void *headers)
{
    // get headers vector from Result
    kulloAssert(headers);
    auto headersVec = static_cast<std::vector<Http::HttpHeader>*>(headers);

    // get header line from curl
    std::string line(buffer, size * nmemb);

    // split and append to headersVec
    boost::smatch matches;
    if (boost::regex_match(line, matches, HEADER_REGEX))
    {
        kulloAssert(matches.size() == 3); // full match + 2 groups
        headersVec->emplace_back(matches[1].str(), matches[2].str());
    }
    return line.size();
}

static size_t writeCallback(
        char *buffer, size_t size, size_t nmemb, void *function)
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

static size_t noopWriteCallback(
        char *buffer, size_t size, size_t nmemb, void *userdata)
{
    (void)buffer;
    (void)userdata;
    return size * nmemb;
}

HttpClientImpl::HttpClientImpl()
    : curlEasy_(new curl_easy),
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

    // CA bundle
    if (CaBundle::available())
    {
        curlEasy_->add<CURLOPT_CAINFO>(CaBundle::path().c_str());
    }

    // timeout
    kulloAssert(timeoutMs <= std::numeric_limits<long>::max());
    if (timeoutMs > 0)
    {
        curlEasy_->add<CURLOPT_TIMEOUT_MS>(static_cast<long>(timeoutMs));
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
            break;

        default:
            result.error = Http::ResponseError::NetworkError;

            std::ostringstream traceback;
            bool first = true;
            for (const auto &pair : ex.get_traceback())
            {
                if (!first) traceback << "\n";
                first = false;
                traceback << "'" << pair.first << "' in " << pair.second;
            }
            Log.e() << "HTTP exception: " << traceback.str() << "\n"
                    << "Details: " << curlErrorBuffer_;
        }
    }

    curlEasy_->reset();
    requestState_.reset(new RequestState);
    return result;
}

void HttpClientImpl::addHeaders(const std::vector<Http::HttpHeader> &headers)
{
    if (!headers.empty())
    {
        requestState_->reqHeaders.reset(new curl_header());
        for (const auto &hdr : headers)
        {
            requestState_->reqHeaders->add(hdr.key + ": " + hdr.value);
        }
        curlEasy_->add(curl_pair<CURLoption, curl_header>(
                           CURLOPT_HTTPHEADER, *requestState_->reqHeaders));
    }
}

void HttpClientImpl::addCancelCallback(Http::ResponseListener *respL)
{
    kulloAssert(respL);

    using namespace std::placeholders;
    requestState_->progressFunction.reset(
                new ProgressFunctionType(
                    std::bind(
                        &Http::ResponseListener::progress,
                        respL, _1, _2, _3, _4
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

    default:
        kulloAssert(false);
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
