kullo_add_simple_library(
    cabundle.cpp
    httpclientfactoryimpl.cpp
    httpclientimpl.cpp
)
kullo_get_target_name(target_name)

target_link_libraries(${target_name}
    PUBLIC
        # from other projects in workspace
        kulloclient
        boost_regex
        curlcpp
        curl
)
