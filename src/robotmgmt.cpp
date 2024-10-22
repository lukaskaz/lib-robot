#include "robot/interfaces/robotmgmt.hpp"

#include "climenu.hpp"
#include "robot/ttstexts.hpp"

#include <nlohmann/json.hpp>

#include <array>
#include <cmath>
#include <future>
#include <iostream>
#include <memory>
#include <random>

namespace robot
{

using json = nlohmann::json;
using xyzt_t = std::tuple<int32_t, int32_t, int32_t, double>;

struct Robothandler::Handler
{
  public:
    Handler(std::shared_ptr<http::HttpIf> httpIf,
            std::shared_ptr<TtsSetup> ttsSetup) :
        httpIf{httpIf},
        tts{ttsSetup}
    {
        if (!this->httpIf)
        {
            throw std::runtime_error("No interface to connect to robot");
        }
    }

    std::string getconninfo()
    {
        return httpIf->info();
    }

    void engage()
    {
        speak(task::initiatating);
        movebase();
    }

    void disengage()
    {
        speak(task::parked);
        setledoff();
        moveparked();
        settorquelocked();
    }

    void movebase()
    {
        sendcommand(R"({"T":100})");
        speak(task::ready);
    }

    void moveleft()
    {
        sendcommand(R"({"T":121,"joint":1,"angle":45,"spd":10,"acc":10})");
    }

    void moveright()
    {
        sendcommand(R"({"T":121,"joint":1,"angle":-45,"spd":10,"acc":10})");
    }

    void moveparked()
    {
        const auto cmd = setposcmd({80, 0, 455, dgrtorad(180 - 35)});
        sendcommand(cmd);
    }

    void settorqueunlocked()
    {
        sendcommand(R"({"T":210,"cmd":0})");
    }

    void settorquelocked()
    {
        sendcommand(R"({"T":210,"cmd":1})");
    }

    void setledon(uint8_t lvl)
    {
        const auto cmd = R"({"T":114,"led":)" + std::to_string(lvl) + "}";
        sendcommand(cmd);
    }

    void setledoff()
    {
        sendcommand(R"({"T":114,"led":0})");
    }

    std::string getwifiinfo()
    {
        return sendcommand(R"({"T":405})");
    }

    std::string getservosinfo()
    {
        return sendcommand(R"({"T":105})");
    }

    void openeoat()
    {
        sendcommand(R"({"T":121,"joint":4,"angle":135,"spd":50,"acc":10})");
    }

    void closeeoat()
    {
        sendcommand(R"({"T":121,"joint":4,"angle":180,"spd":50,"acc":10})");
    }

    std::string getdeviceinfo()
    {
        return sendcommand(R"({"T":302})");
    }

    void shakehand()
    {
        movehandshakepos();
        speak(task::greetstart);
        usleep(2 * 1000 * 1000);

        auto initpos = getxyz();
        while (!Menu::isenterpressed())
        {
            if (initpos != getxyz())
            {
                closeeoat();
                usleep(800 * 1000);
                constexpr int32_t eoatclosedangle{177};
                if (geteoatangle() < eoatclosedangle)
                {
                    speak(task::greetshake);
                    for (uint8_t cnt{}; cnt < 3; cnt++)
                    {
                        dohandshake();
                    }
                    movehandshakepos();
                    speak(task::greetend);
                    usleep(1000 * 1000);
                    break;
                }
                speak(task::greetfail);
                movehandshakepos();
                usleep(2 * 1000 * 1000);
                initpos = getxyz();
            }
        }
        movebase();
    }

