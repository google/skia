/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <array>
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

struct ColorRec {
    uint8_t     r, g, b;
}; static constexpr auto gColors = std::to_array<ColorRec>({
    ColorRec{ 0xf0,0xf8,0xff }, // aliceblue
    ColorRec{ 0xfa,0xeb,0xd7 }, // antiquewhite
    ColorRec{ 0x00,0xff,0xff }, // aqua
    ColorRec{ 0x7f,0xff,0xd4 }, // aquamarine
    ColorRec{ 0xf0,0xff,0xff }, // azure
    ColorRec{ 0xf5,0xf5,0xdc }, // beige
    ColorRec{ 0xff,0xe4,0xc4 }, // bisque
    ColorRec{ 0x00,0x00,0x00 }, // black
    ColorRec{ 0xff,0xeb,0xcd }, // blanchedalmond
    ColorRec{ 0x00,0x00,0xff }, // blue
    ColorRec{ 0x8a,0x2b,0xe2 }, // blueviolet
    ColorRec{ 0xa5,0x2a,0x2a }, // brown
    ColorRec{ 0xde,0xb8,0x87 }, // burlywood
    ColorRec{ 0x5f,0x9e,0xa0 }, // cadetblue
    ColorRec{ 0x7f,0xff,0x00 }, // chartreuse
    ColorRec{ 0xd2,0x69,0x1e }, // chocolate
    ColorRec{ 0xff,0x7f,0x50 }, // coral
    ColorRec{ 0x64,0x95,0xed }, // cornflowerblue
    ColorRec{ 0xff,0xf8,0xdc }, // cornsilk
    ColorRec{ 0xdc,0x14,0x3c }, // crimson
    ColorRec{ 0x00,0xff,0xff }, // cyan
    ColorRec{ 0x00,0x00,0x8b }, // darkblue
    ColorRec{ 0x00,0x8b,0x8b }, // darkcyan
    ColorRec{ 0xb8,0x86,0x0b }, // darkgoldenrod
    ColorRec{ 0xa9,0xa9,0xa9 }, // darkgray
    ColorRec{ 0x00,0x64,0x00 }, // darkgreen
    ColorRec{ 0xbd,0xb7,0x6b }, // darkkhaki
    ColorRec{ 0x8b,0x00,0x8b }, // darkmagenta
    ColorRec{ 0x55,0x6b,0x2f }, // darkolivegreen
    ColorRec{ 0xff,0x8c,0x00 }, // darkorange
    ColorRec{ 0x99,0x32,0xcc }, // darkorchid
    ColorRec{ 0x8b,0x00,0x00 }, // darkred
    ColorRec{ 0xe9,0x96,0x7a }, // darksalmon
    ColorRec{ 0x8f,0xbc,0x8f }, // darkseagreen
    ColorRec{ 0x48,0x3d,0x8b }, // darkslateblue
    ColorRec{ 0x2f,0x4f,0x4f }, // darkslategray
    ColorRec{ 0x00,0xce,0xd1 }, // darkturquoise
    ColorRec{ 0x94,0x00,0xd3 }, // darkviolet
    ColorRec{ 0xff,0x14,0x93 }, // deeppink
    ColorRec{ 0x00,0xbf,0xff }, // deepskyblue
    ColorRec{ 0x69,0x69,0x69 }, // dimgray
    ColorRec{ 0x1e,0x90,0xff }, // dodgerblue
    ColorRec{ 0xb2,0x22,0x22 }, // firebrick
    ColorRec{ 0xff,0xfa,0xf0 }, // floralwhite
    ColorRec{ 0x22,0x8b,0x22 }, // forestgreen
    ColorRec{ 0xff,0x00,0xff }, // fuchsia
    ColorRec{ 0xdc,0xdc,0xdc }, // gainsboro
    ColorRec{ 0xf8,0xf8,0xff }, // ghostwhite
    ColorRec{ 0xff,0xd7,0x00 }, // gold
    ColorRec{ 0xda,0xa5,0x20 }, // goldenrod
    ColorRec{ 0x80,0x80,0x80 }, // gray
    ColorRec{ 0x00,0x80,0x00 }, // green
    ColorRec{ 0xad,0xff,0x2f }, // greenyellow
    ColorRec{ 0xf0,0xff,0xf0 }, // honeydew
    ColorRec{ 0xff,0x69,0xb4 }, // hotpink
    ColorRec{ 0xcd,0x5c,0x5c }, // indianred
    ColorRec{ 0x4b,0x00,0x82 }, // indigo
    ColorRec{ 0xff,0xff,0xf0 }, // ivory
    ColorRec{ 0xf0,0xe6,0x8c }, // khaki
    ColorRec{ 0xe6,0xe6,0xfa }, // lavender
    ColorRec{ 0xff,0xf0,0xf5 }, // lavenderblush
    ColorRec{ 0x7c,0xfc,0x00 }, // lawngreen
    ColorRec{ 0xff,0xfa,0xcd }, // lemonchiffon
    ColorRec{ 0xad,0xd8,0xe6 }, // lightblue
    ColorRec{ 0xf0,0x80,0x80 }, // lightcoral
    ColorRec{ 0xe0,0xff,0xff }, // lightcyan
    ColorRec{ 0xfa,0xfa,0xd2 }, // lightgoldenrodyellow
    ColorRec{ 0x90,0xee,0x90 }, // lightgreen
    ColorRec{ 0xd3,0xd3,0xd3 }, // lightgrey
    ColorRec{ 0xff,0xb6,0xc1 }, // lightpink
    ColorRec{ 0xff,0xa0,0x7a }, // lightsalmon
    ColorRec{ 0x20,0xb2,0xaa }, // lightseagreen
    ColorRec{ 0x87,0xce,0xfa }, // lightskyblue
    ColorRec{ 0x77,0x88,0x99 }, // lightslategray
    ColorRec{ 0xb0,0xc4,0xde }, // lightsteelblue
    ColorRec{ 0xff,0xff,0xe0 }, // lightyellow
    ColorRec{ 0x00,0xff,0x00 }, // lime
    ColorRec{ 0x32,0xcd,0x32 }, // limegreen
    ColorRec{ 0xfa,0xf0,0xe6 }, // linen
    ColorRec{ 0xff,0x00,0xff }, // magenta
    ColorRec{ 0x80,0x00,0x00 }, // maroon
    ColorRec{ 0x66,0xcd,0xaa }, // mediumaquamarine
    ColorRec{ 0x00,0x00,0xcd }, // mediumblue
    ColorRec{ 0xba,0x55,0xd3 }, // mediumorchid
    ColorRec{ 0x93,0x70,0xdb }, // mediumpurple
    ColorRec{ 0x3c,0xb3,0x71 }, // mediumseagreen
    ColorRec{ 0x7b,0x68,0xee }, // mediumslateblue
    ColorRec{ 0x00,0xfa,0x9a }, // mediumspringgreen
    ColorRec{ 0x48,0xd1,0xcc }, // mediumturquoise
    ColorRec{ 0xc7,0x15,0x85 }, // mediumvioletred
    ColorRec{ 0x19,0x19,0x70 }, // midnightblue
    ColorRec{ 0xf5,0xff,0xfa }, // mintcream
    ColorRec{ 0xff,0xe4,0xe1 }, // mistyrose
    ColorRec{ 0xff,0xe4,0xb5 }, // moccasin
    ColorRec{ 0xff,0xde,0xad }, // navajowhite
    ColorRec{ 0x00,0x00,0x80 }, // navy
    ColorRec{ 0xfd,0xf5,0xe6 }, // oldlace
    ColorRec{ 0x80,0x80,0x00 }, // olive
    ColorRec{ 0x6b,0x8e,0x23 }, // olivedrab
    ColorRec{ 0xff,0xa5,0x00 }, // orange
    ColorRec{ 0xff,0x45,0x00 }, // orangered
    ColorRec{ 0xda,0x70,0xd6 }, // orchid
    ColorRec{ 0xee,0xe8,0xaa }, // palegoldenrod
    ColorRec{ 0x98,0xfb,0x98 }, // palegreen
    ColorRec{ 0xaf,0xee,0xee }, // paleturquoise
    ColorRec{ 0xdb,0x70,0x93 }, // palevioletred
    ColorRec{ 0xff,0xef,0xd5 }, // papayawhip
    ColorRec{ 0xff,0xda,0xb9 }, // peachpuff
    ColorRec{ 0xcd,0x85,0x3f }, // peru
    ColorRec{ 0xff,0xc0,0xcb }, // pink
    ColorRec{ 0xdd,0xa0,0xdd }, // plum
    ColorRec{ 0xb0,0xe0,0xe6 }, // powderblue
    ColorRec{ 0x80,0x00,0x80 }, // purple
    ColorRec{ 0xff,0x00,0x00 }, // red
    ColorRec{ 0xbc,0x8f,0x8f }, // rosybrown
    ColorRec{ 0x41,0x69,0xe1 }, // royalblue
    ColorRec{ 0x8b,0x45,0x13 }, // saddlebrown
    ColorRec{ 0xfa,0x80,0x72 }, // salmon
    ColorRec{ 0xf4,0xa4,0x60 }, // sandybrown
    ColorRec{ 0x2e,0x8b,0x57 }, // seagreen
    ColorRec{ 0xff,0xf5,0xee }, // seashell
    ColorRec{ 0xa0,0x52,0x2d }, // sienna
    ColorRec{ 0xc0,0xc0,0xc0 }, // silver
    ColorRec{ 0x87,0xce,0xeb }, // skyblue
    ColorRec{ 0x6a,0x5a,0xcd }, // slateblue
    ColorRec{ 0x70,0x80,0x90 }, // slategray
    ColorRec{ 0xff,0xfa,0xfa }, // snow
    ColorRec{ 0x00,0xff,0x7f }, // springgreen
    ColorRec{ 0x46,0x82,0xb4 }, // steelblue
    ColorRec{ 0xd2,0xb4,0x8c }, // tan
    ColorRec{ 0x00,0x80,0x80 }, // teal
    ColorRec{ 0xd8,0xbf,0xd8 }, // thistle
    ColorRec{ 0xff,0x63,0x47 }, // tomato
    ColorRec{ 0x40,0xe0,0xd0 }, // turquoise
    ColorRec{ 0xee,0x82,0xee }, // violet
    ColorRec{ 0xf5,0xde,0xb3 }, // wheat
    ColorRec{ 0xff,0xff,0xff }, // white
    ColorRec{ 0xf5,0xf5,0xf5 }, // whitesmoke
    ColorRec{ 0xff,0xff,0x00 }, // yellow
    ColorRec{ 0x9a,0xcd,0x32 }, // yellowgreen
});

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
