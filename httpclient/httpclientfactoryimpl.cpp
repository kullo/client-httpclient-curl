/* Copyright 2015–2016 Kullo GmbH. All rights reserved. */
#include "httpclient/httpclientfactoryimpl.h"

#include <curl_info.h>
#include <sstream>

#include "httpclient/httpclientimpl.h"

namespace HttpClient {

std::shared_ptr<Kullo::Http::HttpClient> HttpClientFactoryImpl::createHttpClient()
{
    return std::make_shared<HttpClientImpl>();
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
