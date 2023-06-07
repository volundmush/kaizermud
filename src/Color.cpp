#include "kaizermud/Color.h"

/* Colors that the player can define */
#define COLOR_NORMAL            0
#define COLOR_ROOMNAME            1
#define COLOR_ROOMOBJS            2
#define COLOR_ROOMPEOPLE        3
#define COLOR_HITYOU            4
#define COLOR_YOUHIT            5
#define COLOR_OTHERHIT            6
#define COLOR_CRITICAL            7
#define COLOR_HOLLER            8
#define COLOR_SHOUT            9
#define COLOR_GOSSIP            10
#define COLOR_AUCTION            11
#define COLOR_CONGRAT            12
#define COLOR_TELL            13
#define COLOR_YOUSAY            14
#define COLOR_ROOMSAY            15

#define NUM_COLOR            16

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define KNUL  ""

/* conditional color.  pass it a pointer to a char_data and a color level. */
#define C_OFF    0
#define C_ON    1
#define C_BRI    C_ON    /* Compatibility hack */
#define C_NRM    C_ON    /* Compatibility hack */
#define C_CMP    C_ON    /* Compatibility hack */

#define COLOR_CHOICES(ch)    (IS_NPC(ch) ? nullptr : (ch)->player_specials ? (ch)->player_specials->color_choices : nullptr)
#define _clrlevel(ch) (!IS_NPC(ch) ? (PRF_FLAGGED((ch), PRF_COLOR) ? 1 : 0) : 0)
#define clr(ch, lvl) (_clrlevel(ch) >= (lvl))
#define CCNRM(ch, lvl)  (clr((ch),(lvl))?KNRM:KNUL)
#define CCRED(ch, lvl)  (clr((ch),(lvl))?KRED:KNUL)
#define CCGRN(ch, lvl)  (clr((ch),(lvl))?KGRN:KNUL)
#define CCYEL(ch, lvl)  (clr((ch),(lvl))?KYEL:KNUL)
#define CCBLU(ch, lvl)  (clr((ch),(lvl))?KBLU:KNUL)
#define CCMAG(ch, lvl)  (clr((ch),(lvl))?KMAG:KNUL)
#define CCCYN(ch, lvl)  (clr((ch),(lvl))?KCYN:KNUL)
#define CCWHT(ch, lvl)  (clr((ch),(lvl))?KWHT:KNUL)

#define COLOR_LEV(ch) (_clrlevel(ch))

#define QNRM CCNRM(ch,C_BRI)
#define QRED CCRED(ch,C_BRI)
#define QGRN CCGRN(ch,C_BRI)
#define QYEL CCYEL(ch,C_BRI)
#define QBLU CCBLU(ch,C_BRI)
#define QMAG CCMAG(ch,C_BRI)
#define QCYN CCCYN(ch,C_BRI)
#define QWHT CCWHT(ch,C_BRI)

#define ANSISTART "\x1B["
#define ANSISEP ';'
#define ANSISEPSTR ";"
#define ANSIEND 'm'
#define ANSIENDSTR "m"

/* Attributes */
#define AA_NORMAL       "0"
#define AA_BOLD         "1"
#define AA_UNDERLINE    "4"
#define AA_BLINK        "5"
#define AA_REVERSE      "7"
#define AA_INVIS        "8"
/* Foreground colors */
#define AF_BLACK        "30"
#define AF_RED          "31"
#define AF_GREEN        "32"
#define AF_YELLOW       "33"
#define AF_BLUE         "34"
#define AF_MAGENTA      "35"
#define AF_CYAN         "36"
#define AF_WHITE        "37"
/* Background colors */
#define AB_BLACK        "40"
#define AB_RED          "41"
#define AB_GREEN        "42"
#define AB_YELLOW       "43"
#define AB_BLUE         "44"
#define AB_MAGENTA      "45"
#define AB_CYAN         "46"
#define AB_WHITE        "47"

static char *ANSI[] = {
        "@",
        AA_NORMAL,
        AA_NORMAL ANSISEPSTR AF_BLACK,
        AA_NORMAL ANSISEPSTR AF_BLUE,
        AA_NORMAL ANSISEPSTR AF_GREEN,
        AA_NORMAL ANSISEPSTR AF_CYAN,
        AA_NORMAL ANSISEPSTR AF_RED,
        AA_NORMAL ANSISEPSTR AF_MAGENTA,
        AA_NORMAL ANSISEPSTR AF_YELLOW,
        AA_NORMAL ANSISEPSTR AF_WHITE,
        AA_BOLD ANSISEPSTR AF_BLACK,
        AA_BOLD ANSISEPSTR AF_BLUE,
        AA_BOLD ANSISEPSTR AF_GREEN,
        AA_BOLD ANSISEPSTR AF_CYAN,
        AA_BOLD ANSISEPSTR AF_RED,
        AA_BOLD ANSISEPSTR AF_MAGENTA,
        AA_BOLD ANSISEPSTR AF_YELLOW,
        AA_BOLD ANSISEPSTR AF_WHITE,
        AB_BLACK,
        AB_BLUE,
        AB_GREEN,
        AB_CYAN,
        AB_RED,
        AB_MAGENTA,
        AB_YELLOW,
        AB_WHITE,
        AA_BLINK,
        AA_UNDERLINE,
        AA_BOLD,
        AA_REVERSE,
        "!"
};

