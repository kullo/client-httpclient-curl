/* Copyright 2015–2017 Kullo GmbH. All rights reserved. */
#include "httpclient/utils.h"

#include <regex>

#include <kulloclient/util/assert.h>

namespace HttpClient {

namespace {
const std::regex HEADER_REGEX(
        "^"        // begin
        "([^:]+)"  // header name
        ":[ \\t]*" // colon and optional spaces or tabs
        "(\\S.*?)" // header value, strting with non-whitespace (non-greedy)
        "[\\s]*"   // throw away trailing whitespace like line breaks
        "$"        // end
        );
}

boost::optional<std::pair<std::string, std::string>> Utils::parseHeader(const std::string &in)
{
    std::smatch matches;
    if (std::regex_match(in, matches, HEADER_REGEX))
    {
        kulloAssert(matches.size() == 3); // full match + 2 groups
        return std::pair<std::string, std::string>{
            matches[1].str(), matches[2].str()
        };
    }
    else
    {
        return boost::none;
    }
}

}
