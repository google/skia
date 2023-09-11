/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/utils/SkParse.h"

#include "include/core/SkColor.h"
#include "include/core/SkTypes.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>

static constexpr const char* gColorNames[] = {
    "aliceblue",
    "antiquewhite",
    "aqua",
    "aquamarine",
    "azure",
    "beige",
    "bisque",
    "black",
    "blanchedalmond",
    "blue",
    "blueviolet",
    "brown",
    "burlywood",
    "cadetblue",
    "chartreuse",
    "chocolate",
    "coral",
    "cornflowerblue",
    "cornsilk",
    "crimson",
    "cyan",
    "darkblue",
    "darkcyan",
    "darkgoldenrod",
    "darkgray",
    "darkgreen",
    "darkkhaki",
    "darkmagenta",
    "darkolivegreen",
    "darkorange",
    "darkorchid",
    "darkred",
    "darksalmon",
    "darkseagreen",
    "darkslateblue",
    "darkslategray",
    "darkturquoise",
    "darkviolet",
    "deeppink",
    "deepskyblue",
    "dimgray",
    "dodgerblue",
    "firebrick",
    "floralwhite",
    "forestgreen",
    "fuchsia",
    "gainsboro",
    "ghostwhite",
    "gold",
    "goldenrod",
    "gray",
    "green",
    "greenyellow",
    "honeydew",
    "hotpink",
    "indianred",
    "indigo",
    "ivory",
    "khaki",
    "lavender",
    "lavenderblush",
    "lawngreen",
    "lemonchiffon",
    "lightblue",
    "lightcoral",
    "lightcyan",
    "lightgoldenrodyellow",
    "lightgreen",
    "lightgrey",
    "lightpink",
    "lightsalmon",
    "lightseagreen",
    "lightskyblue",
    "lightslategray",
    "lightsteelblue",
    "lightyellow",
    "lime",
    "limegreen",
    "linen",
    "magenta",
    "maroon",
    "mediumaquamarine",
    "mediumblue",
    "mediumorchid",
    "mediumpurple",
    "mediumseagreen",
    "mediumslateblue",
    "mediumspringgreen",
    "mediumturquoise",
    "mediumvioletred",
    "midnightblue",
    "mintcream",
    "mistyrose",
    "moccasin",
    "navajowhite",
    "navy",
    "oldlace",
    "olive",
    "olivedrab",
    "orange",
    "orangered",
    "orchid",
    "palegoldenrod",
    "palegreen",
    "paleturquoise",
    "palevioletred",
    "papayawhip",
    "peachpuff",
    "peru",
    "pink",
    "plum",
    "powderblue",
    "purple",
    "red",
    "rosybrown",
    "royalblue",
    "saddlebrown",
    "salmon",
    "sandybrown",
    "seagreen",
    "seashell",
    "sienna",
    "silver",
    "skyblue",
    "slateblue",
    "slategray",
    "snow",
    "springgreen",
    "steelblue",
    "tan",
    "teal",
    "thistle",
    "tomato",
    "turquoise",
    "violet",
    "wheat",
    "white",
    "whitesmoke",
    "yellow",
    "yellowgreen",
};

