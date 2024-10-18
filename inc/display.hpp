#pragma once

#include "robotmgmt.hpp"

#include <memory>

class Display
{
  public:
    Display(std::shared_ptr<Robothandler>);
    ~Display();

    void run();

  private:
    struct Handler;
    std::unique_ptr<Handler> handler;
};