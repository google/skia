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
    enum Properties {
        kBidiLevelMask          = (1 << 6) - 1,

        kWhiteSpaceBreak        = 1 << 6,
        kWordBreak              = 1 << 7,
        kSentenceBreak          = 1 << 8,
        kSoftLineBreak          = 1 << 9,
        kHardLineBreak          = 1 << 10,
        kPartOfIntraWordBreak   = 1 << 11,

        kGraphemeStart          = 1 << 12,
        kIsControl              = 1 << 13,
        kIsSpace                = 1 << 14,
    };

    static bool ComputeProperties(SkSpan<const int32_t> unichars,
                                  uint32_t requestedProperties,
                                  uint32_t outProperties[]);
};

#endif
