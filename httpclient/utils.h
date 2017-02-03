/* Copyright 2015–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <boost/optional/optional.hpp>
#include <string>
#include <utility>

namespace HttpClient {

class Utils {
public:
    static boost::optional<std::pair<std::string, std::string>> parseHeader(const std::string &in);
};

}
