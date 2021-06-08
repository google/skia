/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkICU_DEFINED
#define SkICU_DEFINED

#include "include/core/SkSpan.h"

class SkUnicodeProperties {
public:
    /**
     *  Given a block of utf16 text, invoke a callback for each range in the text that can be
     *  identified with a BCP 47 Language tag.
     */
    static void FindScriptRanges(SkSpan<const uint16_t> utf16,
                                 void (*)(size_t fromIndex, size_t toIndex, const char bcp47[],
                                          void* ctx),
                                 void* ctx);


    enum Requests {
        kBidiLevel_Request      = 1 << 0,

        kGraphemeBreak_Request  = 1 << 1,
        kWordBreak_Request      = 1 << 2,   // returns Word and IntraWord break properties
        kLineBreak_Request      = 1 << 3,   // returns Soft and Hard linebreak properties

        kIsControl_Request      = 1 << 4,
        kIsSpace_Request        = 1 << 5,
        kIsWhiteSpace_Request   = 1 << 6,
        // future: isLetter? isDigit?
    };

    enum Properties {
        kBidiLevelBits          = 5,
        kBidiLevelMask          = (1 << kBidiLevelBits) - 1,

        kGraphemeBreak          = 1 << 5,
        kIntraWordBreak         = 1 << 6,
        kWordBreak              = 1 << 7,
        kSoftLineBreak          = 1 << 8,
        kHardLineBreak          = 1 << 9,

        kIsControl              = 1 << 10,
        kIsSpace                = 1 << 11,
        kIsWhiteSpace           = 1 << 12,
    };

    /**
     *  Compute the requested properties for the string of utf16 text.
     *  Each 16bit value has a corresonding 32bi property value, though the 2nd
     *  half of a surrogate-pair will have properties of 0.
     */
    static bool ComputeProperties(SkSpan<const uint16_t> utf16,
                                  const char bcp47[],           // null if unknown
                                  uint32_t requests,
                                  uint32_t properties[]);
};

#endif
