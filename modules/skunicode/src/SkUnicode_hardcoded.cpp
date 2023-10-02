/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkDebug.h"
#include "modules/skunicode/src/SkUnicode_hardcoded.h"
#include <algorithm>
#include <array>
#include <utility>

bool SkUnicodeHardCodedCharProperties::isControl(SkUnichar utf8) {
    return (utf8 < ' ') || (utf8 >= 0x7f && utf8 <= 0x9f) ||
           (utf8 >= 0x200D && utf8 <= 0x200F) ||
           (utf8 >= 0x202A && utf8 <= 0x202E);
}

bool SkUnicodeHardCodedCharProperties::isWhitespace(SkUnichar unichar) {
    static constexpr std::array<SkUnichar, 21> whitespaces {
            0x0009, // character tabulation
            0x000A, // line feed
            0x000B, // line tabulation
            0x000C, // form feed
            0x000D, // carriage return
            0x0020, // space
          //0x0085, // next line
          //0x00A0, // no-break space
            0x1680, // ogham space mark
            0x2000, // en quad
            0x2001, // em quad
            0x2002, // en space
            0x2003, // em space
            0x2004, // three-per-em space
            0x2005, // four-per-em space
            0x2006, // six-per-em space
          //0x2007, // figure space
            0x2008, // punctuation space
            0x2009, // thin space
            0x200A, // hair space
            0x2028, // line separator
            0x2029, // paragraph separator
          //0x202F, // narrow no-break space
            0x205F, // medium mathematical space
            0x3000};// ideographic space
    return std::find(whitespaces.begin(), whitespaces.end(), unichar) != whitespaces.end();
}

bool SkUnicodeHardCodedCharProperties::isSpace(SkUnichar unichar) {
    static constexpr std::array<SkUnichar, 25> spaces {
            0x0009, // character tabulation
            0x000A, // line feed
            0x000B, // line tabulation
            0x000C, // form feed
            0x000D, // carriage return
            0x0020, // space
            0x0085, // next line
            0x00A0, // no-break space
            0x1680, // ogham space mark
            0x2000, // en quad
            0x2001, // em quad
            0x2002, // en space
            0x2003, // em space
            0x2004, // three-per-em space
            0x2005, // four-per-em space
            0x2006, // six-per-em space
            0x2007, // figure space
            0x2008, // punctuation space
            0x2009, // thin space
            0x200A, // hair space
            0x2028, // line separator
            0x2029, // paragraph separator
            0x202F, // narrow no-break space
            0x205F, // medium mathematical space
            0x3000}; // ideographic space
    return std::find(spaces.begin(), spaces.end(), unichar) != spaces.end();
}

bool SkUnicodeHardCodedCharProperties::isTabulation(SkUnichar utf8) {
    return utf8 == '\t';
}

bool SkUnicodeHardCodedCharProperties::isHardBreak(SkUnichar utf8) {
    return utf8 == '\n' || utf8 == u'\u2028';
}

bool SkUnicodeHardCodedCharProperties::isEmoji(SkUnichar unichar) {
    SkDEBUGFAIL("Not implemented");
    return false;
}

bool SkUnicodeHardCodedCharProperties::isIdeographic(SkUnichar unichar) {
    static constexpr std::array<std::pair<SkUnichar, SkUnichar>, 8> ranges {{
          {4352,   4607}, // Hangul Jamo
          {11904, 42191}, // CJK_Radicals
          {43072, 43135}, // Phags_Pa
          {44032, 55215}, // Hangul_Syllables
          {63744, 64255}, // CJK_Compatibility_Ideographs
          {65072, 65103}, // CJK_Compatibility_Forms
          {65381, 65500}, // Katakana_Hangul_Halfwidth
          {131072, 196607}// Supplementary_Ideographic_Plane
    }};
    for (auto range : ranges) {
        if (range.first <= unichar && range.second > unichar) {
            return true;
        }
    }
    return false;
}
