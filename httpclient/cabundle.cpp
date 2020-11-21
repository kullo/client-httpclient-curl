/*
 * Copyright 2015–2019 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "httpclient/cabundle.h"

#include <fstream>
#include <vector>

#include <kulloclient/util/librarylogger.h>

// See for example
// * https://golang.org/src/crypto/x509/root_unix.go#L12
// * https://github.com/bagder/curl/blob/master/acinclude.m4#L2624
namespace {
const auto CA_BUNDLE_PATHS = std::vector<std::string>{
    "/etc/ssl/certs/ca-certificates.crt",  // Debian et al, Gentoo, Arch
    "/etc/pki/tls/certs/ca-bundle.crt",    // Red Hat et al
    "/etc/ssl/ca-bundle.pem"               // OpenSUSE
};
}

namespace HttpClient {

bool CaBundle::available()
{
#ifdef __linux__
    return true;
#else
    return false;
#endif
}

std::string CaBundle::path()
{
    static std::string result;
    if (result.empty())
    {
        for (const auto &path : CA_BUNDLE_PATHS)
        {
            if (std::ifstream(path).good())
            {
                result = path;
                break;
            }
        }
        if (result.empty())
        {
            Log.w() << "Couldn't find working CA bundle path";
        }
    }
    return result;
}

}
