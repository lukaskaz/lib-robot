#include "robot/ttstexts.hpp"

#include <stdexcept>
#include <unordered_map>

namespace robot
{

using langmap = const std::unordered_map<tts::language, std::string>;
const std::unordered_map<task, langmap> ttstextmap = {
    {task::initiatating,
     {
         {tts::language::polish, "rozpoczynam inicjalizację"},
         {tts::language::english, "initializing"},
         {tts::language::german, "initiieren"},
     }},
    {task::ready,
     {
         {tts::language::polish, "gotowy do działania"},
         {tts::language::english, "ready for action"},
         {tts::language::german, "fertig zum laufen"},
     }},
    {task::parked,
     {
         {tts::language::polish, "robot odstawiony"},
         {tts::language::english, "robot parked"},
         {tts::language::german, "gerät geparkt"},
     }},
    {task::greetstart,
     {
         {tts::language::polish, "no podaj łapę no!"},
         {tts::language::english, "c'mon, give me you hand"},
         {tts::language::german, "gib mir deine hand"},
     }},
    {task::greetshake,
     {
         {tts::language::polish, "czeeeeść czołem kluski z rosołem"},
         {tts::language::english, "cheerio dude, how's your mood?"},
         {tts::language::german, "tschüß alter, wie ist deine stimmung?"},
     }},
    {task::greetend,
     {
         {tts::language::polish, "dobra wystarczy bo się zagłaskamy"},
         {tts::language::english,
          "thats enough, otherwise we'll be making out"},
         {tts::language::german, "okay das reicht, sonst küssen wir uns"},
     }},
    {task::greetfail,
     {
         {tts::language::polish,
          "nie chcesz? a to spierdalaj! ale to nie chlew, może się jednak "
          "przywitasz?"},
         {tts::language::english,
          "don't want to? then piss off! but this isn't a shithouse, "
          "maybe you'll say hello anyway?"},
         {tts::language::german,
          "du willst nicht? dann verpiss dich! aber das hier ist kein "
          "scheißhaus, also sagst du vielleicht trotzdem tschüß?"},
     }},
    {task::dancestart,
     {
         {tts::language::polish, "zapraszasz do tańca? :)"},
         {tts::language::english, "inviting me to dance? :)"},
         {tts::language::german, "verlangst du von mir zu tanzen? :)"},
     }},
    {task::songlinefirst,
     {
         {tts::language::polish, "przez twe oczy, te oczy zielone oszalałam!"},
         {tts::language::english,
          "because of your eyes, those green eyes, I went crazy!"},
         {tts::language::german, "wegen deiner augen, dieser grünen "
                                 "augen, bin ich verrückt geworden!"},
     }},
    {task::songlinesecond,
     {
         {tts::language::polish, "lalala!"},
         {tts::language::english, "lalala!"},
         {tts::language::german, "lalala!"},
     }},
    {task::songlinethird,
     {
         {tts::language::polish, "gwiazdy chyba twym oczom oddały cały blask!"},
         {tts::language::english,
          "the stars must have given up all their shine to your eyes!"},
         {tts::language::german,
          "die sterne müssen ihren glanz an deine augen verloren haben!"},
     }},
    {task::songlineforth,
     {
         {tts::language::polish, "lalalala!"},
         {tts::language::english, "lalalala!"},
         {tts::language::german, "lalalala!"},
     }},
    {task::danceend,
     {
         {tts::language::polish, "co to? masz już dość? HEHE HEHE!"},
         {tts::language::english, "how come? you have enough? HEHE HEHE!"},
         {tts::language::german, "wie kommt das? hast du genug? HEHE HEHE!"},
     }},
    {task::enlightstart,
     {
         {tts::language::polish, "oświecić cię?"},
         {tts::language::english, "enlighten you?"},
         {tts::language::german, "dich aufklären?"},
     }},
    {task::enlightend,
     {
         {tts::language::polish, "oświecenie zakończone"},
         {tts::language::english, "enlightenment done"},
         {tts::language::german, "aufklärung abgeschlossen"},

     }},
    {task::enlightbreak,
     {
         {tts::language::polish, "klepnij enter żeby zakończyć"},
         {tts::language::english, "press enter to finish"},
         {tts::language::german, "zum beenden die eingabetaste drücken"},
     }},
    {task::voicechangestart,
     {
         {tts::language::polish, "zmieniam głos"},
         {tts::language::english, "changing my voice"},
         {tts::language::german, "ich ändere meine stimme"},
     }},
    {task::voicechangeend,
     {
         {tts::language::polish, "teraz będę gadał tak"},
         {tts::language::english, "now I will talk this way"},
         {tts::language::german, "jetzt werde ich so sprechen"},
     }},
    {task::langchangestart,
     {
         {tts::language::polish, "zmieniam język"},
         {tts::language::english, "changing my language"},
         {tts::language::german, "ich ändere meine sprache"},
     }},
    {task::langchangeend,
     {
         {tts::language::polish, "teraz będę gadał po polsku"},
         {tts::language::english, "now I will talk in english"},
         {tts::language::german, "jetzt werde ich auf deutsch sprechen"},
     }},
    {task::nothingtodo,
     {
         {tts::language::polish, "nic nie trzeba robić"},
         {tts::language::english, "nothing to be done"},
         {tts::language::german, "es gibt nichts zu tun"},
     }}};

std::string getttstext(task what, tts::language inlang)
{
    if (ttstextmap.contains(what))
    {
        const auto& langmap = ttstextmap.at(what);
        if (langmap.contains(inlang))
        {
            return langmap.at(inlang);
        }
        throw std::runtime_error("Given language for TTS text not available");
    }
    throw std::runtime_error("Given task for TTS text not available");
}

} // namespace robot
