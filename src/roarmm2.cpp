#include "robot/interfaces/roarmm2.hpp"

#include "menu/interfaces/cli.hpp"
#include "random/generators/uniform.hpp"
#include "robot/helpers.hpp"
#include "robot/ttstexts.hpp"
#include "shellcommand.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <future>
#include <iostream>
#include <memory>
#include <numbers>
#include <source_location>
#include <type_traits>

namespace robot::roarmm2
{

static constexpr int32_t anglemargin = 1;
static constexpr int32_t xyztmargin = 10;
static constexpr int32_t posmargin = 1;
static constexpr int32_t eoatjoint = 4;
static constexpr int32_t eoatopenedangle = 45;
static constexpr int32_t eoatclosedangle = 0;
static constexpr int32_t eoatanglebase = 180;

using xyzt_t = std::tuple<int32_t, int32_t, int32_t, int32_t>;
using xyz_t = std::tuple<int32_t, int32_t, int32_t>;

template <typename T>
concept Positional = robothelpers::Numerical<T>;

template <typename T>
concept Difflimit = std::is_integral_v<T>;

template <typename T>
concept TupleType = requires(T t) {
    std::tuple_size<T>::value;
    std::get<0>(t);
};

using actiontype = std::function<void(std::stop_token, bool)>;

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

    void movebase(bool inform = true)
    {
        sendcommand({{"T", 100}});
        if (inform)
            speak(task::ready);
    }

    void moveleft()
    {
        movebase(false);
        auto pos = getxyzt();
        auto& axistomove = std::get<1>(pos);
        axistomove += 200;
        movetopos(pos, 1.f);
    }

    void moveright()
    {
        movebase(false);
        auto pos = getxyzt();
        auto& axistomove = std::get<1>(pos);
        axistomove -= 200;
        movetopos(pos, 1.f);
    }

