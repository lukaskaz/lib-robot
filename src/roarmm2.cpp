#include "robot/interfaces/roarmm2.hpp"

#include "menu/interfaces/cli.hpp"
#include "robot/ttstexts.hpp"
#include "shellcommand.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <future>
#include <iostream>
#include <memory>
#include <random>
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
concept Positional = std::is_arithmetic_v<T>;

template <typename T>
concept Difflimit = std::is_integral_v<T>;

template <typename T>
concept TupleType = requires(T t) {
    std::tuple_size<T>::value;
    std::get<0>(t);
};

using soundactiontype = std::function<void(std::atomic_bool&, bool)>;

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

    void dance(const soundactiontype& soundaction = [](const std::atomic_bool&,
                                                       bool) {})
    {
        std::atomic<bool> running{true};
        auto lightaction = [this](std::atomic_bool& running, bool kill) {
            if (kill)
            {
                running = false;
                return;
            }

            bool increase{true};
            int32_t level{}, step{5};
            while (running)
            {
                increase = level > 120 ? false : level < 10 ? true : increase;
                level += increase ? step : -step;
                setledon((uint8_t)level);
                usleep(1000);
            }
            setledoff();
        };

        movedancebasepos();
        speak(task::dancestart, false);
        auto soundasync =
            std::async(std::launch::async,
                       std::bind(soundaction, std::ref(running), false));
        auto lightasync =
            std::async(std::launch::async,
                       std::bind(lightaction, std::ref(running), false));

        static constexpr auto danceposes =
            std::to_array<xyzt_t>({{-65, 145, 160, 65},
                                   {-140, 320, 355, 0},
                                   {65, 35, 95, 25},
                                   {60, 110, 455, 45},
                                   {12, 400, -75, 10}});
        std::random_device os_seed;
        const uint32_t seed = os_seed();
        std::mt19937 generator(seed);
        std::uniform_int_distribution<uint32_t> rand(0, danceposes.size() - 1);

        while (true)
        {
            if (isenterpressed())
            {
                speak(task::danceend);
                lightaction(running, true);
                soundaction(running, true);
                movedancebasepos();
                lightasync.wait();
                soundasync.wait();
                break;
            }
            uint32_t curridx{}, previdx{UINT32_MAX};
            while ((curridx = rand(generator)) == previdx)
                ;
            movetopos(danceposes[curridx], 1000u);
            previdx = curridx;
        }
        movebase();
    }

    void enlight()
    {
        speak(task::enlightstart);
        setledoff();
        movehandshakepos();
        movetopos({424, 75, 168, 0});

        auto ledcall = std::async(std::launch::async, [this]() {
            int32_t step{2};
            for (int32_t level{0}; level < 120; level += step)
            {
                setledon((uint8_t)level);
                usleep(10 * 1000);
            }
            speak(task::enlightbreak, false);
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

    void doheman(const soundactiontype& soundaction)
    {
        std::atomic<bool> running{true};

        auto movingaction = [this](std::atomic_bool& running, bool kill) {
            if (kill)
            {
                running = false;
                return;
            }

            uint8_t level{255};
            while (running)
            {
                if (level == 255)
                {
                    setledon(level = 50);
                    movetopos({1, 1, 450, 45}, 100.f);
                    usleep(500 * 1000);
                }
                else
                {
                    movetopos({-1, -1, 518, 15}, 100.f);
                    setledon(level = 255);
                    usleep(1000 * 1000);
                }
            }
            setledoff();
        };

        speak(task::hemanstart);
        movetopos({-1, -1, 518, 15}, 100.f);

        auto soundasync =
            std::async(std::launch::async,
                       std::bind(soundaction, std::ref(running), false));
        auto movingasync =
            std::async(std::launch::async,
                       std::bind(movingaction, std::ref(running), false));

        while (true)
        {
            if (isenterpressed() || !running)
            {
                movingaction(running, true);
                soundaction(running, true);
                speak(task::hemanend);
                movingasync.wait();
                soundasync.wait();
                break;
            }
            usleep(100 * 1000);
        }
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

    constexpr int32_t angleadapt(int32_t angle) const
    {
        return eoatanglebase - angle;
    }

    constexpr double radtodgr(double rad) const
    {
        return round(rad * 180. / M_PI);
    }

    constexpr double dgrtorad(double dgr) const
    {
        return dgr * M_PI / 180.;
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

    handler->dance([this](std::atomic_bool& running, bool kill) {
        if (kill)
        {
            running = false;
            tts::TextToVoiceIf::kill();
            return;
        };

        auto phrases =
            std::to_array<task>({task::songlinefirst, task::songlinesecond,
                                 task::songlinethird, task::songlineforth});
        uint32_t cnt{};
        while (running)
        {
            auto num = (cnt++) % phrases.size();
            handler->speak(phrases[num], false);
        }
    });
    return false;
}

bool Robot::dancestream(bool isshown)
{
    if (isshown)
        return true;

    handler->dance([this](std::atomic_bool& running, bool kill) {
        if (kill)
        {
            running = false;
            shell::BashCommand().run("killall -s KILL ffplay");
            return;
        }

        while (running)
        {
            shell::BashCommand().run(
                "yt-dlp https://youtu.be/DedaEVIbTkY -o - 2>/dev/null | ffplay "
                "-nodisp -autoexit -nostats - &> /dev/null");
        }
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

    handler->doheman([this](std::atomic_bool& running, bool kill) {
        if (kill)
        {
            shell::BashCommand().run("killall -s KILL ffplay");
            return;
        }

        shell::BashCommand().run(
            "yt-dlp https://youtu.be/V8h8snfYidg -o - 2>/dev/null | ffplay "
            "-nodisp -autoexit -nostats - &> /dev/null");
        running = false;
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
