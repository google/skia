/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#pragma once

/* ANSI-C code produced by gperf version 3.0.4 */
/* Command-line: gperf --output-file=svg_color_names.h svg_color_names.gperf  */
/* Computed positions: -k'1,3,6-8,12-13' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

#line 10 "svg_color_names.gperf"
struct svg_color_name;
/* maximum key range = 562, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
svg_color_name_hash (register const char *str, register unsigned int len)
{
  static const unsigned short asso_values[] =
    {
      566, 566, 566, 566, 566, 566, 566, 566, 566, 566,
      566, 566, 566, 566, 566, 566, 566, 566, 566, 566,
      566, 566, 566, 566, 566, 566, 566, 566, 566, 566,
      566, 566, 566, 566, 566, 566, 566, 566, 566, 566,
      566, 566, 566, 566, 566, 566, 566, 566, 566, 566,
      566, 566, 566, 566, 566, 566, 566, 566, 566, 566,
      566, 566, 566, 566, 566, 566, 566, 566, 566, 566,
      566, 566, 566, 566, 566, 566, 566, 566, 566, 566,
      566, 566, 566, 566, 566, 566, 566, 566, 566, 566,
      566, 566, 566, 566, 566, 566, 566,   5,  55,   0,
       35,   0,  75,  10,   5,   0, 566, 250,  10,  40,
       85,  60,  70, 144,   0,  20,  45,  10,  30, 185,
       95, 195, 566,   0, 566, 566, 566, 566, 566, 566,
      566, 566, 566, 566, 566, 566, 566, 566, 566, 566,
      566, 566, 566, 566, 566, 566, 566, 566, 566, 566,
      566, 566, 566, 566, 566, 566, 566, 566, 566, 566,
      566, 566, 566, 566, 566, 566, 566, 566, 566, 566,
      566, 566, 566, 566, 566, 566, 566, 566, 566, 566,
      566, 566, 566, 566, 566, 566, 566, 566, 566, 566,
      566, 566, 566, 566, 566, 566, 566, 566, 566, 566,
      566, 566, 566, 566, 566, 566, 566, 566, 566, 566,
      566, 566, 566, 566, 566, 566, 566, 566, 566, 566,
      566, 566, 566, 566, 566, 566, 566, 566, 566, 566,
      566, 566, 566, 566, 566, 566, 566, 566, 566, 566,
      566, 566, 566, 566, 566, 566, 566, 566, 566, 566,
      566, 566, 566, 566, 566, 566, 566, 566
    };
  register int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[12]];
      /*FALLTHROUGH*/
      case 12:
        hval += asso_values[(unsigned char)str[11]];
      /*FALLTHROUGH*/
      case 11:
      case 10:
      case 9:
      case 8:
        hval += asso_values[(unsigned char)str[7]];
      /*FALLTHROUGH*/
      case 7:
        hval += asso_values[(unsigned char)str[6]];
      /*FALLTHROUGH*/
      case 6:
        hval += asso_values[(unsigned char)str[5]];
      /*FALLTHROUGH*/
      case 5:
      case 4:
      case 3:
        hval += asso_values[(unsigned char)str[2]+2];
      /*FALLTHROUGH*/
      case 2:
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval;
}