static constexpr struct ColorRec {
    uint8_t     r, g, b;
} gColors[] = {
    { 0xf0,0xf8,0xff }, // aliceblue
    { 0xfa,0xeb,0xd7 }, // antiquewhite
    { 0x00,0xff,0xff }, // aqua
    { 0x7f,0xff,0xd4 }, // aquamarine
    { 0xf0,0xff,0xff }, // azure
    { 0xf5,0xf5,0xdc }, // beige
    { 0xff,0xe4,0xc4 }, // bisque
    { 0x00,0x00,0x00 }, // black
    { 0xff,0xeb,0xcd }, // blanchedalmond
    { 0x00,0x00,0xff }, // blue
    { 0x8a,0x2b,0xe2 }, // blueviolet
    { 0xa5,0x2a,0x2a }, // brown
    { 0xde,0xb8,0x87 }, // burlywood
    { 0x5f,0x9e,0xa0 }, // cadetblue
    { 0x7f,0xff,0x00 }, // chartreuse
    { 0xd2,0x69,0x1e }, // chocolate
    { 0xff,0x7f,0x50 }, // coral
    { 0x64,0x95,0xed }, // cornflowerblue
    { 0xff,0xf8,0xdc }, // cornsilk
    { 0xdc,0x14,0x3c }, // crimson
    { 0x00,0xff,0xff }, // cyan
    { 0x00,0x00,0x8b }, // darkblue
    { 0x00,0x8b,0x8b }, // darkcyan
    { 0xb8,0x86,0x0b }, // darkgoldenrod
    { 0xa9,0xa9,0xa9 }, // darkgray
    { 0x00,0x64,0x00 }, // darkgreen
    { 0xbd,0xb7,0x6b }, // darkkhaki
    { 0x8b,0x00,0x8b }, // darkmagenta
    { 0x55,0x6b,0x2f }, // darkolivegreen
    { 0xff,0x8c,0x00 }, // darkorange
    { 0x99,0x32,0xcc }, // darkorchid
    { 0x8b,0x00,0x00 }, // darkred
    { 0xe9,0x96,0x7a }, // darksalmon
    { 0x8f,0xbc,0x8f }, // darkseagreen
    { 0x48,0x3d,0x8b }, // darkslateblue
    { 0x2f,0x4f,0x4f }, // darkslategray
    { 0x00,0xce,0xd1 }, // darkturquoise
    { 0x94,0x00,0xd3 }, // darkviolet
    { 0xff,0x14,0x93 }, // deeppink
    { 0x00,0xbf,0xff }, // deepskyblue
    { 0x69,0x69,0x69 }, // dimgray
    { 0x1e,0x90,0xff }, // dodgerblue
    { 0xb2,0x22,0x22 }, // firebrick
    { 0xff,0xfa,0xf0 }, // floralwhite
    { 0x22,0x8b,0x22 }, // forestgreen
    { 0xff,0x00,0xff }, // fuchsia
    { 0xdc,0xdc,0xdc }, // gainsboro
    { 0xf8,0xf8,0xff }, // ghostwhite
    { 0xff,0xd7,0x00 }, // gold
    { 0xda,0xa5,0x20 }, // goldenrod
    { 0x80,0x80,0x80 }, // gray
    { 0x00,0x80,0x00 }, // green
    { 0xad,0xff,0x2f }, // greenyellow
    { 0xf0,0xff,0xf0 }, // honeydew
    { 0xff,0x69,0xb4 }, // hotpink
    { 0xcd,0x5c,0x5c }, // indianred
    { 0x4b,0x00,0x82 }, // indigo
    { 0xff,0xff,0xf0 }, // ivory
    { 0xf0,0xe6,0x8c }, // khaki
    { 0xe6,0xe6,0xfa }, // lavender
    { 0xff,0xf0,0xf5 }, // lavenderblush
    { 0x7c,0xfc,0x00 }, // lawngreen
    { 0xff,0xfa,0xcd }, // lemonchiffon
    { 0xad,0xd8,0xe6 }, // lightblue
    { 0xf0,0x80,0x80 }, // lightcoral
    { 0xe0,0xff,0xff }, // lightcyan
    { 0xfa,0xfa,0xd2 }, // lightgoldenrodyellow
    { 0x90,0xee,0x90 }, // lightgreen
    { 0xd3,0xd3,0xd3 }, // lightgrey
    { 0xff,0xb6,0xc1 }, // lightpink
    { 0xff,0xa0,0x7a }, // lightsalmon
    { 0x20,0xb2,0xaa }, // lightseagreen
    { 0x87,0xce,0xfa }, // lightskyblue
    { 0x77,0x88,0x99 }, // lightslategray
    { 0xb0,0xc4,0xde }, // lightsteelblue
    { 0xff,0xff,0xe0 }, // lightyellow
    { 0x00,0xff,0x00 }, // lime
    { 0x32,0xcd,0x32 }, // limegreen
    { 0xfa,0xf0,0xe6 }, // linen
    { 0xff,0x00,0xff }, // magenta
    { 0x80,0x00,0x00 }, // maroon
    { 0x66,0xcd,0xaa }, // mediumaquamarine
    { 0x00,0x00,0xcd }, // mediumblue
    { 0xba,0x55,0xd3 }, // mediumorchid
    { 0x93,0x70,0xdb }, // mediumpurple
    { 0x3c,0xb3,0x71 }, // mediumseagreen
    { 0x7b,0x68,0xee }, // mediumslateblue
    { 0x00,0xfa,0x9a }, // mediumspringgreen
    { 0x48,0xd1,0xcc }, // mediumturquoise
    { 0xc7,0x15,0x85 }, // mediumvioletred
    { 0x19,0x19,0x70 }, // midnightblue
    { 0xf5,0xff,0xfa }, // mintcream
    { 0xff,0xe4,0xe1 }, // mistyrose
    { 0xff,0xe4,0xb5 }, // moccasin
    { 0xff,0xde,0xad }, // navajowhite
    { 0x00,0x00,0x80 }, // navy
    { 0xfd,0xf5,0xe6 }, // oldlace
    { 0x80,0x80,0x00 }, // olive
    { 0x6b,0x8e,0x23 }, // olivedrab
    { 0xff,0xa5,0x00 }, // orange
    { 0xff,0x45,0x00 }, // orangered
    { 0xda,0x70,0xd6 }, // orchid
    { 0xee,0xe8,0xaa }, // palegoldenrod
    { 0x98,0xfb,0x98 }, // palegreen
    { 0xaf,0xee,0xee }, // paleturquoise
    { 0xdb,0x70,0x93 }, // palevioletred
    { 0xff,0xef,0xd5 }, // papayawhip
    { 0xff,0xda,0xb9 }, // peachpuff
    { 0xcd,0x85,0x3f }, // peru
    { 0xff,0xc0,0xcb }, // pink
    { 0xdd,0xa0,0xdd }, // plum
    { 0xb0,0xe0,0xe6 }, // powderblue
    { 0x80,0x00,0x80 }, // purple
    { 0xff,0x00,0x00 }, // red
    { 0xbc,0x8f,0x8f }, // rosybrown
    { 0x41,0x69,0xe1 }, // royalblue
    { 0x8b,0x45,0x13 }, // saddlebrown
    { 0xfa,0x80,0x72 }, // salmon
    { 0xf4,0xa4,0x60 }, // sandybrown
    { 0x2e,0x8b,0x57 }, // seagreen
    { 0xff,0xf5,0xee }, // seashell
    { 0xa0,0x52,0x2d }, // sienna
    { 0xc0,0xc0,0xc0 }, // silver
    { 0x87,0xce,0xeb }, // skyblue
    { 0x6a,0x5a,0xcd }, // slateblue
    { 0x70,0x80,0x90 }, // slategray
    { 0xff,0xfa,0xfa }, // snow
    { 0x00,0xff,0x7f }, // springgreen
    { 0x46,0x82,0xb4 }, // steelblue
    { 0xd2,0xb4,0x8c }, // tan
    { 0x00,0x80,0x80 }, // teal
    { 0xd8,0xbf,0xd8 }, // thistle
    { 0xff,0x63,0x47 }, // tomato
    { 0x40,0xe0,0xd0 }, // turquoise
    { 0xee,0x82,0xee }, // violet
    { 0xf5,0xde,0xb3 }, // wheat
    { 0xff,0xff,0xff }, // white
    { 0xf5,0xf5,0xf5 }, // whitesmoke
    { 0xff,0xff,0x00 }, // yellow
    { 0x9a,0xcd,0x32 }, // yellowgreen
};

