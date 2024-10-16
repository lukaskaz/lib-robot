#pragma once

#include <memory>
#include <string>

class Httphandler
{
  public:
    Httphandler();
    ~Httphandler();
    std::string get(const std::string&);
    std::string info();

  private:
    struct Handler;
    std::unique_ptr<Handler> handler;
};
