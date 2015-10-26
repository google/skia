/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <ctype.h>

#include "SkData.h"
#include "SkGlyphCache.h"
#include "SkPaint.h"
#include "SkPDFCanon.h"
#include "SkPDFDevice.h"
#include "SkPDFFont.h"
#include "SkPDFFontImpl.h"
#include "SkPDFStream.h"
#include "SkPDFTypes.h"
#include "SkPDFUtils.h"
#include "SkRefCnt.h"
#include "SkScalar.h"
#include "SkStream.h"
#include "SkTypefacePriv.h"
#include "SkTypes.h"
#include "SkUtils.h"

#if defined (GOOGLE3)
    #if !defined (SK_BUILD_FOR_ANDROID)
        // #including #defines doesn't work with this build system.
        #include "typography/font/sfntly/src/sample/chromium/font_subsetter.h"
        #define SK_SFNTLY_SUBSETTER  // For the benefit of #ifdefs below.
    #endif
#elif defined (SK_SFNTLY_SUBSETTER)
    #include SK_SFNTLY_SUBSETTER
#endif

// PDF's notion of symbolic vs non-symbolic is related to the character set, not
// symbols vs. characters.  Rarely is a font the right character set to call it
// non-symbolic, so always call it symbolic.  (PDF 1.4 spec, section 5.7.1)
static const int kPdfSymbolic = 4;

