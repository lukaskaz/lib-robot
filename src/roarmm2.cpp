#include "robot/interfaces/roarmm2.hpp"

#include "menu/interfaces/cli.hpp"
#include "robot/ttstexts.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <future>
#include <iostream>
#include <memory>
#include <random>

namespace robot::roarmm2
{

static constexpr int32_t posmargin = 1;
static constexpr int32_t eoatclosedangle = 180;

using xyzt_t = std::tuple<int32_t, int32_t, int32_t, double>;
using xyz_t = std::tuple<int32_t, int32_t, int32_t>;
struct HttoOutputVisitor
{
    auto operator()([[maybe_unused]] const std::monostate& arg) -> std::string
    {
        return {};
    }

    auto operator()(const std::string& arg) -> std::string
    {
        return arg;
    }

    auto operator()(const auto& arg) -> std::string
    {
        return std::to_string(arg);
    }
};

struct Robot::Handler
{
  public:
    Handler(std::shared_ptr<http::HttpIf> httpIf,
            std::shared_ptr<tts::TextToVoiceIf> ttsIf,
            std::shared_ptr<logging::LogIf> logIf) :
        httpIf{httpIf},
        ttsIf{ttsIf}, logIf{logIf}
    {
        if (!this->httpIf)
        {
            throw std::runtime_error("No interface to connect to robot");
        }
    }

    bool isenterpressed()
    {
        return menu::cli::Menu::isenterpressed();
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
        sendcommand({{"T", 100}});
        speak(task::ready);
    }

    void moveleft()
    {
        sendcommand({{"T", 121},
                     {"joint", 1},
                     {"angle", 45},
                     {"spd", 10},
                     {"acc", 10}});
    }

    void moveright()
    {
        sendcommand({{"T", 121},
                     {"joint", 1},
                     {"angle", -45},
                     {"spd", 10},
                     {"acc", 10}});
    }

    void moveparked()
    {
        sendcommand(setposcmd({80, 0, 455, dgrtorad(180 - 35)}));
    }

    void settorqueunlocked()
    {
        sendcommand({{"T", 210}, {"cmd", 0}});
    }

    void settorquelocked()
    {
        sendcommand({{"T", 210}, {"cmd", 1}});
    }

    void setledon(uint8_t lvl)
    {
        sendcommand({{"T", 114}, {"led", lvl}});
        ledstatus = true;
    }

    void setledoff()
    {
        sendcommand({{"T", 114}, {"led", 0}});

        ledstatus = false;
    }

    std::string getwifiinfo()
    {
        http::outputtype output;
        if (sendcommand({{"T", 405}}, output))
        {
            return getstrfromhttp(output);
        }
        return {};
    }

    std::string getservosinfo()
    {
        http::outputtype output;
        if (sendcommand({{"T", 105}}, output))
        {
            auto eoatdgr = (int32_t)radtodgr(std::get<double>(output.at("t")));
            return getstrfromhttp(output) + "t : " + std::to_string(eoatdgr);
        }
        return {};
    }

    bool seteoat(int32_t rawangle, int32_t& retangle)
    {
        struct Eoat
        {
            Eoat(Handler* handler, int32_t rawangle) :
                handler{handler}, setpoint{convert(rawangle)},
                currangle{handler->geteoatangle()}
            {}

            void move()
            {
                handler->sendcommand({{"T", 121},
                                      {"joint", 4},
                                      {"angle", setpoint},
                                      {"spd", 50},
                                      {"acc", 10}});
                waitmoving();
            }

            bool isatposition() const
            {
                return handler->isposaccepted(currangle, setpoint);
            }

            int32_t getposition() const
            {
                return convert(currangle);
            }

          private:
            const Handler* handler;
            const int32_t setpoint;
            const uint32_t maxrepeats{3};
            int32_t currangle{}, prevangle{};
            uint32_t repeats{};

            void waitmoving()
            {
                while ((currangle = handler->geteoatangle()) != setpoint)
                {
                    if (currangle == prevangle)
                    {
                        if (handler->isposaccepted(currangle, setpoint))
                        {
                            handler->log(
                                logging::type::debug,
                                "Eaot not in setpoint, but within margin");
                            break;
                        }
                        if (repeats++ == maxrepeats)
                        {
                            handler->log(logging::type::warning,
                                         "Eaot cannot reach setpoint");
                            break;
                        }
                    }
                    else
                    {
                        repeats = 0;
                        prevangle = currangle;
                    }
                }
            }

            int32_t convert(int32_t angle) const
            {
                return eoatclosedangle - angle;
            }
        };

        Eoat eoat{this, rawangle};
        if (!eoat.isatposition())
            eoat.move();
        retangle = eoat.getposition();
        return eoat.isatposition();
    }

    bool openeoat()
    {
        [[maybe_unused]] int32_t retangle{};
        return seteoat(45, retangle);
    }

    bool closeeoat()
    {
        [[maybe_unused]] int32_t retangle{};
        return seteoat(0, retangle);
    }

