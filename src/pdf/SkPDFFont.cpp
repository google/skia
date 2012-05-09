
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include <ctype.h>

#include "SkData.h"
#include "SkFontHost.h"
#include "SkGlyphCache.h"
#include "SkPaint.h"
#include "SkPDFCatalog.h"
#include "SkPDFDevice.h"
#include "SkPDFFont.h"
#include "SkPDFFontImpl.h"
#include "SkPDFStream.h"
#include "SkPDFTypes.h"
#include "SkPDFUtils.h"
#include "SkRefCnt.h"
#include "SkScalar.h"
#include "SkStream.h"
#include "SkTypeface.h"
#include "SkTypes.h"
#include "SkUtils.h"

#if defined (SK_SFNTLY_SUBSETTER)
#include SK_SFNTLY_SUBSETTER
#endif

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
           parsePFBSection(&srcPtr, &remaining, 3, NULL);
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

SkStream* handleType1Stream(SkStream* srcStream, size_t* headerLen,
                            size_t* dataLen, size_t* trailerLen) {
    // srcStream may be backed by a file or a unseekable fd, so we may not be
    // able to use skip(), rewind(), or getMemoryBase().  read()ing through
    // the input only once is doable, but very ugly. Furthermore, it'd be nice
    // if the data was NUL terminated so that we can use strstr() to search it.
    // Make as few copies as possible given these constraints.
    SkDynamicMemoryWStream dynamicStream;
    SkRefPtr<SkMemoryStream> staticStream;
    SkData* data = NULL;
    const uint8_t* src;
    size_t srcLen;
    if ((srcLen = srcStream->getLength()) > 0) {
        staticStream = new SkMemoryStream(srcLen + 1);
        staticStream->unref();  // new and SkRefPtr both took a ref.
        src = (const uint8_t*)staticStream->getMemoryBase();
        if (srcStream->getMemoryBase() != NULL) {
            memcpy((void *)src, srcStream->getMemoryBase(), srcLen);
        } else {
            size_t read = 0;
            while (read < srcLen) {
                size_t got = srcStream->read((void *)staticStream->getAtPos(),
                                             srcLen - read);
                if (got == 0) {
                    return NULL;
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
        dynamicStream.write(&amount, 1);  // NULL terminator.
        data = dynamicStream.copyToData();
        src = data->bytes();
        srcLen = data->size() - 1;
    }

    // this handles releasing the data we may have gotten from dynamicStream.
    // if data is null, it is a no-op
    SkAutoDataUnref aud(data);

    if (parsePFB(src, srcLen, headerLen, dataLen, trailerLen)) {
        SkMemoryStream* result =
            new SkMemoryStream(*headerLen + *dataLen + *trailerLen);
        memcpy((char*)result->getAtPos(), src + 6, *headerLen);
        result->seek(*headerLen);
        memcpy((char*)result->getAtPos(), src + 6 + *headerLen + 6, *dataLen);
        result->seek(*headerLen + *dataLen);
        memcpy((char*)result->getAtPos(), src + 6 + *headerLen + 6 + *dataLen,
               *trailerLen);
        result->rewind();
        return result;
    }

    // A PFA has to be converted for PDF.
    size_t hexDataLen;
    if (parsePFA((const char*)src, srcLen, headerLen, &hexDataLen, dataLen,
                 trailerLen)) {
        SkMemoryStream* result =
            new SkMemoryStream(*headerLen + *dataLen + *trailerLen);
        memcpy((char*)result->getAtPos(), src, *headerLen);
        result->seek(*headerLen);

        const uint8_t* hexData = src + *headerLen;
        const uint8_t* trailer = hexData + hexDataLen;
        size_t outputOffset = 0;
        uint8_t dataByte = 0;  // To hush compiler.
        bool highNibble = true;
        for (; hexData < trailer; hexData++) {
            char curNibble = hexToBin(*hexData);
            if (curNibble < 0) {
                continue;
            }
            if (highNibble) {
                dataByte = curNibble << 4;
                highNibble = false;
            } else {
                dataByte |= curNibble;
                highNibble = true;
                ((char *)result->getAtPos())[outputOffset++] = dataByte;
            }
        }
        if (!highNibble) {
            ((char *)result->getAtPos())[outputOffset++] = dataByte;
        }
        SkASSERT(outputOffset == *dataLen);
        result->seek(*headerLen + outputOffset);

        memcpy((char *)result->getAtPos(), src + *headerLen + hexDataLen,
               *trailerLen);
        result->rewind();
        return result;
    }

    return NULL;
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
    SkPDFScalar::Append(width, content);
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
    for (; advanceInfo != NULL; advanceInfo = advanceInfo->fNext.get()) {
        switch (advanceInfo->fType) {
            case SkAdvancedTypefaceMetrics::WidthRange::kDefault: {
                SkASSERT(advanceInfo->fAdvance.count() == 1);
                *defaultAdvance = advanceInfo->fAdvance[0];
                break;
            }
            case SkAdvancedTypefaceMetrics::WidthRange::kRange: {
                SkRefPtr<SkPDFArray> advanceArray = new SkPDFArray();
                advanceArray->unref();  // SkRefPtr and new both took a ref.
                for (int j = 0; j < advanceInfo->fAdvance.count(); j++)
                    appendAdvance(advanceInfo->fAdvance[j], emSize,
                                  advanceArray.get());
                result->appendInt(advanceInfo->fStartId);
                result->append(advanceArray.get());
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

static void append_tounicode_header(SkDynamicMemoryWStream* cmap) {
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
    // We specify codespacerange from 0x0000 to 0xFFFF because we convert our
    // code table from unsigned short (16-bits). Codespace range just tells the
    // PDF processor the valid range. It does not matter whether a complete
    // mapping is provided or not.
    const char* kTypeInfo =
        "/CMapName /Adobe-Identity-UCS def\n"
        "/CMapType 2 def\n"
        "1 begincodespacerange\n"
        "<0000> <FFFF>\n"
        "endcodespacerange\n";
    cmap->writeText(kTypeInfo);
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
// overlap, but succeeding maps superceded preceding maps."
//
// In case of searching text in PDF, bfrange will have higher precedence so
// typing char id 0x0014 in search box will get glyph id 0x0004 first.  However,
// the spec does not mention how will this kind of conflict being resolved.
//
// For the worst case (having 65536 continuous unicode and we use every other
// one of them), the possible savings by aggressive optimization is 416KB
// pre-compressed and does not provide enough motivation for implementation.
void append_cmap_sections(const SkTDArray<SkUnichar>& glyphToUnicode,
                          const SkPDFGlyphSet* subset,
                          SkDynamicMemoryWStream* cmap) {
    if (glyphToUnicode.isEmpty()) {
        return;
    }

    SkTDArray<BFChar> bfcharEntries;
    SkTDArray<BFRange> bfrangeEntries;

    BFRange currentRangeEntry = {0, 0, 0};
    bool rangeEmpty = true;
    const int count = glyphToUnicode.count();

    for (int i = 0; i < count + 1; ++i) {
        bool inSubset = i < count && (subset == NULL || subset->has(i));
        if (!rangeEmpty) {
            // PDF spec requires bfrange not changing the higher byte,
            // e.g. <1035> <10FF> <2222> is ok, but
            //      <1035> <1100> <2222> is no good
            bool inRange =
                i == currentRangeEntry.fEnd + 1 &&
                i >> 8 == currentRangeEntry.fStart >> 8 &&
                i < count &&
                glyphToUnicode[i] == currentRangeEntry.fUnicode + i -
                                         currentRangeEntry.fStart;
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
              currentRangeEntry.fUnicode = glyphToUnicode[i];
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
        const SkPDFGlyphSet* subset) {
    SkDynamicMemoryWStream cmap;
    append_tounicode_header(&cmap);
    append_cmap_sections(glyphToUnicode, subset, &cmap);
    append_cmap_footer(&cmap);
    SkRefPtr<SkMemoryStream> cmapStream = new SkMemoryStream();
    cmapStream->unref();  // SkRefPtr and new took a reference.
    cmapStream->setData(cmap.copyToData())->unref();
    return new SkPDFStream(cmapStream.get());
}

static void sk_delete_array(const void* ptr, size_t, void*) {
    // Use C-style cast to cast away const and cast type simultaneously.
    delete[] (unsigned char*)ptr;
}

static int get_subset_font_stream(const char* fontName,
                                  const SkTypeface* typeface,
                                  const SkTDArray<uint32_t>& subset,
                                  SkPDFStream** fontStream) {
    SkRefPtr<SkStream> fontData =
            SkFontHost::OpenStream(SkTypeface::UniqueID(typeface));
    fontData->unref();  // SkRefPtr and OpenStream both took a ref.

    int fontSize = fontData->getLength();

#if defined (SK_SFNTLY_SUBSETTER)
    // Read font into buffer.
    SkPDFStream* subsetFontStream = NULL;
    SkTDArray<unsigned char> originalFont;
    originalFont.setCount(fontSize);
    if (fontData->read(originalFont.begin(), fontSize) == (size_t)fontSize) {
        unsigned char* subsetFont = NULL;
        // sfntly requires unsigned int* to be passed in, as far as we know,
        // unsigned int is equivalent to uint32_t on all platforms.
        SK_COMPILE_ASSERT(sizeof(unsigned int) == sizeof(uint32_t),
                          unsigned_int_not_32_bits);
        int subsetFontSize = SfntlyWrapper::SubsetFont(fontName,
                                                       originalFont.begin(),
                                                       fontSize,
                                                       subset.begin(),
                                                       subset.count(),
                                                       &subsetFont);
        if (subsetFontSize > 0 && subsetFont != NULL) {
            SkAutoDataUnref data(SkData::NewWithProc(subsetFont,
                                                     subsetFontSize,
                                                     sk_delete_array,
                                                     NULL));
            subsetFontStream = new SkPDFStream(data.get());
            fontSize = subsetFontSize;
        }
    }
    if (subsetFontStream) {
        *fontStream = subsetFontStream;
        return fontSize;
    }
#endif

    // Fail over: just embed the whole font.
    *fontStream = new SkPDFStream(fontData.get());
    return fontSize;
}

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

SkPDFGlyphSetMap::FontGlyphSetPair* SkPDFGlyphSetMap::F2BIter::next() const {
    if (fIndex >= fMap->count()) {
        return NULL;
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
        delete fMap[i].fGlyphSet;  // Should not be NULL.
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

SkPDFFont::~SkPDFFont() {
    SkAutoMutexAcquire lock(CanonicalFontsMutex());
    int index;
    if (Find(SkTypeface::UniqueID(fTypeface.get()), fFirstGlyphID, &index) &&
            CanonicalFonts()[index].fFont == this) {
        CanonicalFonts().removeShuffle(index);
    }
    fResources.unrefAll();
}

void SkPDFFont::getResources(SkTDArray<SkPDFObject*>* resourceList) {
    GetResourcesHelper(&fResources, resourceList);
}

SkTypeface* SkPDFFont::typeface() {
    return fTypeface.get();
}

SkAdvancedTypefaceMetrics::FontType SkPDFFont::getType() {
    return fFontType;
}

bool SkPDFFont::hasGlyph(uint16_t id) {
    return (id >= fFirstGlyphID && id <= fLastGlyphID) || id == 0;
}

size_t SkPDFFont::glyphsToPDFFontEncoding(uint16_t* glyphIDs,
                                          size_t numGlyphs) {
    // A font with multibyte glyphs will support all glyph IDs in a single font.
    if (this->multiByteGlyphs()) {
        return numGlyphs;
    }

    for (size_t i = 0; i < numGlyphs; i++) {
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
SkPDFFont* SkPDFFont::GetFontResource(SkTypeface* typeface, uint16_t glyphID) {
    SkAutoMutexAcquire lock(CanonicalFontsMutex());
    const uint32_t fontID = SkTypeface::UniqueID(typeface);
    int relatedFontIndex;
    if (Find(fontID, glyphID, &relatedFontIndex)) {
        CanonicalFonts()[relatedFontIndex].fFont->ref();
        return CanonicalFonts()[relatedFontIndex].fFont;
    }

    SkRefPtr<SkAdvancedTypefaceMetrics> fontMetrics;
    SkPDFDict* relatedFontDescriptor = NULL;
    if (relatedFontIndex >= 0) {
        SkPDFFont* relatedFont = CanonicalFonts()[relatedFontIndex].fFont;
        fontMetrics = relatedFont->fontInfo();
        relatedFontDescriptor = relatedFont->getFontDescriptor();
    } else {
        SkAdvancedTypefaceMetrics::PerGlyphInfo info;
        info = SkAdvancedTypefaceMetrics::kGlyphNames_PerGlyphInfo;
        info = SkTBitOr<SkAdvancedTypefaceMetrics::PerGlyphInfo>(
                  info, SkAdvancedTypefaceMetrics::kToUnicode_PerGlyphInfo);
#if !defined (SK_SFNTLY_SUBSETTER)
        info = SkTBitOr<SkAdvancedTypefaceMetrics::PerGlyphInfo>(
                  info, SkAdvancedTypefaceMetrics::kHAdvance_PerGlyphInfo);
#endif
        fontMetrics =
            SkFontHost::GetAdvancedTypefaceMetrics(fontID, info, NULL, 0);
        SkSafeUnref(fontMetrics.get());  // SkRefPtr and Get both took a ref.
#if defined (SK_SFNTLY_SUBSETTER)
        if (fontMetrics &&
            fontMetrics->fType != SkAdvancedTypefaceMetrics::kTrueType_Font) {
            // Font does not support subsetting, get new info with advance.
            info = SkTBitOr<SkAdvancedTypefaceMetrics::PerGlyphInfo>(
                      info, SkAdvancedTypefaceMetrics::kHAdvance_PerGlyphInfo);
            fontMetrics =
                SkFontHost::GetAdvancedTypefaceMetrics(fontID, info, NULL, 0);
            SkSafeUnref(fontMetrics.get());  // SkRefPtr and Get both took a ref
        }
#endif
    }

    SkPDFFont* font = Create(fontMetrics.get(), typeface, glyphID,
                             relatedFontDescriptor);
    FontRec newEntry(font, fontID, font->fFirstGlyphID);
    CanonicalFonts().push(newEntry);
    return font;  // Return the reference new SkPDFFont() created.
}

SkPDFFont* SkPDFFont::getFontSubset(const SkPDFGlyphSet* usage) {
    return NULL;  // Default: no support.
}

// static
SkTDArray<SkPDFFont::FontRec>& SkPDFFont::CanonicalFonts() {
    // This initialization is only thread safe with gcc.
    static SkTDArray<FontRec> gCanonicalFonts;
    return gCanonicalFonts;
}

// static
SkBaseMutex& SkPDFFont::CanonicalFontsMutex() {
    // This initialization is only thread safe with gcc, or when
    // POD-style mutex initialization is used.
    SK_DECLARE_STATIC_MUTEX(gCanonicalFontsMutex);
    return gCanonicalFontsMutex;
}

// static
bool SkPDFFont::Find(uint32_t fontID, uint16_t glyphID, int* index) {
    // TODO(vandebo): Optimize this, do only one search?
    FontRec search(NULL, fontID, glyphID);
    *index = CanonicalFonts().find(search);
    if (*index >= 0) {
        return true;
    }
    search.fGlyphID = 0;
    *index = CanonicalFonts().find(search);
    return false;
}

SkPDFFont::SkPDFFont(SkAdvancedTypefaceMetrics* info, SkTypeface* typeface,
                     uint16_t glyphID, bool descendantFont)
        : SkPDFDict("Font"),
          fTypeface(typeface),
          fFirstGlyphID(1),
          fLastGlyphID(info ? info->fLastGlyphID : 0),
          fFontInfo(info) {
    if (info == NULL) {
        fFontType = SkAdvancedTypefaceMetrics::kNotEmbeddable_Font;
    } else if (info->fMultiMaster) {
        fFontType = SkAdvancedTypefaceMetrics::kOther_Font;
    } else {
        fFontType = info->fType;
    }
}

// static
SkPDFFont* SkPDFFont::Create(SkAdvancedTypefaceMetrics* info,
                             SkTypeface* typeface, uint16_t glyphID,
                             SkPDFDict* relatedFontDescriptor) {
    SkAdvancedTypefaceMetrics::FontType type =
        info ? info->fType : SkAdvancedTypefaceMetrics::kNotEmbeddable_Font;

    if (info && info->fMultiMaster) {
        NOT_IMPLEMENTED(true, true);
        return new SkPDFType3Font(info,
                                  typeface,
                                  glyphID,
                                  relatedFontDescriptor);
    }
    if (type == SkAdvancedTypefaceMetrics::kType1CID_Font ||
        type == SkAdvancedTypefaceMetrics::kTrueType_Font) {
        SkASSERT(relatedFontDescriptor == NULL);
        return new SkPDFType0Font(info, typeface);
    }
    if (type == SkAdvancedTypefaceMetrics::kType1_Font) {
        return new SkPDFType1Font(info,
                                  typeface,
                                  glyphID,
                                  relatedFontDescriptor);
    }

    SkASSERT(type == SkAdvancedTypefaceMetrics::kCFF_Font ||
             type == SkAdvancedTypefaceMetrics::kOther_Font ||
             type == SkAdvancedTypefaceMetrics::kNotEmbeddable_Font);

    return new SkPDFType3Font(info, typeface, glyphID, relatedFontDescriptor);
}

SkAdvancedTypefaceMetrics* SkPDFFont::fontInfo() {
    return fFontInfo.get();
}

void SkPDFFont::setFontInfo(SkAdvancedTypefaceMetrics* info) {
    if (info == NULL || info == fFontInfo.get()) {
        return;
    }
    fFontInfo = info;
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

void SkPDFFont::addResource(SkPDFObject* object) {
    SkASSERT(object != NULL);
    fResources.push(object);
}

SkPDFDict* SkPDFFont::getFontDescriptor() {
    return fDescriptor.get();
}

void SkPDFFont::setFontDescriptor(SkPDFDict* descriptor) {
    fDescriptor = descriptor;
}

bool SkPDFFont::addCommonFontDescriptorEntries(int16_t defaultWidth) {
    if (fDescriptor.get() == NULL) {
        return false;
    }

    const uint16_t emSize = fFontInfo->fEmSize;

    fDescriptor->insertName("FontName", fFontInfo->fFontName);
    fDescriptor->insertInt("Flags", fFontInfo->fStyle);
    fDescriptor->insertScalar("Ascent",
            scaleFromFontUnits(fFontInfo->fAscent, emSize));
    fDescriptor->insertScalar("Descent",
            scaleFromFontUnits(fFontInfo->fDescent, emSize));
    fDescriptor->insertScalar("StemV",
            scaleFromFontUnits(fFontInfo->fStemV, emSize));
    fDescriptor->insertScalar("CapHeight",
            scaleFromFontUnits(fFontInfo->fCapHeight, emSize));
    fDescriptor->insertInt("ItalicAngle", fFontInfo->fItalicAngle);
    fDescriptor->insert("FontBBox", makeFontBBox(fFontInfo->fBBox,
                                                 fFontInfo->fEmSize))->unref();

    if (defaultWidth > 0) {
        fDescriptor->insertScalar("MissingWidth",
                scaleFromFontUnits(defaultWidth, emSize));
    }
    return true;
}

void SkPDFFont::adjustGlyphRangeForSingleByteEncoding(int16_t glyphID) {
    // Single byte glyph encoding supports a max of 255 glyphs.
    fFirstGlyphID = glyphID - (glyphID - 1) % 255;
    if (fLastGlyphID > fFirstGlyphID + 255 - 1) {
        fLastGlyphID = fFirstGlyphID + 255 - 1;
    }
}

bool SkPDFFont::FontRec::operator==(const SkPDFFont::FontRec& b) const {
    if (fFontID != b.fFontID) {
        return false;
    }
    if (fFont != NULL && b.fFont != NULL) {
        return fFont->fFirstGlyphID == b.fFont->fFirstGlyphID &&
            fFont->fLastGlyphID == b.fFont->fLastGlyphID;
    }
    if (fGlyphID == 0 || b.fGlyphID == 0) {
        return true;
    }

    if (fFont != NULL) {
        return fFont->fFirstGlyphID <= b.fGlyphID &&
            b.fGlyphID <= fFont->fLastGlyphID;
    } else if (b.fFont != NULL) {
        return b.fFont->fFirstGlyphID <= fGlyphID &&
            fGlyphID <= b.fFont->fLastGlyphID;
    }
    return fGlyphID == b.fGlyphID;
}

SkPDFFont::FontRec::FontRec(SkPDFFont* font, uint32_t fontID, uint16_t glyphID)
    : fFont(font),
      fFontID(fontID),
      fGlyphID(glyphID) {
}

void SkPDFFont::populateToUnicodeTable(const SkPDFGlyphSet* subset) {
    if (fFontInfo == NULL || fFontInfo->fGlyphToUnicode.begin() == NULL) {
        return;
    }
    SkRefPtr<SkPDFStream> pdfCmap =
        generate_tounicode_cmap(fFontInfo->fGlyphToUnicode, subset);
    addResource(pdfCmap.get());  // Pass reference from new.
    insert("ToUnicode", new SkPDFObjRef(pdfCmap.get()))->unref();
}

///////////////////////////////////////////////////////////////////////////////
// class SkPDFType0Font
///////////////////////////////////////////////////////////////////////////////

SkPDFType0Font::SkPDFType0Font(SkAdvancedTypefaceMetrics* info,
                               SkTypeface* typeface)
        : SkPDFFont(info, typeface, 0, false) {
    SkDEBUGCODE(fPopulated = false);
}

SkPDFType0Font::~SkPDFType0Font() {}

SkPDFFont* SkPDFType0Font::getFontSubset(const SkPDFGlyphSet* subset) {
    SkPDFType0Font* newSubset = new SkPDFType0Font(fontInfo(), typeface());
    newSubset->populate(subset);
    return newSubset;
}

#ifdef SK_DEBUG
void SkPDFType0Font::emitObject(SkWStream* stream, SkPDFCatalog* catalog,
                                bool indirect) {
    SkASSERT(fPopulated);
    return INHERITED::emitObject(stream, catalog, indirect);
}
#endif

bool SkPDFType0Font::populate(const SkPDFGlyphSet* subset) {
    insertName("Subtype", "Type0");
    insertName("BaseFont", fontInfo()->fFontName);
    insertName("Encoding", "Identity-H");

    SkPDFCIDFont* newCIDFont;
    newCIDFont = new SkPDFCIDFont(fontInfo(), typeface(), subset);

    // Pass ref new created to fResources.
    addResource(newCIDFont);
    SkRefPtr<SkPDFArray> descendantFonts = new SkPDFArray();
    descendantFonts->unref();  // SkRefPtr and new took a reference.
    descendantFonts->append(new SkPDFObjRef(newCIDFont))->unref();
    insert("DescendantFonts", descendantFonts.get());

    populateToUnicodeTable(subset);

    SkDEBUGCODE(fPopulated = true);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// class SkPDFCIDFont
///////////////////////////////////////////////////////////////////////////////

SkPDFCIDFont::SkPDFCIDFont(SkAdvancedTypefaceMetrics* info,
                           SkTypeface* typeface, const SkPDFGlyphSet* subset)
        : SkPDFFont(info, typeface, 0, true) {
    populate(subset);
}

SkPDFCIDFont::~SkPDFCIDFont() {}

bool SkPDFCIDFont::addFontDescriptor(int16_t defaultWidth,
                                     const SkTDArray<uint32_t>* subset) {
    SkRefPtr<SkPDFDict> descriptor = new SkPDFDict("FontDescriptor");
    descriptor->unref();  // SkRefPtr and new both took a ref.
    setFontDescriptor(descriptor.get());

    switch (getType()) {
        case SkAdvancedTypefaceMetrics::kTrueType_Font: {
            SkASSERT(subset);
            // Font subsetting
            SkPDFStream* rawStream = NULL;
            int fontSize = get_subset_font_stream(fontInfo()->fFontName.c_str(),
                                                  typeface(),
                                                  *subset,
                                                  &rawStream);
            SkASSERT(fontSize);
            SkASSERT(rawStream);
            SkRefPtr<SkPDFStream> fontStream = rawStream;
            // SkRefPtr and new both ref()'d fontStream, pass one.
            addResource(fontStream.get());

            fontStream->insertInt("Length1", fontSize);
            descriptor->insert("FontFile2",
                                new SkPDFObjRef(fontStream.get()))->unref();
            break;
        }
        case SkAdvancedTypefaceMetrics::kCFF_Font:
        case SkAdvancedTypefaceMetrics::kType1CID_Font: {
            SkRefPtr<SkStream> fontData =
                SkFontHost::OpenStream(SkTypeface::UniqueID(typeface()));
            fontData->unref();  // SkRefPtr and OpenStream both took a ref.
            SkRefPtr<SkPDFStream> fontStream = new SkPDFStream(fontData.get());
            // SkRefPtr and new both ref()'d fontStream, pass one.
            addResource(fontStream.get());

            if (getType() == SkAdvancedTypefaceMetrics::kCFF_Font) {
                fontStream->insertName("Subtype", "Type1C");
            } else {
                fontStream->insertName("Subtype", "CIDFontType0c");
            }
            descriptor->insert("FontFile3",
                                new SkPDFObjRef(fontStream.get()))->unref();
            break;
        }
        default:
            SkASSERT(false);
    }

    addResource(descriptor.get());
    descriptor->ref();

    insert("FontDescriptor", new SkPDFObjRef(descriptor.get()))->unref();
    return addCommonFontDescriptorEntries(defaultWidth);
}

bool SkPDFCIDFont::populate(const SkPDFGlyphSet* subset) {
    // Generate new font metrics with advance info for true type fonts.
    if (fontInfo()->fType == SkAdvancedTypefaceMetrics::kTrueType_Font) {
        // Generate glyph id array.
        SkTDArray<uint32_t> glyphIDs;
        glyphIDs.push(0);  // Always include glyph 0.
        if (subset) {
            subset->exportTo(&glyphIDs);
        }

        SkRefPtr<SkAdvancedTypefaceMetrics> fontMetrics;
        SkAdvancedTypefaceMetrics::PerGlyphInfo info;
        info = SkAdvancedTypefaceMetrics::kGlyphNames_PerGlyphInfo;
        info = SkTBitOr<SkAdvancedTypefaceMetrics::PerGlyphInfo>(
                  info, SkAdvancedTypefaceMetrics::kHAdvance_PerGlyphInfo);
        uint32_t* glyphs = (glyphIDs.count() == 1) ? NULL : glyphIDs.begin();
        uint32_t glyphsCount = glyphs ? glyphIDs.count() : 0;
        fontMetrics =
            SkFontHost::GetAdvancedTypefaceMetrics(
                    SkTypeface::UniqueID(typeface()),
                    info,
                    glyphs,
                    glyphsCount);
        SkSafeUnref(fontMetrics.get());  // SkRefPtr and Get both took a ref
        setFontInfo(fontMetrics.get());
        addFontDescriptor(0, &glyphIDs);
    } else {
        // Other CID fonts
        addFontDescriptor(0, NULL);
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

    SkRefPtr<SkPDFDict> sysInfo = new SkPDFDict;
    sysInfo->unref();  // SkRefPtr and new both took a reference.
    sysInfo->insert("Registry", new SkPDFString("Adobe"))->unref();
    sysInfo->insert("Ordering", new SkPDFString("Identity"))->unref();
    sysInfo->insertInt("Supplement", 0);
    insert("CIDSystemInfo", sysInfo.get());

    if (fontInfo()->fGlyphWidths.get()) {
        int16_t defaultWidth = 0;
        SkRefPtr<SkPDFArray> widths =
            composeAdvanceData(fontInfo()->fGlyphWidths.get(),
                               fontInfo()->fEmSize, &appendWidth,
                               &defaultWidth);
        widths->unref();  // SkRefPtr and compose both took a reference.
        if (widths->size())
            insert("W", widths.get());
        if (defaultWidth != 0) {
            insertScalar("DW", scaleFromFontUnits(defaultWidth,
                                                  fontInfo()->fEmSize));
        }
    }
    if (fontInfo()->fVerticalMetrics.get()) {
        struct SkAdvancedTypefaceMetrics::VerticalMetric defaultAdvance;
        defaultAdvance.fVerticalAdvance = 0;
        defaultAdvance.fOriginXDisp = 0;
        defaultAdvance.fOriginYDisp = 0;
        SkRefPtr<SkPDFArray> advances =
            composeAdvanceData(fontInfo()->fVerticalMetrics.get(),
                               fontInfo()->fEmSize, &appendVerticalAdvance,
                               &defaultAdvance);
        advances->unref();  // SkRefPtr and compose both took a ref.
        if (advances->size())
            insert("W2", advances.get());
        if (defaultAdvance.fVerticalAdvance ||
                defaultAdvance.fOriginXDisp ||
                defaultAdvance.fOriginYDisp) {
            insert("DW2", appendVerticalAdvance(defaultAdvance,
                                                fontInfo()->fEmSize,
                                                new SkPDFArray))->unref();
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// class SkPDFType1Font
///////////////////////////////////////////////////////////////////////////////

SkPDFType1Font::SkPDFType1Font(SkAdvancedTypefaceMetrics* info,
                               SkTypeface* typeface,
                               uint16_t glyphID,
                               SkPDFDict* relatedFontDescriptor)
        : SkPDFFont(info, typeface, glyphID, false) {
    populate(glyphID);
}

SkPDFType1Font::~SkPDFType1Font() {}

bool SkPDFType1Font::addFontDescriptor(int16_t defaultWidth) {
    SkRefPtr<SkPDFDict> descriptor = getFontDescriptor();
    if (descriptor.get() != NULL) {
        addResource(descriptor.get());
        descriptor->ref();
        insert("FontDescriptor", new SkPDFObjRef(descriptor.get()))->unref();
        return true;
    }

    descriptor = new SkPDFDict("FontDescriptor");
    descriptor->unref();  // SkRefPtr and new both took a ref.
    setFontDescriptor(descriptor.get());

    size_t header SK_INIT_TO_AVOID_WARNING;
    size_t data SK_INIT_TO_AVOID_WARNING;
    size_t trailer SK_INIT_TO_AVOID_WARNING;
    SkRefPtr<SkStream> rawFontData =
        SkFontHost::OpenStream(SkTypeface::UniqueID(typeface()));
    rawFontData->unref();  // SkRefPtr and OpenStream both took a ref.
    SkStream* fontData = handleType1Stream(rawFontData.get(), &header, &data,
                                           &trailer);
    if (fontData == NULL) {
        return false;
    }
    SkRefPtr<SkPDFStream> fontStream = new SkPDFStream(fontData);
    // SkRefPtr and new both ref()'d fontStream, pass one.
    addResource(fontStream.get());
    fontStream->insertInt("Length1", header);
    fontStream->insertInt("Length2", data);
    fontStream->insertInt("Length3", trailer);
    descriptor->insert("FontFile", new SkPDFObjRef(fontStream.get()))->unref();

    addResource(descriptor.get());
    descriptor->ref();
    insert("FontDescriptor", new SkPDFObjRef(descriptor.get()))->unref();

    return addCommonFontDescriptorEntries(defaultWidth);
}

bool SkPDFType1Font::populate(int16_t glyphID) {
    SkASSERT(!fontInfo()->fVerticalMetrics.get());
    SkASSERT(fontInfo()->fGlyphWidths.get());

    adjustGlyphRangeForSingleByteEncoding(glyphID);

    int16_t defaultWidth = 0;
    const SkAdvancedTypefaceMetrics::WidthRange* widthRangeEntry = NULL;
    const SkAdvancedTypefaceMetrics::WidthRange* widthEntry;
    for (widthEntry = fontInfo()->fGlyphWidths.get();
            widthEntry != NULL;
            widthEntry = widthEntry->fNext.get()) {
        switch (widthEntry->fType) {
            case SkAdvancedTypefaceMetrics::WidthRange::kDefault:
                defaultWidth = widthEntry->fAdvance[0];
                break;
            case SkAdvancedTypefaceMetrics::WidthRange::kRun:
                SkASSERT(false);
                break;
            case SkAdvancedTypefaceMetrics::WidthRange::kRange:
                SkASSERT(widthRangeEntry == NULL);
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

    SkRefPtr<SkPDFDict> encoding = new SkPDFDict("Encoding");
    encoding->unref();  // SkRefPtr and new both took a reference.
    insert("Encoding", encoding.get());

    SkRefPtr<SkPDFArray> encDiffs = new SkPDFArray;
    encDiffs->unref();  // SkRefPtr and new both took a reference.
    encoding->insert("Differences", encDiffs.get());

    encDiffs->reserve(lastGlyphID() - firstGlyphID() + 2);
    encDiffs->appendInt(1);
    for (int gID = firstGlyphID(); gID <= lastGlyphID(); gID++) {
        encDiffs->appendName(fontInfo()->fGlyphNames->get()[gID].c_str());
    }

    return true;
}

void SkPDFType1Font::addWidthInfoFromRange(
        int16_t defaultWidth,
        const SkAdvancedTypefaceMetrics::WidthRange* widthRangeEntry) {
    SkRefPtr<SkPDFArray> widthArray = new SkPDFArray();
    widthArray->unref();  // SkRefPtr and new both took a ref.
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
    insertInt("FirstChar", firstChar);
    insertInt("LastChar", firstChar + widthArray->size() - 1);
    insert("Widths", widthArray.get());
}

///////////////////////////////////////////////////////////////////////////////
// class SkPDFType3Font
///////////////////////////////////////////////////////////////////////////////

SkPDFType3Font::SkPDFType3Font(SkAdvancedTypefaceMetrics* info,
                               SkTypeface* typeface,
                               uint16_t glyphID,
                               SkPDFDict* relatedFontDescriptor)
        : SkPDFFont(info, typeface, glyphID, false) {
    populate(glyphID);
}

SkPDFType3Font::~SkPDFType3Font() {}

bool SkPDFType3Font::populate(int16_t glyphID) {
    SkPaint paint;
    paint.setTypeface(typeface());
    paint.setTextSize(1000);
    SkAutoGlyphCache autoCache(paint, NULL);
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
    insert("FontMatrix", SkPDFUtils::MatrixToArray(fontMatrix))->unref();

    SkRefPtr<SkPDFDict> charProcs = new SkPDFDict;
    charProcs->unref();  // SkRefPtr and new both took a reference.
    insert("CharProcs", charProcs.get());

    SkRefPtr<SkPDFDict> encoding = new SkPDFDict("Encoding");
    encoding->unref();  // SkRefPtr and new both took a reference.
    insert("Encoding", encoding.get());

    SkRefPtr<SkPDFArray> encDiffs = new SkPDFArray;
    encDiffs->unref();  // SkRefPtr and new both took a reference.
    encoding->insert("Differences", encDiffs.get());
    encDiffs->reserve(lastGlyphID() - firstGlyphID() + 2);
    encDiffs->appendInt(1);

    SkRefPtr<SkPDFArray> widthArray = new SkPDFArray();
    widthArray->unref();  // SkRefPtr and new both took a ref.

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
        SkRefPtr<SkMemoryStream> glyphStream = new SkMemoryStream();
        glyphStream->unref();  // SkRefPtr and new both took a ref.
        glyphStream->setData(content.copyToData())->unref();

        SkRefPtr<SkPDFStream> glyphDescription =
            new SkPDFStream(glyphStream.get());
        // SkRefPtr and new both ref()'d charProcs, pass one.
        addResource(glyphDescription.get());
        charProcs->insert(characterName.c_str(),
                          new SkPDFObjRef(glyphDescription.get()))->unref();
    }

    insert("FontBBox", makeFontBBox(bbox, 1000))->unref();
    insertInt("FirstChar", firstGlyphID());
    insertInt("LastChar", lastGlyphID());
    insert("Widths", widthArray.get());
    insertName("CIDToGIDMap", "Identity");

    populateToUnicodeTable(NULL);
    return true;
}
