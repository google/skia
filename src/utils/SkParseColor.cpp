/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "include/utils/SkParse.h"

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
            return nullptr;
        else
            hi = mid;
    }
    return nullptr;
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
