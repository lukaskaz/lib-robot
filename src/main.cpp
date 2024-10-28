#include "display.hpp"
#include "http/interfaces/cpr.hpp"
#include "log/interfaces/console.hpp"
#include "log/interfaces/group.hpp"
#include "log/interfaces/storage.hpp"
#include "robot/interfaces/roarmm2.hpp"
#include "tts/interfaces/googlecloud.hpp"

#include <boost/program_options.hpp>

#include <csignal>
#include <iostream>

void signalHandler(int signal)
{
    if (signal == SIGINT)
    {
        // blocking ctrl+c signal
    }
}

int main(int argc, char* argv[])
{
    uint32_t loglvl = 1;
    std::signal(SIGINT, signalHandler);
    if (argc > 1)
        [argc, argv, &loglvl]() {
            boost::program_options::options_description desc("Allowed options");
            desc.add_options()("help,h", "produce help message")(
                "address,a", boost::program_options::value<std::string>(),
                "serial device node")(
                "speed,s", boost::program_options::value<std::string>(),
                "speed of serial communication")(
                "loglvl,l", boost::program_options::value<uint32_t>(),
                "level of logging [0-4], default error [1]");

            boost::program_options::variables_map vm;
            boost::program_options::store(
                boost::program_options::parse_command_line(argc, argv, desc),
                vm);
            boost::program_options::notify(vm);

            if (vm.contains("help"))
            {
                std::cout << desc;
                exit(0);
            }

            loglvl =
                vm.contains("loglvl") ? vm.at("loglvl").as<uint32_t>() : loglvl;
        }();

    try
    {
        auto lvl = static_cast<logging::type>(loglvl);
        auto logconsole =
            logging::LogFactory::create<logging::console::Log>(lvl);
        auto logstorage =
            logging::LogFactory::create<logging::storage::Log>(lvl);
        auto logIf = logging::LogFactory::create<logging::group::Log>(
            {logconsole, logstorage});
        auto httpIf = http::HttpFactory::create<http::cpr::Http>(logIf);
        auto ttsIf =
            tts::TextToVoiceFactory::create<tts::googlecloud::TextToVoice>(
                {tts::language::polish, tts::gender::female, 1});
        auto robotIf = robot::RobotFactory::create<robot::roarmm2::Robot>(
            httpIf, ttsIf, logIf);
        auto menu = display::Display(robotIf);
        menu.run();
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << "\n";
    }
    catch (...)
    {
        std::cerr << "Unknown exception occured, aborting!\n";
    }
    return 0;
}