    void dance()
    {
        std::atomic<bool> singing{true};
        movedancebasepos();
        speak(task::dancestart);
        auto singingcall = std::async(std::launch::async, [this, &singing]() {
            auto phrases =
                std::to_array<task>({task::songlinefirst, task::songlinesecond,
                                     task::songlinethird, task::songlineforth});
            uint32_t cnt{};
            while (singing)
            {
                auto num = (cnt++) % phrases.size();
                speak(phrases[num]);
                usleep(250 * 1000);
            }
            speak(task::danceend);
        });

        auto ledcall = std::async(std::launch::async, [this, &singing]() {
            bool increase{true};
            int32_t level{}, step{5};
            while (singing)
            {
                increase = level > 120 ? false : level < 10 ? true : increase;
                level += increase ? step : -step;
                setledon((uint8_t)level);
                usleep(1000);
            }
            setledoff();
        });

        auto dancestates =
            std::to_array<xyzt_t>({{-65, 145, 160, dgrtorad(180 - 65)},
                                   {-140, 320, 355, dgrtorad(180 - 0)},
                                   {65, 35, 95, dgrtorad(180 - 25)},
                                   {60, 110, 455, dgrtorad(180 - 45)},
                                   {12, 400, -75, dgrtorad(180 - 10)}});

        std::random_device os_seed;
        const uint32_t seed = os_seed();
        std::mt19937 generator(seed);
        std::uniform_int_distribution<uint32_t> rand(0, dancestates.size() - 1);

        while (!Menu::isenterpressed())
        {
            static uint32_t prevpos{UINT32_MAX};
            uint32_t pos{};
            while ((pos = rand(generator)) == prevpos)
                ;
            movetopos(dancestates[pos]);
            prevpos = pos;
            usleep(1200 * 1000);
        }
        singing = false;
        movedancebasepos();
        ledcall.wait();
        singingcall.wait();
        movebase();
    }

    void enlight()
    {
        speak(task::enlightstart);
        setledoff();
        movehandshakepos();
        movetopos({424, 75, 168, dgrtorad(180 - 0)});

        auto ledcall = std::async(std::launch::async, [this]() {
            int32_t step{2};
            for (int32_t level{0}; level < 120; level += step)
            {
                setledon((uint8_t)level);
                usleep(10 * 1000);
            }
            speak(task::enlightbreak);
        });

        while (!Menu::isenterpressed())
        {
            usleep(100 * 1000);
        }

        ledcall.wait();
        speak(task::enlightend);

        ledcall = std::async(std::launch::async, [this]() {
            int32_t step{2};
            for (int32_t level{120}; level > 0; level -= step)
            {
                setledon((uint8_t)level);
                usleep(1000);
            }
        });

        ledcall.wait();
        setledoff();
        movebase();
    }

    void sendusercmd()
    {
        static const auto exitTag = "q";

        std::cout << "Commands mode, to exit enter: " << exitTag << "\n";
        while (true)
        {
            std::cout << "CMD> ";
            std::string usercmd;
            std::cin >> usercmd;
            std::cin.clear();
            std::cin.ignore(INT_MAX, '\n');

            if (usercmd != exitTag)
            {
                try
                {
                    auto json = json::parse(usercmd);
                    std::cout << sendcommand(json.dump()) << "\n";
                }
                catch (const json::parse_error& e)
                {
                    std::cerr << "Cannot covert string to json\n";
                }
            }
            else
            {
                std::cout << "\n";
                break;
            }
        }
    }

    void changevoice()
    {
        [[maybe_unused]] auto newvoice =
            std::get<1>(tts->voice) == tts::gender::female
                ? tts::gender::male
                : tts::gender::female;
    }

  private:
    std::shared_ptr<http::HttpIf> httpIf;
    std::shared_ptr<TtsSetup> tts;
    std::future<void> ttsasync;

    std::string str(auto num)
    {
        return std::to_string(num);
    }

    std::string setposcmd(const xyzt_t& pos, double spd)
    {
        const auto [x, y, z, t] = pos;
        return R"({"T":104,"x":)" + str(x) + R"(,"y":)" + str(y) + R"(,"z":)" +
               str(z) + R"(,"t":)" + str(t) + R"(,"spd":)" + str(spd) + "}";
    };