    std::string getdeviceinfo()
    {
        http::outputtype output;
        if (sendcommand({{"T", 302}}, output))
        {
            return getstrfromhttp(output);
        }
        return {};
    }

    void shakehand()
    {
        movehandshakepos();
        speak(task::greetstart);
        usleep(2 * 1000 * 1000);

        auto initpos = getxyz();
        while (!isenterpressed())
        {
            if (initpos != getxyz())
            {
                if (!closeeoat())
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
                speak(phrases[num], false);
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

        while (!isenterpressed())
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
        tts::TextToVoiceIf::kill();
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

        while (!isenterpressed())
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
        static const std::string exitTag = "q";

        std::cout << "Commands mode, to exit enter: " << exitTag << "\n";
        while (true)
        {
            std::cout << "CMD> ";
            std::string usercmd;
            std::getline(std::cin, usercmd);
            if (usercmd != exitTag)
            {
                try
                {
                    log(logging::type::info, sendcommand(usercmd));
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Given json is invalid: " << e.what() << "\n";
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
        speak(task::voicechangestart);
        auto voice = ttsIf->getvoice();
        auto& gender = std::get<1>(voice);
        gender = gender == tts::gender::male ? tts::gender::female
                                             : tts::gender::male;
        waitspeakdone();
        ttsIf->setvoice(voice);
        speak(task::voicechangeend);
    }

    void changelangtopolish()
    {
        auto voice = ttsIf->getvoice();
        auto& lang = std::get<0>(voice);
        if (lang != tts::language::polish)
        {
            lang = tts::language::polish;
            speak(task::langchangestart);
            waitspeakdone();
            ttsIf->setvoice(voice);
            speak(task::langchangeend);
        }
        else
        {
            speak(task::nothingtodo);
        }
    }

    void changelangtoenglish()
    {
        auto voice = ttsIf->getvoice();
        auto& lang = std::get<0>(voice);
        if (lang != tts::language::english)
        {
            lang = tts::language::english;
            speak(task::langchangestart);
            waitspeakdone();
            ttsIf->setvoice(voice);
            speak(task::langchangeend);
        }
        else
        {
            speak(task::nothingtodo);
        }
    }

    void changelangtogerman()
    {
        auto voice = ttsIf->getvoice();
        auto& lang = std::get<0>(voice);
        if (lang != tts::language::german)
        {
            lang = tts::language::german;
            speak(task::langchangestart);
            waitspeakdone();
            ttsIf->setvoice(voice);
            speak(task::langchangeend);
        }
        else
        {
            speak(task::nothingtodo);
        }
    }

    void log(logging::type type, const std::string& msg) const
    {
        if (logIf)
        {
            logIf->log(type, module, msg);
        }
    }

    tts::language getlanguage()
    {
        auto voice = ttsIf->getvoice();
        return std::get<0>(voice);
    }

    bool iseoatclosed()
    {
        return isposaccepted(geteoatangle(), eoatclosedangle);
    }

    bool isledon()
    {
        return ledstatus;
    }

  private:
    std::string module{"librobot"};
    std::shared_ptr<http::HttpIf> httpIf;
    std::shared_ptr<tts::TextToVoiceIf> ttsIf;
    std::shared_ptr<logging::LogIf> logIf;
    std::future<void> ttsasync;
    bool ledstatus{};

    http::inputtype setposcmd(const xyzt_t& pos, double spd)
    {
        const auto [x, y, z, t] = pos;
        return {{"T", 104}, {"x", x}, {"y", y},
                {"z", z},   {"t", t}, {"spd", spd}};
    };

    http::inputtype setposcmd(const xyzt_t& pos)
    {
        const auto [x, y, z, t] = pos;
        return {{"T", 1041}, {"x", x}, {"y", y}, {"z", z}, {"t", t}};
    }

    constexpr double radtodgr(double rad) const
    {
        return round(rad * 180. / M_PI);
    }

    constexpr double dgrtorad(double dgr) const
    {
        return dgr * M_PI / 180.;
    }

    xyzt_t getxyzt()
    {
        http::outputtype ret;
        sendcommand({{"T", 105}}, ret);
        return {(int32_t)std::get<double>(ret.at("x")),
                (int32_t)std::get<double>(ret.at("y")),
                (int32_t)std::get<double>(ret.at("z")),
                std::get<double>(ret.at("t"))};
    }

    xyz_t getxyz()
    {
        http::outputtype ret;
        sendcommand({{"T", 105}}, ret);
        return {(int32_t)std::get<double>(ret.at("x")),
                (int32_t)std::get<double>(ret.at("y")),
                (int32_t)std::get<double>(ret.at("z"))};
    }

    int32_t geteoatangle() const
    {
        http::outputtype ret;
        sendcommand({{"T", 105}}, ret);
        return (int32_t)radtodgr(std::get<double>(ret.at("t")));
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

    bool sendcommand(const http::inputtype& in, http::outputtype& out) const
    {
        return httpIf->get(in, out);
    }

    template <typename In = http::inputtype>
    std::string sendcommand(const In& in) const
    {
        std::string resp;
        httpIf->get(in, resp);
        return resp;
    }

    void speak(task what, bool async = true)
    {
        if (ttsIf)
        {
            if (async)
            {
                waitspeakdone();
                ttsasync = std::async(std::launch::async, [this, what]() {
                    auto inlang = std::get<0>(ttsIf->getvoice());
                    ttsIf->speak(getttstext(what, inlang));
                });
            }
            else
            {
                waitspeakdone();
                auto inlang = std::get<0>(ttsIf->getvoice());
                ttsIf->speak(getttstext(what, inlang));
            }
        }
    }

    void waitspeakdone()
    {
        if (ttsasync.valid())
        {
            ttsasync.wait();
        }
    }

    std::string getstrfromhttp(const http::outputtype& out)
    {
        std::string str;
        std::ranges::for_each(out, [&str](const auto& item) {
            str += item.first + " : " +
                   std::visit(HttoOutputVisitor(), item.second) + "\n";
        });
        return str;
    }

    bool isposaccepted(int32_t present, int32_t expected) const
    {
        return std::abs(present - expected) <= posmargin;
    }
};

Robot::Robot(std::shared_ptr<http::HttpIf> httpIf,
             std::shared_ptr<tts::TextToVoiceIf> ttsIf,
             std::shared_ptr<logging::LogIf> logIf) :
    handler{std::make_unique<Handler>(httpIf, ttsIf, logIf)}
{}

Robot::~Robot() = default;

std::string Robot::conninfo()
{
    return handler->getconninfo();
}

bool Robot::readwifiinfo(bool isshown)
{
    if (isshown)
        return true;
    handler->log(logging::type::info, handler->getwifiinfo());
    return true;
}

bool Robot::readservosinfo(bool isshown)
{
    if (isshown)
        return true;
    handler->log(logging::type::info, handler->getservosinfo());
    return true;
}

bool Robot::openeoat(bool isshown)
{
    if (isshown)
        return handler->iseoatclosed();
    return !handler->openeoat();
}

bool Robot::closeeoat(bool isshown)
{
    if (isshown)
        return !handler->iseoatclosed();
    return !handler->closeeoat();
}

bool Robot::readdeviceinfo(bool isshown)
{
    if (isshown)
        return true;
    handler->log(logging::type::info, handler->getdeviceinfo());
    return true;
}

bool Robot::settorqueunlocked(bool isshown)
{
    if (isshown)
        return true;
    handler->settorqueunlocked();
    return false;
}

bool Robot::settorquelocked(bool isshown)
{
    if (isshown)
        return true;
    handler->settorquelocked();
    return false;
}

bool Robot::setledon(bool isshown, uint8_t lvl)
{
    if (isshown)
        return !handler->isledon();
    handler->setledon(lvl);
    return false;
}

bool Robot::setledoff(bool isshown)
{
    if (isshown)
        return handler->isledon();
    handler->setledoff();
    return false;
}

bool Robot::movebase(bool isshown)
{
    if (isshown)
        return true;
    handler->movebase();
    return false;
}

bool Robot::moveleft(bool isshown)
{
    if (isshown)
        return true;
    handler->moveleft();
    return false;
}

bool Robot::moveright(bool isshown)
{
    if (isshown)
        return true;
    handler->moveright();
    return false;
}

bool Robot::shakehand(bool isshown)
{
    if (isshown)
        return true;
    handler->shakehand();
    return false;
}

bool Robot::dance(bool isshown)
{
    if (isshown)
        return true;
    handler->dance();
    return false;
}

bool Robot::moveparked(bool isshown)
{
    if (isshown)
        return true;
    handler->moveparked();
    return false;
}

bool Robot::enlight(bool isshown)
{
    if (isshown)
        return true;
    handler->enlight();
    return false;
}

bool Robot::engage()
{
    handler->engage();
    return true;
}

bool Robot::disengage()
{
    handler->disengage();
    return true;
}

bool Robot::sendusercmd(bool isshown)
{
    if (isshown)
        return true;
    handler->sendusercmd();
    return true;
}

bool Robot::changevoice(bool isshown)
{
    if (isshown)
        return true;
    handler->changevoice();
    return false;
}

bool Robot::changelangtopolish(bool isshown)
{
    if (isshown)
        return handler->getlanguage() != tts::language::polish;
    handler->changelangtopolish();
    return false;
}

bool Robot::changelangtoenglish(bool isshown)
{
    if (isshown)
        return handler->getlanguage() != tts::language::english;
    handler->changelangtoenglish();
    return false;
}

bool Robot::changelangtogerman(bool isshown)
{
    if (isshown)
        return handler->getlanguage() != tts::language::german;
    handler->changelangtogerman();
    return false;
}

} // namespace robot::roarmm2
