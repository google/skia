// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "include/core/SkTypes.h"
#include "src/base/SkUTF.h"
#include "tests/Test.h"

#include <array>
#include <cstdint>
#include <cstddef>
#include <cstring>

DEF_TEST(SkUTF_UTF16, reporter) {
    // Test non-basic-multilingual-plane unicode.
    static const SkUnichar gUni[] = {
        0x10000, 0x18080, 0x20202, 0xFFFFF, 0x101234
    };
    for (SkUnichar uni : gUni) {
        uint16_t buf[2];
        size_t count = SkUTF::ToUTF16(uni, buf);
        REPORTER_ASSERT(reporter, count == 2);
        size_t count2 = SkUTF::CountUTF16(buf, sizeof(buf));
        REPORTER_ASSERT(reporter, count2 == 1);
        const uint16_t* ptr = buf;
        SkUnichar c = SkUTF::NextUTF16(&ptr, buf + std::size(buf));
        REPORTER_ASSERT(reporter, c == uni);
        REPORTER_ASSERT(reporter, ptr - buf == 2);
    }
}

DEF_TEST(SkUTF_UTF8, reporter) {
    static const struct {
        const char* fUtf8;
        SkUnichar   fUni;
    } gTest[] = {
        { "a",                  'a' },
        { "\x7f",               0x7f },
        { "\xC2\x80",           0x80 },
        { "\xC3\x83",           (3 << 6) | 3    },
        { "\xDF\xBF",           0x7ff },
        { "\xE0\xA0\x80",       0x800 },
        { "\xE0\xB0\xB8",       0xC38 },
        { "\xE3\x83\x83",       (3 << 12) | (3 << 6) | 3    },
        { "\xEF\xBF\xBF",       0xFFFF },
        { "\xF0\x90\x80\x80",   0x10000 },
        { "\xF3\x83\x83\x83",   (3 << 18) | (3 << 12) | (3 << 6) | 3    }
    };
    for (auto test : gTest) {
        const char* p = test.fUtf8;
        const char* stop = p + strlen(p);
        int         n = SkUTF::CountUTF8(p, strlen(p));
        SkUnichar   u1 = SkUTF::NextUTF8(&p, stop);

        REPORTER_ASSERT(reporter, n == 1);
        REPORTER_ASSERT(reporter, u1 == test.fUni);
        REPORTER_ASSERT(reporter, p - test.fUtf8 == (int)strlen(test.fUtf8));
    }
}

#define ASCII_BYTE         "X"
#define CONTINUATION_BYTE  "\xA1"
#define LEADING_TWO_BYTE   "\xC2"
#define LEADING_THREE_BYTE "\xE1"
#define LEADING_FOUR_BYTE  "\xF0"
#define INVALID_BYTE       "\xFC"
DEF_TEST(SkUTF_CountUTF8, r) {
    static const struct {
        int expectedCount;
        const char* utf8String;
    } testCases[] = {
        { 0, "" },
        { 1, ASCII_BYTE },
        { 2, ASCII_BYTE ASCII_BYTE },
        { 1, LEADING_TWO_BYTE CONTINUATION_BYTE },
        { 2, ASCII_BYTE LEADING_TWO_BYTE CONTINUATION_BYTE },
        { 3, ASCII_BYTE ASCII_BYTE LEADING_TWO_BYTE CONTINUATION_BYTE },
        { 1, LEADING_THREE_BYTE CONTINUATION_BYTE CONTINUATION_BYTE },
        { 2, ASCII_BYTE LEADING_THREE_BYTE CONTINUATION_BYTE CONTINUATION_BYTE },
        { 3, ASCII_BYTE ASCII_BYTE LEADING_THREE_BYTE CONTINUATION_BYTE CONTINUATION_BYTE },
        { 1, LEADING_FOUR_BYTE CONTINUATION_BYTE CONTINUATION_BYTE CONTINUATION_BYTE },
        { 2, ASCII_BYTE LEADING_FOUR_BYTE CONTINUATION_BYTE CONTINUATION_BYTE CONTINUATION_BYTE },
        { 3, ASCII_BYTE ASCII_BYTE LEADING_FOUR_BYTE CONTINUATION_BYTE CONTINUATION_BYTE
             CONTINUATION_BYTE },
        { -1, INVALID_BYTE },
        { -1, INVALID_BYTE CONTINUATION_BYTE },
        { -1, INVALID_BYTE CONTINUATION_BYTE CONTINUATION_BYTE },
        { -1, INVALID_BYTE CONTINUATION_BYTE CONTINUATION_BYTE CONTINUATION_BYTE },
        { -1, LEADING_TWO_BYTE },
        { -1, CONTINUATION_BYTE },
        { -1, CONTINUATION_BYTE CONTINUATION_BYTE },
        { -1, LEADING_THREE_BYTE CONTINUATION_BYTE },
        { -1, CONTINUATION_BYTE CONTINUATION_BYTE CONTINUATION_BYTE },
        { -1, LEADING_FOUR_BYTE CONTINUATION_BYTE },
        { -1, CONTINUATION_BYTE CONTINUATION_BYTE CONTINUATION_BYTE CONTINUATION_BYTE },
    };
    for (auto testCase : testCases) {
        const char* str = testCase.utf8String;
        REPORTER_ASSERT(r, testCase.expectedCount == SkUTF::CountUTF8(str, strlen(str)));
    }
}

DEF_TEST(SkUTF_NextUTF8_ToUTF8, r) {
    struct {
        SkUnichar expected;
        const char* utf8String;
    } testCases[] = {
        { -1, INVALID_BYTE },
        { -1, "" },
        { 0x0058, ASCII_BYTE },
        { 0x00A1, LEADING_TWO_BYTE CONTINUATION_BYTE },
        { 0x1861, LEADING_THREE_BYTE CONTINUATION_BYTE CONTINUATION_BYTE },
        { 0x010330, LEADING_FOUR_BYTE "\x90\x8C\xB0" },
    };
    for (auto testCase : testCases) {
        const char* str = testCase.utf8String;
        SkUnichar uni = SkUTF::NextUTF8(&str, str + strlen(str));
        REPORTER_ASSERT(r, str == testCase.utf8String + strlen(testCase.utf8String));
        REPORTER_ASSERT(r, uni == testCase.expected);
        char buff[5] = {0, 0, 0, 0, 0};
        size_t len = SkUTF::ToUTF8(uni, buff);
        if (buff[len] != 0) {
            ERRORF(r, "unexpected write");
            continue;
        }
        if (uni == -1) {
            REPORTER_ASSERT(r, len == 0);
            continue;
        }
        if (len == 0) {
           ERRORF(r, "unexpected failure.");
           continue;
        }
        if (len > 4) {
           ERRORF(r, "wrote too much");
           continue;
        }
        str = testCase.utf8String;
        REPORTER_ASSERT(r, len == strlen(buff));
        REPORTER_ASSERT(r, len == strlen(str));
        REPORTER_ASSERT(r, 0 == strcmp(str, buff));
    }
}
#undef ASCII_BYTE
#undef CONTINUATION_BYTE
#undef LEADING_TWO_BYTE
#undef LEADING_THREE_BYTE
#undef LEADING_FOUR_BYTE
#undef INVALID_BYTE
