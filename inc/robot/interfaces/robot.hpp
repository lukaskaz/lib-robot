#pragma once

#include "http/interfaces/http.hpp"
#include "tts/interfaces/texttovoice.hpp"

#include <cstdint>
#include <memory>

namespace robot
{

class RobotIf
{
  public:
    virtual ~RobotIf() = default;

    virtual void readwifiinfo() = 0;
    virtual void readservosinfo() = 0;
    virtual void settorqueunlocked() = 0;
    virtual void settorquelocked() = 0;
    virtual void openeoat() = 0;
    virtual void closeeoat() = 0;
    virtual void readdeviceinfo() = 0;
    virtual void setledon(uint8_t lvl) = 0;
    virtual void setledoff() = 0;
    virtual void movebase() = 0;
    virtual void moveleft() = 0;
    virtual void moveright() = 0;
    virtual void moveparked() = 0;
    virtual void sendusercmd() = 0;

    virtual void shakehand() = 0;
    virtual void dance() = 0;
    virtual void enlight() = 0;
    virtual void engage() = 0;
    virtual void disengage() = 0;

    virtual void changevoice() = 0;
    virtual void changelangtopolish() = 0;
    virtual void changelangtoenglish() = 0;
    virtual void changelangtogerman() = 0;

    virtual std::string conninfo() = 0;
};

} // namespace robot
