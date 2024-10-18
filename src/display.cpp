#include "display.hpp"

#include "climenu.hpp"
#include "helpers.hpp"

#include <functional>
#include <iostream>
#include <memory>

struct Display::Handler
{
  public:
    Handler(std::shared_ptr<Robothandler> robotIf) : robotIf{robotIf}
    {
        std::cout << "Initiating robot\n";
        robotIf->engage();
    }
    ~Handler()
    {
        std::cout << "Cleaning and closing\n";
        robotIf->disengage();
    }

    void exitprogram()
    {}

    void showmenu()
    {
        Menu menu{
            "[JSON commands via " + robotIf->conninfo() + " on " +
                robothelpers::gettimestr() + "]",
            {{"get wifi info", std::bind(&Robothandler::readwifiinfo, robotIf)},
             {"get device info",
              std::bind(&Robothandler::readdeviceinfo, robotIf)},
             {"get servos position",
              std::bind(&Robothandler::readservosinfo, robotIf)},
             {"unlock torque",
              std::bind(&Robothandler::settorqueunlocked, robotIf)},
             {"lock torque",
              std::bind(&Robothandler::settorquelocked, robotIf)},
             {"open clamp", std::bind(&Robothandler::openeoat, robotIf)},
             {"close clamp", std::bind(&Robothandler::closeeoat, robotIf)},
             {"shake hand", std::bind(&Robothandler::shakehand, robotIf)},
             {"dance", std::bind(&Robothandler::dance, robotIf)},
             {"enlight", std::bind(&Robothandler::enlight, robotIf)},
             {"move base", std::bind(&Robothandler::movebase, robotIf)},
             {"move left", std::bind(&Robothandler::moveleft, robotIf)},
             {"move right", std::bind(&Robothandler::moveright, robotIf)},
             {"move parked", std::bind(&Robothandler::moveparked, robotIf)},
             {"set led on", std::bind(&Robothandler::setledon, robotIf, 255)},
             {"set led off", std::bind(&Robothandler::setledoff, robotIf)},
             {"send user command",
              std::bind(&Robothandler::sendusercmd, robotIf)},
             {"exit program", [this]() { exitprogram(); }}}};
        menu.run();
    }

  public:
    std::shared_ptr<Robothandler> robotIf;
};

Display::Display(std::shared_ptr<Robothandler> robotIf) :
    handler{std::make_unique<Handler>(robotIf)}
{}

Display::~Display() = default;

void Display::run()
{
    handler->showmenu();
}
