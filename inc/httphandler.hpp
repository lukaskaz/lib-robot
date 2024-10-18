#pragma once

#include <memory>
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

template <typename T>
class HttpIfFactory
{
  public:
    static std::shared_ptr<HttpIf> create()
    {
        return std::shared_ptr<T>(new T());
    }
};

namespace cpr
{
class Http : public HttpIf
{
  public:
    ~Http();
    std::string get(const std::string&) override;
    std::string info() override;

  private:
    friend class HttpIfFactory<Http>;
    Http();
    struct Handler;
    std::unique_ptr<Handler> handler;
};

} // namespace cpr

} // namespace http
