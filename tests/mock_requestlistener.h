/*
 * Copyright 2015–2019 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <gmock/gmock.h>
#include <kulloclient/http/RequestListener.h>

class MockRequestListener : public Kullo::Http::RequestListener
{
public:
    MOCK_METHOD1(read, std::vector<uint8_t>(int64_t maxSize));
};
