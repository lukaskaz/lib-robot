#pragma once

#include "robot/factory.hpp"

#include <cstdint>
#include <memory>

namespace robot::roarmm2
{

class Robot : public RobotIf
{
  public:
    ~Robot();

    bool readwifiinfo(bool) override;
    bool readservosinfo(bool) override;
    bool settorqueunlocked(bool) override;
    bool settorquelocked(bool) override;
    bool openeoat(bool) override;
    bool closeeoat(bool) override;
    bool readdeviceinfo(bool) override;
    bool setledon(bool, uint8_t lvl) override;
    bool setledoff(bool) override;
    bool movebase(bool) override;
    bool moveleft(bool) override;
    bool moveright(bool) override;
    bool moveparked(bool) override;
    bool sendusercmd(bool) override;

    bool shakehand(bool) override;
    bool dance(bool) override;
    bool enlight(bool) override;
    bool engage() override;
    bool disengage() override;

    bool changevoice(bool) override;
    bool changelangtopolish(bool) override;
    bool changelangtoenglish(bool) override;
    bool changelangtogerman(bool) override;

    std::string conninfo() override;

  private:
    friend class robot::RobotFactory;
    Robot(std::shared_ptr<http::HttpIf>, std::shared_ptr<tts::TextToVoiceIf>,
          std::shared_ptr<logging::LogIf>);
    struct Handler;
    std::unique_ptr<Handler> handler;
};

} // namespace robot::roarmm2
