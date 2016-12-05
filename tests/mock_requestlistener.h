/* Copyright 2015–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <gmock/gmock.h>
#include <kulloclient/http/RequestListener.h>

class MockRequestListener : public Kullo::Http::RequestListener
{
public:
    MOCK_METHOD1(read, std::vector<uint8_t>(int64_t maxSize));
};
