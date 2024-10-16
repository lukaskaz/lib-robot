#include "display.hpp"

#include <boost/program_options.hpp>

#include <csignal>
#include <iostream>

void signalHandler(int signal)
{
    if (signal == SIGINT)
    {
        // block ctrl+c signal
    }
}

int main(int argc, char* argv[])
{
    std::signal(SIGINT, signalHandler);
    if (argc > 1)
        [argc, argv]() {
            boost::program_options::options_description desc("Allowed options");
            desc.add_options()("help,h", "produce help message")(
                "address,a", boost::program_options::value<std::string>(),
                "serial device node")(
                "speed,s", boost::program_options::value<std::string>(),
                "speed of serial communication");

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
        }();

    try
    {
        Display().run();
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
