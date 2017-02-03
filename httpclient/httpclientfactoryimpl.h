/* Copyright 2015–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <kulloclient/http/HttpClientFactory.h>

namespace HttpClient {

class HttpClientFactoryImpl : public Kullo::Http::HttpClientFactory
{
public:
    virtual std::shared_ptr<Kullo::Http::HttpClient> createHttpClient() override;
    virtual std::unordered_map<std::string, std::string> versions() override;
};

}
