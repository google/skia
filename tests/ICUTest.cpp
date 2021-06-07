/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skshaper/include/SkICU.h"
#include "src/utils/SkUTF.h"
#include "tests/Test.h"

DEF_TEST(skicu, r) {
    const char text[] = "This is one sentence. This is another.";
    constexpr size_t N = sizeof(text) - 1;  // don't count the final zero

    uint16_t text16[N];
    const int count = SkUTF::UTF8ToUTF16(text16, N, text, strlen(text));

    uint32_t properties[N];

    uint32_t requests = SkICU::kGraphemeBreak_Request   |
                        SkICU::kWordBreak_Request       |
                        SkICU::kLineBreak_Request       |
                        SkICU::kSentenceBreak_Request   |
                        SkICU::kIsControl_Request       |
                        SkICU::kIsSpace_Request         |
                        SkICU::kIsWhiteSpace_Request;

    bool result = SkICU::ComputeProperties(SkMakeSpan(text16, count), requests, properties);
    REPORTER_ASSERT(r, result);

    for (int i = 0; i < count; ++i) {
        SkDebugf("'%c' : %08X\n", text[i], properties[i]);
    }
}
