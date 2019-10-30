/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#ifdef SK_SUPPORT_PDF

#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "include/private/SkTo.h"
#include "src/pdf/SkPDFMakeToUnicodeCmap.h"
#include "src/utils/SkBitSet.h"

static constexpr SkGlyphID kMaximumGlyphIndex = UINT16_MAX;

static bool stream_equals(const SkDynamicMemoryWStream& stream, size_t offset,
                          const char* buffer, size_t len) {
    if (len != strlen(buffer)) {
        return false;
    }

    const size_t streamSize = stream.bytesWritten();

    if (offset + len > streamSize) {
        return false;
    }

    SkAutoTMalloc<char> data(streamSize);
    stream.copyTo(data.get());
    return memcmp(data.get() + offset, buffer, len) == 0;
}

DEF_TEST(SkPDF_ToUnicode, reporter) {
    SkTDArray<SkUnichar> glyphToUnicode;
    SkTDArray<uint16_t> glyphsInSubset;
    SkPDFGlyphUse subset(1, kMaximumGlyphIndex);

    glyphToUnicode.push_back(0);  // 0
    glyphToUnicode.push_back(0);  // 1
    glyphToUnicode.push_back(0);  // 2
    glyphsInSubset.push_back(3);
    glyphToUnicode.push_back(0x20);  // 3
    glyphsInSubset.push_back(4);
    glyphToUnicode.push_back(0x25);  // 4
    glyphsInSubset.push_back(5);
    glyphToUnicode.push_back(0x27);  // 5
    glyphsInSubset.push_back(6);
    glyphToUnicode.push_back(0x28);  // 6
    glyphsInSubset.push_back(7);
    glyphToUnicode.push_back(0x29);  // 7
    glyphsInSubset.push_back(8);
    glyphToUnicode.push_back(0x2F);  // 8
    glyphsInSubset.push_back(9);
    glyphToUnicode.push_back(0x33);  // 9
    glyphToUnicode.push_back(0);  // 10
    glyphsInSubset.push_back(11);
    glyphToUnicode.push_back(0x35);  // 11
    glyphsInSubset.push_back(12);
    glyphToUnicode.push_back(0x36);  // 12
    glyphsInSubset.push_back(13);
    glyphToUnicode.push_back(0x37);  // 13
    for (uint16_t i = 14; i < 0xFE; ++i) {
        glyphToUnicode.push_back(0);  // Zero from index 0x9 to 0xFD
    }
    glyphsInSubset.push_back(0xFE);
    glyphToUnicode.push_back(0x1010);
    glyphsInSubset.push_back(0xFF);
    glyphToUnicode.push_back(0x1011);
    glyphsInSubset.push_back(0x100);
    glyphToUnicode.push_back(0x1012);
    glyphsInSubset.push_back(0x101);
    glyphToUnicode.push_back(0x1013);

    SkGlyphID lastGlyphID = SkToU16(glyphToUnicode.count() - 1);

    SkDynamicMemoryWStream buffer;
    for (uint16_t v : glyphsInSubset) {
        subset.set(v);
    }
    SkPDFAppendCmapSections(&glyphToUnicode[0], &subset, &buffer, true, 0,
                            SkTMin<SkGlyphID>(0xFFFF,  lastGlyphID));

    char expectedResult[] =
"4 beginbfchar\n\
<0003> <0020>\n\
<0004> <0025>\n\
<0008> <002F>\n\
<0009> <0033>\n\
endbfchar\n\
4 beginbfrange\n\
<0005> <0007> <0027>\n\
<000B> <000D> <0035>\n\
<00FE> <00FF> <1010>\n\
<0100> <0101> <1012>\n\
endbfrange\n";

    REPORTER_ASSERT(reporter, stream_equals(buffer, 0, expectedResult,
                                            buffer.bytesWritten()));

    // Remove characters and ranges.
    buffer.reset();

    SkPDFAppendCmapSections(&glyphToUnicode[0], &subset, &buffer, true, 8,
                            SkTMin<SkGlyphID>(0x00FF, lastGlyphID));

    char expectedResultChop1[] =
"2 beginbfchar\n\
<0008> <002F>\n\
<0009> <0033>\n\
endbfchar\n\
2 beginbfrange\n\
<000B> <000D> <0035>\n\
<00FE> <00FF> <1010>\n\
endbfrange\n";

    REPORTER_ASSERT(reporter, stream_equals(buffer, 0, expectedResultChop1,
                                            buffer.bytesWritten()));

    // Remove characters from range to downdrade it to one char.
    buffer.reset();

    SkPDFAppendCmapSections(&glyphToUnicode[0], &subset, &buffer, true, 0x00D,
                            SkTMin<SkGlyphID>(0x00FE, lastGlyphID));

    char expectedResultChop2[] =
"2 beginbfchar\n\
<000D> <0037>\n\
<00FE> <1010>\n\
endbfchar\n";

    REPORTER_ASSERT(reporter, stream_equals(buffer, 0, expectedResultChop2,
                                            buffer.bytesWritten()));

    buffer.reset();

    SkPDFAppendCmapSections(&glyphToUnicode[0], nullptr, &buffer, false, 0xFC,
                            SkTMin<SkGlyphID>(0x110, lastGlyphID));

    char expectedResultSingleBytes[] =
"2 beginbfchar\n\
<01> <0000>\n\
<02> <0000>\n\
endbfchar\n\
1 beginbfrange\n\
<03> <06> <1010>\n\
endbfrange\n";

    REPORTER_ASSERT(reporter, stream_equals(buffer, 0,
                                            expectedResultSingleBytes,
                                            buffer.bytesWritten()));

    glyphToUnicode.reset();
    glyphsInSubset.reset();
    SkPDFGlyphUse subset2(1, kMaximumGlyphIndex);

    // Test mapping:
    //           I  n  s  t  a  l
    // Glyph id 2c 51 56 57 44 4f
    // Unicode  49 6e 73 74 61 6c
    for (SkUnichar i = 0; i < 100; ++i) {
      glyphToUnicode.push_back(i + 29);
    }
    lastGlyphID = SkToU16(glyphToUnicode.count() - 1);

    glyphsInSubset.push_back(0x2C);
    glyphsInSubset.push_back(0x44);
    glyphsInSubset.push_back(0x4F);
    glyphsInSubset.push_back(0x51);
    glyphsInSubset.push_back(0x56);
    glyphsInSubset.push_back(0x57);

    SkDynamicMemoryWStream buffer2;
    for (uint16_t v : glyphsInSubset) {
        subset2.set(v);
    }
    SkPDFAppendCmapSections(&glyphToUnicode[0], &subset2, &buffer2, true, 0,
                            SkTMin<SkGlyphID>(0xFFFF, lastGlyphID));

    char expectedResult2[] =
"4 beginbfchar\n\
<002C> <0049>\n\
<0044> <0061>\n\
<004F> <006C>\n\
<0051> <006E>\n\
endbfchar\n\
1 beginbfrange\n\
<0056> <0057> <0073>\n\
endbfrange\n";

    REPORTER_ASSERT(reporter, stream_equals(buffer2, 0, expectedResult2,
                                            buffer2.bytesWritten()));
}

#endif
