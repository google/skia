/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#pragma once

/* ANSI-C code produced by gperf version 3.0.4 */
/* Command-line: gperf --output-file=svg_attribs.h svg_attribs.gperf  */
/* Computed positions: -k'1-2' */

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

#line 10 "svg_attribs.gperf"
struct svg_attrib;
/* maximum key range = 40, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
svg_attrib_hash (register const char *str, register unsigned int len)
{
  static const unsigned char asso_values[] =
    {
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 23,
      13, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41,  3,
      30, 10,  0, 41,  5,  0, 41, 41, 41, 41,
      41, 10, 10, 41,  5,  5,  0, 41, 41, 30,
      15,  0, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41
    };
  register int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[1]];
      /*FALLTHROUGH*/
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
const struct svg_attrib *
svg_attrib_lookup (register const char *str, register unsigned int len)
{
  enum
    {
      TOTAL_KEYWORDS = 25,
      MIN_WORD_LENGTH = 1,
      MAX_WORD_LENGTH = 14,
      MIN_HASH_VALUE = 1,
      MAX_HASH_VALUE = 40
    };

  static const struct svg_attrib wordlist[] =
    {
      {""},
#line 20 "svg_attribs.gperf"
      {"y",               parse_attrib_y},
      {""}, {""},
#line 31 "svg_attribs.gperf"
      {"fill",            parse_attrib_fill_color},
#line 16 "svg_attribs.gperf"
      {"cy",              parse_attrib_cy},
#line 14 "svg_attribs.gperf"
      {"r",               parse_attrib_r},
#line 18 "svg_attribs.gperf"
      {"ry",              parse_attrib_ry},
      {""},
#line 33 "svg_attribs.gperf"
      {"fill-rule",       parse_attrib_fill_rule},
#line 37 "svg_attribs.gperf"
      {"style",           parse_attrib_style},
#line 34 "svg_attribs.gperf"
      {"stroke",          parse_attrib_stroke_color},
#line 32 "svg_attribs.gperf"
      {"fill-opacity",    parse_attrib_fill_opacity},
      {""},
#line 29 "svg_attribs.gperf"
      {"transform",       parse_attrib_transform},
#line 26 "svg_attribs.gperf"
      {"y2",              parse_attrib_y2},
#line 19 "svg_attribs.gperf"
      {"x",               parse_attrib_x},
#line 36 "svg_attribs.gperf"
      {"stroke-width",    parse_attrib_stroke_width},
      {""},
#line 35 "svg_attribs.gperf"
      {"stroke-opacity",  parse_attrib_stroke_opacity},
#line 15 "svg_attribs.gperf"
      {"cx",              parse_attrib_cx},
#line 22 "svg_attribs.gperf"
      {"height",          parse_attrib_height},
#line 17 "svg_attribs.gperf"
      {"rx",              parse_attrib_rx},
      {""}, {""},
#line 24 "svg_attribs.gperf"
      {"y1",              parse_attrib_y1},
#line 28 "svg_attribs.gperf"
      {"points",          parse_attrib_points},
#line 30 "svg_attribs.gperf"
      {"opacity",         parse_attrib_opacity},
      {""}, {""},
#line 25 "svg_attribs.gperf"
      {"x2",              parse_attrib_x2},
#line 27 "svg_attribs.gperf"
      {"d",               parse_attrib_d},
#line 13 "svg_attribs.gperf"
      {"id",              parse_attrib_id},
      {""}, {""},
#line 21 "svg_attribs.gperf"
      {"width",           parse_attrib_width},
      {""}, {""}, {""}, {""},
#line 23 "svg_attribs.gperf"
      {"x1",              parse_attrib_x1}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = svg_attrib_hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register const char *s = wordlist[key].name;

          if (*str == *s && !strcmp (str + 1, s + 1))
            return &wordlist[key];
        }
    }
  return 0;
}
