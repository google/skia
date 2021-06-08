/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skshaper/include/SkUnicodeProperties.h"
#include "src/utils/SkUTF.h"
#include "tests/Test.h"

DEF_TEST(skicu, r) {
    const char text[] = "This is one sentence. This is another.";
    constexpr size_t N = sizeof(text) - 1;  // don't count the final zero

    uint16_t text16[N];
    const int count = SkUTF::UTF8ToUTF16(text16, N, text, strlen(text));

    uint32_t properties[N];

    uint32_t requests = SkUnicodeProperties::kGraphemeBreak_Request   |
                        SkUnicodeProperties::kWordBreak_Request       |
                        SkUnicodeProperties::kLineBreak_Request       |
                        SkUnicodeProperties::kIsControl_Request       |
                        SkUnicodeProperties::kIsSpace_Request         |
                        SkUnicodeProperties::kIsWhiteSpace_Request;

    bool result = SkUnicodeProperties::ComputeProperties(SkMakeSpan(text16, count),
                                                         nullptr,
                                                         requests, properties);
    REPORTER_ASSERT(r, result);

    for (int i = 0; i < count; ++i) {
        SkDebugf("'%c' : %08X\n", text[i], properties[i]);
    }
}