#ifdef __GNUC__
__inline
#if defined __GNUC_STDC_INLINE__ || defined __GNUC_GNU_INLINE__
__attribute__ ((__gnu_inline__))
#endif
#endif
const struct svg_color_name *
svg_color_name_lookup (register const char *str, register unsigned int len)
{
  enum
    {
      TOTAL_KEYWORDS = 147,
      MIN_WORD_LENGTH = 3,
      MAX_WORD_LENGTH = 20,
      MIN_HASH_VALUE = 4,
      MAX_HASH_VALUE = 565
    };

  static const struct svg_color_name wordlist[] =
    {
      {""}, {""}, {""}, {""},
#line 33 "svg_color_names.gperf"
      {"cyan",                         SVG_RGB( 0, 255, 255)},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 66 "svg_color_names.gperf"
      {"gray",                         SVG_RGB(128, 128, 128)},
      {""}, {""}, {""}, {""}, {""},
#line 27 "svg_color_names.gperf"
      {"chartreuse",                   SVG_RGB(127, 255, 0)},
      {""}, {""}, {""},
#line 67 "svg_color_names.gperf"
      {"grey",                         SVG_RGB(128, 128, 128)},
#line 68 "svg_color_names.gperf"
      {"green",                        SVG_RGB( 0, 128, 0)},
      {""}, {""}, {""},
#line 86 "svg_color_names.gperf"
      {"lightgrey",                    SVG_RGB(211, 211, 211)},
#line 85 "svg_color_names.gperf"
      {"lightgreen",                   SVG_RGB(144, 238, 144)},
      {""}, {""}, {""},
#line 84 "svg_color_names.gperf"
      {"lightgray",                    SVG_RGB(211, 211, 211)},
      {""}, {""},
#line 142 "svg_color_names.gperf"
      {"skyblue",                      SVG_RGB(135, 206, 235)},
      {""},
#line 145 "svg_color_names.gperf"
      {"slategrey",                    SVG_RGB(112, 128, 144)},
      {""},
#line 140 "svg_color_names.gperf"
      {"sienna",                       SVG_RGB(160, 82, 45)},
      {""}, {""},
#line 144 "svg_color_names.gperf"
      {"slategray",                    SVG_RGB(112, 128, 144)},
      {""}, {""}, {""},
#line 139 "svg_color_names.gperf"
      {"seashell",                     SVG_RGB(255, 245, 238)},
#line 150 "svg_color_names.gperf"
      {"teal",                         SVG_RGB( 0, 128, 128)},
#line 29 "svg_color_names.gperf"
      {"coral",                        SVG_RGB(255, 127, 80)},
      {""}, {""}, {""}, {""}, {""},
#line 88 "svg_color_names.gperf"
      {"lightsalmon",                  SVG_RGB(255, 160, 122)},
      {""}, {""},
#line 92 "svg_color_names.gperf"
      {"lightslategrey",               SVG_RGB(119, 136, 153)},
#line 20 "svg_color_names.gperf"
      {"black",                        SVG_RGB( 0, 0, 0)},
      {""}, {""}, {""},
#line 91 "svg_color_names.gperf"
      {"lightslategray",               SVG_RGB(119, 136, 153)},
      {""},
#line 118 "svg_color_names.gperf"
      {"orange",                       SVG_RGB(255, 165, 0)},
      {""}, {""},
#line 119 "svg_color_names.gperf"
      {"orangered",                    SVG_RGB(255, 69, 0)},
      {""},
#line 19 "svg_color_names.gperf"
      {"bisque",                       SVG_RGB(255, 228, 196)},
      {""}, {""},
#line 95 "svg_color_names.gperf"
      {"lime",                         SVG_RGB( 0, 255, 0)},
      {""}, {""}, {""},
#line 132 "svg_color_names.gperf"
      {"red",                          SVG_RGB(255, 0, 0)},
#line 96 "svg_color_names.gperf"
      {"limegreen",                    SVG_RGB( 50, 205, 50)},
#line 81 "svg_color_names.gperf"
      {"lightcoral",                   SVG_RGB(240, 128, 128)},
      {""}, {""}, {""},
#line 134 "svg_color_names.gperf"
      {"royalblue",                    SVG_RGB( 65, 105, 225)},
#line 97 "svg_color_names.gperf"
      {"linen",                        SVG_RGB(250, 240, 230)},
      {""},
#line 61 "svg_color_names.gperf"
      {"fuchsia",                      SVG_RGB(255, 0, 255)},
      {""},
#line 38 "svg_color_names.gperf"
      {"darkgreen",                    SVG_RGB( 0, 100, 0)},
      {""}, {""}, {""}, {""},
#line 80 "svg_color_names.gperf"
      {"lightblue",                    SVG_RGB(173, 216, 230)},
#line 44 "svg_color_names.gperf"
      {"darkorchid",                   SVG_RGB(153, 50, 204)},
#line 147 "svg_color_names.gperf"
      {"springgreen",                  SVG_RGB( 0, 255, 127)},
#line 98 "svg_color_names.gperf"
      {"magenta",                      SVG_RGB(255, 0, 255)},
      {""},
#line 64 "svg_color_names.gperf"
      {"gold",                         SVG_RGB(255, 215, 0)},
      {""},
#line 120 "svg_color_names.gperf"
      {"orchid",                       SVG_RGB(218, 112, 214)},
      {""}, {""},
#line 143 "svg_color_names.gperf"
      {"slateblue",                    SVG_RGB(106, 90, 205)},
      {""},
#line 41 "svg_color_names.gperf"
      {"darkmagenta",                  SVG_RGB(139, 0, 139)},
      {""},
#line 34 "svg_color_names.gperf"
      {"darkblue",                     SVG_RGB( 0, 0, 139)},
#line 93 "svg_color_names.gperf"
      {"lightsteelblue",               SVG_RGB(176, 196, 222)},
      {""},
#line 141 "svg_color_names.gperf"
      {"silver",                       SVG_RGB(192, 192, 192)},
      {""},
#line 138 "svg_color_names.gperf"
      {"seagreen",                     SVG_RGB( 46, 139, 87)},
#line 148 "svg_color_names.gperf"
      {"steelblue",                    SVG_RGB( 70, 130, 180)},
      {""}, {""}, {""},
#line 149 "svg_color_names.gperf"
      {"tan",                          SVG_RGB(210, 180, 140)},
#line 127 "svg_color_names.gperf"
      {"peru",                         SVG_RGB(205, 133, 63)},
      {""},
#line 131 "svg_color_names.gperf"
      {"purple",                       SVG_RGB(128, 0, 128)},
#line 45 "svg_color_names.gperf"
      {"darkred",                      SVG_RGB(139, 0, 0)},
      {""},
#line 110 "svg_color_names.gperf"
      {"mintcream",                    SVG_RGB(245, 255, 250)},
      {""}, {""}, {""}, {""},
#line 58 "svg_color_names.gperf"
      {"firebrick",                    SVG_RGB(178, 34, 34)},
      {""}, {""}, {""},
#line 89 "svg_color_names.gperf"
      {"lightseagreen",                SVG_RGB( 32, 178, 170)},
#line 42 "svg_color_names.gperf"
      {"darkolivegreen",               SVG_RGB( 85, 107, 47)},
      {""}, {""}, {""}, {""},
#line 111 "svg_color_names.gperf"
      {"mistyrose",                    SVG_RGB(255, 228, 225)},
      {""},
#line 73 "svg_color_names.gperf"
      {"indigo",                       SVG_RGB( 75, 0, 130)},
#line 115 "svg_color_names.gperf"
      {"oldlace",                      SVG_RGB(253, 245, 230)},
      {""},
#line 128 "svg_color_names.gperf"
      {"pink",                         SVG_RGB(255, 192, 203)},
#line 46 "svg_color_names.gperf"
      {"darksalmon",                   SVG_RGB(233, 150, 122)},
      {""}, {""},
#line 76 "svg_color_names.gperf"
      {"lavender",                     SVG_RGB(230, 230, 250)},
#line 74 "svg_color_names.gperf"
      {"ivory",                        SVG_RGB(255, 255, 240)},
      {""}, {""}, {""},
#line 112 "svg_color_names.gperf"
      {"moccasin",                     SVG_RGB(255, 228, 181)},
      {""}, {""}, {""}, {""}, {""},
#line 26 "svg_color_names.gperf"
      {"cadetblue",                    SVG_RGB( 95, 158, 160)},
#line 52 "svg_color_names.gperf"
      {"darkviolet",                   SVG_RGB(148, 0, 211)},
#line 135 "svg_color_names.gperf"
      {"saddlebrown",                  SVG_RGB(139, 69, 19)},
      {""},
#line 48 "svg_color_names.gperf"
      {"darkslateblue",                SVG_RGB( 72, 61, 139)},
#line 122 "svg_color_names.gperf"
      {"palegreen",                    SVG_RGB(152, 251, 152)},
      {""}, {""}, {""},
#line 146 "svg_color_names.gperf"
      {"snow",                         SVG_RGB(255, 250, 250)},
#line 72 "svg_color_names.gperf"
      {"indianred",                    SVG_RGB(205, 92, 92)},
#line 83 "svg_color_names.gperf"
      {"lightgoldenrodyellow",         SVG_RGB(250, 250, 210)},
#line 152 "svg_color_names.gperf"
      {"tomato",                       SVG_RGB(255, 99, 71)},
#line 79 "svg_color_names.gperf"
      {"lemonchiffon",                 SVG_RGB(255, 250, 205)},
      {""},
#line 87 "svg_color_names.gperf"
      {"lightpink",                    SVG_RGB(255, 182, 193)},
      {""},
#line 99 "svg_color_names.gperf"
      {"maroon",                       SVG_RGB(128, 0, 0)},
      {""},
#line 77 "svg_color_names.gperf"
      {"lavenderblush",                SVG_RGB(255, 240, 245)},
#line 153 "svg_color_names.gperf"
      {"turquoise",                    SVG_RGB( 64, 224, 208)},
#line 43 "svg_color_names.gperf"
      {"darkorange",                   SVG_RGB(255, 140, 0)},
      {""}, {""}, {""},
#line 114 "svg_color_names.gperf"
      {"navy",                         SVG_RGB( 0, 0, 128)},
#line 57 "svg_color_names.gperf"
      {"dodgerblue",                   SVG_RGB( 30, 144, 255)},
#line 60 "svg_color_names.gperf"
      {"forestgreen",                  SVG_RGB( 34, 139, 34)},
#line 109 "svg_color_names.gperf"
      {"midnightblue",                 SVG_RGB( 25, 25, 112)},
      {""},
#line 104 "svg_color_names.gperf"
      {"mediumseagreen",               SVG_RGB( 60, 179, 113)},
      {""}, {""},
#line 47 "svg_color_names.gperf"
      {"darkseagreen",                 SVG_RGB(143, 188, 143)},
      {""},
#line 15 "svg_color_names.gperf"
      {"aqua",                         SVG_RGB( 0, 255, 255)},
#line 17 "svg_color_names.gperf"
      {"azure",                        SVG_RGB(240, 255, 255)},
#line 136 "svg_color_names.gperf"
      {"salmon",                       SVG_RGB(250, 128, 114)},
      {""}, {""}, {""},
#line 155 "svg_color_names.gperf"
      {"wheat",                        SVG_RGB(245, 222, 179)},
      {""}, {""}, {""},
#line 24 "svg_color_names.gperf"
      {"brown",                        SVG_RGB(165, 42, 42)},
#line 16 "svg_color_names.gperf"
      {"aquamarine",                   SVG_RGB(127, 255, 212)},
      {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 28 "svg_color_names.gperf"
      {"chocolate",                    SVG_RGB(210, 105, 30)},
#line 78 "svg_color_names.gperf"
      {"lawngreen",                    SVG_RGB(124, 252, 0)},
#line 137 "svg_color_names.gperf"
      {"sandybrown",                   SVG_RGB(244, 164, 96)},
      {""}, {""}, {""},
#line 82 "svg_color_names.gperf"
      {"lightcyan",                    SVG_RGB(224, 255, 255)},
      {""}, {""}, {""}, {""}, {""},
#line 154 "svg_color_names.gperf"
      {"violet",                       SVG_RGB(238, 130, 238)},
#line 94 "svg_color_names.gperf"
      {"lightyellow",                  SVG_RGB(255, 255, 224)},
      {""}, {""}, {""},
#line 101 "svg_color_names.gperf"
      {"mediumblue",                   SVG_RGB( 0, 0, 205)},
      {""}, {""}, {""},
#line 126 "svg_color_names.gperf"
      {"peachpuff",                    SVG_RGB(255, 218, 185)},
      {""},
#line 69 "svg_color_names.gperf"
      {"greenyellow",                  SVG_RGB(173, 255, 47)},
      {""}, {""}, {""}, {""}, {""},
#line 14 "svg_color_names.gperf"
      {"antiquewhite",                 SVG_RGB(250, 235, 215)},
      {""},
#line 22 "svg_color_names.gperf"
      {"blue",                         SVG_RGB( 0, 0, 255)},
#line 108 "svg_color_names.gperf"
      {"mediumvioletred",              SVG_RGB(199, 21, 133)},
      {""},
#line 103 "svg_color_names.gperf"
      {"mediumpurple",                 SVG_RGB(147, 112, 219)},
      {""},
#line 65 "svg_color_names.gperf"
      {"goldenrod",                    SVG_RGB(218, 165, 32)},
      {""}, {""}, {""}, {""},
#line 21 "svg_color_names.gperf"
      {"blanchedalmond",               SVG_RGB(255, 235, 205)},
#line 75 "svg_color_names.gperf"
      {"khaki",                        SVG_RGB(240, 230, 140)},
      {""}, {""}, {""},
#line 129 "svg_color_names.gperf"
      {"plum",                         SVG_RGB(221, 160, 221)},
      {""}, {""},
#line 102 "svg_color_names.gperf"
      {"mediumorchid",                 SVG_RGB(186, 85, 211)},
      {""},
#line 133 "svg_color_names.gperf"
      {"rosybrown",                    SVG_RGB(188, 143, 143)},
#line 105 "svg_color_names.gperf"
      {"mediumslateblue",              SVG_RGB(123, 104, 238)},
      {""},
#line 51 "svg_color_names.gperf"
      {"darkturquoise",                SVG_RGB( 0, 206, 209)},
      {""}, {""}, {""}, {""}, {""},
#line 124 "svg_color_names.gperf"
      {"palevioletred",                SVG_RGB(219, 112, 147)},
      {""},
#line 125 "svg_color_names.gperf"
      {"papayawhip",                   SVG_RGB(255, 239, 213)},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 106 "svg_color_names.gperf"
      {"mediumspringgreen",            SVG_RGB( 0, 250, 154)},
#line 39 "svg_color_names.gperf"
      {"darkgrey",                     SVG_RGB(169, 169, 169)},
      {""},
#line 107 "svg_color_names.gperf"
      {"mediumturquoise",              SVG_RGB( 72, 209, 204)},
      {""}, {""},
#line 37 "svg_color_names.gperf"
      {"darkgray",                     SVG_RGB(169, 169, 169)},
      {""}, {""}, {""}, {""},
#line 36 "svg_color_names.gperf"
      {"darkgoldenrod",                SVG_RGB(184, 134, 11)},
      {""}, {""}, {""},
#line 56 "svg_color_names.gperf"
      {"dimgrey",                      SVG_RGB(105, 105, 105)},
      {""}, {""}, {""}, {""},
#line 55 "svg_color_names.gperf"
      {"dimgray",                      SVG_RGB(105, 105, 105)},
#line 70 "svg_color_names.gperf"
      {"honeydew",                     SVG_RGB(240, 255, 240)},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 18 "svg_color_names.gperf"
      {"beige",                        SVG_RGB(245, 245, 220)},
      {""},
#line 151 "svg_color_names.gperf"
      {"thistle",                      SVG_RGB(216, 191, 216)},
#line 31 "svg_color_names.gperf"
      {"cornsilk",                     SVG_RGB(255, 248, 220)},
      {""},
#line 116 "svg_color_names.gperf"
      {"olive",                        SVG_RGB(128, 128, 0)},
      {""}, {""}, {""}, {""},
#line 23 "svg_color_names.gperf"
      {"blueviolet",                   SVG_RGB(138, 43, 226)},
      {""}, {""}, {""}, {""},
#line 100 "svg_color_names.gperf"
      {"mediumaquamarine",             SVG_RGB(102, 205, 170)},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 30 "svg_color_names.gperf"
      {"cornflowerblue",               SVG_RGB(100, 149, 237)},
      {""}, {""}, {""}, {""},
#line 13 "svg_color_names.gperf"
      {"aliceblue",                    SVG_RGB(240, 248, 255)},
#line 130 "svg_color_names.gperf"
      {"powderblue",                   SVG_RGB(176, 224, 230)},
      {""},
#line 123 "svg_color_names.gperf"
      {"paleturquoise",                SVG_RGB(175, 238, 238)},
      {""}, {""}, {""}, {""}, {""},
#line 50 "svg_color_names.gperf"
      {"darkslategrey",                SVG_RGB( 47, 79, 79)},
#line 40 "svg_color_names.gperf"
      {"darkkhaki",                    SVG_RGB(189, 183, 107)},
      {""}, {""}, {""},
#line 49 "svg_color_names.gperf"
      {"darkslategray",                SVG_RGB( 47, 79, 79)},
#line 63 "svg_color_names.gperf"
      {"ghostwhite",                   SVG_RGB(248, 248, 255)},
      {""}, {""}, {""}, {""},
#line 117 "svg_color_names.gperf"
      {"olivedrab",                    SVG_RGB(107, 142, 35)},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 121 "svg_color_names.gperf"
      {"palegoldenrod",                SVG_RGB(238, 232, 170)},
      {""}, {""}, {""}, {""},
#line 35 "svg_color_names.gperf"
      {"darkcyan",                     SVG_RGB( 0, 139, 139)},
      {""}, {""}, {""},
#line 71 "svg_color_names.gperf"
      {"hotpink",                      SVG_RGB(255, 105, 180)},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 62 "svg_color_names.gperf"
      {"gainsboro",                    SVG_RGB(220, 220, 220)},
      {""}, {""}, {""},
#line 53 "svg_color_names.gperf"
      {"deeppink",                     SVG_RGB(255, 20, 147)},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""},
#line 32 "svg_color_names.gperf"
      {"crimson",                      SVG_RGB(220, 20, 60)},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""},
#line 25 "svg_color_names.gperf"
      {"burlywood",                    SVG_RGB(222, 184, 135)},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 59 "svg_color_names.gperf"
      {"floralwhite",                  SVG_RGB(255, 250, 240)},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 156 "svg_color_names.gperf"
      {"white",                        SVG_RGB(255, 255, 255)},
#line 113 "svg_color_names.gperf"
      {"navajowhite",                  SVG_RGB(255, 222, 173)},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""},
#line 158 "svg_color_names.gperf"
      {"yellow",                       SVG_RGB(255, 255, 0)},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""},
#line 159 "svg_color_names.gperf"
      {"yellowgreen",                  SVG_RGB(154, 205, 50)},
#line 90 "svg_color_names.gperf"
      {"lightskyblue",                 SVG_RGB(135, 206, 250)},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""},
#line 54 "svg_color_names.gperf"
      {"deepskyblue",                  SVG_RGB( 0, 191, 255)},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 157 "svg_color_names.gperf"
      {"whitesmoke",                   SVG_RGB(245, 245, 245)}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = svg_color_name_hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register const char *s = wordlist[key].name;

          if (*str == *s && !strcmp (str + 1, s + 1))
            return &wordlist[key];
        }
    }
  return 0;
}
