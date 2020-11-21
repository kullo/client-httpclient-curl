/*
 * Copyright 2015–2019 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <string>

namespace HttpClient {

class CaBundle
{
public:
    /// Returns true iff a CA path should be available on this OS
    static bool available();

    /// Returns the path of the CA bundle file or "" if none has been found
    static std::string path();
};

}
