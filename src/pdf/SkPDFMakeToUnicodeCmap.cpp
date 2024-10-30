/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/pdf/SkPDFMakeToUnicodeCmap.h"

#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkUTF.h"
#include "src/pdf/SkPDFGlyphUse.h"
#include "src/pdf/SkPDFUtils.h"

#include <algorithm>
#include <cstddef>
#include <vector>

using namespace skia_private;

static void append_tounicode_header(SkDynamicMemoryWStream* cmap,
                                    bool multibyte) {
    // 12 dict begin: 12 is an Adobe-suggested value. Shall not change.
    // It's there to prevent old version Adobe Readers from malfunctioning.
    const char* kHeader =
        "/CIDInit /ProcSet findresource begin\n"
        "12 dict begin\n"
        "begincmap\n";
    cmap->writeText(kHeader);

    // The /CIDSystemInfo must be consistent to the one in
    // SkPDFFont::populateCIDFont().
    // We can not pass over the system info object here because the format is
    // different. This is not a reference object.
    const char* kSysInfo =
        "/CIDSystemInfo\n"
        "<<  /Registry (Adobe)\n"
        "/Ordering (UCS)\n"
        "/Supplement 0\n"
        ">> def\n";
    cmap->writeText(kSysInfo);

    // The CMapName must be consistent to /CIDSystemInfo above.
    // /CMapType 2 means ToUnicode.
    // Codespace range just tells the PDF processor the valid range.
    const char* kTypeInfoHeader =
        "/CMapName /Adobe-Identity-UCS def\n"
        "/CMapType 2 def\n"
        "1 begincodespacerange\n";
    cmap->writeText(kTypeInfoHeader);
    if (multibyte) {
        cmap->writeText("<0000> <FFFF>\n");
    } else {
        cmap->writeText("<00> <FF>\n");
    }
    cmap->writeText("endcodespacerange\n");
}

static void append_cmap_footer(SkDynamicMemoryWStream* cmap) {
    const char kFooter[] =
        "endcmap\n"
        "CMapName currentdict /CMap defineresource pop\n"
        "end\n"
        "end";
    cmap->writeText(kFooter);
}

namespace {
struct BFChar {
    SkGlyphID fGlyphId;
    SkUnichar fUnicode;
};

struct BFRange {
    SkGlyphID fStart;
    SkGlyphID fEnd;
    SkUnichar fUnicode;
};
}  // namespace

static void write_glyph(SkDynamicMemoryWStream* cmap,
                        bool multiByte,
                        SkGlyphID gid) {
    if (multiByte) {
        SkPDFUtils::WriteUInt16BE(cmap, gid);
    } else {
        SkPDFUtils::WriteUInt8(cmap, SkToU8(gid));
    }
}

static void append_bfchar_section(const std::vector<BFChar>& bfchar,
                                  bool multiByte,
                                  SkDynamicMemoryWStream* cmap) {
    // PDF spec defines that every bf* list can have at most 100 entries.
    for (size_t i = 0; i < bfchar.size(); i += 100) {
        int count = SkToInt(bfchar.size() - i);
        count = std::min(count, 100);
        cmap->writeDecAsText(count);
        cmap->writeText(" beginbfchar\n");
        for (int j = 0; j < count; ++j) {
            cmap->writeText("<");
            write_glyph(cmap, multiByte, bfchar[i + j].fGlyphId);
            cmap->writeText("> <");
            SkPDFUtils::WriteUTF16beHex(cmap, bfchar[i + j].fUnicode);
            cmap->writeText(">\n");
        }
        cmap->writeText("endbfchar\n");
    }
}

static void append_bfchar_section_ex(const THashMap<SkGlyphID, SkString>& glyphToUnicodeEx,
                                     bool multiByte, SkGlyphID firstGlyphID, SkGlyphID lastGlyphID,
                                     SkDynamicMemoryWStream* cmap) {
    size_t glyphCount = 0;
    glyphToUnicodeEx.foreach([&](const SkGlyphID& glyphId, const SkString& a) {
        if (glyphId < firstGlyphID || lastGlyphID < glyphId) {
            return;
        }
        ++glyphCount;
    });

    int glyphOffset = 0;
    if (!multiByte) {
        glyphOffset = firstGlyphID - 1;
    }
    // PDF spec defines that every bf* list can have at most 100 entries.
    size_t i = 0;
    glyphToUnicodeEx.foreach([&](const SkGlyphID& glyphId, const SkString& a) {
        if (glyphId < firstGlyphID || lastGlyphID < glyphId) {
            return;
        }
        if (i % 100 == 0) {
            size_t count = glyphCount - i;
            count = std::min(count, SkToSizeT(100));
            cmap->writeDecAsText(count);
            cmap->writeText(" beginbfchar\n");
        }

        cmap->writeText("<");
        write_glyph(cmap, multiByte, glyphId - glyphOffset);
        cmap->writeText("> <");
        const char* textPtr = a.begin();
        const char* textEnd = a.end();
        while (textPtr < textEnd) {
            SkUnichar unichar = SkUTF::NextUTF8(&textPtr, textEnd);
            SkPDFUtils::WriteUTF16beHex(cmap, unichar);
        }
        cmap->writeText(">\n");

        if (i % 100 == 99 || i == glyphCount - 1) {
            cmap->writeText("endbfchar\n");
        }
        ++i;
    });
}

