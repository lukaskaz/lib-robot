#pragma once

#include <string>

namespace http
{

class HttpIf
{
  public:
    virtual ~HttpIf() = default;
    virtual std::string get(const std::string&) = 0;
    virtual std::string info() = 0;
};

} // namespace http
