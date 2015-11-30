#pragma once

#include <gmock/gmock.h>
#include <kulloclient/http/ResponseListener.h>

class MockResponseListener : public Kullo::Http::ResponseListener
{
public:
    MOCK_METHOD4(progress, Kullo::Http::ProgressResult(
                     int64_t uploadTransferred, int64_t uploadTotal,
                     int64_t downloadTransferred, int64_t downloadTotal));
    MOCK_METHOD1(dataReceived, void(const std::vector<uint8_t> &data));
};