static void append_bfrange_section(const std::vector<BFRange>& bfrange,
                                   bool multiByte,
                                   SkDynamicMemoryWStream* cmap) {
    // PDF spec defines that every bf* list can have at most 100 entries.
    for (size_t i = 0; i < bfrange.size(); i += 100) {
        int count = SkToInt(bfrange.size() - i);
        count = std::min(count, 100);
        cmap->writeDecAsText(count);
        cmap->writeText(" beginbfrange\n");
        for (int j = 0; j < count; ++j) {
            cmap->writeText("<");
            write_glyph(cmap, multiByte, bfrange[i + j].fStart);
            cmap->writeText("> <");
            write_glyph(cmap, multiByte, bfrange[i + j].fEnd);
            cmap->writeText("> <");
            SkPDFUtils::WriteUTF16beHex(cmap, bfrange[i + j].fUnicode);
            cmap->writeText(">\n");
        }
        cmap->writeText("endbfrange\n");
    }
}

// Generate <bfchar> and <bfrange> table according to PDF spec 1.4 and Adobe
// Technote 5014.
// The function is not static so we can test it in unit tests.
//
// Current implementation guarantees bfchar and bfrange entries do not overlap.
//
// Current implementation does not attempt aggressive optimizations against
// following case because the specification is not clear.
//
// 4 beginbfchar          1 beginbfchar
// <0003> <0013>          <0020> <0014>
// <0005> <0015>    to    endbfchar
// <0007> <0017>          1 beginbfrange
// <0020> <0014>          <0003> <0007> <0013>
// endbfchar              endbfrange
//
// Adobe Technote 5014 said: "Code mappings (unlike codespace ranges) may
// overlap, but succeeding maps supersede preceding maps."
//
// In case of searching text in PDF, bfrange will have higher precedence so
// typing char id 0x0014 in search box will get glyph id 0x0004 first.  However,
// the spec does not mention how will this kind of conflict being resolved.
//
// For the worst case (having 65536 continuous unicode and we use every other
// one of them), the possible savings by aggressive optimization is 416KB
// pre-compressed and does not provide enough motivation for implementation.
void SkPDFAppendCmapSections(const SkUnichar* glyphToUnicode,
                             const THashMap<SkGlyphID, SkString>& glyphToUnicodeEx,
                             const SkPDFGlyphUse* subset,
                             SkDynamicMemoryWStream* cmap,
                             bool multiByteGlyphs,
                             SkGlyphID firstGlyphID,
                             SkGlyphID lastGlyphID) {
    int glyphOffset = 0;
    if (!multiByteGlyphs) {
        glyphOffset = firstGlyphID - 1;
    }

    std::vector<BFChar> bfcharEntries;
    std::vector<BFRange> bfrangeEntries;

    BFRange currentRangeEntry = {0, 0, 0};
    bool rangeEmpty = true;
    const int limit = (int)lastGlyphID + 1 - glyphOffset;

    for (int i = firstGlyphID - glyphOffset; i < limit + 1; ++i) {
        SkGlyphID gid = i + glyphOffset;
        bool inSubset = i < limit && (subset == nullptr || subset->has(gid));
        if (!rangeEmpty) {
            // PDF spec requires bfrange not changing the higher byte,
            // e.g. <1035> <10FF> <2222> is ok, but
            //      <1035> <1100> <2222> is no good
            bool inRange =
                i == currentRangeEntry.fEnd + 1 &&
                i >> 8 == currentRangeEntry.fStart >> 8 &&
                i < limit &&
                glyphToUnicode[gid] ==
                    currentRangeEntry.fUnicode + i - currentRangeEntry.fStart;
            if (!inSubset || !inRange) {
                if (currentRangeEntry.fEnd > currentRangeEntry.fStart) {
                    bfrangeEntries.push_back(currentRangeEntry);
                } else {
                    bfcharEntries.push_back({currentRangeEntry.fStart, currentRangeEntry.fUnicode});
                }
                rangeEmpty = true;
            }
        }
        if (inSubset) {
            currentRangeEntry.fEnd = i;
            if (rangeEmpty) {
              currentRangeEntry.fStart = i;
              currentRangeEntry.fUnicode = glyphToUnicode[gid];
              rangeEmpty = false;
            }
        }
    }

    // The spec requires all bfchar entries for a font must come before bfrange
    // entries.
    append_bfchar_section(bfcharEntries, multiByteGlyphs, cmap);
    append_bfchar_section_ex(glyphToUnicodeEx, multiByteGlyphs, firstGlyphID, lastGlyphID, cmap);
    append_bfrange_section(bfrangeEntries, multiByteGlyphs, cmap);
}

std::unique_ptr<SkStreamAsset> SkPDFMakeToUnicodeCmap(
        const SkUnichar* glyphToUnicode,
        const THashMap<SkGlyphID, SkString>& glyphToUnicodeEx,
        const SkPDFGlyphUse* subset,
        bool multiByteGlyphs,
        SkGlyphID firstGlyphID,
        SkGlyphID lastGlyphID) {
    SkDynamicMemoryWStream cmap;
    append_tounicode_header(&cmap, multiByteGlyphs);
    SkPDFAppendCmapSections(glyphToUnicode, glyphToUnicodeEx, subset, &cmap, multiByteGlyphs,
                            firstGlyphID, lastGlyphID);
    append_cmap_footer(&cmap);
    return cmap.detachAsStream();
}
