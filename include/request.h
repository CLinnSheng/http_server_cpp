#pragma once
#include <string>
#include <unordered_map>

struct Request
{
    // Request Line
    std::string method;
    std::string path;
    std::string http_version;

    // Headers
    // Each header sepearete by \r\n
    std::unordered_map<std::string, std::string> headers;

    std::string body;
};
