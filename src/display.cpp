#include "display.hpp"

#include "menu/interfaces/cli.hpp"
#include "robot/helpers.hpp"

#include <functional>
#include <memory>

namespace display
{

struct Display::Handler
{
  public:
    Handler(std::shared_ptr<logging::LogIf> logIf,
            std::shared_ptr<robot::RobotIf> robotIf) :
        logIf{logIf},
        robotIf{robotIf}
    {
        robotIf->engage();
    }
    ~Handler()
    {
        robotIf->disengage();
    }

    bool exitprogram()
    {
        return false;
    }

    void showmenu()
    {
        auto menu = menu::MenuFactory::create<menu::cli::Menu>(
            logIf,
            "[JSON commands via " + robotIf->conninfo() + " on " +
                robothelpers::gettimestr() + "]",
            {{"get wifi info",
              std::bind(&robot::RobotIf::readwifiinfo, robotIf, true),
              std::bind(&robot::RobotIf::readwifiinfo, robotIf, false)},
             {"get device info",
              std::bind(&robot::RobotIf::readdeviceinfo, robotIf, true),
              std::bind(&robot::RobotIf::readdeviceinfo, robotIf, false)},
             {"get servos position",
              std::bind(&robot::RobotIf::readservosinfo, robotIf, true),
              std::bind(&robot::RobotIf::readservosinfo, robotIf, false)},
             {"unlock torque",
              std::bind(&robot::RobotIf::settorqueunlocked, robotIf, true),
              std::bind(&robot::RobotIf::settorqueunlocked, robotIf, false)},
             {"lock torque",
              std::bind(&robot::RobotIf::settorquelocked, robotIf, true),
              std::bind(&robot::RobotIf::settorquelocked, robotIf, false)},
             {"open clamp", std::bind(&robot::RobotIf::openeoat, robotIf, true),
              std::bind(&robot::RobotIf::openeoat, robotIf, false)},
             {"close clamp",
              std::bind(&robot::RobotIf::closeeoat, robotIf, true),
              std::bind(&robot::RobotIf::closeeoat, robotIf, false)},
             {"shake hand",
              std::bind(&robot::RobotIf::shakehand, robotIf, true),
              std::bind(&robot::RobotIf::shakehand, robotIf, false)},
             {"dance & sing",
              std::bind(&robot::RobotIf::dancesing, robotIf, true),
              std::bind(&robot::RobotIf::dancesing, robotIf, false)},
             {"dance & stream",
              std::bind(&robot::RobotIf::dancestream, robotIf, true),
              std::bind(&robot::RobotIf::dancestream, robotIf, false)},
             {"enlight", std::bind(&robot::RobotIf::enlight, robotIf, true),
              std::bind(&robot::RobotIf::enlight, robotIf, false)},
             {"play he-man", std::bind(&robot::RobotIf::doheman, robotIf, true),
              std::bind(&robot::RobotIf::doheman, robotIf, false)},
             {"move base", std::bind(&robot::RobotIf::movebase, robotIf, true),
              std::bind(&robot::RobotIf::movebase, robotIf, false)},
             {"move left", std::bind(&robot::RobotIf::moveleft, robotIf, true),
              std::bind(&robot::RobotIf::moveleft, robotIf, false)},
             {"move right",
              std::bind(&robot::RobotIf::moveright, robotIf, true),
              std::bind(&robot::RobotIf::moveright, robotIf, false)},
             {"move parked",
              std::bind(&robot::RobotIf::moveparked, robotIf, true),
              std::bind(&robot::RobotIf::moveparked, robotIf, false)},
             {"set led on",
              std::bind(&robot::RobotIf::setledon, robotIf, true, 255),
              std::bind(&robot::RobotIf::setledon, robotIf, false, 255)},
             {"set led off",
              std::bind(&robot::RobotIf::setledoff, robotIf, true),
              std::bind(&robot::RobotIf::setledoff, robotIf, false)},
             {"send user command",
              std::bind(&robot::RobotIf::sendusercmd, robotIf, true),
              std::bind(&robot::RobotIf::sendusercmd, robotIf, false)},
             {"change voice",
              std::bind(&robot::RobotIf::changevoice, robotIf, true),
              std::bind(&robot::RobotIf::changevoice, robotIf, false)},
             {"change language to polish",
              std::bind(&robot::RobotIf::changelangtopolish, robotIf, true),
              std::bind(&robot::RobotIf::changelangtopolish, robotIf, false)},
             {"change language to english",
              std::bind(&robot::RobotIf::changelangtoenglish, robotIf, true),
              std::bind(&robot::RobotIf::changelangtoenglish, robotIf, false)},
             {"change language to german",
              std::bind(&robot::RobotIf::changelangtogerman, robotIf, true),
              std::bind(&robot::RobotIf::changelangtogerman, robotIf, false)},
             {"exit program", [this]() { return exitprogram(); },
              [this]() { return exitprogram(); }}});
        menu->run();
    }

  public:
    std::shared_ptr<logging::LogIf> logIf;
    std::shared_ptr<robot::RobotIf> robotIf;
};

Display::Display(std::shared_ptr<logging::LogIf> logIf,
                 std::shared_ptr<robot::RobotIf> robotIf) :
    handler{std::make_unique<Handler>(logIf, robotIf)}
{}

Display::~Display() = default;

void Display::run()
{
    handler->showmenu();
}

} // namespace display
