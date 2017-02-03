/* Copyright 2015–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <gmock/gmock.h>
#include <kulloclient/http/ResponseListener.h>
#include <kulloclient/http/TransferProgress.h>

class MockResponseListener : public Kullo::Http::ResponseListener
{
public:
    MOCK_METHOD1(progressed, Kullo::Http::ProgressResult(
                     const Kullo::Http::TransferProgress &progress));
    MOCK_METHOD1(dataReceived, void(const std::vector<uint8_t> &data));
};
