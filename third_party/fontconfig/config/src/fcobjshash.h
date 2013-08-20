/* ANSI-C code produced by gperf version 3.0.3 */
/* Command-line: gperf -m 100 fcobjshash.gperf  */
/* Computed positions: -k'2-3' */

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

#line 1 "fcobjshash.gperf"

#line 13 "fcobjshash.gperf"
struct FcObjectTypeInfo {
int name;
int id;
};
#include <string.h>
/* maximum key range = 52, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
FcObjectTypeHash (register const char *str, register unsigned int len)
{
  static const unsigned char asso_values[] =
    {
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 17, 12,  1,
      35,  0, 56, 27, 15,  0, 56, 56,  0,  7,
       7,  0, 22, 56, 21, 10, 13,  0, 56, 56,
       0, 26,  1, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56
    };
  return len + asso_values[(unsigned char)str[2]] + asso_values[(unsigned char)str[1]];
}

struct FcObjectTypeNamePool_t
  {
    char FcObjectTypeNamePool_str4[sizeof("file")];
    char FcObjectTypeNamePool_str5[sizeof("size")];
    char FcObjectTypeNamePool_str6[sizeof("weight")];
    char FcObjectTypeNamePool_str7[sizeof("foundry")];
    char FcObjectTypeNamePool_str8[sizeof("fullname")];
    char FcObjectTypeNamePool_str9[sizeof("pixelsize")];
    char FcObjectTypeNamePool_str11[sizeof("decorative")];
    char FcObjectTypeNamePool_str12[sizeof("fullnamelang")];
    char FcObjectTypeNamePool_str13[sizeof("globaladvance")];
    char FcObjectTypeNamePool_str14[sizeof("hinting")];
    char FcObjectTypeNamePool_str15[sizeof("minspace")];
    char FcObjectTypeNamePool_str16[sizeof("hintstyle")];
    char FcObjectTypeNamePool_str17[sizeof("fontformat")];
    char FcObjectTypeNamePool_str18[sizeof("fontversion")];
    char FcObjectTypeNamePool_str19[sizeof("fontfeatures")];
    char FcObjectTypeNamePool_str20[sizeof("outline")];
    char FcObjectTypeNamePool_str21[sizeof("autohint")];
    char FcObjectTypeNamePool_str22[sizeof("slant")];
    char FcObjectTypeNamePool_str23[sizeof("scale")];
    char FcObjectTypeNamePool_str24[sizeof("postscriptname")];
    char FcObjectTypeNamePool_str25[sizeof("dpi")];
    char FcObjectTypeNamePool_str26[sizeof("scalable")];
    char FcObjectTypeNamePool_str27[sizeof("embolden")];
    char FcObjectTypeNamePool_str28[sizeof("lang")];
    char FcObjectTypeNamePool_str29[sizeof("antialias")];
    char FcObjectTypeNamePool_str30[sizeof("family")];
    char FcObjectTypeNamePool_str31[sizeof("hash")];
    char FcObjectTypeNamePool_str32[sizeof("namelang")];
    char FcObjectTypeNamePool_str33[sizeof("embeddedbitmap")];
    char FcObjectTypeNamePool_str34[sizeof("familylang")];
    char FcObjectTypeNamePool_str35[sizeof("verticallayout")];
    char FcObjectTypeNamePool_str36[sizeof("matrix")];
    char FcObjectTypeNamePool_str37[sizeof("rasterizer")];
    char FcObjectTypeNamePool_str38[sizeof("aspect")];
    char FcObjectTypeNamePool_str39[sizeof("charset")];
    char FcObjectTypeNamePool_str40[sizeof("width")];
    char FcObjectTypeNamePool_str41[sizeof("charwidth")];
    char FcObjectTypeNamePool_str42[sizeof("charheight")];
    char FcObjectTypeNamePool_str43[sizeof("rgba")];
    char FcObjectTypeNamePool_str44[sizeof("style")];
    char FcObjectTypeNamePool_str45[sizeof("lcdfilter")];
    char FcObjectTypeNamePool_str46[sizeof("spacing")];
    char FcObjectTypeNamePool_str47[sizeof("index")];
    char FcObjectTypeNamePool_str48[sizeof("stylelang")];
    char FcObjectTypeNamePool_str49[sizeof("capability")];
    char FcObjectTypeNamePool_str55[sizeof("prgname")];
  };
static const struct FcObjectTypeNamePool_t FcObjectTypeNamePool_contents =
  {
    "file",
    "size",
    "weight",
    "foundry",
    "fullname",
    "pixelsize",
    "decorative",
    "fullnamelang",
    "globaladvance",
    "hinting",
    "minspace",
    "hintstyle",
    "fontformat",
    "fontversion",
    "fontfeatures",
    "outline",
    "autohint",
    "slant",
    "scale",
    "postscriptname",
    "dpi",
    "scalable",
    "embolden",
    "lang",
    "antialias",
    "family",
    "hash",
    "namelang",
    "embeddedbitmap",
    "familylang",
    "verticallayout",
    "matrix",
    "rasterizer",
    "aspect",
    "charset",
    "width",
    "charwidth",
    "charheight",
    "rgba",
    "style",
    "lcdfilter",
    "spacing",
    "index",
    "stylelang",
    "capability",
    "prgname"
  };
#define FcObjectTypeNamePool ((const char *) &FcObjectTypeNamePool_contents)
#ifdef __GNUC__
__inline
#ifdef __GNUC_STDC_INLINE__
__attribute__ ((__gnu_inline__))
#endif
#endif
const struct FcObjectTypeInfo *
FcObjectTypeLookup (register const char *str, register unsigned int len)
{
  enum
    {
      TOTAL_KEYWORDS = 46,
      MIN_WORD_LENGTH = 3,
      MAX_WORD_LENGTH = 14,
      MIN_HASH_VALUE = 4,
      MAX_HASH_VALUE = 55
    };

  static const struct FcObjectTypeInfo wordlist[] =
    {
      {-1}, {-1}, {-1}, {-1},
#line 38 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str4,FC_FILE_OBJECT},
#line 27 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str5,FC_SIZE_OBJECT},
#line 25 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str6,FC_WEIGHT_OBJECT},
#line 31 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str7,FC_FOUNDRY_OBJECT},
#line 22 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str8,FC_FULLNAME_OBJECT},
#line 29 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str9,FC_PIXEL_SIZE_OBJECT},
      {-1},
#line 57 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str11,FC_DECORATIVE_OBJECT},
#line 23 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str12,FC_FULLNAMELANG_OBJECT},
#line 37 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str13,FC_GLOBAL_ADVANCE_OBJECT},
#line 34 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str14,FC_HINTING_OBJECT},
#line 46 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str15,FC_MINSPACE_OBJECT},
#line 33 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str16,FC_HINT_STYLE_OBJECT},
#line 54 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str17,FC_FONTFORMAT_OBJECT},
#line 52 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str18,FC_FONTVERSION_OBJECT},
#line 60 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str19,FC_FONT_FEATURES_OBJECT},
#line 41 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str20,FC_OUTLINE_OBJECT},
#line 36 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str21,FC_AUTOHINT_OBJECT},
#line 24 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str22,FC_SLANT_OBJECT},
#line 45 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str23,FC_SCALE_OBJECT},
#line 63 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str24,FC_POSTSCRIPT_NAME_OBJECT},
#line 43 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str25,FC_DPI_OBJECT},
#line 42 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str26,FC_SCALABLE_OBJECT},
#line 55 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str27,FC_EMBOLDEN_OBJECT},
#line 51 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str28,FC_LANG_OBJECT},
#line 32 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str29,FC_ANTIALIAS_OBJECT},
#line 18 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str30,FC_FAMILY_OBJECT},
#line 62 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str31,FC_HASH_OBJECT},
#line 59 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str32,FC_NAMELANG_OBJECT},
#line 56 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str33,FC_EMBEDDED_BITMAP_OBJECT},
#line 19 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str34,FC_FAMILYLANG_OBJECT},
#line 35 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str35,FC_VERTICAL_LAYOUT_OBJECT},
#line 49 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str36,FC_MATRIX_OBJECT},
#line 40 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str37,FC_RASTERIZER_OBJECT},
#line 28 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str38,FC_ASPECT_OBJECT},
#line 50 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str39,FC_CHARSET_OBJECT},
#line 26 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str40,FC_WIDTH_OBJECT},
#line 47 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str41,FC_CHAR_WIDTH_OBJECT},
#line 48 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str42,FC_CHAR_HEIGHT_OBJECT},
#line 44 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str43,FC_RGBA_OBJECT},
#line 20 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str44,FC_STYLE_OBJECT},
#line 58 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str45,FC_LCD_FILTER_OBJECT},
#line 30 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str46,FC_SPACING_OBJECT},
#line 39 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str47,FC_INDEX_OBJECT},
#line 21 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str48,FC_STYLELANG_OBJECT},
#line 53 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str49,FC_CAPABILITY_OBJECT},
      {-1}, {-1}, {-1}, {-1}, {-1},
#line 61 "fcobjshash.gperf"
      {(int)(long)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str55,FC_PRGNAME_OBJECT}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = FcObjectTypeHash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register int o = wordlist[key].name;
          if (o >= 0)
            {
              register const char *s = o + FcObjectTypeNamePool;

              if (*str == *s && !strcmp (str + 1, s + 1))
                return &wordlist[key];
            }
        }
    }
  return 0;
}
