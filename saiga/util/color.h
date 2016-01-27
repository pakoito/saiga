#pragma once


#include "saiga/util/glm.h"


class SAIGA_GLOBAL Color{
public:
    u_int8_t r,g,b,a;

    Color(int r=255, int g=255, int b=255, int a=255);
    Color(u_int8_t r=255, u_int8_t g=255, u_int8_t b=255, u_int8_t a=255);
    Color(float r=255, float g=255, float b=255, float a=255);

    Color(vec3 c);
    Color(vec4 c);

    vec3 toVec3();
    vec4 toVec4();

    //TODO:
    void toSRGB();
};

namespace Colors{
//RGB Color table copied from http://www.rapidtables.com/web/color/RGB_Color.htm
static const Color maroon = Color(128,0,0);
static const Color darkred = Color(139,0,0);
static const Color brown = Color(165,42,42);
static const Color firebrick = Color(178,34,34);
static const Color crimson = Color(220,20,60);
static const Color red = Color(255,0,0);
static const Color tomato = Color(255,99,71);
static const Color coral = Color(255,127,80);
static const Color indianred = Color(205,92,92);
static const Color lightcoral = Color(240,128,128);
static const Color darksalmon = Color(233,150,122);
static const Color salmon = Color(250,128,114);
static const Color lightsalmon = Color(255,160,122);
static const Color orangered = Color(255,69,0);
static const Color darkorange = Color(255,140,0);
static const Color orange = Color(255,165,0);
static const Color gold = Color(255,215,0);
static const Color darkgoldenrod = Color(184,134,11);
static const Color goldenrod = Color(218,165,32);
static const Color palegoldenrod = Color(238,232,170);
static const Color darkkhaki = Color(189,183,107);
static const Color khaki = Color(240,230,140);
static const Color olive = Color(128,128,0);
static const Color yellow = Color(255,255,0);
static const Color yellowgreen = Color(154,205,50);
static const Color darkolivegreen = Color(85,107,47);
static const Color olivedrab = Color(107,142,35);
static const Color lawngreen = Color(124,252,0);
static const Color chartreuse = Color(127,255,0);
static const Color greenyellow = Color(173,255,47);
static const Color dargreen = Color(0,100,0);
static const Color green = Color(0,128,0);
static const Color forestgreen = Color(34,139,34);
static const Color lime = Color(0,255,0);
static const Color limegreen = Color(50,205,50);
static const Color lightgreen = Color(144,238,144);
static const Color palegreen = Color(152,251,152);
static const Color darkseagreen = Color(143,188,143);
static const Color mediumspringgreen = Color(0,250,154);
static const Color springgreen = Color(0,255,127);
static const Color seagreen = Color(46,139,87);
static const Color mediumaquamarine = Color(102,205,170);
static const Color mediumseagreen = Color(60,179,113);
static const Color lightseagreen = Color(32,178,170);
static const Color darkslategray = Color(47,79,79);
static const Color teal = Color(0,128,128);
static const Color darkcyan = Color(0,139,139);
static const Color aqua = Color(0,255,255);
static const Color cyan = Color(0,255,255);
static const Color lightcyan = Color(224,255,255);
static const Color darkturquoise = Color(0,206,209);
static const Color turquoise = Color(64,224,208);
static const Color mediumturquoise = Color(72,209,204);
static const Color paleturquoise = Color(175,238,238);
static const Color aquamarine = Color(127,255,212);
static const Color powderblue = Color(176,224,230);
static const Color cadetblue = Color(95,158,160);
static const Color steelblue = Color(70,130,180);
static const Color cornflowerblue = Color(100,149,237);
static const Color deepskyblue = Color(0,191,255);
static const Color dodgerblue = Color(30,144,255);
static const Color lightblue = Color(173,216,230);
static const Color skyblue = Color(135,206,235);
static const Color lightskyblue = Color(135,206,250);
static const Color midnightblue = Color(25,25,112);
static const Color navy = Color(0,0,128);
static const Color darkblue = Color(0,0,139);
static const Color mediumblue = Color(0,0,205);
static const Color blue = Color(0,0,255);
static const Color royalblue = Color(65,105,225);
static const Color blueviolet = Color(138,43,226);
static const Color indigo = Color(75,0,130);
static const Color darkslateblue = Color(72,61,139);
static const Color slateblue = Color(106,90,205);
static const Color mediumslateblue = Color(123,104,238);
static const Color mediumpurple = Color(147,112,219);
static const Color darkmagenta = Color(139,0,139);
static const Color darkviolet = Color(148,0,211);
static const Color darkorchid = Color(153,50,204);
static const Color mediumorchid = Color(186,85,211);
static const Color purple = Color(128,0,128);
static const Color thistle = Color(216,191,216);
static const Color plum = Color(221,160,221);
static const Color violet = Color(238,130,238);
static const Color magenta = Color(255,0,255);
static const Color orchid = Color(218,112,214);
static const Color mediumvioletred = Color(199,21,133);
static const Color palevioletred = Color(219,112,147);
static const Color deeppink = Color(255,20,147);
static const Color hotpink = Color(255,105,180);
static const Color lightpink = Color(255,182,193);
static const Color pink = Color(255,192,203);
static const Color antiquewhite = Color(250,235,215);
static const Color beige = Color(245,245,220);
static const Color bisque = Color(255,228,196);
static const Color blanchedalmond = Color(255,235,205);
static const Color wheat = Color(245,222,179);
static const Color cornsilk = Color(255,248,220);
static const Color lemonchiffon = Color(255,250,205);
static const Color lightgoldenrodyellow = Color(250,250,210);
static const Color lightyellow = Color(255,255,224);
static const Color saddlebrown = Color(139,69,19);
static const Color sienna = Color(160,82,45);
static const Color chocolate = Color(210,105,30);
static const Color peru = Color(205,133,63);
static const Color sandybrown = Color(244,164,96);
static const Color burlywood = Color(222,184,135);
static const Color tan = Color(210,180,140);
static const Color rosybrown = Color(188,143,143);
static const Color moccasin = Color(255,228,181);
static const Color navajowhite = Color(255,222,173);
static const Color peachpuff = Color(255,218,185);
static const Color mistyrose = Color(255,228,225);
static const Color lavenderblush = Color(255,240,245);
static const Color linen = Color(250,240,230);
static const Color oldlace = Color(253,245,230);
static const Color papayawhip = Color(255,239,213);
static const Color seashell = Color(255,245,238);
static const Color mintcream = Color(245,255,250);
static const Color slategray = Color(112,128,144);
static const Color lightslategray = Color(119,136,153);
static const Color lightsteelblue = Color(176,196,222);
static const Color lavender = Color(230,230,250);
static const Color floralwhite = Color(255,250,240);
static const Color aliceblue = Color(240,248,255);
static const Color ghostwhite = Color(248,248,255);
static const Color honeydew = Color(240,255,240);
static const Color ivory = Color(255,255,240);
static const Color azure = Color(240,255,255);
static const Color snow = Color(255,250,250);
static const Color black = Color(0,0,0);
static const Color dimgray = Color(105,105,105);
static const Color gray = Color(128,128,128);
static const Color darkgray = Color(169,169,169);
static const Color silver = Color(192,192,192);
static const Color lightgray = Color(211,211,211);
static const Color gainsboro = Color(220,220,220);
static const Color whitesmoke = Color(245,245,245);
static const Color white = Color(255,255,255);
}