namespace {

///////////////////////////////////////////////////////////////////////////////
// File-Local Functions
///////////////////////////////////////////////////////////////////////////////

bool parsePFBSection(const uint8_t** src, size_t* len, int sectionType,
                     size_t* size) {
    // PFB sections have a two or six bytes header. 0x80 and a one byte
    // section type followed by a four byte section length.  Type one is
    // an ASCII section (includes a length), type two is a binary section
    // (includes a length) and type three is an EOF marker with no length.
    const uint8_t* buf = *src;
    if (*len < 2 || buf[0] != 0x80 || buf[1] != sectionType) {
        return false;
    } else if (buf[1] == 3) {
        return true;
    } else if (*len < 6) {
        return false;
    }

    *size = (size_t)buf[2] | ((size_t)buf[3] << 8) | ((size_t)buf[4] << 16) |
            ((size_t)buf[5] << 24);
    size_t consumed = *size + 6;
    if (consumed > *len) {
        return false;
    }
    *src = *src + consumed;
    *len = *len - consumed;
    return true;
}

bool parsePFB(const uint8_t* src, size_t size, size_t* headerLen,
              size_t* dataLen, size_t* trailerLen) {
    const uint8_t* srcPtr = src;
    size_t remaining = size;

    return parsePFBSection(&srcPtr, &remaining, 1, headerLen) &&
           parsePFBSection(&srcPtr, &remaining, 2, dataLen) &&
           parsePFBSection(&srcPtr, &remaining, 1, trailerLen) &&
           parsePFBSection(&srcPtr, &remaining, 3, nullptr);
}

/* The sections of a PFA file are implicitly defined.  The body starts
 * after the line containing "eexec," and the trailer starts with 512
 * literal 0's followed by "cleartomark" (plus arbitrary white space).
 *
 * This function assumes that src is NUL terminated, but the NUL
 * termination is not included in size.
 *
 */
bool parsePFA(const char* src, size_t size, size_t* headerLen,
              size_t* hexDataLen, size_t* dataLen, size_t* trailerLen) {
    const char* end = src + size;

    const char* dataPos = strstr(src, "eexec");
    if (!dataPos) {
        return false;
    }
    dataPos += strlen("eexec");
    while ((*dataPos == '\n' || *dataPos == '\r' || *dataPos == ' ') &&
            dataPos < end) {
        dataPos++;
    }
    *headerLen = dataPos - src;

    const char* trailerPos = strstr(dataPos, "cleartomark");
    if (!trailerPos) {
        return false;
    }
    int zeroCount = 0;
    for (trailerPos--; trailerPos > dataPos && zeroCount < 512; trailerPos--) {
        if (*trailerPos == '\n' || *trailerPos == '\r' || *trailerPos == ' ') {
            continue;
        } else if (*trailerPos == '0') {
            zeroCount++;
        } else {
            return false;
        }
    }
    if (zeroCount != 512) {
        return false;
    }

    *hexDataLen = trailerPos - src - *headerLen;
    *trailerLen = size - *headerLen - *hexDataLen;

    // Verify that the data section is hex encoded and count the bytes.
    int nibbles = 0;
    for (; dataPos < trailerPos; dataPos++) {
        if (isspace(*dataPos)) {
            continue;
        }
        if (!isxdigit(*dataPos)) {
            return false;
        }
        nibbles++;
    }
    *dataLen = (nibbles + 1) / 2;

    return true;
}

int8_t hexToBin(uint8_t c) {
    if (!isxdigit(c)) {
        return -1;
    } else if (c <= '9') {
        return c - '0';
    } else if (c <= 'F') {
        return c - 'A' + 10;
    } else if (c <= 'f') {
        return c - 'a' + 10;
    }
    return -1;
}

static SkData* handle_type1_stream(SkStream* srcStream, size_t* headerLen,
                                   size_t* dataLen, size_t* trailerLen) {
    // srcStream may be backed by a file or a unseekable fd, so we may not be
    // able to use skip(), rewind(), or getMemoryBase().  read()ing through
    // the input only once is doable, but very ugly. Furthermore, it'd be nice
    // if the data was NUL terminated so that we can use strstr() to search it.
    // Make as few copies as possible given these constraints.
    SkDynamicMemoryWStream dynamicStream;
    SkAutoTDelete<SkMemoryStream> staticStream;
    SkData* data = nullptr;
    const uint8_t* src;
    size_t srcLen;
    if ((srcLen = srcStream->getLength()) > 0) {
        staticStream.reset(new SkMemoryStream(srcLen + 1));
        src = (const uint8_t*)staticStream->getMemoryBase();
        if (srcStream->getMemoryBase() != nullptr) {
            memcpy((void *)src, srcStream->getMemoryBase(), srcLen);
        } else {
            size_t read = 0;
            while (read < srcLen) {
                size_t got = srcStream->read((void *)staticStream->getAtPos(),
                                             srcLen - read);
                if (got == 0) {
                    return nullptr;
                }
                read += got;
                staticStream->seek(read);
            }
        }
        ((uint8_t *)src)[srcLen] = 0;
    } else {
        static const size_t kBufSize = 4096;
        uint8_t buf[kBufSize];
        size_t amount;
        while ((amount = srcStream->read(buf, kBufSize)) > 0) {
            dynamicStream.write(buf, amount);
        }
        amount = 0;
        dynamicStream.write(&amount, 1);  // nullptr terminator.
        data = dynamicStream.copyToData();
        src = data->bytes();
        srcLen = data->size() - 1;
    }

    // this handles releasing the data we may have gotten from dynamicStream.
    // if data is null, it is a no-op
    SkAutoDataUnref aud(data);

    if (parsePFB(src, srcLen, headerLen, dataLen, trailerLen)) {
        static const int kPFBSectionHeaderLength = 6;
        const size_t length = *headerLen + *dataLen + *trailerLen;
        SkASSERT(length > 0);
        SkASSERT(length + (2 * kPFBSectionHeaderLength) <= srcLen);

        SkData* data = SkData::NewUninitialized(length);

        const uint8_t* const srcHeader = src + kPFBSectionHeaderLength;
        // There is a six-byte section header before header and data
        // (but not trailer) that we're not going to copy.
        const uint8_t* const srcData = srcHeader + *headerLen + kPFBSectionHeaderLength;
        const uint8_t* const srcTrailer = srcData + *headerLen;

        uint8_t* const resultHeader = (uint8_t*)data->writable_data();
        uint8_t* const resultData = resultHeader + *headerLen;
        uint8_t* const resultTrailer = resultData + *dataLen;

        SkASSERT(resultTrailer + *trailerLen == resultHeader + length);

        memcpy(resultHeader,  srcHeader,  *headerLen);
        memcpy(resultData,    srcData,    *dataLen);
        memcpy(resultTrailer, srcTrailer, *trailerLen);

        return data;
    }

    // A PFA has to be converted for PDF.
    size_t hexDataLen;
    if (parsePFA((const char*)src, srcLen, headerLen, &hexDataLen, dataLen,
                 trailerLen)) {
        const size_t length = *headerLen + *dataLen + *trailerLen;
        SkASSERT(length > 0);
        SkAutoTMalloc<uint8_t> buffer(length);

        memcpy(buffer.get(), src, *headerLen);
        uint8_t* const resultData = &(buffer[SkToInt(*headerLen)]);

        const uint8_t* hexData = src + *headerLen;
        const uint8_t* trailer = hexData + hexDataLen;
        size_t outputOffset = 0;
        uint8_t dataByte = 0;  // To hush compiler.
        bool highNibble = true;
        for (; hexData < trailer; hexData++) {
            int8_t curNibble = hexToBin(*hexData);
            if (curNibble < 0) {
                continue;
            }
            if (highNibble) {
                dataByte = curNibble << 4;
                highNibble = false;
            } else {
                dataByte |= curNibble;
                highNibble = true;
                resultData[outputOffset++] = dataByte;
            }
        }
        if (!highNibble) {
            resultData[outputOffset++] = dataByte;
        }
        SkASSERT(outputOffset == *dataLen);

        uint8_t* const resultTrailer = &(buffer[SkToInt(*headerLen + outputOffset)]);
        memcpy(resultTrailer, src + *headerLen + hexDataLen, *trailerLen);

        return SkData::NewFromMalloc(buffer.detach(), length);
    }
    return nullptr;
}

// scale from em-units to base-1000, returning as a SkScalar
SkScalar scaleFromFontUnits(int16_t val, uint16_t emSize) {
    SkScalar scaled = SkIntToScalar(val);
    if (emSize == 1000) {
        return scaled;
    } else {
        return SkScalarMulDiv(scaled, 1000, emSize);
    }
}

void setGlyphWidthAndBoundingBox(SkScalar width, SkIRect box,
                                 SkWStream* content) {
    // Specify width and bounding box for the glyph.
    SkPDFUtils::AppendScalar(width, content);
    content->writeText(" 0 ");
    content->writeDecAsText(box.fLeft);
    content->writeText(" ");
    content->writeDecAsText(box.fTop);
    content->writeText(" ");
    content->writeDecAsText(box.fRight);
    content->writeText(" ");
    content->writeDecAsText(box.fBottom);
    content->writeText(" d1\n");
}

SkPDFArray* makeFontBBox(SkIRect glyphBBox, uint16_t emSize) {
    SkPDFArray* bbox = new SkPDFArray;
    bbox->reserve(4);
    bbox->appendScalar(scaleFromFontUnits(glyphBBox.fLeft, emSize));
    bbox->appendScalar(scaleFromFontUnits(glyphBBox.fBottom, emSize));
    bbox->appendScalar(scaleFromFontUnits(glyphBBox.fRight, emSize));
    bbox->appendScalar(scaleFromFontUnits(glyphBBox.fTop, emSize));
    return bbox;
}

SkPDFArray* appendWidth(const int16_t& width, uint16_t emSize,
                        SkPDFArray* array) {
    array->appendScalar(scaleFromFontUnits(width, emSize));
    return array;
}

SkPDFArray* appendVerticalAdvance(
        const SkAdvancedTypefaceMetrics::VerticalMetric& advance,
        uint16_t emSize, SkPDFArray* array) {
    appendWidth(advance.fVerticalAdvance, emSize, array);
    appendWidth(advance.fOriginXDisp, emSize, array);
    appendWidth(advance.fOriginYDisp, emSize, array);
    return array;
}

template <typename Data>
SkPDFArray* composeAdvanceData(
        SkAdvancedTypefaceMetrics::AdvanceMetric<Data>* advanceInfo,
        uint16_t emSize,
        SkPDFArray* (*appendAdvance)(const Data& advance, uint16_t emSize,
                                     SkPDFArray* array),
        Data* defaultAdvance) {
    SkPDFArray* result = new SkPDFArray();
    for (; advanceInfo != nullptr; advanceInfo = advanceInfo->fNext.get()) {
        switch (advanceInfo->fType) {
            case SkAdvancedTypefaceMetrics::WidthRange::kDefault: {
                SkASSERT(advanceInfo->fAdvance.count() == 1);
                *defaultAdvance = advanceInfo->fAdvance[0];
                break;
            }
            case SkAdvancedTypefaceMetrics::WidthRange::kRange: {
                SkAutoTUnref<SkPDFArray> advanceArray(new SkPDFArray());
                for (int j = 0; j < advanceInfo->fAdvance.count(); j++)
                    appendAdvance(advanceInfo->fAdvance[j], emSize,
                                  advanceArray.get());
                result->appendInt(advanceInfo->fStartId);
                result->appendObject(advanceArray.detach());
                break;
            }
            case SkAdvancedTypefaceMetrics::WidthRange::kRun: {
                SkASSERT(advanceInfo->fAdvance.count() == 1);
                result->appendInt(advanceInfo->fStartId);
                result->appendInt(advanceInfo->fEndId);
                appendAdvance(advanceInfo->fAdvance[0], emSize, result);
                break;
            }
        }
    }
    return result;
}

}  // namespace

static void append_tounicode_header(SkDynamicMemoryWStream* cmap,
                                    uint16_t firstGlyphID,
                                    uint16_t lastGlyphID) {
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

    // e.g.     "<0000> <FFFF>\n"
    SkString range;
    range.appendf("<%04X> <%04X>\n", firstGlyphID, lastGlyphID);
    cmap->writeText(range.c_str());

    const char* kTypeInfoFooter = "endcodespacerange\n";
    cmap->writeText(kTypeInfoFooter);
}

