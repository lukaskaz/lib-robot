#pragma once

#include "http/interfaces/http.hpp"
#include "log/interfaces/logging.hpp"
#include "robot/interfaces/robot.hpp"
#include "tts/interfaces/texttovoice.hpp"

#include <memory>

namespace robot
{

class RobotFactory
{
  public:
    template <typename T>
    static std::shared_ptr<RobotIf>
        create(std::shared_ptr<http::HttpIf> httpIf,
               std::shared_ptr<tts::TextToVoiceIf> ttsIf,
               std::shared_ptr<logging::LogIf> logIf)
    {
        return std::shared_ptr<T>(new T(httpIf, ttsIf, logIf));
    }
};

} // namespace robot
