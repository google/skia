// Copyright 2021 Google LLC.
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkEncodedImageFormat.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkImageEncoder.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include "experimental/sktext/include/Text.h"

#include <string.h>
#include <algorithm>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

struct GrContextOptions;

#define VeryLongCanvasWidth 1000000
#define TestCanvasWidth 1000
#define TestCanvasHeight 600

using namespace skia::text;

namespace {
    bool operator==(SkSpan<const char16_t> a, SkSpan<const char16_t> b) {
        if (a.size() != b.size()) {
            return false;
        }
        for (size_t i = 0; i < a.size(); ++i) {
            if (a[i] != b[i]) {
                return false;
            }
        }
        return true;
    }
}

UNIX_ONLY_TEST(SkText_UnicodeText_Flags, reporter) {
    REPORTER_ASSERT(reporter, true);
    //                       01234567890  1234567890
    std::u16string utf16(u"Hello word\nHello world");
    SkString utf8("Hello word\nHello world");
    UnicodeText unicodeText16(SkUnicode::Make(), SkSpan<uint16_t>((uint16_t*)utf16.data(), utf16.size()));
    UnicodeText unicodeText8(SkUnicode::Make(), utf8);

    REPORTER_ASSERT(reporter, unicodeText16.getText16() == unicodeText8.getText16(), "UTF16 and UTF8 texts should be the same\n");
    auto lineBreak = utf16.find_first_of(u"\n");
    for (size_t i = 0; i < unicodeText16.getText16().size(); ++i) {
        if (i == lineBreak) {
            REPORTER_ASSERT(reporter, unicodeText16.hasProperty(i, CodeUnitFlags::kHardLineBreakBefore), "Pos16 %d should point to hard line break\n", lineBreak);
            REPORTER_ASSERT(reporter, unicodeText8 .hasProperty(i, CodeUnitFlags::kHardLineBreakBefore), "Pos8 %d should point to hard line break\n", lineBreak);
        } else {
            REPORTER_ASSERT(reporter, unicodeText16.hasProperty(i, CodeUnitFlags::kGraphemeStart), "Pos16 %d should be a grapheme start\n", i);
            REPORTER_ASSERT(reporter, unicodeText8 .hasProperty(i, CodeUnitFlags::kGraphemeStart), "Pos8 %d should be a grapheme start\n", i);
        }
    }

    auto space1 = utf16.find_first_of(u" ");
    auto space2 = utf16.find_last_of(u" ");

    REPORTER_ASSERT(reporter, unicodeText16.hasProperty(space1, CodeUnitFlags::kPartOfWhiteSpace), "Pos16 %d should be a part of whitespaces\n", space1);
    REPORTER_ASSERT(reporter, unicodeText16.hasProperty(space1 + 1, CodeUnitFlags::kSoftLineBreakBefore), "Pos16 %d should have soft line break before\n", space1 + 1);
    REPORTER_ASSERT(reporter, unicodeText16.hasProperty(space2, CodeUnitFlags::kPartOfWhiteSpace), "Pos16 %d should be a part of whitespaces\n", space2);
    REPORTER_ASSERT(reporter, unicodeText16.hasProperty(space2 + 1, CodeUnitFlags::kSoftLineBreakBefore), "Pos16 %d should have soft line break before\n", space2 + 1);

    REPORTER_ASSERT(reporter, unicodeText8 .hasProperty(space1, CodeUnitFlags::kPartOfWhiteSpace), "Pos8 %d should be a part of whitespaces\n", space1);
    REPORTER_ASSERT(reporter, unicodeText8 .hasProperty(space1 + 1, CodeUnitFlags::kSoftLineBreakBefore), "Pos8 %d should have soft line break before\n", space1 + 1);
    REPORTER_ASSERT(reporter, unicodeText8 .hasProperty(space2, CodeUnitFlags::kPartOfWhiteSpace), "Pos8 %d should be a part of whitespaces\n", space2);
    REPORTER_ASSERT(reporter, unicodeText8 .hasProperty(space2 + 1, CodeUnitFlags::kSoftLineBreakBefore), "Pos8 %d should have soft line break before\n", space2 + 1);
}

// TODO: Test RTL text
