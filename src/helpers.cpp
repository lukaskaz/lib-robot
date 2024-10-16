#include "helpers.hpp"

#include <time.h>

#include <algorithm>

namespace robothelpers
{

std::string gettimestr()
{
    time_t rawtime{0};
    tm* timeinfo{0};

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    std::string timestr{asctime(timeinfo)};
    timestr.erase(std::remove(timestr.begin(), timestr.end(), '\n'),
                  timestr.end());
    return timestr;
}
} // namespace robothelpers