const char* SkParse::FindNamedColor(const char* name, size_t len, SkColor* color) {
    const auto rec = std::lower_bound(std::begin(gColorNames),
                                      std::end  (gColorNames),
                                      name, // key
                                      [](const char* name, const char* key) {
                                          return strcmp(name, key) < 0;
                                      });

    if (rec == std::end(gColorNames) || 0 != strcmp(name, *rec)) {
        return nullptr;
    }

    if (color) {
        int index = rec - gColorNames;
        *color = SkColorSetRGB(gColors[index].r, gColors[index].g, gColors[index].b);
    }

    return name + strlen(*rec);
}

// !!! move to char utilities
//static int count_separators(const char* str, const char* sep) {
//  char c;
//  int separators = 0;
//  while ((c = *str++) != '\0') {
//      if (strchr(sep, c) == nullptr)
//          continue;
//      do {
//          if ((c = *str++) == '\0')
//              goto goHome;
//      } while (strchr(sep, c) != nullptr);
//      separators++;
//  }
//goHome:
//  return separators;
//}

static inline unsigned nib2byte(unsigned n)
{
    SkASSERT((n & ~0xF) == 0);
    return (n << 4) | n;
}

const char* SkParse::FindColor(const char* value, SkColor* colorPtr) {
    unsigned int oldAlpha = SkColorGetA(*colorPtr);
    if (value[0] == '#') {
        uint32_t    hex;
        const char* end = SkParse::FindHex(value + 1, &hex);
//      SkASSERT(end);
        if (end == nullptr)
            return end;
        size_t len = end - value - 1;
        if (len == 3 || len == 4) {
            unsigned a = len == 4 ? nib2byte(hex >> 12) : oldAlpha;
            unsigned r = nib2byte((hex >> 8) & 0xF);
            unsigned g = nib2byte((hex >> 4) & 0xF);
            unsigned b = nib2byte(hex & 0xF);
            *colorPtr = SkColorSetARGB(a, r, g, b);
            return end;
        } else if (len == 6 || len == 8) {
            if (len == 6)
                hex |= oldAlpha << 24;
            *colorPtr = hex;
            return end;
        } else {
//          SkASSERT(0);
            return nullptr;
        }
//  } else if (strchr(value, ',')) {
//      SkScalar array[4];
//      int count = count_separators(value, ",") + 1; // !!! count commas, add 1
//      SkASSERT(count == 3 || count == 4);
//      array[0] = SK_Scalar1 * 255;
//      const char* end = SkParse::FindScalars(value, &array[4 - count], count);
//      if (end == nullptr)
//          return nullptr;
        // !!! range check for errors?
//      *colorPtr = SkColorSetARGB(SkScalarRoundToInt(array[0]), SkScalarRoundToInt(array[1]),
//          SkScalarRoundToInt(array[2]), SkScalarRoundToInt(array[3]));
//      return end;
    } else
        return FindNamedColor(value, strlen(value), colorPtr);
}
