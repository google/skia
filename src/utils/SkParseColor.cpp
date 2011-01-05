/* libs/graphics/xml/SkParseColor.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include "SkParse.h"

#ifdef SK_DEBUG
#include "SkString.h"

    // compress names 6 chars per long (packed 5 bits/char )
        // note: little advantage to splitting chars across longs, since 3 longs at 2 unused bits each
        // allow for one additional split char (vs. the 18 unsplit chars in the three longs)
    // use extra two bits to represent:
        // 00 : final 6 (or fewer) chars (if 'a' is 0x01, zero could have special meaning)
        // 01 : not final 6 chars
        // 10 : color
        // 11 : unused, except as debugging sentinal? (could be -1 for easier test)
    // !!! the bit to end the word (last) is at the low bit for binary search
    // lookup first character in offset for quick start
        // offset is 27-entry table of bytes(?) that trims linear search to at most 21 entries ('d')
    // shift match into long; set bit 30 if it all doesn't fit
    // while longs don't match, march forward
        // if they do match, and bit 30 is set, advance match, clearing bit 30 if
        // final chars, and advance to next test
        // if they do match, and bit 30 is clear, get next long (color) and return it
    // stop at lookup of first char + 1
static const struct SkNameRGB {
    const char* name;
    int rgb;
} colorNames[] = {
    { "aliceblue",            0xF0F8FF },
    { "antiquewhite",         0xFAEBD7 },
    { "aqua",                 0x00FFFF },
    { "aquamarine",           0x7FFFD4 },
    { "azure",                0xF0FFFF },
    { "beige",                0xF5F5DC },
    { "bisque",               0xFFE4C4 },
    { "black",                0x000000 },
    { "blanchedalmond",       0xFFEBCD },
    { "blue",                 0x0000FF },
    { "blueviolet",           0x8A2BE2 },
    { "brown",                0xA52A2A },
    { "burlywood",            0xDEB887 },
    { "cadetblue",            0x5F9EA0 },
    { "chartreuse",           0x7FFF00 },
    { "chocolate",            0xD2691E },
    { "coral",                0xFF7F50 },
    { "cornflowerblue",       0x6495ED },
    { "cornsilk",             0xFFF8DC },
    { "crimson",              0xDC143C },
    { "cyan",                 0x00FFFF },
    { "darkblue",             0x00008B },
    { "darkcyan",             0x008B8B },
    { "darkgoldenrod",        0xB8860B },
    { "darkgray",             0xA9A9A9 },
    { "darkgreen",            0x006400 },
    { "darkkhaki",            0xBDB76B },
    { "darkmagenta",          0x8B008B },
    { "darkolivegreen",       0x556B2F },
    { "darkorange",           0xFF8C00 },
    { "darkorchid",           0x9932CC },
    { "darkred",              0x8B0000 },
    { "darksalmon",           0xE9967A },
    { "darkseagreen",         0x8FBC8F },
    { "darkslateblue",        0x483D8B },
    { "darkslategray",        0x2F4F4F },
    { "darkturquoise",        0x00CED1 },
    { "darkviolet",           0x9400D3 },
    { "deeppink",             0xFF1493 },
    { "deepskyblue",          0x00BFFF },
    { "dimgray",              0x696969 },
    { "dodgerblue",           0x1E90FF },
    { "firebrick",            0xB22222 },
    { "floralwhite",          0xFFFAF0 },
    { "forestgreen",          0x228B22 },
    { "fuchsia",              0xFF00FF },
    { "gainsboro",            0xDCDCDC },
    { "ghostwhite",           0xF8F8FF },
    { "gold",                 0xFFD700 },
    { "goldenrod",            0xDAA520 },
    { "gray",                 0x808080 },
    { "green",                0x008000 },
    { "greenyellow",          0xADFF2F },
    { "honeydew",             0xF0FFF0 },
    { "hotpink",              0xFF69B4 },
    { "indianred",            0xCD5C5C },
    { "indigo",               0x4B0082 },
    { "ivory",                0xFFFFF0 },
    { "khaki",                0xF0E68C },
    { "lavender",             0xE6E6FA },
    { "lavenderblush",        0xFFF0F5 },
    { "lawngreen",            0x7CFC00 },
    { "lemonchiffon",         0xFFFACD },
    { "lightblue",            0xADD8E6 },
    { "lightcoral",           0xF08080 },
    { "lightcyan",            0xE0FFFF },
    { "lightgoldenrodyellow", 0xFAFAD2 },
    { "lightgreen",           0x90EE90 },
    { "lightgrey",            0xD3D3D3 },
    { "lightpink",            0xFFB6C1 },
    { "lightsalmon",          0xFFA07A },
    { "lightseagreen",        0x20B2AA },
    { "lightskyblue",         0x87CEFA },
    { "lightslategray",       0x778899 },
    { "lightsteelblue",       0xB0C4DE },
    { "lightyellow",          0xFFFFE0 },
    { "lime",                 0x00FF00 },
    { "limegreen",            0x32CD32 },
    { "linen",                0xFAF0E6 },
    { "magenta",              0xFF00FF },
    { "maroon",               0x800000 },
    { "mediumaquamarine",     0x66CDAA },
    { "mediumblue",           0x0000CD },
    { "mediumorchid",         0xBA55D3 },
    { "mediumpurple",         0x9370DB },
    { "mediumseagreen",       0x3CB371 },
    { "mediumslateblue",      0x7B68EE },
    { "mediumspringgreen",    0x00FA9A },
    { "mediumturquoise",      0x48D1CC },
    { "mediumvioletred",      0xC71585 },
    { "midnightblue",         0x191970 },
    { "mintcream",            0xF5FFFA },
    { "mistyrose",            0xFFE4E1 },
    { "moccasin",             0xFFE4B5 },
    { "navajowhite",          0xFFDEAD },
    { "navy",                 0x000080 },
    { "oldlace",              0xFDF5E6 },
    { "olive",                0x808000 },
    { "olivedrab",            0x6B8E23 },
    { "orange",               0xFFA500 },
    { "orangered",            0xFF4500 },
    { "orchid",               0xDA70D6 },
    { "palegoldenrod",        0xEEE8AA },
    { "palegreen",            0x98FB98 },
    { "paleturquoise",        0xAFEEEE },
    { "palevioletred",        0xDB7093 },
    { "papayawhip",           0xFFEFD5 },
    { "peachpuff",            0xFFDAB9 },
    { "peru",                 0xCD853F },
    { "pink",                 0xFFC0CB },
    { "plum",                 0xDDA0DD },
    { "powderblue",           0xB0E0E6 },
    { "purple",               0x800080 },
    { "red",                  0xFF0000 },
    { "rosybrown",            0xBC8F8F },
    { "royalblue",            0x4169E1 },
    { "saddlebrown",          0x8B4513 },
    { "salmon",               0xFA8072 },
    { "sandybrown",           0xF4A460 },
    { "seagreen",             0x2E8B57 },
    { "seashell",             0xFFF5EE },
    { "sienna",               0xA0522D },
    { "silver",               0xC0C0C0 },
    { "skyblue",              0x87CEEB },
    { "slateblue",            0x6A5ACD },
    { "slategray",            0x708090 },
    { "snow",                 0xFFFAFA },
    { "springgreen",          0x00FF7F },
    { "steelblue",            0x4682B4 },
    { "tan",                  0xD2B48C },
    { "teal",                 0x008080 },
    { "thistle",              0xD8BFD8 },
    { "tomato",               0xFF6347 },
    { "turquoise",            0x40E0D0 },
    { "violet",               0xEE82EE },
    { "wheat",                0xF5DEB3 },
    { "white",                0xFFFFFF },
    { "whitesmoke",           0xF5F5F5 },
    { "yellow",               0xFFFF00 },
    { "yellowgreen",          0x9ACD32 }
};

int colorNamesSize = sizeof(colorNames) / sizeof(colorNames[0]);

#ifdef SK_SUPPORT_UNITTEST
static void CreateTable() {
    SkString comment;
    size_t originalSize = 0;
    int replacement = 0;
    for (int index = 0; index < colorNamesSize; index++) {
        SkNameRGB nameRGB =  colorNames[index];
        const char* name = nameRGB.name;
        size_t len = strlen(name);
        originalSize += len + 9;
        bool first = true;
        bool last = false;
        do {
            int compressed = 0;
            const char* start = name;
            for (int chIndex = 0; chIndex < 6; chIndex++) {
                compressed <<= 5;
                compressed |= *name ? *name++ - 'a' + 1 : 0 ;
            }
            replacement += sizeof(int);
            compressed <<= 1;
            compressed |= 1;
            if (first) {
                compressed |= 0x80000000;
                first = false;
            }
            if (len <= 6) { // last
                compressed &= ~1;
                last = true;
            }
            len -= 6;
            SkDebugf("0x%08x, ", compressed);
            comment.append(start, name - start);
        } while (last == false);
        replacement += sizeof(int);
        SkDebugf("0x%08x, ", nameRGB.rgb);
        SkDebugf("// %s\n", comment.c_str());
        comment.reset();
    }
    SkDebugf("// original = %d : replacement = %d\n", originalSize, replacement);
    SkASSERT(0); // always stop after creating table
}
#endif

#endif

static const unsigned int gColorNames[] = {
0x85891945, 0x32a50000, 0x00f0f8ff, // aliceblue
0x85d44c6b, 0x16e84d0a, 0x00faebd7, // antiquewhite
0x86350800, 0x0000ffff, // aqua
0x86350b43, 0x492e2800, 0x007fffd4, // aquamarine
0x87559140, 0x00f0ffff, // azure
0x88a93940, 0x00f5f5dc, // beige
0x89338d4a, 0x00ffe4c4, // bisque
0x89811ac0, 0x00000000, // black
0x898170d1, 0x1481635f, 0x38800000, 0x00ffebcd, // blanchedalmond
0x89952800, 0x000000ff, // blue
0x89952d93, 0x3d85a000, 0x008a2be2, // blueviolet
0x8a4fbb80, 0x00a52a2a, // brown
0x8ab2666f, 0x3de40000, 0x00deb887, // burlywood
0x8c242d05, 0x32a50000, 0x005f9ea0, // cadetblue
0x8d019525, 0x16b32800, 0x007fff00, // chartreuse
0x8d0f1bd9, 0x06850000, 0x00d2691e, // chocolate
0x8df20b00, 0x00ff7f50, // coral
0x8df27199, 0x3ee59099, 0x54a00000, 0x006495ed, // cornflowerblue
0x8df274d3, 0x31600000, 0x00fff8dc, // cornsilk
0x8e496cdf, 0x38000000, 0x00dc143c, // crimson
0x8f217000, 0x0000ffff, // cyan
0x90325899, 0x54a00000, 0x0000008b, // darkblue
0x903258f3, 0x05c00000, 0x00008b8b, // darkcyan
0x903259df, 0x3085749f, 0x10000000, 0x00b8860b, // darkgoldenrod
0x903259e5, 0x07200000, 0x00a9a9a9, // darkgray
0x903259e5, 0x14ae0000, 0x00006400, // darkgreen
0x90325ad1, 0x05690000, 0x00bdb76b, // darkkhaki
0x90325b43, 0x1caea040, 0x008b008b, // darkmagenta
0x90325bd9, 0x26c53c8b, 0x15c00000, 0x00556b2f, // darkolivegreen
0x90325be5, 0x05c72800, 0x00ff8c00, // darkorange
0x90325be5, 0x0d092000, 0x009932cc, // darkorchid
0x90325c8b, 0x10000000, 0x008b0000, // darkred
0x90325cc3, 0x31af7000, 0x00e9967a, // darksalmon
0x90325ccb, 0x04f2295c, 0x008fbc8f, // darkseagreen
0x90325cd9, 0x0685132b, 0x14000000, 0x00483d8b, // darkslateblue
0x90325cd9, 0x06853c83, 0x64000000, 0x002f4f4f, // darkslategray
0x90325d2b, 0x4a357a67, 0x14000000, 0x0000ced1, // darkturquoise
0x90325d93, 0x3d85a000, 0x009400d3, // darkviolet
0x90a58413, 0x39600000, 0x00ff1493, // deeppink
0x90a584d7, 0x644ca940, 0x0000bfff, // deepskyblue
0x912d3c83, 0x64000000, 0x00696969, // dimgray
0x91e43965, 0x09952800, 0x001e90ff, // dodgerblue
0x993228a5, 0x246b0000, 0x00b22222, // firebrick
0x998f9059, 0x5d09a140, 0x00fffaf0, // floralwhite
0x99f22ce9, 0x1e452b80, 0x00228b22, // forestgreen
0x9aa344d3, 0x04000000, 0x00ff00ff, // fuchsia
0x9c2974c5, 0x3e4f0000, 0x00dcdcdc, // gainsboro
0x9d0f9d2f, 0x21342800, 0x00f8f8ff, // ghostwhite
0x9dec2000, 0x00ffd700, // gold
0x9dec215d, 0x49e40000, 0x00daa520, // goldenrod
0x9e41c800, 0x00808080, // gray
0x9e452b80, 0x00008000, // green
0x9e452bb3, 0x158c7dc0, 0x00adff2f, // greenyellow
0xa1ee2e49, 0x16e00000, 0x00f0fff0, // honeydew
0xa1f4825d, 0x2c000000, 0x00ff69b4, // hotpink
0xa5c4485d, 0x48a40000, 0x00cd5c5c, // indianred
0xa5c449de, 0x004b0082, // indigo
0xa6cf9640, 0x00fffff0, // ivory
0xad015a40, 0x00f0e68c, // khaki
0xb0362b89, 0x16400000, 0x00e6e6fa, // lavender
0xb0362b89, 0x16426567, 0x20000000, 0x00fff0f5, // lavenderblush
0xb03771e5, 0x14ae0000, 0x007cfc00, // lawngreen
0xb0ad7b87, 0x212633dc, 0x00fffacd, // lemonchiffon
0xb1274505, 0x32a50000, 0x00add8e6, // lightblue
0xb1274507, 0x3e416000, 0x00f08080, // lightcoral
0xb1274507, 0x642e0000, 0x00e0ffff, // lightcyan
0xb127450f, 0x3d842ba5, 0x3c992b19, 0x3ee00000, 0x00fafad2, // lightgoldenrodyellow
0xb127450f, 0x48a57000, 0x0090ee90, // lightgreen
0xb127450f, 0x48b90000, 0x00d3d3d3, // lightgrey
0xb1274521, 0x25cb0000, 0x00ffb6c1, // lightpink
0xb1274527, 0x058d7b80, 0x00ffa07a, // lightsalmon
0xb1274527, 0x1427914b, 0x38000000, 0x0020b2aa, // lightseagreen
0xb1274527, 0x2f22654a, 0x0087cefa, // lightskyblue
0xb1274527, 0x303429e5, 0x07200000, 0x00778899, // lightslategray
0xb1274527, 0x50a56099, 0x54a00000, 0x00b0c4de, // lightsteelblue
0xb1274533, 0x158c7dc0, 0x00ffffe0, // lightyellow
0xb12d2800, 0x0000ff00, // lime
0xb12d29e5, 0x14ae0000, 0x0032cd32, // limegreen
0xb12e2b80, 0x00faf0e6, // linen
0xb4272ba9, 0x04000000, 0x00ff00ff, // magenta
0xb4327bdc, 0x00800000, // maroon
0xb4a44d5b, 0x06350b43, 0x492e2800, 0x0066cdaa, // mediumaquamarine
0xb4a44d5b, 0x09952800, 0x000000cd, // mediumblue
0xb4a44d5b, 0x3e434248, 0x00ba55d3, // mediumorchid
0xb4a44d5b, 0x42b2830a, 0x009370db, // mediumpurple
0xb4a44d5b, 0x4ca13c8b, 0x15c00000, 0x003cb371, // mediumseagreen
0xb4a44d5b, 0x4d81a145, 0x32a50000, 0x007b68ee, // mediumslateblue
0xb4a44d5b, 0x4e124b8f, 0x1e452b80, 0x0000fa9a, // mediumspringgreen
0xb4a44d5b, 0x52b28d5f, 0x26650000, 0x0048d1cc, // mediumturquoise
0xb4a44d5b, 0x592f6169, 0x48a40000, 0x00c71585, // mediumvioletred
0xb524724f, 0x2282654a, 0x00191970, // midnightblue
0xb52ea0e5, 0x142d0000, 0x00f5fffa, // mintcream
0xb533a665, 0x3e650000, 0x00ffe4e1, // mistyrose
0xb5e31867, 0x25c00000, 0x00ffe4b5, // moccasin
0xb8360a9f, 0x5d09a140, 0x00ffdead, // navajowhite
0xb836c800, 0x00000080, // navy
0xbd846047, 0x14000000, 0x00fdf5e6, // oldlace
0xbd89b140, 0x00808000, // olive
0xbd89b149, 0x48220000, 0x006b8e23, // olivedrab
0xbe4171ca, 0x00ffa500, // orange
0xbe4171cb, 0x48a40000, 0x00ff4500, // orangered
0xbe434248, 0x00da70d6, // orchid
0xc02c29df, 0x3085749f, 0x10000000, 0x00eee8aa, // palegoldenrod
0xc02c29e5, 0x14ae0000, 0x0098fb98, // palegreen
0xc02c2d2b, 0x4a357a67, 0x14000000, 0x00afeeee, // paleturquoise
0xc02c2d93, 0x3d85a48b, 0x10000000, 0x00db7093, // palevioletred
0xc0300e43, 0x5d098000, 0x00ffefd5, // papayawhip
0xc0a11a21, 0x54c60000, 0x00ffdab9, // peachpuff
0xc0b2a800, 0x00cd853f, // peru
0xc12e5800, 0x00ffc0cb, // pink
0xc1956800, 0x00dda0dd, // plum
0xc1f72165, 0x09952800, 0x00b0e0e6, // powderblue
0xc2b2830a, 0x00800080, // purple
0xc8a40000, 0x00ff0000, // red
0xc9f3c8a5, 0x3eee0000, 0x00bc8f8f, // rosybrown
0xc9f90b05, 0x32a50000, 0x004169e1, // royalblue
0xcc24230b, 0x0a4fbb80, 0x008b4513, // saddlebrown
0xcc2c6bdc, 0x00fa8072, // salmon
0xcc2e2645, 0x49f77000, 0x00f4a460, // sandybrown
0xcca13c8b, 0x15c00000, 0x002e8b57, // seagreen
0xcca19a0b, 0x31800000, 0x00fff5ee, // seashell
0xcd257382, 0x00a0522d, // sienna
0xcd2cb164, 0x00c0c0c0, // silver
0xcd79132b, 0x14000000, 0x0087ceeb, // skyblue
0xcd81a145, 0x32a50000, 0x006a5acd, // slateblue
0xcd81a14f, 0x48390000, 0x00708090, // slategray
0xcdcfb800, 0x00fffafa, // snow
0xce124b8f, 0x1e452b80, 0x0000ff7f, // springgreen
0xce852b05, 0x32a50000, 0x004682b4, // steelblue
0xd02e0000, 0x00d2b48c, // tan
0xd0a16000, 0x00008080, // teal
0xd1099d19, 0x14000000, 0x00d8bfd8, // thistle
0xd1ed0d1e, 0x00ff6347, // tomato
0xd2b28d5f, 0x26650000, 0x0040e0d0, // turquoise
0xd92f6168, 0x00ee82ee, // violet
0xdd050d00, 0x00f5deb3, // wheat
0xdd09a140, 0x00ffffff, // white
0xdd09a167, 0x35eb2800, 0x00f5f5f5, // whitesmoke
0xe4ac63ee, 0x00ffff00, // yellow
0xe4ac63ef, 0x1e452b80, 0x009acd32 // yellowgreen
}; // original = 2505 : replacement = 1616


const char* SkParse::FindNamedColor(const char* name, size_t len, SkColor* color) {
    const char* namePtr = name;
    unsigned int sixMatches[4];
    unsigned int* sixMatchPtr = sixMatches;
    bool first = true;
    bool last = false;
    char ch;
    do {
        unsigned int sixMatch = 0;
        for (int chIndex = 0; chIndex < 6; chIndex++) {
            sixMatch <<= 5;
            ch = *namePtr  | 0x20;
            if (ch < 'a' || ch > 'z')
                ch = 0;
            else {
                ch = ch - 'a' + 1;
                namePtr++;
            }
            sixMatch |= ch ;  // turn 'A' (0x41) into 'a' (0x61);
        }
        sixMatch <<= 1;
        sixMatch |= 1;
        if (first) {
            sixMatch |= 0x80000000;
            first = false;
        }
        ch = *namePtr | 0x20;
        last = ch < 'a' || ch > 'z';
        if (last)
            sixMatch &= ~1;
        len -= 6;
        *sixMatchPtr++ = sixMatch;
    } while (last == false && len > 0);
    const int colorNameSize = sizeof(gColorNames) / sizeof(unsigned int);
    int lo = 0;
    int hi = colorNameSize - 3; // back off to beginning of yellowgreen
    while (lo <= hi) {
        int mid = (hi + lo) >> 1;
        while ((int) gColorNames[mid] >= 0)
            --mid;
        sixMatchPtr = sixMatches;
        while (gColorNames[mid] == *sixMatchPtr) {
            ++mid;
            if ((*sixMatchPtr & 1) == 0) { // last
                *color = gColorNames[mid] | 0xFF000000;
                return namePtr;
            }
            ++sixMatchPtr;
        }
        int sixMask = *sixMatchPtr & ~0x80000000;
        int midMask = gColorNames[mid] & ~0x80000000;
        if (sixMask > midMask) {
            lo = mid + 2;   // skip color
            while ((int) gColorNames[lo] >= 0)
                ++lo;
        } else if (hi == mid)
            return NULL;
        else
            hi = mid;
    }
    return NULL;
}

// !!! move to char utilities
//static int count_separators(const char* str, const char* sep) {
//  char c;
//  int separators = 0;
//  while ((c = *str++) != '\0') {
//      if (strchr(sep, c) == NULL)
//          continue;
//      do {
//          if ((c = *str++) == '\0')
//              goto goHome;
//      } while (strchr(sep, c) != NULL);
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
        if (end == NULL)
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
            return NULL;
        }
//  } else if (strchr(value, ',')) {
//      SkScalar array[4];
//      int count = count_separators(value, ",") + 1; // !!! count commas, add 1
//      SkASSERT(count == 3 || count == 4);
//      array[0] = SK_Scalar1 * 255;
//      const char* end = SkParse::FindScalars(value, &array[4 - count], count);
//      if (end == NULL)
//          return NULL;
        // !!! range check for errors?
//      *colorPtr = SkColorSetARGB(SkScalarRound(array[0]), SkScalarRound(array[1]),
//          SkScalarRound(array[2]), SkScalarRound(array[3]));
//      return end;
    } else
        return FindNamedColor(value, strlen(value), colorPtr);
}

#ifdef SK_SUPPORT_UNITTEST
void SkParse::TestColor() {
    if (false)
        CreateTable();  // regenerates data table in the output window
    SkColor result;
    int index;
    for (index = 0; index < colorNamesSize; index++) {
        result = SK_ColorBLACK;
        SkNameRGB nameRGB = colorNames[index];
        SkASSERT(FindColor(nameRGB.name, &result) != NULL);
        SkASSERT(result == (SkColor) (nameRGB.rgb | 0xFF000000));
    }
    for (index = 0; index < colorNamesSize; index++) {
        result = SK_ColorBLACK;
        SkNameRGB nameRGB = colorNames[index];
        char bad[24];
        size_t len = strlen(nameRGB.name);
        memcpy(bad, nameRGB.name, len);
        bad[len - 1] -= 1;
        SkASSERT(FindColor(bad, &result) == false);
        bad[len - 1] += 2;
        SkASSERT(FindColor(bad, &result) == false);
    }
    result = SK_ColorBLACK;
    SkASSERT(FindColor("lightGrey", &result));
    SkASSERT(result == 0xffd3d3d3);
//  SkASSERT(FindColor("12,34,56,78", &result));
//  SkASSERT(result == ((12 << 24) | (34 << 16) | (56 << 8) | (78 << 0)));
    result = SK_ColorBLACK;
    SkASSERT(FindColor("#ABCdef", &result));
    SkASSERT(result == 0XFFABCdef);
    SkASSERT(FindColor("#12ABCdef", &result));
    SkASSERT(result == 0X12ABCdef);
    result = SK_ColorBLACK;
    SkASSERT(FindColor("#123", &result));
    SkASSERT(result == 0Xff112233);
    SkASSERT(FindColor("#abcd", &result));
    SkASSERT(result == 0Xaabbccdd);
    result = SK_ColorBLACK;
//  SkASSERT(FindColor("71,162,253", &result));
//  SkASSERT(result == ((0xFF << 24) | (71 << 16) | (162 << 8) | (253 << 0)));
}
#endif

