#include "httphandler.hpp"

#include <cpr/cpr.h>

#include <nlohmann/json.hpp>

#include <cstdlib>
#include <iostream>

static constexpr auto httpipaddrvar = "HTTPIPADDRESS";
using json = nlohmann::json;

struct Httphandler::Handler
{
  public:
    Handler() :
        ip{[](const char* var) {
            if (auto ip = std::getenv(var))
                return ip;
            throw std::runtime_error("[Http request] Please set envvar: " +
                                     std::string{var});
        }(httpipaddrvar)},
        url{[this]() { return "http://" + ip + "/js?json="; }()}
    {}

    std::string getmethod(const std::string& params)
    {
        auto json = json::parse(params);
        auto resp = cpr::Get(cpr::Url{url + params});
        if (resp.status_code == 200)
        {
            return resp.text;
        }
        if (resp.status_code == 0)
        {
            throw std::runtime_error("Given host IP address is invalid");
        }
        return {};
    }

    std::string getinfo()
    {
        return "http@" + ip;
    }

  private:
    const std::string ip;
    const std::string url;

    void displayresp(const std::string& jsonstr)
    {
        auto json = json::parse(jsonstr);
        for (const auto& [key, val] : json.items())
        {
            std::cout << key << ": " << val << '\n';
        }
        std::cout << "\n";
    }
};

Httphandler::Httphandler() : handler{std::make_unique<Handler>()}
{}

Httphandler::~Httphandler() = default;

std::string Httphandler::get(const std::string& params)
{
    return handler->getmethod(params);
}

std::string Httphandler::info()
{
    return handler->getinfo();
}
