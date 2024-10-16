#include "robotmgmt.hpp"

#include "climenu.hpp"
#include "httphandler.hpp"
#include "tts/interfaces/googlecloud.hpp"

#include <nlohmann/json.hpp>

#include <array>
#include <cmath>
#include <future>
#include <iostream>
#include <random>

using json = nlohmann::json;
using xyzt_t = std::tuple<int32_t, int32_t, int32_t, double>;

struct Robothandler::Handler
{
  public:
    Handler(std::shared_ptr<Httphandler> conn,
            std::shared_ptr<tts::TextToVoiceIf> tts) :
        http{conn},
        tts{tts}
    {}

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
        return http->get(cmd);
    }

  public:
    std::shared_ptr<Httphandler> http;
    std::shared_ptr<tts::TextToVoiceIf> tts;
};

Robothandler::Robothandler() :
    handler{std::make_unique<Handler>(
        std::make_shared<Httphandler>(),
        tts::TextToVoiceFactory<tts::googlecloud::TextToVoice>::create(
            {tts::language::polish, tts::gender::female, 1}))}
{}

Robothandler::~Robothandler() = default;

std::string Robothandler::conninfo()
{
    return handler->http->info();
}

void Robothandler::readwifiinfo()
{
    std::cout << handler->sendcommand(R"({"T":405})") << "\n";
}

void Robothandler::readservosinfo()
{
    std::cout << handler->sendcommand(R"({"T":105})") << "\n";
}

void Robothandler::openeoat()
{
    handler->sendcommand(
        R"({"T":121,"joint":4,"angle":135,"spd":50,"acc":10})");
}

void Robothandler::closeeoat()
{
    handler->sendcommand(
        R"({"T":121,"joint":4,"angle":180,"spd":50,"acc":10})");
}

void Robothandler::readdeviceinfo()
{
    std::cout << handler->sendcommand(R"({"T":302})") << "\n";
}

void Robothandler::settorqueunlocked()
{
    handler->sendcommand(R"({"T":210,"cmd":0})");
}

void Robothandler::settorquelocked()
{
    handler->sendcommand(R"({"T":210,"cmd":1})");
}

void Robothandler::setledon(uint8_t lvl)
{
    const auto cmd = R"({"T":114,"led":)" + std::to_string(lvl) + "}";
    handler->sendcommand(cmd);
}

void Robothandler::setledoff()
{
    handler->sendcommand(R"({"T":114,"led":0})");
}

void Robothandler::movebase()
{
    handler->sendcommand(R"({"T":100})");
    handler->tts->speak("Robot gotowy do działania");
}

void Robothandler::moveleft()
{
    handler->sendcommand(R"({"T":121,"joint":1,"angle":45,"spd":10,"acc":10})");
}

void Robothandler::moveright()
{
    handler->sendcommand(
        R"({"T":121,"joint":1,"angle":-45,"spd":10,"acc":10})");
}

void Robothandler::shakehand()
{
    handler->movehandshakepos();
    std::future<void> ttscall = std::async(std::launch::async, [this]() {
        handler->tts->speak("No podaj łapę no!");
    });
    usleep(2 * 1000 * 1000);
    auto initpos = handler->getxyz();
    while (!Menu::isenterpressed())
    {
        if (initpos != handler->getxyz())
        {
            closeeoat();
            usleep(800 * 1000);
            constexpr int32_t eoatclosedangle{177};
            if (handler->geteoatangle() < eoatclosedangle)
            {
                ttscall = std::async(std::launch::async, [this]() {
                    handler->tts->speak("Czeeeeść czołem kluski z rosołem");
                });
                for (uint8_t cnt{}; cnt < 3; cnt++)
                {
                    handler->dohandshake();
                }
                handler->movehandshakepos();
                ttscall = std::async(std::launch::async, [this]() {
                    handler->tts->speak("Dobra wystarczy bo się zagłaskamy");
                });
                usleep(1000 * 1000);
                break;
            }
            ttscall = std::async(std::launch::async, [this]() {
                handler->tts->speak("Nie chcesz? A to spierdalaj!");
                handler->tts->speak(
                    "Ale to nie chlew, może się jednak przywitasz?");
            });
            handler->movehandshakepos();
            usleep(2 * 1000 * 1000);
            initpos = handler->getxyz();
        }
    }
    movebase();
}

void Robothandler::dance()
{
    std::atomic<bool> singing{true};
    handler->movedancebasepos();
    handler->tts->speak("Zapraszasz do tańca? :)");
    auto ttscall = std::async(std::launch::async, [this, &singing]() {
        auto phrases = std::to_array<std::string>(
            {"Przez twe oczy, te oczy zielone oszalałam!", "Lalala!",
             "Gwiazdy chyba twym oczom oddały cały blask!", "Lalalala!"});
        uint32_t cnt{};
        while (singing)
        {
            auto num = (cnt++) % phrases.size();
            handler->tts->speak(phrases[num]);
            usleep(250 * 1000);
        }
        handler->tts->speak("Co to?? Masz już dość?? HEHE HEHE!");
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
        std::to_array<xyzt_t>({{-65, 145, 160, handler->dgrtorad(180 - 65)},
                               {-140, 320, 355, handler->dgrtorad(180 - 0)},
                               {65, 35, 95, handler->dgrtorad(180 - 25)},
                               {60, 110, 455, handler->dgrtorad(180 - 45)},
                               {12, 400, -75, handler->dgrtorad(180 - 10)}});

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
        handler->movetopos(dancestates[pos]);
        prevpos = pos;
        usleep(1200 * 1000);
    }
    singing = false;
    handler->movedancebasepos();
    ledcall.wait();
    ttscall.wait();
    movebase();
}

void Robothandler::moveparked()
{
    const auto cmd =
        handler->setposcmd({80, 0, 455, handler->dgrtorad(180 - 35)});
    handler->sendcommand(cmd);
}

void Robothandler::enlight()
{
    auto ttscall = std::async(
        std::launch::async, [this]() { handler->tts->speak("Oświecić cię?"); });
    setledoff();
    handler->movehandshakepos();
    handler->movetopos({424, 75, 168, handler->dgrtorad(180 - 0)});

    ttscall.wait();
    auto ledcall = std::async(std::launch::async, [this]() {
        int32_t step{2};
        for (int32_t level{0}; level < 120; level += step)
        {
            setledon((uint8_t)level);
            usleep(10 * 1000);
        }
        handler->tts->speak("Klepnij enter żeby zakończyć");
    });

    while (!Menu::isenterpressed())
    {
        usleep(100 * 1000);
    }

    ledcall.wait();
    ttscall = std::async(std::launch::async, [this]() {
        handler->tts->speak("Oświecenie zakończone");
    });

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
    ttscall.wait();
}

void Robothandler::engage()
{
    auto ttscall = std::async(std::launch::async, [this]() {
        handler->tts->speak("Rozpoczynam inicjalizację");
    });
    movebase();
    ttscall.wait();
}

void Robothandler::disengage()
{
    auto ttscall = std::async(std::launch::async, [this]() {
        handler->tts->speak("Robot został odstawiony");
    });
    setledoff();
    moveparked();
    settorquelocked();
    ttscall.wait();
}

void Robothandler::sendusercmd()
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
                std::cout << handler->sendcommand(json.dump()) << "\n";
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
