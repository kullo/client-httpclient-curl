/* Copyright 2015–2017 Kullo GmbH. All rights reserved. */
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