static char *default_color_choices[NUM_COLOR + 1] = {
/* COLOR_NORMAL */    AA_NORMAL,
/* COLOR_ROOMNAME */    AA_NORMAL ANSISEPSTR AF_CYAN,
/* COLOR_ROOMOBJS */    AA_NORMAL ANSISEPSTR AF_GREEN,
/* COLOR_ROOMPEOPLE */    AA_NORMAL ANSISEPSTR AF_YELLOW,
/* COLOR_HITYOU */    AA_NORMAL ANSISEPSTR AF_RED,
/* COLOR_YOUHIT */    AA_NORMAL ANSISEPSTR AF_GREEN,
/* COLOR_OTHERHIT */    AA_NORMAL ANSISEPSTR AF_YELLOW,
/* COLOR_CRITICAL */    AA_BOLD ANSISEPSTR AF_YELLOW,
/* COLOR_HOLLER */    AA_BOLD ANSISEPSTR AF_YELLOW,
/* COLOR_SHOUT */    AA_BOLD ANSISEPSTR AF_YELLOW,
/* COLOR_GOSSIP */    AA_NORMAL ANSISEPSTR AF_YELLOW,
/* COLOR_AUCTION */    AA_NORMAL ANSISEPSTR AF_CYAN,
/* COLOR_CONGRAT */    AA_NORMAL ANSISEPSTR AF_GREEN,
/* COLOR_TELL */    AA_NORMAL ANSISEPSTR AF_RED,
/* COLOR_YOUSAY */    AA_NORMAL ANSISEPSTR AF_CYAN,
/* COLOR_ROOMSAY */    AA_NORMAL ANSISEPSTR AF_WHITE,
                      nullptr
};

static char CCODE[] = "@ndbgcrmywDBGCRMYW01234567luoex!";
/*
  Codes are:      @n - normal
  @d - black      @D - gray           @0 - background black
  @b - blue       @B - bright blue    @1 - background blue
  @g - green      @G - bright green   @2 - background green
  @c - cyan       @C - bright cyan    @3 - background cyan
  @r - red        @R - bright red     @4 - background red
  @m - magneta    @M - bright magneta @5 - background magneta
  @y - yellow     @Y - bright yellow  @6 - background yellow
  @w - white      @W - bright white   @7 - background white
  @x - random
Extra codes:      @l - blink          @o - bold
  @u - underline  @e - reverse video  @@ - single @

  @[num] - user color choice num, [] are required
*/
static char RANDOM_COLORS[] = "bgcrmywBGCRMWY";

namespace kaizer {

    std::string renderAnsiColor(std::string_view input, ColorType color) {
        char *dest_char, *source_char, *color_char, *save_pos, *replacement = nullptr;
        int i, temp_color;
        size_t wanted;
        char **choices = nullptr;
        std::string txt(input);

        if (!txt.size() || !strchr(txt.c_str(), '@')) /* skip out if no color codes     */
            return txt;


        std::string out;
        source_char = (char *) txt.c_str();

        save_pos = dest_char;
        for (; *source_char;) {
            /* no color code - just copy */
            if (*source_char != '@') {
                out.push_back(*source_char++);
                continue;
            }

            /* if we get here we have a color code */

            source_char++; /* source_char now points to the code */

            /* look for a random color code picks a random number between 1 and 14 */
            if (*source_char == 'x') {
                temp_color = (rand() % 14);
                *source_char = RANDOM_COLORS[temp_color];
            }

            if (*source_char == '\0') { /* string was terminated with color code - just put it in */
                out.push_back('@');
                /* source_char will now point to '\0' in the for() check */
                continue;
            }

            if (color == ColorType::NoColor) { /* not parsing, just skip the code, unless it's @@ */
                if (*source_char == '@') {
                    out.push_back('@');
                }
                if (*source_char == '[') { /* Multi-character code */
                    source_char++;
                    while (*source_char && isdigit(*source_char))
                        source_char++;
                    if (!*source_char)
                        source_char--;
                }
                source_char++; /* skip to next (non-colorcode) char */
                continue;
            }

            /* parse the color code */
            if (*source_char == '[') { /* User configurable color */
                source_char++;
                if (*source_char) {
                    i = atoi(source_char);
                    if (i < 0 || i >= NUM_COLOR)
                        i = COLOR_NORMAL;
                    replacement = default_color_choices[i];
                    if (choices && choices[i])
                        replacement = choices[i];
                    while (*source_char && isdigit(*source_char))
                        source_char++;
                    if (!*source_char)
                        source_char--;
                }
            } else if (*source_char == 'n') {
                replacement = default_color_choices[COLOR_NORMAL];
                if (choices && choices[COLOR_NORMAL])
                    replacement = choices[COLOR_NORMAL];
            } else {
                for (i = 0; CCODE[i] != '!'; i++) { /* do we find it ? */
                    if ((*source_char) == CCODE[i]) {           /* if so :*/
                        replacement = ANSI[i];
                        break;
                    }
                }
            }
            if (replacement) {
                if (isdigit(replacement[0]))
                    for (color_char = ANSISTART; *color_char;)
                        out.push_back(*color_char++);
                for (color_char = replacement; *color_char;)
                    out.push_back(*color_char++);
                if (isdigit(replacement[0]))
                    out.push_back(ANSIEND);
                replacement = nullptr;
            }
            /* If we couldn't find any correct color code, or we found it and
             * substituted above, let's just process the next character.
             * - Welcor
             */
            source_char++;

        } /* for loop */

        return out;
    }

    std::function<std::string(std::string_view, ColorType)> renderAnsi(renderAnsiColor);

    std::string stripAnsi(std::string_view input) {
        return renderAnsi(input, ColorType::NoColor);
    }

}