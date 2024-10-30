#pragma once

#include "log/interfaces/logging.hpp"
#include "robot/interfaces/robot.hpp"

#include <memory>

namespace display
{
class Display
{
  public:
    Display(std::shared_ptr<logging::LogIf>, std::shared_ptr<robot::RobotIf>);
    ~Display();

    void run();

  private:
    struct Handler;
    std::unique_ptr<Handler> handler;
};

} // namespace display
