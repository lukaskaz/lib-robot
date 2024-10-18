#pragma once

#include "http/interfaces/http.hpp"

#include <memory>

namespace http
{

template <typename T>
class HttpIfFactory
{
  public:
    static std::shared_ptr<HttpIf> create()
    {
        return std::shared_ptr<T>(new T());
    }
};

} // namespace http
