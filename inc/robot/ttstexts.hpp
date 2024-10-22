#pragma once

#include "tts/interfaces/texttovoice.hpp"

#include <string>

namespace robot
{

enum class task
{
    initiatating,
    ready,
    parked,
    greetstart,
    greetshake,
    greetend,
    greetfail,
    dancestart,
    songlinefirst,
    songlinesecond,
    songlinethird,
    songlineforth,
    danceend,
    enlightstart,
    enlightbreak,
    enlightend,
};

std::string getttstext(task, tts::language);

} // namespace robot
