#pragma once

#include <cstdint>
#include <memory>

class Robothandler
{
  public:
    Robothandler();
    ~Robothandler();

    void readwifiinfo();
    void readservosinfo();
    void settorqueunlocked();
    void settorquelocked();
    void openeoat();
    void closeeoat();
    void readdeviceinfo();
    void setledon(uint8_t lvl = 255);
    void setledoff();
    void movebase();
    void moveleft();
    void moveright();
    void moveparked();
    void sendusercmd();

    void shakehand();
    void dance();
    void enlight();
    void engage();
    void disengage();

    std::string conninfo();

  private:
    struct Handler;
    std::unique_ptr<Handler> handler;
};
