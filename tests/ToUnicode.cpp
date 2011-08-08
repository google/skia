
/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include <string>

#include "Test.h"
#include "SkData.h"
#include "SkPDFTypes.h"
#include "SkPDFFont.h"
#include "SkStream.h"

static bool stream_equals(const SkDynamicMemoryWStream& stream, size_t offset,
                          const void* buffer, size_t len) {
    SkAutoDataUnref data(stream.copyToData());
    if (offset + len > data.size()) {
        return false;
    }
    return memcmp(data.bytes() + offset, buffer, len) == 0;
}

void append_cmap_sections(const SkTDArray<SkUnichar>& glyphToUnicode,
                          const SkPDFGlyphSet* subset,
                          SkDynamicMemoryWStream* cmap);
static void TestToUnicode(skiatest::Reporter* reporter) {
    SkTDArray<SkUnichar> glyphToUnicode;
    SkTDArray<uint16_t> glyphsInSubset;
    SkPDFGlyphSet subset;

    glyphToUnicode.push(0);  // 0
    glyphToUnicode.push(0);  // 1
    glyphToUnicode.push(0);  // 2
    glyphsInSubset.push(3);
    glyphToUnicode.push(0x20);  // 3
    glyphsInSubset.push(4);
    glyphToUnicode.push(0x25);  // 4
    glyphsInSubset.push(5);
    glyphToUnicode.push(0x27);  // 5
    glyphsInSubset.push(6);
    glyphToUnicode.push(0x28);  // 6
    glyphsInSubset.push(7);
    glyphToUnicode.push(0x29);  // 7
    glyphsInSubset.push(8);
    glyphToUnicode.push(0x2F);  // 8
    glyphsInSubset.push(9);
    glyphToUnicode.push(0x33);  // 9
    glyphToUnicode.push(0);  // 10
    glyphsInSubset.push(11);
    glyphToUnicode.push(0x35);  // 11
    glyphsInSubset.push(12);
    glyphToUnicode.push(0x36);  // 12
    for (uint16_t i = 13; i < 0xFE; ++i) {
        glyphToUnicode.push(0);  // Zero from index 0x9 to 0xFD
    }
    glyphsInSubset.push(0xFE);
    glyphToUnicode.push(0x1010);
    glyphsInSubset.push(0xFF);
    glyphToUnicode.push(0x1011);
    glyphsInSubset.push(0x100);
    glyphToUnicode.push(0x1012);
    glyphsInSubset.push(0x101);
    glyphToUnicode.push(0x1013);

    SkDynamicMemoryWStream buffer;
    subset.set(glyphsInSubset.begin(), glyphsInSubset.count());
    append_cmap_sections(glyphToUnicode, &subset, &buffer);

    char expectedResult[] =
"4 beginbfchar\n\
<0003> <0020>\n\
<0004> <0025>\n\
<0008> <002F>\n\
<0009> <0033>\n\
endbfchar\n\
4 beginbfrange\n\
<0005> <0007> <0027>\n\
<000B> <000C> <0035>\n\
<00FE> <00FF> <1010>\n\
<0100> <0101> <1012>\n\
endbfrange\n";

    REPORTER_ASSERT(reporter, stream_equals(buffer, 0, expectedResult,
                                            buffer.getOffset()));
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("ToUnicode", ToUnicodeTestClass, TestToUnicode)
