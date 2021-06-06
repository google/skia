/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkICU_DEFINED
#define SkICU_DEFINED

#include "include/core/SkSpan.h"

class SkICU {
public:
    enum Requests {
        kBidiLevel_Request      = 1 << 0,

        kGraphemeBreak_Request  = 1 << 1,
        kWordBreak_Request      = 1 << 1,
        kLineBreak_Request      = 1 << 2,
        kSentenceBreak_Request  = 1 << 3,

        kIsControl_Request      = 1 << 4,
        kIsSpace_Request        = 1 << 5,
        kIsWhiteSpace_Request   = 1 << 6,
    };

    enum Properties {
        kBidiLevelMask          = (1 << 6) - 1,

        kGraphemeBreak          = 1 << 6,
        kWordBreak              = 1 << 7,
        kSentenceBreak          = 1 << 8,
        kSoftLineBreak          = 1 << 9,
        kHardLineBreak          = 1 << 10,

        kIsControl              = 1 << 11,
        kIsSpace                = 1 << 12,
        kIsWhiteSpace           = 1 << 13,
    };

    static bool ComputeProperties(SkSpan<const uint16_t> utf16,
                                  uint32_t requests,
                                  SkSpan<uint32_t> properties);
};

#endif
