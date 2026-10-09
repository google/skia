/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkDebug.h"
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
    if (unichar == '#' || unichar == '*' || (unichar >= '0' && unichar <= '9')) {
        return true;
    }
    static constexpr std::pair<SkUnichar, SkUnichar> emojiRanges[] = {
        {0x203C, 0x203C}, {0x2049, 0x2049}, {0x2122, 0x2122}, {0x2139, 0x2139},
        {0x2194, 0x2199}, {0x21A9, 0x21AA}, {0x2300, 0x23FF}, {0x25AA, 0x25AB},
        {0x25B6, 0x25B6}, {0x25C0, 0x25C0}, {0x25FB, 0x25FE}, {0x2600, 0x27BF},
        {0x2934, 0x2935}, {0x2B05, 0x2B07}, {0x2B1B, 0x2B1C}, {0x2B50, 0x2B50},
        {0x2B55, 0x2B55}, {0x3030, 0x3030}, {0x303D, 0x303D}, {0x3297, 0x3297},
        {0x3299, 0x3299}, {0x1F000, 0x1F02F}, {0x1F0A0, 0x1F0FF}, {0x1F100, 0x1F64F},
        {0x1F680, 0x1F6FF}, {0x1F700, 0x1F77F}, {0x1F780, 0x1F7FF}, {0x1F800, 0x1F8FF},
        {0x1F900, 0x1FAFF}
    };
    for (const auto& r : emojiRanges) {
        if (unichar >= r.first && unichar <= r.second) {
            return true;
        }
    }
    return false;
}

bool SkUnicodeHardCodedCharProperties::isEmojiComponent(SkUnichar unichar) {
    return (unichar >= 0x1F3FB && unichar <= 0x1F3FF) ||
           (unichar >= 0x1F1E6 && unichar <= 0x1F1FF) ||
           unichar == 0x200D || unichar == 0xFE0F;
}

bool SkUnicodeHardCodedCharProperties::isEmojiModifier(SkUnichar unichar) {
    return unichar >= 0x1F3FB && unichar <= 0x1F3FF;
}

bool SkUnicodeHardCodedCharProperties::isEmojiModifierBase(SkUnichar unichar) {
    return this->isEmoji(unichar) && !this->isEmojiModifier(unichar);
}

bool SkUnicodeHardCodedCharProperties::isRegionalIndicator(SkUnichar unichar) {
    return unichar >= 0x1F1E6 && unichar <= 0x1F1FF;
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
