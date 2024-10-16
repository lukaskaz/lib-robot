#pragma once

#include <memory>

class Display
{
  public:
    Display();
    ~Display();

    void run();

  private:
    struct Handler;
    std::unique_ptr<Handler> handler;
};