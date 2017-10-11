/* Copyright 2015–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <boost/optional.hpp>
#include <kulloclient/http/HttpClientFactory.h>

namespace HttpClient {

class HttpClientFactoryImpl : public Kullo::Http::HttpClientFactory
{
public:
    void setAcceptLanguage(const boost::optional<std::string> &acceptLanguage_);

    virtual Kullo::nn_shared_ptr<Kullo::Http::HttpClient> createHttpClient() override;
    virtual std::unordered_map<std::string, std::string> versions() override;

private:
    boost::optional<std::string> acceptLanguage_;
};

}
