/*
 * Copyright 2015–2019 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "httpclient/httpclientfactoryimpl.h"

#include <curl_info.h>
#include <sstream>

#include "httpclient/httpclientimpl.h"

namespace HttpClient {

void HttpClientFactoryImpl::setAcceptLanguage(
        const boost::optional<std::string> &acceptLanguage)
{
    this->acceptLanguage_ = acceptLanguage;
}

Kullo::nn_shared_ptr<Kullo::Http::HttpClient> HttpClientFactoryImpl::createHttpClient()
{
    return Kullo::nn_make_shared<HttpClientImpl>(acceptLanguage_);
}

std::unordered_map<std::string, std::string> HttpClientFactoryImpl::versions()
{
    curl::curl_info curlInfo;
    auto versionNum = curlInfo.get_version_number();
    auto curlVersion =
            std::to_string((versionNum >> 16) & 0xff) + "." +
            std::to_string((versionNum >> 8) & 0xff) + "." +
            std::to_string(versionNum & 0xff);

    // If the SSL lib has a version number, it looks like that: OpenSSL/1.0.1m
    std::stringstream sslVersionStream{curlInfo.get_ssl_version()};
    std::string sslName, sslVersion;
    if (std::getline(sslVersionStream, sslName, '/'))
    {
        std::getline(sslVersionStream, sslVersion);
    }

    std::unordered_map<std::string, std::string> result;
    result["cURL"] = curlVersion;
    if (!sslVersion.empty()) result[sslName] = sslVersion;
    return result;
}

}
