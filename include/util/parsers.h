#pragma once

#include "request.h"
#include <cctype>
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

inline std::string to_lower(std::string str)
{
    for (char &c : str)
    {
        c = std::tolower(c);
    }

    return str;
}

inline std::string trim(std::string_view str)
{
    size_t b = str.find_first_not_of(" \t");

    if (b == std::string_view::npos)
    {
        return {};
    }

    size_t e = str.find_last_not_of(" \t");
    return std::string(str.substr(b, e - b + 1));
}

inline bool parse_header(std::string_view headers, Request &req)
{
    std::istringstream stream{std::string(headers)};
    std::string line;

    // Request Line
    if (!std::getline(stream, line))
    { // Read entire line until the delimiter '\n'
        return false;
    }
    // Popping the last character which is '\r'
    if (!line.empty() && line.back() == '\r')
    {
        line.pop_back();
    }

    std::istringstream req_header(line);
    if (!(req_header >> req.method >> req.path >> req.http_version))
    {
        return false;
    }

    // Parsing Headers
    while (std::getline(stream, line))
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
        if (pos == std::string::npos)
        {
            return false;
        }

        req.headers[to_lower(line.substr(0, pos))] = trim(std::string_view(line.substr(pos + 1)));
    }

    return true;
}
} // namespace util
