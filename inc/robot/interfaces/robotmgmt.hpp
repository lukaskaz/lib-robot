#pragma once

#include "http/interfaces/http.hpp"
#include "tts/interfaces/texttovoice.hpp"

#include <cstdint>
#include <memory>

namespace robot
{

struct TtsSetup
{
    tts::voice_t voice;
    std::shared_ptr<tts::TextToVoiceIf> iface;
};

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

    virtual std::string conninfo() = 0;
};

class Robothandler : public RobotIf
{
  public:
    Robothandler(std::shared_ptr<http::HttpIf>, std::shared_ptr<TtsSetup>);
    ~Robothandler();

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

    std::string conninfo() override;

  private:
    struct Handler;
    std::unique_ptr<Handler> handler;
};

} // namespace robot
