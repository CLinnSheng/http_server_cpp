#pragma once

#include "request.h"
#include <sstream>
#include <string>
#include <string_view>
namespace util
{
inline Request parse_request(const std::string &raw_req)
{
    Request req;
    std::istringstream stream(raw_req);
    std::string line;

    // getline is split by '\n' or '\t'

    // Parse Request Line
    if (std::getline(stream, line))
    {
        // Remove the trailing '\r'
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        std::istringstream l(line);

        l >> req.method >> req.path >> req.http_version;
    }

    // Parse headers
    while (std::getline(stream, line) && !line.empty())
    {
        // Remove the trailing '\r'
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        // Handle the last header line
        // '\r\n\r\n
        if (line.empty())
        {
            break;
        }

        size_t pos = line.find(":");
        if (pos != std::string::npos)
        {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            req.headers[std::move(key)] = std::move(value);
        }
    }

    return req;
}
} // namespace util
