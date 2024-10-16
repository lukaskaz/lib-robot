#include "display.hpp"

#include "climenu.hpp"
#include "helpers.hpp"
#include "robotmgmt.hpp"

#include <functional>
#include <iostream>

struct Display::Handler
{
  public:
    Handler() : robot{std::make_shared<Robothandler>()}
    {
        std::cout << "Initiating robot\n";
        robot->engage();
    }
    ~Handler()
    {
        std::cout << "Cleaning and closing\n";
        robot->disengage();
    }

    void exitprogram()
    {}

    void showmenu()
    {
        Menu menu{
            "[JSON commands via " + robot->conninfo() + " on " +
                robothelpers::gettimestr() + "]",
            {{"get wifi info", std::bind(&Robothandler::readwifiinfo, robot)},
             {"get device info",
              std::bind(&Robothandler::readdeviceinfo, robot)},
             {"get servos position",
              std::bind(&Robothandler::readservosinfo, robot)},
             {"unlock torque",
              std::bind(&Robothandler::settorqueunlocked, robot)},
             {"lock torque", std::bind(&Robothandler::settorquelocked, robot)},
             {"open clamp", std::bind(&Robothandler::openeoat, robot)},
             {"close clamp", std::bind(&Robothandler::closeeoat, robot)},
             {"shake hand", std::bind(&Robothandler::shakehand, robot)},
             {"dance", std::bind(&Robothandler::dance, robot)},
             {"enlight", std::bind(&Robothandler::enlight, robot)},
             {"move base", std::bind(&Robothandler::movebase, robot)},
             {"move left", std::bind(&Robothandler::moveleft, robot)},
             {"move right", std::bind(&Robothandler::moveright, robot)},
             {"move parked", std::bind(&Robothandler::moveparked, robot)},
             {"set led on", std::bind(&Robothandler::setledon, robot, 255)},
             {"set led off", std::bind(&Robothandler::setledoff, robot)},
             {"send user command",
              std::bind(&Robothandler::sendusercmd, robot)},
             {"exit program", [this]() { exitprogram(); }}}};
        menu.run();
    }

  public:
    std::shared_ptr<Robothandler> robot;
};

Display::Display() : handler{std::make_unique<Handler>()}
{}

Display::~Display() = default;

void Display::run()
{
    handler->showmenu();
}