    std::string setposcmd(const xyzt_t& pos)
    {
        const auto [x, y, z, t] = pos;
        return R"({"T":1041,"x":)" + str(x) + R"(,"y":)" + str(y) + R"(,"z":)" +
               str(z) + R"(,"t":)" + str(t) + "}";
    };

    constexpr double radtodgr(double rad)
    {
        return round(rad * 180. / M_PI);
    }

    constexpr double dgrtorad(double dgr)
    {
        return dgr * M_PI / 180.;
    }

    std::tuple<double, double, double> getxyz()
    {
        auto json{json::parse(sendcommand(R"({"T":105})"))};
        return {json[0]["x"], json[0]["y"], json[0]["z"]};
    }

    int32_t geteoatangle()
    {
        auto json{json::parse(sendcommand(R"({"T":105})"))};
        return (int32_t)radtodgr(json[0]["t"]);
    }

    void movetopos(xyzt_t pos)
    {
        sendcommand(setposcmd(pos));
    }

    void movetopos(xyzt_t pos, uint32_t spd)
    {
        sendcommand(setposcmd(pos, spd));
    }

    void movehandshakepos()
    {
        movetopos({175, 235, 325, dgrtorad(180 - 35)}, 100);
    }

    void dohandshake()
    {
        movetopos({245, 310, 215, dgrtorad(180)}, 150);
        usleep(500 * 1000);
        movetopos({215, 280, 335, dgrtorad(180)}, 150);
        usleep(500 * 1000);
    }

    void movedancebasepos()
    {
        movetopos({175, 235, 325, dgrtorad(180 - 35)}, 100);
    }

    std::string sendcommand(const std::string& cmd)
    {
        return httpIf->get(cmd);
    }

    void speak(task what)
    {
        if (tts)
        {
            if (ttsasync.valid())
                ttsasync.wait();
            ttsasync = std::async(std::launch::async, [this, what]() {
                auto inlang = std::get<0>(tts->voice);
                tts->iface->speak(getttstext(what, inlang));
            });
        }
    }
};

Robothandler::Robothandler(std::shared_ptr<http::HttpIf> httpIf,
                           std::shared_ptr<TtsSetup> ttsSetup) :
    handler{std::make_unique<Handler>(httpIf, ttsSetup)}
{}

Robothandler::~Robothandler() = default;

std::string Robothandler::conninfo()
{
    return handler->getconninfo();
}

void Robothandler::readwifiinfo()
{
    std::cout << handler->getwifiinfo();
}

void Robothandler::readservosinfo()
{
    std::cout << handler->getservosinfo();
}

void Robothandler::openeoat()
{
    handler->openeoat();
}

void Robothandler::closeeoat()
{
    handler->closeeoat();
}

void Robothandler::readdeviceinfo()
{
    std::cout << handler->getdeviceinfo();
}

void Robothandler::settorqueunlocked()
{
    handler->settorqueunlocked();
}

void Robothandler::settorquelocked()
{
    handler->settorquelocked();
}

void Robothandler::setledon(uint8_t lvl)
{
    handler->setledon(lvl);
}

void Robothandler::setledoff()
{
    handler->setledoff();
}

void Robothandler::movebase()
{
    handler->movebase();
}

void Robothandler::moveleft()
{
    handler->moveleft();
}

void Robothandler::moveright()
{
    handler->moveright();
}

void Robothandler::shakehand()
{
    handler->shakehand();
}

void Robothandler::dance()
{
    handler->dance();
}

void Robothandler::moveparked()
{
    handler->moveparked();
}

void Robothandler::enlight()
{
    handler->enlight();
}

void Robothandler::engage()
{
    handler->engage();
}

void Robothandler::disengage()
{
    handler->disengage();
}

void Robothandler::sendusercmd()
{
    handler->sendusercmd();
}

void Robothandler::changevoice()
{
    handler->changevoice();
}

} // namespace robot
