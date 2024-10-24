#include "display.hpp"

#include "climenu.hpp"
#include "robot/helpers.hpp"

#include <functional>
#include <memory>

namespace display
{

struct Display::Handler
{
  public:
    Handler(std::shared_ptr<robot::RobotIf> robotIf) : robotIf{robotIf}
    {
        robotIf->engage();
    }
    ~Handler()
    {
        robotIf->disengage();
    }

    void exitprogram()
    {}

    void showmenu()
    {
        Menu menu{
            "[JSON commands via " + robotIf->conninfo() + " on " +
                robothelpers::gettimestr() + "]",
            {{"get wifi info",
              std::bind(&robot::RobotIf::readwifiinfo, robotIf)},
             {"get device info",
              std::bind(&robot::RobotIf::readdeviceinfo, robotIf)},
             {"get servos position",
              std::bind(&robot::RobotIf::readservosinfo, robotIf)},
             {"unlock torque",
              std::bind(&robot::RobotIf::settorqueunlocked, robotIf)},
             {"lock torque",
              std::bind(&robot::RobotIf::settorquelocked, robotIf)},
             {"open clamp", std::bind(&robot::RobotIf::openeoat, robotIf)},
             {"close clamp", std::bind(&robot::RobotIf::closeeoat, robotIf)},
             {"shake hand", std::bind(&robot::RobotIf::shakehand, robotIf)},
             {"dance", std::bind(&robot::RobotIf::dance, robotIf)},
             {"enlight", std::bind(&robot::RobotIf::enlight, robotIf)},
             {"move base", std::bind(&robot::RobotIf::movebase, robotIf)},
             {"move left", std::bind(&robot::RobotIf::moveleft, robotIf)},
             {"move right", std::bind(&robot::RobotIf::moveright, robotIf)},
             {"move parked", std::bind(&robot::RobotIf::moveparked, robotIf)},
             {"set led on", std::bind(&robot::RobotIf::setledon, robotIf, 255)},
             {"set led off", std::bind(&robot::RobotIf::setledoff, robotIf)},
             {"send user command",
              std::bind(&robot::RobotIf::sendusercmd, robotIf)},
             {"change voice", std::bind(&robot::RobotIf::changevoice, robotIf)},
             {"change language to polish",
              std::bind(&robot::RobotIf::changelangtopolish, robotIf)},
             {"change language to english",
              std::bind(&robot::RobotIf::changelangtoenglish, robotIf)},
             {"change language to german",
              std::bind(&robot::RobotIf::changelangtogerman, robotIf)},
             {"exit program", [this]() { exitprogram(); }}}};
        menu.run();
    }

  public:
    std::shared_ptr<robot::RobotIf> robotIf;
};

Display::Display(std::shared_ptr<robot::RobotIf> robotIf) :
    handler{std::make_unique<Handler>(robotIf)}
{}

Display::~Display() = default;

void Display::run()
{
    handler->showmenu();
}

} // namespace display