static void append_cmap_footer(SkDynamicMemoryWStream* cmap) {
    const char* kFooter =
        "endcmap\n"
        "CMapName currentdict /CMap defineresource pop\n"
        "end\n"
        "end";
    cmap->writeText(kFooter);
}

struct BFChar {
    uint16_t fGlyphId;
    SkUnichar fUnicode;
};

struct BFRange {
    uint16_t fStart;
    uint16_t fEnd;
    SkUnichar fUnicode;
};

static void append_bfchar_section(const SkTDArray<BFChar>& bfchar,
                                  SkDynamicMemoryWStream* cmap) {
    // PDF spec defines that every bf* list can have at most 100 entries.
    for (int i = 0; i < bfchar.count(); i += 100) {
        int count = bfchar.count() - i;
        count = SkMin32(count, 100);
        cmap->writeDecAsText(count);
        cmap->writeText(" beginbfchar\n");
        for (int j = 0; j < count; ++j) {
            cmap->writeText("<");
            cmap->writeHexAsText(bfchar[i + j].fGlyphId, 4);
            cmap->writeText("> <");
            cmap->writeHexAsText(bfchar[i + j].fUnicode, 4);
            cmap->writeText(">\n");
        }
        cmap->writeText("endbfchar\n");
    }
}

static void append_bfrange_section(const SkTDArray<BFRange>& bfrange,
                                   SkDynamicMemoryWStream* cmap) {
    // PDF spec defines that every bf* list can have at most 100 entries.
    for (int i = 0; i < bfrange.count(); i += 100) {
        int count = bfrange.count() - i;
        count = SkMin32(count, 100);
        cmap->writeDecAsText(count);
        cmap->writeText(" beginbfrange\n");
        for (int j = 0; j < count; ++j) {
            cmap->writeText("<");
            cmap->writeHexAsText(bfrange[i + j].fStart, 4);
            cmap->writeText("> <");
            cmap->writeHexAsText(bfrange[i + j].fEnd, 4);
            cmap->writeText("> <");
            cmap->writeHexAsText(bfrange[i + j].fUnicode, 4);
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
// Current implementation does not attempt aggresive optimizations against
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

// FIXME: this should be in a header so that it is separately testable
// ( see caller in tests/ToUnicode.cpp )
void append_cmap_sections(const SkTDArray<SkUnichar>& glyphToUnicode,
                          const SkPDFGlyphSet* subset,
                          SkDynamicMemoryWStream* cmap,
                          bool multiByteGlyphs,
                          uint16_t firstGlyphID,
                          uint16_t lastGlyphID);

void append_cmap_sections(const SkTDArray<SkUnichar>& glyphToUnicode,
                          const SkPDFGlyphSet* subset,
                          SkDynamicMemoryWStream* cmap,
                          bool multiByteGlyphs,
                          uint16_t firstGlyphID,
                          uint16_t lastGlyphID) {
    if (glyphToUnicode.isEmpty()) {
        return;
    }
    int glyphOffset = 0;
    if (!multiByteGlyphs) {
        glyphOffset = firstGlyphID - 1;
    }

    SkTDArray<BFChar> bfcharEntries;
    SkTDArray<BFRange> bfrangeEntries;

    BFRange currentRangeEntry = {0, 0, 0};
    bool rangeEmpty = true;
    const int limit =
            SkMin32(lastGlyphID + 1, glyphToUnicode.count()) - glyphOffset;

    for (int i = firstGlyphID - glyphOffset; i < limit + 1; ++i) {
        bool inSubset = i < limit &&
                        (subset == nullptr || subset->has(i + glyphOffset));
        if (!rangeEmpty) {
            // PDF spec requires bfrange not changing the higher byte,
            // e.g. <1035> <10FF> <2222> is ok, but
            //      <1035> <1100> <2222> is no good
            bool inRange =
                i == currentRangeEntry.fEnd + 1 &&
                i >> 8 == currentRangeEntry.fStart >> 8 &&
                i < limit &&
                glyphToUnicode[i + glyphOffset] ==
                    currentRangeEntry.fUnicode + i - currentRangeEntry.fStart;
            if (!inSubset || !inRange) {
                if (currentRangeEntry.fEnd > currentRangeEntry.fStart) {
                    bfrangeEntries.push(currentRangeEntry);
                } else {
                    BFChar* entry = bfcharEntries.append();
                    entry->fGlyphId = currentRangeEntry.fStart;
                    entry->fUnicode = currentRangeEntry.fUnicode;
                }
                rangeEmpty = true;
            }
        }
        if (inSubset) {
            currentRangeEntry.fEnd = i;
            if (rangeEmpty) {
              currentRangeEntry.fStart = i;
              currentRangeEntry.fUnicode = glyphToUnicode[i + glyphOffset];
              rangeEmpty = false;
            }
        }
    }

    // The spec requires all bfchar entries for a font must come before bfrange
    // entries.
    append_bfchar_section(bfcharEntries, cmap);
    append_bfrange_section(bfrangeEntries, cmap);
}

static SkPDFStream* generate_tounicode_cmap(
        const SkTDArray<SkUnichar>& glyphToUnicode,
        const SkPDFGlyphSet* subset,
        bool multiByteGlyphs,
        uint16_t firstGlyphID,
        uint16_t lastGlyphID) {
    SkDynamicMemoryWStream cmap;
    if (multiByteGlyphs) {
        append_tounicode_header(&cmap, firstGlyphID, lastGlyphID);
    } else {
        append_tounicode_header(&cmap, 1, lastGlyphID - firstGlyphID + 1);
    }
    append_cmap_sections(glyphToUnicode, subset, &cmap, multiByteGlyphs,
                         firstGlyphID, lastGlyphID);
    append_cmap_footer(&cmap);
    SkAutoTUnref<SkData> cmapData(cmap.copyToData());
    return new SkPDFStream(cmapData.get());
}

#if defined (SK_SFNTLY_SUBSETTER)
static void sk_delete_array(const void* ptr, void*) {
    // Use C-style cast to cast away const and cast type simultaneously.
    delete[] (unsigned char*)ptr;
}
#endif

#if defined(SK_SFNTLY_SUBSETTER)
static size_t get_subset_font_stream(const char* fontName,
                                     const SkTypeface* typeface,
                                     const SkTDArray<uint32_t>& subset,
                                     SkPDFStream** fontStream) {
    int ttcIndex;
    SkAutoTDelete<SkStream> fontData(typeface->openStream(&ttcIndex));
    SkASSERT(fontData.get());

    size_t fontSize = fontData->getLength();

    // Read font into buffer.
    SkPDFStream* subsetFontStream = nullptr;
    SkTDArray<unsigned char> originalFont;
    originalFont.setCount(SkToInt(fontSize));
    if (fontData->read(originalFont.begin(), fontSize) == fontSize) {
        unsigned char* subsetFont = nullptr;
        // sfntly requires unsigned int* to be passed in, as far as we know,
        // unsigned int is equivalent to uint32_t on all platforms.
        static_assert(sizeof(unsigned int) == sizeof(uint32_t), "unsigned_int_not_32_bits");
        int subsetFontSize = SfntlyWrapper::SubsetFont(fontName,
                                                       originalFont.begin(),
                                                       fontSize,
                                                       subset.begin(),
                                                       subset.count(),
                                                       &subsetFont);
        if (subsetFontSize > 0 && subsetFont != nullptr) {
            SkAutoDataUnref data(SkData::NewWithProc(subsetFont,
                                                     subsetFontSize,
                                                     sk_delete_array,
                                                     nullptr));
            subsetFontStream = new SkPDFStream(data.get());
            fontSize = subsetFontSize;
        }
    }
    if (subsetFontStream) {
        *fontStream = subsetFontStream;
        return fontSize;
    }
    fontData->rewind();

    // Fail over: just embed the whole font.
    *fontStream = new SkPDFStream(fontData.get());
    return fontSize;
}
#endif

///////////////////////////////////////////////////////////////////////////////
// class SkPDFGlyphSet
///////////////////////////////////////////////////////////////////////////////

SkPDFGlyphSet::SkPDFGlyphSet() : fBitSet(SK_MaxU16 + 1) {
}

void SkPDFGlyphSet::set(const uint16_t* glyphIDs, int numGlyphs) {
    for (int i = 0; i < numGlyphs; ++i) {
        fBitSet.setBit(glyphIDs[i], true);
    }
}

bool SkPDFGlyphSet::has(uint16_t glyphID) const {
    return fBitSet.isBitSet(glyphID);
}

void SkPDFGlyphSet::merge(const SkPDFGlyphSet& usage) {
    fBitSet.orBits(usage.fBitSet);
}

void SkPDFGlyphSet::exportTo(SkTDArray<unsigned int>* glyphIDs) const {
    fBitSet.exportTo(glyphIDs);
}

///////////////////////////////////////////////////////////////////////////////
// class SkPDFGlyphSetMap
///////////////////////////////////////////////////////////////////////////////
SkPDFGlyphSetMap::FontGlyphSetPair::FontGlyphSetPair(SkPDFFont* font,
                                                     SkPDFGlyphSet* glyphSet)
        : fFont(font),
          fGlyphSet(glyphSet) {
}

SkPDFGlyphSetMap::F2BIter::F2BIter(const SkPDFGlyphSetMap& map) {
    reset(map);
}

const SkPDFGlyphSetMap::FontGlyphSetPair* SkPDFGlyphSetMap::F2BIter::next() const {
    if (fIndex >= fMap->count()) {
        return nullptr;
    }
    return &((*fMap)[fIndex++]);
}

void SkPDFGlyphSetMap::F2BIter::reset(const SkPDFGlyphSetMap& map) {
    fMap = &(map.fMap);
    fIndex = 0;
}

SkPDFGlyphSetMap::SkPDFGlyphSetMap() {
}

SkPDFGlyphSetMap::~SkPDFGlyphSetMap() {
    reset();
}

void SkPDFGlyphSetMap::merge(const SkPDFGlyphSetMap& usage) {
    for (int i = 0; i < usage.fMap.count(); ++i) {
        SkPDFGlyphSet* myUsage = getGlyphSetForFont(usage.fMap[i].fFont);
        myUsage->merge(*(usage.fMap[i].fGlyphSet));
    }
}

void SkPDFGlyphSetMap::reset() {
    for (int i = 0; i < fMap.count(); ++i) {
        delete fMap[i].fGlyphSet;  // Should not be nullptr.
    }
    fMap.reset();
}

void SkPDFGlyphSetMap::noteGlyphUsage(SkPDFFont* font, const uint16_t* glyphIDs,
                                      int numGlyphs) {
    SkPDFGlyphSet* subset = getGlyphSetForFont(font);
    if (subset) {
        subset->set(glyphIDs, numGlyphs);
    }
}

SkPDFGlyphSet* SkPDFGlyphSetMap::getGlyphSetForFont(SkPDFFont* font) {
    int index = fMap.count();
    for (int i = 0; i < index; ++i) {
        if (fMap[i].fFont == font) {
            return fMap[i].fGlyphSet;
        }
    }
    fMap.append();
    index = fMap.count() - 1;
    fMap[index].fFont = font;
    fMap[index].fGlyphSet = new SkPDFGlyphSet();
    return fMap[index].fGlyphSet;
}

///////////////////////////////////////////////////////////////////////////////
// class SkPDFFont
///////////////////////////////////////////////////////////////////////////////

/* Font subset design: It would be nice to be able to subset fonts
 * (particularly type 3 fonts), but it's a lot of work and not a priority.
 *
 * Resources are canonicalized and uniqueified by pointer so there has to be
 * some additional state indicating which subset of the font is used.  It
 * must be maintained at the page granularity and then combined at the document
 * granularity. a) change SkPDFFont to fill in its state on demand, kind of
 * like SkPDFGraphicState.  b) maintain a per font glyph usage class in each
 * page/pdf device. c) in the document, retrieve the per font glyph usage
 * from each page and combine it and ask for a resource with that subset.
 */

SkPDFFont::~SkPDFFont() {}

SkTypeface* SkPDFFont::typeface() {
    return fTypeface.get();
}

SkAdvancedTypefaceMetrics::FontType SkPDFFont::getType() {
    return fFontType;
}

bool SkPDFFont::canEmbed() const {
    if (!fFontInfo.get()) {
        SkASSERT(fFontType == SkAdvancedTypefaceMetrics::kOther_Font);
        return true;
    }
    return (fFontInfo->fFlags &
            SkAdvancedTypefaceMetrics::kNotEmbeddable_FontFlag) == 0;
}

bool SkPDFFont::canSubset() const {
    if (!fFontInfo.get()) {
        SkASSERT(fFontType == SkAdvancedTypefaceMetrics::kOther_Font);
        return true;
    }
    return (fFontInfo->fFlags &
            SkAdvancedTypefaceMetrics::kNotSubsettable_FontFlag) == 0;
}

bool SkPDFFont::hasGlyph(uint16_t id) {
    return (id >= fFirstGlyphID && id <= fLastGlyphID) || id == 0;
}

int SkPDFFont::glyphsToPDFFontEncoding(uint16_t* glyphIDs, int numGlyphs) {
    // A font with multibyte glyphs will support all glyph IDs in a single font.
    if (this->multiByteGlyphs()) {
        return numGlyphs;
    }

    for (int i = 0; i < numGlyphs; i++) {
        if (glyphIDs[i] == 0) {
            continue;
        }
        if (glyphIDs[i] < fFirstGlyphID || glyphIDs[i] > fLastGlyphID) {
            return i;
        }
        glyphIDs[i] -= (fFirstGlyphID - 1);
    }

    return numGlyphs;
}

// static
SkPDFFont* SkPDFFont::GetFontResource(SkPDFCanon* canon,
                                      SkTypeface* typeface,
                                      uint16_t glyphID) {
    SkASSERT(canon);
    SkAutoResolveDefaultTypeface autoResolve(typeface);
    typeface = autoResolve.get();
    const uint32_t fontID = typeface->uniqueID();

    SkPDFFont* relatedFont;
    if (SkPDFFont* pdfFont = canon->findFont(fontID, glyphID, &relatedFont)) {
        return SkRef(pdfFont);
    }

    SkAutoTUnref<const SkAdvancedTypefaceMetrics> fontMetrics;
    SkPDFDict* relatedFontDescriptor = nullptr;
    if (relatedFont) {
        fontMetrics.reset(SkSafeRef(relatedFont->fontInfo()));
        relatedFontDescriptor = relatedFont->getFontDescriptor();

        // This only is to catch callers who pass invalid glyph ids.
        // If glyph id is invalid, then we will create duplicate entries
        // for TrueType fonts.
        SkAdvancedTypefaceMetrics::FontType fontType =
            fontMetrics.get() ? fontMetrics.get()->fType :
                                SkAdvancedTypefaceMetrics::kOther_Font;

        if (fontType == SkAdvancedTypefaceMetrics::kType1CID_Font ||
            fontType == SkAdvancedTypefaceMetrics::kTrueType_Font) {
            return SkRef(relatedFont);
        }
    } else {
        SkTypeface::PerGlyphInfo info;
        info = SkTypeface::kGlyphNames_PerGlyphInfo;
        info = SkTBitOr<SkTypeface::PerGlyphInfo>(
                  info, SkTypeface::kToUnicode_PerGlyphInfo);
#if !defined (SK_SFNTLY_SUBSETTER)
        info = SkTBitOr<SkTypeface::PerGlyphInfo>(
                  info, SkTypeface::kHAdvance_PerGlyphInfo);
#endif
        fontMetrics.reset(
            typeface->getAdvancedTypefaceMetrics(info, nullptr, 0));
#if defined (SK_SFNTLY_SUBSETTER)
        if (fontMetrics.get() &&
            fontMetrics->fType != SkAdvancedTypefaceMetrics::kTrueType_Font) {
            // Font does not support subsetting, get new info with advance.
            info = SkTBitOr<SkTypeface::PerGlyphInfo>(
                      info, SkTypeface::kHAdvance_PerGlyphInfo);
            fontMetrics.reset(
                typeface->getAdvancedTypefaceMetrics(info, nullptr, 0));
        }
#endif
    }

    SkPDFFont* font = SkPDFFont::Create(canon, fontMetrics.get(), typeface,
                                        glyphID, relatedFontDescriptor);
    canon->addFont(font, fontID, font->fFirstGlyphID);
    return font;
}

SkPDFFont* SkPDFFont::getFontSubset(const SkPDFGlyphSet*) {
    return nullptr;  // Default: no support.
}

SkPDFFont::SkPDFFont(const SkAdvancedTypefaceMetrics* info,
                     SkTypeface* typeface,
                     SkPDFDict* relatedFontDescriptor)
    : SkPDFDict("Font")
    , fTypeface(ref_or_default(typeface))
    , fFirstGlyphID(1)
    , fLastGlyphID(info ? info->fLastGlyphID : 0)
    , fFontInfo(SkSafeRef(info))
    , fDescriptor(SkSafeRef(relatedFontDescriptor)) {
    if (info == nullptr ||
            info->fFlags & SkAdvancedTypefaceMetrics::kMultiMaster_FontFlag) {
        fFontType = SkAdvancedTypefaceMetrics::kOther_Font;
    } else {
        fFontType = info->fType;
    }
}

// static
SkPDFFont* SkPDFFont::Create(SkPDFCanon* canon,
                             const SkAdvancedTypefaceMetrics* info,
                             SkTypeface* typeface,
                             uint16_t glyphID,
                             SkPDFDict* relatedFontDescriptor) {
    SkAdvancedTypefaceMetrics::FontType type =
        info ? info->fType : SkAdvancedTypefaceMetrics::kOther_Font;

    if (info && (info->fFlags & SkAdvancedTypefaceMetrics::kMultiMaster_FontFlag)) {
        return new SkPDFType3Font(info, typeface, glyphID);
    }
    if (type == SkAdvancedTypefaceMetrics::kType1CID_Font ||
        type == SkAdvancedTypefaceMetrics::kTrueType_Font) {
        SkASSERT(relatedFontDescriptor == nullptr);
        return new SkPDFType0Font(info, typeface);
    }
    if (type == SkAdvancedTypefaceMetrics::kType1_Font) {
        return new SkPDFType1Font(info, typeface, glyphID, relatedFontDescriptor);
    }

    SkASSERT(type == SkAdvancedTypefaceMetrics::kCFF_Font ||
             type == SkAdvancedTypefaceMetrics::kOther_Font);

    return new SkPDFType3Font(info, typeface, glyphID);
}

const SkAdvancedTypefaceMetrics* SkPDFFont::fontInfo() {
    return fFontInfo.get();
}

void SkPDFFont::setFontInfo(const SkAdvancedTypefaceMetrics* info) {
    if (info == nullptr || info == fFontInfo.get()) {
        return;
    }
    fFontInfo.reset(info);
    SkSafeRef(info);
}

uint16_t SkPDFFont::firstGlyphID() const {
    return fFirstGlyphID;
}

uint16_t SkPDFFont::lastGlyphID() const {
    return fLastGlyphID;
}

void SkPDFFont::setLastGlyphID(uint16_t glyphID) {
    fLastGlyphID = glyphID;
}

SkPDFDict* SkPDFFont::getFontDescriptor() {
    return fDescriptor.get();
}

void SkPDFFont::setFontDescriptor(SkPDFDict* descriptor) {
    fDescriptor.reset(descriptor);
    SkSafeRef(descriptor);
}

bool SkPDFFont::addCommonFontDescriptorEntries(int16_t defaultWidth) {
    if (fDescriptor.get() == nullptr) {
        return false;
    }

    const uint16_t emSize = fFontInfo->fEmSize;

    fDescriptor->insertName("FontName", fFontInfo->fFontName);
    fDescriptor->insertInt("Flags", fFontInfo->fStyle | kPdfSymbolic);
    fDescriptor->insertScalar("Ascent",
            scaleFromFontUnits(fFontInfo->fAscent, emSize));
    fDescriptor->insertScalar("Descent",
            scaleFromFontUnits(fFontInfo->fDescent, emSize));
    fDescriptor->insertScalar("StemV",
            scaleFromFontUnits(fFontInfo->fStemV, emSize));

    fDescriptor->insertScalar("CapHeight",
            scaleFromFontUnits(fFontInfo->fCapHeight, emSize));
    fDescriptor->insertInt("ItalicAngle", fFontInfo->fItalicAngle);
    fDescriptor->insertObject(
            "FontBBox", makeFontBBox(fFontInfo->fBBox, fFontInfo->fEmSize));

    if (defaultWidth > 0) {
        fDescriptor->insertScalar("MissingWidth",
                scaleFromFontUnits(defaultWidth, emSize));
    }
    return true;
}

void SkPDFFont::adjustGlyphRangeForSingleByteEncoding(uint16_t glyphID) {
    // Single byte glyph encoding supports a max of 255 glyphs.
    fFirstGlyphID = glyphID - (glyphID - 1) % 255;
    if (fLastGlyphID > fFirstGlyphID + 255 - 1) {
        fLastGlyphID = fFirstGlyphID + 255 - 1;
    }
}

void SkPDFFont::populateToUnicodeTable(const SkPDFGlyphSet* subset) {
    if (fFontInfo == nullptr || fFontInfo->fGlyphToUnicode.begin() == nullptr) {
        return;
    }
    this->insertObjRef("ToUnicode",
                       generate_tounicode_cmap(fFontInfo->fGlyphToUnicode,
                                               subset,
                                               multiByteGlyphs(),
                                               firstGlyphID(),
                                               lastGlyphID()));
}

///////////////////////////////////////////////////////////////////////////////
// class SkPDFType0Font
///////////////////////////////////////////////////////////////////////////////

SkPDFType0Font::SkPDFType0Font(const SkAdvancedTypefaceMetrics* info, SkTypeface* typeface)
    : SkPDFFont(info, typeface, nullptr) {
    SkDEBUGCODE(fPopulated = false);
    if (!canSubset()) {
        this->populate(nullptr);
    }
}

SkPDFType0Font::~SkPDFType0Font() {}

SkPDFFont* SkPDFType0Font::getFontSubset(const SkPDFGlyphSet* subset) {
    if (!canSubset()) {
        return nullptr;
    }
    SkPDFType0Font* newSubset = new SkPDFType0Font(fontInfo(), typeface());
    newSubset->populate(subset);
    return newSubset;
}

#ifdef SK_DEBUG
void SkPDFType0Font::emitObject(SkWStream* stream,
                                const SkPDFObjNumMap& objNumMap,
                                const SkPDFSubstituteMap& substitutes) const {
    SkASSERT(fPopulated);
    return INHERITED::emitObject(stream, objNumMap, substitutes);
}
#endif

bool SkPDFType0Font::populate(const SkPDFGlyphSet* subset) {
    insertName("Subtype", "Type0");
    insertName("BaseFont", fontInfo()->fFontName);
    insertName("Encoding", "Identity-H");

    SkAutoTUnref<SkPDFCIDFont> newCIDFont(
            new SkPDFCIDFont(fontInfo(), typeface(), subset));
    SkAutoTUnref<SkPDFArray> descendantFonts(new SkPDFArray());
    descendantFonts->appendObjRef(newCIDFont.detach());
    this->insertObject("DescendantFonts", descendantFonts.detach());

    this->populateToUnicodeTable(subset);

    SkDEBUGCODE(fPopulated = true);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// class SkPDFCIDFont
///////////////////////////////////////////////////////////////////////////////

SkPDFCIDFont::SkPDFCIDFont(const SkAdvancedTypefaceMetrics* info,
                           SkTypeface* typeface,
                           const SkPDFGlyphSet* subset)
    : SkPDFFont(info, typeface, nullptr) {
    this->populate(subset);
}

SkPDFCIDFont::~SkPDFCIDFont() {}

bool SkPDFCIDFont::addFontDescriptor(int16_t defaultWidth,
                                     const SkTDArray<uint32_t>* subset) {
    SkAutoTUnref<SkPDFDict> descriptor(new SkPDFDict("FontDescriptor"));
    setFontDescriptor(descriptor.get());
    if (!addCommonFontDescriptorEntries(defaultWidth)) {
        this->insertObjRef("FontDescriptor", descriptor.detach());
        return false;
    }
    SkASSERT(this->canEmbed());

    switch (getType()) {
        case SkAdvancedTypefaceMetrics::kTrueType_Font: {
            size_t fontSize = 0;
#if defined(SK_SFNTLY_SUBSETTER)
            if (this->canSubset()) {
                SkAutoTUnref<SkPDFStream> fontStream;
                SkPDFStream* rawStream = nullptr;
                fontSize = get_subset_font_stream(fontInfo()->fFontName.c_str(),
                                                  typeface(),
                                                  *subset,
                                                  &rawStream);
                if (rawStream) {
                    fontStream.reset(rawStream);
                    fontStream->insertInt("Length1", fontSize);
                    descriptor->insertObjRef("FontFile2", fontStream.detach());
                    break;
                }
            }
#endif
            SkAutoTUnref<SkPDFSharedStream> fontStream;
            SkAutoTDelete<SkStreamAsset> fontData(
                    this->typeface()->openStream(nullptr));
            SkASSERT(fontData);
            fontSize = fontData->getLength();
            SkASSERT(fontSize > 0);
            fontStream.reset(new SkPDFSharedStream(fontData.detach()));
            fontStream->dict()->insertInt("Length1", fontSize);
            descriptor->insertObjRef("FontFile2", fontStream.detach());
            break;
        }
        case SkAdvancedTypefaceMetrics::kCFF_Font:
        case SkAdvancedTypefaceMetrics::kType1CID_Font: {
            SkAutoTUnref<SkPDFSharedStream> fontStream(
                    new SkPDFSharedStream(this->typeface()->openStream(nullptr)));

            if (getType() == SkAdvancedTypefaceMetrics::kCFF_Font) {
                fontStream->dict()->insertName("Subtype", "Type1C");
            } else {
                fontStream->dict()->insertName("Subtype", "CIDFontType0c");
            }
            descriptor->insertObjRef("FontFile3", fontStream.detach());
            break;
        }
        default:
            SkASSERT(false);
    }
    this->insertObjRef("FontDescriptor", descriptor.detach());
    return true;
}

bool SkPDFCIDFont::populate(const SkPDFGlyphSet* subset) {
    // Generate new font metrics with advance info for true type fonts.
    if (fontInfo()->fType == SkAdvancedTypefaceMetrics::kTrueType_Font) {
        // Generate glyph id array.
        SkTDArray<uint32_t> glyphIDs;
        if (subset) {
            // Always include glyph 0.
            if (!subset->has(0)) {
                glyphIDs.push(0);
            }
            subset->exportTo(&glyphIDs);
        }

        SkTypeface::PerGlyphInfo info;
        info = SkTypeface::kGlyphNames_PerGlyphInfo;
        info = SkTBitOr<SkTypeface::PerGlyphInfo>(
                  info, SkTypeface::kHAdvance_PerGlyphInfo);
        uint32_t* glyphs = (glyphIDs.count() == 0) ? nullptr : glyphIDs.begin();
        uint32_t glyphsCount = glyphs ? glyphIDs.count() : 0;
        SkAutoTUnref<const SkAdvancedTypefaceMetrics> fontMetrics(
            typeface()->getAdvancedTypefaceMetrics(info, glyphs, glyphsCount));
        setFontInfo(fontMetrics.get());
        addFontDescriptor(0, &glyphIDs);
    } else {
        // Other CID fonts
        addFontDescriptor(0, nullptr);
    }

    insertName("BaseFont", fontInfo()->fFontName);

    if (getType() == SkAdvancedTypefaceMetrics::kType1CID_Font) {
        insertName("Subtype", "CIDFontType0");
    } else if (getType() == SkAdvancedTypefaceMetrics::kTrueType_Font) {
        insertName("Subtype", "CIDFontType2");
        insertName("CIDToGIDMap", "Identity");
    } else {
        SkASSERT(false);
    }

    SkAutoTUnref<SkPDFDict> sysInfo(new SkPDFDict);
    sysInfo->insertString("Registry", "Adobe");
    sysInfo->insertString("Ordering", "Identity");
    sysInfo->insertInt("Supplement", 0);
    this->insertObject("CIDSystemInfo", sysInfo.detach());

    if (fontInfo()->fGlyphWidths.get()) {
        int16_t defaultWidth = 0;
        SkAutoTUnref<SkPDFArray> widths(
            composeAdvanceData(fontInfo()->fGlyphWidths.get(),
                               fontInfo()->fEmSize, &appendWidth,
                               &defaultWidth));
        if (widths->size())
            this->insertObject("W", widths.detach());
        if (defaultWidth != 0) {
            this->insertScalar(
                    "DW",
                    scaleFromFontUnits(defaultWidth, fontInfo()->fEmSize));
        }
    }
    if (fontInfo()->fVerticalMetrics.get()) {
        struct SkAdvancedTypefaceMetrics::VerticalMetric defaultAdvance;
        defaultAdvance.fVerticalAdvance = 0;
        defaultAdvance.fOriginXDisp = 0;
        defaultAdvance.fOriginYDisp = 0;
        SkAutoTUnref<SkPDFArray> advances(
            composeAdvanceData(fontInfo()->fVerticalMetrics.get(),
                               fontInfo()->fEmSize, &appendVerticalAdvance,
                               &defaultAdvance));
        if (advances->size())
            this->insertObject("W2", advances.detach());
        if (defaultAdvance.fVerticalAdvance ||
                defaultAdvance.fOriginXDisp ||
                defaultAdvance.fOriginYDisp) {
            this->insertObject("DW2",
                               appendVerticalAdvance(defaultAdvance,
                                                     fontInfo()->fEmSize,
                                                     new SkPDFArray));
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// class SkPDFType1Font
///////////////////////////////////////////////////////////////////////////////

SkPDFType1Font::SkPDFType1Font(const SkAdvancedTypefaceMetrics* info,
                               SkTypeface* typeface,
                               uint16_t glyphID,
                               SkPDFDict* relatedFontDescriptor)
    : SkPDFFont(info, typeface, relatedFontDescriptor) {
    this->populate(glyphID);
}

SkPDFType1Font::~SkPDFType1Font() {}

bool SkPDFType1Font::addFontDescriptor(int16_t defaultWidth) {
    if (SkPDFDict* descriptor = getFontDescriptor()) {
        this->insertObjRef("FontDescriptor", SkRef(descriptor));
        return true;
    }

    SkAutoTUnref<SkPDFDict> descriptor(new SkPDFDict("FontDescriptor"));
    setFontDescriptor(descriptor.get());

    int ttcIndex;
    size_t header SK_INIT_TO_AVOID_WARNING;
    size_t data SK_INIT_TO_AVOID_WARNING;
    size_t trailer SK_INIT_TO_AVOID_WARNING;
    SkAutoTDelete<SkStream> rawFontData(typeface()->openStream(&ttcIndex));
    SkAutoTUnref<SkData> fontData(handle_type1_stream(rawFontData.get(), &header,
                                                      &data, &trailer));
    if (fontData.get() == nullptr) {
        return false;
    }
    SkASSERT(this->canEmbed());
    SkAutoTUnref<SkPDFStream> fontStream(new SkPDFStream(fontData.get()));
    fontStream->insertInt("Length1", header);
    fontStream->insertInt("Length2", data);
    fontStream->insertInt("Length3", trailer);
    descriptor->insertObjRef("FontFile", fontStream.detach());

    this->insertObjRef("FontDescriptor", descriptor.detach());

    return addCommonFontDescriptorEntries(defaultWidth);
}

bool SkPDFType1Font::populate(int16_t glyphID) {
    SkASSERT(!fontInfo()->fVerticalMetrics.get());
    SkASSERT(fontInfo()->fGlyphWidths.get());

    adjustGlyphRangeForSingleByteEncoding(glyphID);

    int16_t defaultWidth = 0;
    const SkAdvancedTypefaceMetrics::WidthRange* widthRangeEntry = nullptr;
    const SkAdvancedTypefaceMetrics::WidthRange* widthEntry;
    for (widthEntry = fontInfo()->fGlyphWidths.get();
            widthEntry != nullptr;
            widthEntry = widthEntry->fNext.get()) {
        switch (widthEntry->fType) {
            case SkAdvancedTypefaceMetrics::WidthRange::kDefault:
                defaultWidth = widthEntry->fAdvance[0];
                break;
            case SkAdvancedTypefaceMetrics::WidthRange::kRun:
                SkASSERT(false);
                break;
            case SkAdvancedTypefaceMetrics::WidthRange::kRange:
                SkASSERT(widthRangeEntry == nullptr);
                widthRangeEntry = widthEntry;
                break;
        }
    }

    if (!addFontDescriptor(defaultWidth)) {
        return false;
    }

    insertName("Subtype", "Type1");
    insertName("BaseFont", fontInfo()->fFontName);

    addWidthInfoFromRange(defaultWidth, widthRangeEntry);


    SkAutoTUnref<SkPDFArray> encDiffs(new SkPDFArray);
    encDiffs->reserve(lastGlyphID() - firstGlyphID() + 2);
    encDiffs->appendInt(1);
    for (int gID = firstGlyphID(); gID <= lastGlyphID(); gID++) {
        encDiffs->appendName(fontInfo()->fGlyphNames->get()[gID].c_str());
    }

    SkAutoTUnref<SkPDFDict> encoding(new SkPDFDict("Encoding"));
    encoding->insertObject("Differences", encDiffs.detach());
    this->insertObject("Encoding", encoding.detach());
    return true;
}

void SkPDFType1Font::addWidthInfoFromRange(
        int16_t defaultWidth,
        const SkAdvancedTypefaceMetrics::WidthRange* widthRangeEntry) {
    SkAutoTUnref<SkPDFArray> widthArray(new SkPDFArray());
    int firstChar = 0;
    if (widthRangeEntry) {
        const uint16_t emSize = fontInfo()->fEmSize;
        int startIndex = firstGlyphID() - widthRangeEntry->fStartId;
        int endIndex = startIndex + lastGlyphID() - firstGlyphID() + 1;
        if (startIndex < 0)
            startIndex = 0;
        if (endIndex > widthRangeEntry->fAdvance.count())
            endIndex = widthRangeEntry->fAdvance.count();
        if (widthRangeEntry->fStartId == 0) {
            appendWidth(widthRangeEntry->fAdvance[0], emSize, widthArray.get());
        } else {
            firstChar = startIndex + widthRangeEntry->fStartId;
        }
        for (int i = startIndex; i < endIndex; i++) {
            appendWidth(widthRangeEntry->fAdvance[i], emSize, widthArray.get());
        }
    } else {
        appendWidth(defaultWidth, 1000, widthArray.get());
    }
    this->insertInt("FirstChar", firstChar);
    this->insertInt("LastChar", firstChar + widthArray->size() - 1);
    this->insertObject("Widths", widthArray.detach());
}

///////////////////////////////////////////////////////////////////////////////
// class SkPDFType3Font
///////////////////////////////////////////////////////////////////////////////

SkPDFType3Font::SkPDFType3Font(const SkAdvancedTypefaceMetrics* info,
                               SkTypeface* typeface,
                               uint16_t glyphID)
    : SkPDFFont(info, typeface, nullptr) {
    this->populate(glyphID);
}

SkPDFType3Font::~SkPDFType3Font() {}

bool SkPDFType3Font::populate(uint16_t glyphID) {
    SkPaint paint;
    paint.setTypeface(typeface());
    paint.setTextSize(1000);
    const SkSurfaceProps props(0, kUnknown_SkPixelGeometry);
    SkAutoGlyphCache autoCache(paint, &props, nullptr);
    SkGlyphCache* cache = autoCache.getCache();
    // If fLastGlyphID isn't set (because there is not fFontInfo), look it up.
    if (lastGlyphID() == 0) {
        setLastGlyphID(cache->getGlyphCount() - 1);
    }

    adjustGlyphRangeForSingleByteEncoding(glyphID);

    insertName("Subtype", "Type3");
    // Flip about the x-axis and scale by 1/1000.
    SkMatrix fontMatrix;
    fontMatrix.setScale(SkScalarInvert(1000), -SkScalarInvert(1000));
    this->insertObject("FontMatrix", SkPDFUtils::MatrixToArray(fontMatrix));

    SkAutoTUnref<SkPDFDict> charProcs(new SkPDFDict);
    SkAutoTUnref<SkPDFDict> encoding(new SkPDFDict("Encoding"));

    SkAutoTUnref<SkPDFArray> encDiffs(new SkPDFArray);
    encDiffs->reserve(lastGlyphID() - firstGlyphID() + 2);
    encDiffs->appendInt(1);

    SkAutoTUnref<SkPDFArray> widthArray(new SkPDFArray());

    SkIRect bbox = SkIRect::MakeEmpty();
    for (int gID = firstGlyphID(); gID <= lastGlyphID(); gID++) {
        SkString characterName;
        characterName.printf("gid%d", gID);
        encDiffs->appendName(characterName.c_str());

        const SkGlyph& glyph = cache->getGlyphIDMetrics(gID);
        widthArray->appendScalar(SkFixedToScalar(glyph.fAdvanceX));
        SkIRect glyphBBox = SkIRect::MakeXYWH(glyph.fLeft, glyph.fTop,
                                              glyph.fWidth, glyph.fHeight);
        bbox.join(glyphBBox);

        SkDynamicMemoryWStream content;
        setGlyphWidthAndBoundingBox(SkFixedToScalar(glyph.fAdvanceX), glyphBBox,
                                    &content);
        const SkPath* path = cache->findPath(glyph);
        if (path) {
            SkPDFUtils::EmitPath(*path, paint.getStyle(), &content);
            SkPDFUtils::PaintPath(paint.getStyle(), path->getFillType(),
                                  &content);
        }
        SkAutoTDelete<SkMemoryStream> glyphStream(new SkMemoryStream());
        glyphStream->setData(content.copyToData())->unref();

        charProcs->insertObjRef(characterName,
                                new SkPDFStream(glyphStream.get()));
    }

    encoding->insertObject("Differences", encDiffs.detach());

    this->insertObject("CharProcs", charProcs.detach());
    this->insertObject("Encoding", encoding.detach());

    this->insertObject("FontBBox", makeFontBBox(bbox, 1000));
    this->insertInt("FirstChar", 1);
    this->insertInt("LastChar", lastGlyphID() - firstGlyphID() + 1);
    this->insertObject("Widths", widthArray.detach());
    this->insertName("CIDToGIDMap", "Identity");

    this->populateToUnicodeTable(nullptr);
    return true;
}

SkPDFFont::Match SkPDFFont::IsMatch(SkPDFFont* existingFont,
                                    uint32_t existingFontID,
                                    uint16_t existingGlyphID,
                                    uint32_t searchFontID,
                                    uint16_t searchGlyphID) {
    if (existingFontID != searchFontID) {
        return SkPDFFont::kNot_Match;
    }
    if (existingGlyphID == 0 || searchGlyphID == 0) {
        return SkPDFFont::kExact_Match;
    }
    if (existingFont != nullptr) {
        return (existingFont->fFirstGlyphID <= searchGlyphID &&
                searchGlyphID <= existingFont->fLastGlyphID)
                       ? SkPDFFont::kExact_Match
                       : SkPDFFont::kRelated_Match;
    }
    return (existingGlyphID == searchGlyphID) ? SkPDFFont::kExact_Match
                                              : SkPDFFont::kRelated_Match;
}

//  Since getAdvancedTypefaceMetrics is expensive, cache the result.
bool SkPDFFont::CanEmbedTypeface(SkTypeface* typeface, SkPDFCanon* canon) {
    SkAutoResolveDefaultTypeface face(typeface);
    uint32_t id = face->uniqueID();
    if (bool* value = canon->fCanEmbedTypeface.find(id)) {
        return *value;
    }
    bool canEmbed = true;
    SkAutoTUnref<const SkAdvancedTypefaceMetrics> fontMetrics(
            face->getAdvancedTypefaceMetrics(
                    SkTypeface::kNo_PerGlyphInfo, nullptr, 0));
    if (fontMetrics) {
        canEmbed = !SkToBool(
                fontMetrics->fFlags &
                SkAdvancedTypefaceMetrics::kNotEmbeddable_FontFlag);
    }
    return *canon->fCanEmbedTypeface.set(id, canEmbed);
}
