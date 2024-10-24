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

    void readwifiinfo() override;
    void readservosinfo() override;
    void settorqueunlocked() override;
    void settorquelocked() override;
    void openeoat() override;
    void closeeoat() override;
    void readdeviceinfo() override;
    void setledon(uint8_t lvl) override;
    void setledoff() override;
    void movebase() override;
    void moveleft() override;
    void moveright() override;
    void moveparked() override;
    void sendusercmd() override;

    void shakehand() override;
    void dance() override;
    void enlight() override;
    void engage() override;
    void disengage() override;

    void changevoice() override;
    void changelangtopolish() override;
    void changelangtoenglish() override;
    void changelangtogerman() override;

    std::string conninfo() override;

  private:
    friend class robot::RobotFactory;
    Robot(std::shared_ptr<http::HttpIf>, std::shared_ptr<tts::TextToVoiceIf>,
          std::shared_ptr<logging::LogIf>);
    struct Handler;
    std::unique_ptr<Handler> handler;
};

} // namespace robot::roarmm2
