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

    virtual bool readwifiinfo(bool) = 0;
    virtual bool readservosinfo(bool) = 0;
    virtual bool settorqueunlocked(bool) = 0;
    virtual bool settorquelocked(bool) = 0;
    virtual bool openeoat(bool) = 0;
    virtual bool closeeoat(bool) = 0;
    virtual bool readdeviceinfo(bool) = 0;
    virtual bool setledon(bool, uint8_t lvl) = 0;
    virtual bool setledoff(bool) = 0;
    virtual bool movebase(bool) = 0;
    virtual bool moveleft(bool) = 0;
    virtual bool moveright(bool) = 0;
    virtual bool moveparked(bool) = 0;
    virtual bool sendusercmd(bool) = 0;

    virtual bool shakehand(bool) = 0;
    virtual bool dancesing(bool) = 0;
    virtual bool dancestream(bool) = 0;
    virtual bool enlight(bool) = 0;
    virtual bool doheman(bool) = 0;
    virtual bool engage() = 0;
    virtual bool disengage() = 0;

    virtual bool changevoice(bool) = 0;
    virtual bool changelangtopolish(bool) = 0;
    virtual bool changelangtoenglish(bool) = 0;
    virtual bool changelangtogerman(bool) = 0;

    virtual std::string conninfo() = 0;
};

} // namespace robot
