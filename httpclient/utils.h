/*
 * Copyright 2015–2019 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