    void moveparked()
    {
        movetopos({80, 0, 455, 35});
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
            auto eoatdgr = eoatanglebase -
                           (int32_t)radtodgr(std::get<double>(output.at("t")));
            return getstrfromhttp(output) + "t : " + std::to_string(eoatdgr);
        }
        return {};
    }

    std::pair<bool, int32_t> seteoat(const int32_t setangle)
    {
        MoveJoint eoat{this, eoatjoint, setangle};
        return eoat.getstatus();
    }

    bool openeoat()
    {
        return std::get<bool>(seteoat(eoatopenedangle));
    }

    bool closeeoat()
    {
        return std::get<bool>(seteoat(eoatclosedangle));
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
        speak(task::greetstart);
        auto prevxyz = movehandshakepos();
        while (!isenterpressed())
        {
            if (prevxyz != getxyz())
            {
                if (!closeeoat())
                {
                    speak(task::greetshake);
                    dohandshake(3);
                    speak(task::greetend);
                    break;
                }
                speak(task::greetfail);
                prevxyz = movehandshakepos();
            }
        }
        movehandshakepos();
        movebase();
    }

    void dance(
        const actiontype& soundaction = [](std::stop_token, bool) {},
        const actiontype& lightaction = [](std::stop_token, bool) {})
    {
        std::stop_source running;
        movedancebasepos();
        speak(task::dancestart, false);
        auto soundasync =
            std::async(std::launch::async,
                       std::bind(soundaction, running.get_token(), false));
        auto lightasync =
            std::async(std::launch::async,
                       std::bind(lightaction, running.get_token(), false));

        static constexpr auto danceposes =
            std::to_array<xyzt_t>({{-65, 145, 160, 65},
                                   {-140, 320, 355, 0},
                                   {65, 35, 95, 25},
                                   {60, 110, 455, 45},
                                   {12, 400, -75, 10}});

        auto randpos = Builder::get<Uniform<uint32_t>>(
            std::make_pair(0, danceposes.size() - 1));
        while (!isenterpressed())
        {
            uint32_t currpos{}, prevpos{UINT32_MAX};
            while ((currpos = randpos()) == prevpos)
                ;
            movetopos(danceposes.at(currpos), 1000u);
            prevpos = currpos;
        }

        speak(task::danceend);
        lightaction(running.get_token(), true);
        soundaction(running.get_token(), true);
        running.request_stop();
        movedancebasepos();
        lightasync.wait();
        soundasync.wait();
        movebase();
    }

    void enlight()
    {
        static constexpr uint8_t ledmin{0}, ledmax{120}, ledstep{2};
        speak(task::enlightstart);
        setledoff();
        movehandshakepos();
        movetopos({424, 75, 168, 0});
        speak(task::enlightbreak);
        for (uint8_t level{ledmin}; level < ledmax; level += ledstep)
        {
            setledon(level);
            usleep(10 * 1000);
        }
        while (!isenterpressed())
        {
            usleep(100 * 1000);
        }
        speak(task::enlightend);
        for (uint8_t level{ledmax}; level > ledmin; level -= ledstep)
        {
            setledon(level);
            usleep(1000);
        }
        setledoff();
        movebase();
    }

    void doheman(const actiontype& soundaction = [](std::stop_token, bool) {})
    {
        std::stop_source running;
        auto movingaction = [this](std::stop_token running, bool cleanup) {
            if (cleanup)
            {
                return;
            }

            static constexpr uint8_t levelmax{255}, levelmin{50};
            uint8_t level{levelmax};
            while (!running.stop_requested())
            {
                if (level == levelmax)
                {
                    setledon(level = levelmin);
                    movetopos({1, 1, 450, 45}, 100.f);
                    usleep(500 * 1000);
                }
                else
                {
                    movetopos({-1, -1, 518, 15}, 100.f);
                    setledon(level = levelmax);
                    usleep(1000 * 1000);
                }
            }
            setledoff();
        };

        speak(task::hemanstart);
        movetopos({-1, -1, 518, 15}, 100.f);

        auto soundasync =
            std::async(std::launch::async,
                       std::bind(soundaction, running.get_token(), false));
        auto movingasync =
            std::async(std::launch::async,
                       std::bind(movingaction, running.get_token(), false));

        while (!isenterpressed() && soundasync.wait_for(std::chrono::seconds(
                                        0)) != std::future_status::ready)
        {
            usleep(100 * 1000);
        }

        movingaction(running.get_token(), true);
        soundaction(running.get_token(), true);
        running.request_stop();
        speak(task::hemanend);
        movingasync.wait();
        soundasync.wait();
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

    bool changelangtopolish()
    {
        return changelanguage(tts::language::polish);
    }

    bool changelangtoenglish()
    {
        return changelanguage(tts::language::english);
    }

    bool changelangtogerman()
    {
        return changelanguage(tts::language::german);
    }

    bool isttssupported()
    {
        return ttsIf ? true : false;
    }

    void log(
        logging::type type, const std::string& msg,
        const std::source_location loc = std::source_location::current()) const
    {
        if (logIf)
        {
            logIf->log(type, module,
                       "[" + std::string(loc.function_name()) + "] " + msg);
        }
    }

    tts::language getlanguage()
    {
        auto voice = ttsIf->getvoice();
        return std::get<0>(voice);
    }

    bool iseoatclosed()
    {
        return isposaccepted(geteoatangle(), eoatclosedangle, anglemargin);
    }

    bool iseoatopened()
    {
        return isposaccepted(geteoatangle(), eoatopenedangle, anglemargin);
    }

    bool isledon()
    {
        return ledstatus;
    }

    void speak(task what, bool async = true)
    {
        if (isttssupported())
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
        else
        {
            log(logging::type::debug, "tts not supported");
        }
    }

  private:
    std::string module{"librobot"};
    std::shared_ptr<http::HttpIf> httpIf;
    std::shared_ptr<tts::TextToVoiceIf> ttsIf;
    std::shared_ptr<logging::LogIf> logIf;
    std::future<void> ttsasync;
    bool ledstatus{};
    struct MoveJoint
    {
        MoveJoint(Handler* handler, uint32_t joint, int32_t setangle) :
            handler{handler}, setpoint{setangle},
            currangle{handler->geteoatangle()}
        {
            if (!isatposition())
            {
                auto angle = handler->angleadapt(setpoint);
                handler->sendcommand({{"T", 121},
                                      {"joint", joint},
                                      {"angle", angle},
                                      {"spd", 50},
                                      {"acc", 10}});
                waitmoving();
            }
        }

        std::pair<bool, int32_t> getstatus() const
        {
            return {isatposition(), currangle};
        }

      private:
        const Handler* handler;
        const int32_t setpoint;
        const uint32_t maxrepeats{3};
        int32_t currangle{}, prevangle{};
        uint32_t repeats{};

        bool isatposition() const
        {
            return handler->isposaccepted(currangle, setpoint, anglemargin);
        }

        void waitmoving()
        {
            while ((currangle = handler->geteoatangle()) != setpoint)
            {
                if (currangle == prevangle)
                {
                    if (isatposition())
                    {
                        handler->log(logging::type::debug,
                                     "Joint not in setpoint, but within "
                                     "acceptable position");
                        break;
                    }
                    if (repeats++ == maxrepeats)
                    {
                        handler->log(logging::type::warning,
                                     "Joint cannot reach setpoint");
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
    };
    struct MoveXyz
    {
        MoveXyz(Handler* handler, xyzt_t setpos) :
            MoveXyz(handler, setpos, 0, [&]() {
                const auto [x, y, z, t] = setpos;
                auto angle = handler->dgrtorad(handler->angleadapt(t));
                handler->sendcommand(
                    {{"T", 1041}, {"x", x}, {"y", y}, {"z", z}, {"t", angle}});
            })
        {}

        MoveXyz(Handler* handler, xyzt_t setpos, uint32_t delayms) :
            MoveXyz(handler, setpos, delayms, [&]() {
                const auto [x, y, z, t] = setpos;
                auto angle = handler->dgrtorad(handler->angleadapt(t));
                handler->sendcommand(
                    {{"T", 1041}, {"x", x}, {"y", y}, {"z", z}, {"t", angle}});
            })
        {}

        MoveXyz(Handler* handler, const xyzt_t& setpos, double speed) :
            MoveXyz(handler, setpos, 0, [&]() {
                const auto [x, y, z, t] = setpos;
                auto angle = handler->dgrtorad(handler->angleadapt(t));
                handler->sendcommand({{"T", 104},
                                      {"x", x},
                                      {"y", y},
                                      {"z", z},
                                      {"t", angle},
                                      {"spd", speed}});
            })
        {}

        std::pair<bool, xyz_t> getstatus() const
        {
            return {isatposition(), currpos};
        }

      private:
        MoveXyz(
            Handler* handler, const xyzt_t& setpos, uint32_t delayms,
            auto sendcommand = []() {}) :
            handler{handler},
            setpoint{adapt(setpos)}
        {
            sendcommand();
            if (!delayms)
                waitmoving();
            else
                usleep(delayms * 1000);
        }

      private:
        const Handler* handler;
        const xyz_t setpoint;
        const uint32_t maxrepeats{3};
        xyz_t currpos{}, prevpos{};
        uint32_t repeats{};

        xyz_t adapt(xyzt_t pos) const
        {
            return {std::get<0>(pos), std::get<1>(pos), std::get<2>(pos)};
        }

        bool isatposition() const
        {
            return handler->isposaccepted(currpos, setpoint, xyztmargin);
        }

        void waitmoving()
        {
            while ((currpos = handler->getxyz()) != setpoint)
            {
                if (currpos == prevpos)
                {
                    if (isatposition())
                    {
                        handler->log(logging::type::debug,
                                     "xyzt not in setpoint, but within "
                                     "acceptable position");
                        break;
                    }
                    if (repeats++ == maxrepeats)
                    {
                        handler->log(logging::type::warning,
                                     "xyzt position cannot reach setpoint");
                        break;
                    }
                }
                else
                {
                    repeats = 0;
                    prevpos = currpos;
                }
            }
        }
    };

    bool changelanguage(tts::language newlang)
    {
        if (isttssupported())
        {
            auto voice = ttsIf->getvoice();
            auto& lang = std::get<0>(voice);
            if (lang != newlang)
            {
                lang = newlang;
                speak(task::langchangestart);
                waitspeakdone();
                ttsIf->setvoice(voice);
                speak(task::langchangeend);
            }
            else
            {
                speak(task::nothingtodo);
            }
            return true;
        }
        log(logging::type::debug, "tts not supported");
        return false;
    }

    constexpr int32_t angleadapt(int32_t angle) const
    {
        return eoatanglebase - angle;
    }

    constexpr double radtodgr(double rad) const
    {
        return std::round(rad * 180. / std::numbers::pi_v<double>);
    }

    constexpr double dgrtorad(double dgr) const
    {
        return dgr * std::numbers::pi_v<double> / 180.;
    }

    xyz_t getxyz() const
    {
        http::outputtype ret;
        sendcommand({{"T", 105}}, ret);
        return {(int32_t)std::get<double>(ret.at("x")),
                (int32_t)std::get<double>(ret.at("y")),
                (int32_t)std::get<double>(ret.at("z"))};
    }

    xyzt_t getxyzt() const
    {
        http::outputtype ret;
        sendcommand({{"T", 105}}, ret);
        return {(int32_t)std::get<double>(ret.at("x")),
                (int32_t)std::get<double>(ret.at("y")),
                (int32_t)std::get<double>(ret.at("z")),
                eoatanglebase -
                    (int32_t)radtodgr(std::get<double>(ret.at("t")))};
    }

    int32_t geteoatangle() const
    {
        return std::get<3>(getxyzt());
    }

    std::pair<bool, xyz_t> movetopos(const xyzt_t& pos)
    {
        MoveXyz eoat{this, pos};
        return eoat.getstatus();
    }

    std::pair<bool, xyz_t> movetopos(const xyzt_t& pos, uint32_t delayms)
    {
        MoveXyz eoat{this, pos, delayms};
        return eoat.getstatus();
    }

    std::pair<bool, xyz_t> movetopos(xyzt_t pos, double spd)
    {
        MoveXyz eoat{this, pos, spd};
        return eoat.getstatus();
    }

    xyz_t movehandshakepos()
    {
        return std::get<xyz_t>(movetopos({175, 235, 325, 35}));
    }

    void dohandshake(uint32_t num)
    {
        while (num--)
        {
            movetopos({220, 280, 40, 0}, 700u);
            movetopos({210, 255, 300, 0}, 700u);
        }
    }

    void movedancebasepos()
    {
        movetopos({175, 235, 325, 35}, 100.f);
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

    bool isposaccepted(Positional auto present, Positional auto expected,
                       Difflimit auto limit) const
    {
        return std::abs(present - expected) <= limit;
    }

    bool isposaccepted(const TupleType auto& present,
                       const TupleType auto& expected,
                       Difflimit auto limit) const
    {
        return std::apply(
            [&](auto... x) {
                return std::apply(
                    [&](auto... y) {
                        return (isposaccepted(x, y, limit) && ...);
                    },
                    expected);
            },
            present);
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
        return !handler->iseoatopened();
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

bool Robot::dancesing(bool isshown)
{
    if (isshown)
        return true;

    handler->dance(
        [this](std::stop_token running, bool cleanup) {
            if (cleanup)
            {
                tts::TextToVoiceIf::kill();
                return;
            };

            auto phrases =
                std::to_array<task>({task::songlinefirst, task::songlinesecond,
                                     task::songlinethird, task::songlineforth});
            uint32_t cnt{};
            while (!running.stop_requested())
            {
                auto num = (cnt++) % phrases.size();
                handler->speak(phrases[num], false);
            }
        },
        [this](std::stop_token running, bool cleanup) {
            if (cleanup)
            {
                return;
            }

            bool increase{true};
            int32_t level{}, step{5};
            while (!running.stop_requested())
            {
                increase = level > 120 ? false : level < 10 ? true : increase;
                level += increase ? step : -step;
                handler->setledon((uint8_t)level);
                usleep(1000);
            }
            handler->setledoff();
        });
    return false;
}

bool Robot::dancestream(bool isshown)
{
    if (isshown)
        return true;

    handler->dance(
        [this](std::stop_token running, bool cleanup) {
            if (cleanup)
            {
                shell::BashCommand().run("killall -s KILL ffplay");
                return;
            }

            while (!running.stop_requested())
            {
                shell::BashCommand().run(
                    "yt-dlp https://youtu.be/DedaEVIbTkY -o - 2>/dev/null | "
                    "ffplay "
                    "-nodisp -autoexit -nostats - &> /dev/null");
            }
        },
        [this](std::stop_token running, bool cleanup) {
            if (cleanup)
            {
                return;
            }

            bool increase{true};
            int32_t level{}, step{5};
            while (!running.stop_requested())
            {
                increase = level > 120 ? false : level < 10 ? true : increase;
                level += increase ? step : -step;
                handler->setledon((uint8_t)level);
                usleep(1000);
            }
            handler->setledoff();
        });
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

bool Robot::doheman(bool isshown)
{
    if (isshown)
        return true;

    handler->doheman([this](std::stop_token, bool cleanup) {
        if (cleanup)
        {
            shell::BashCommand().run("killall -s KILL ffplay");
            return;
        }

        shell::BashCommand().run(
            "yt-dlp https://youtu.be/V8h8snfYidg -o - 2>/dev/null | ffplay "
            "-nodisp -autoexit -nostats - &> /dev/null");
    });
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
        return handler->isttssupported();
    handler->changevoice();
    return false;
}

bool Robot::changelangtopolish(bool isshown)
{
    if (isshown)
        return handler->isttssupported() &&
               handler->getlanguage() != tts::language::polish;
    return !handler->changelangtopolish();
}

bool Robot::changelangtoenglish(bool isshown)
{
    if (isshown)
        return handler->isttssupported() &&
               handler->getlanguage() != tts::language::english;
    return !handler->changelangtoenglish();
}

bool Robot::changelangtogerman(bool isshown)
{
    if (isshown)
        return handler->isttssupported() &&
               handler->getlanguage() != tts::language::german;
    return !handler->changelangtogerman();
}

} // namespace robot::roarmm2
