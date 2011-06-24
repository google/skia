/*
 * Copyright (C) 2011 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ctype.h>

#include "SkData.h"
#include "SkFontHost.h"
#include "SkGlyphCache.h"
#include "SkPaint.h"
#include "SkPDFDevice.h"
#include "SkPDFFont.h"
#include "SkPDFStream.h"
#include "SkPDFTypes.h"
#include "SkPDFUtils.h"
#include "SkRefCnt.h"
#include "SkScalar.h"
#include "SkStream.h"
#include "SkTypeface.h"
#include "SkTypes.h"
#include "SkUtils.h"

namespace {

bool parsePFBSection(const uint8_t** src, size_t* len, int sectionType,
                     size_t* size) {
    // PFB sections have a two or six bytes header. 0x80 and a one byte
    // section type followed by a four byte section length.  Type one is
    // an ASCII section (includes a length), type two is a binary section
    // (includes a length) and type three is an EOF marker with no length.
    const uint8_t* buf = *src;
    if (*len < 2 || buf[0] != 0x80 || buf[1] != sectionType)
        return false;
    if (buf[1] == 3)
        return true;
    if (*len < 6)
        return false;

    *size = (size_t)buf[2] | ((size_t)buf[3] << 8) | ((size_t)buf[4] << 16) |
            ((size_t)buf[5] << 24);
    size_t consumed = *size + 6;
    if (consumed > *len)
        return false;
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
    if (!dataPos)
        return false;
    dataPos += strlen("eexec");
    while ((*dataPos == '\n' || *dataPos == '\r' || *dataPos == ' ') &&
            dataPos < end)
        dataPos++;
    *headerLen = dataPos - src;

    const char* trailerPos = strstr(dataPos, "cleartomark");
    if (!trailerPos)
        return false;
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
    if (zeroCount != 512)
        return false;

    *hexDataLen = trailerPos - src - *headerLen;
    *trailerLen = size - *headerLen - *hexDataLen;

    // Verify that the data section is hex encoded and count the bytes.
    int nibbles = 0;
    for (; dataPos < trailerPos; dataPos++) {
        if (isspace(*dataPos))
            continue;
        if (!isxdigit(*dataPos))
            return false;
        nibbles++;
    }
    *dataLen = (nibbles + 1) / 2;

    return true;
}

int8_t hexToBin(uint8_t c) {
    if (!isxdigit(c))
        return -1;
    if (c <= '9') return c - '0';
    if (c <= 'F') return c - 'A' + 10;
    if (c <= 'f') return c - 'a' + 10;
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
                if (got == 0)
                    return NULL;
                read += got;
                staticStream->seek(read);
            }
        }
        ((uint8_t *)src)[srcLen] = 0;
    } else {
        static const size_t bufSize = 4096;
        uint8_t buf[bufSize];
        size_t amount;
        while ((amount = srcStream->read(buf, bufSize)) > 0)
            dynamicStream.write(buf, amount);
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
            if (curNibble < 0)
                continue;
            if (highNibble) {
                dataByte = curNibble << 4;
                highNibble = false;
            } else {
                dataByte |= curNibble;
                highNibble = true;
                ((char *)result->getAtPos())[outputOffset++] = dataByte;
            }
        }
        if (!highNibble)
            ((char *)result->getAtPos())[outputOffset++] = dataByte;
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
    bbox->append(new SkPDFScalar(scaleFromFontUnits(glyphBBox.fLeft,
                                                    emSize)))->unref();
    bbox->append(new SkPDFScalar(scaleFromFontUnits(glyphBBox.fBottom,
                                                    emSize)))->unref();
    bbox->append(new SkPDFScalar(scaleFromFontUnits(glyphBBox.fRight,
                                                    emSize)))->unref();
    bbox->append(new SkPDFScalar(scaleFromFontUnits(glyphBBox.fTop,
                                                    emSize)))->unref();
    return bbox;
}

SkPDFArray* appendWidth(const int16_t& width, uint16_t emSize,
                        SkPDFArray* array) {
    array->append(new SkPDFScalar(scaleFromFontUnits(width, emSize)))->unref();
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
                result->append(new SkPDFInt(advanceInfo->fStartId))->unref();
                result->append(advanceArray.get());
                break;
            }
            case SkAdvancedTypefaceMetrics::WidthRange::kRun: {
                SkASSERT(advanceInfo->fAdvance.count() == 1);
                result->append(new SkPDFInt(advanceInfo->fStartId))->unref();
                result->append(new SkPDFInt(advanceInfo->fEndId))->unref();
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

static void append_cmap_bfchar_table(uint16_t* glyph_id, SkUnichar* unicode,
                                     size_t count,
                                     SkDynamicMemoryWStream* cmap) {
    cmap->writeDecAsText(count);
    cmap->writeText(" beginbfchar\n");
    for (size_t i = 0; i < count; ++i) {
        cmap->writeText("<");
        cmap->writeHexAsText(glyph_id[i], 4);
        cmap->writeText("> <");
        cmap->writeHexAsText(unicode[i], 4);
        cmap->writeText(">\n");
    }
    cmap->writeText("endbfchar\n");
}

static void append_cmap_footer(SkDynamicMemoryWStream* cmap) {
    const char* kFooter =
        "endcmap\n"
        "CMapName currentdict /CMap defineresource pop\n"
        "end\n"
        "end";
    cmap->writeText(kFooter);
}

// Generate <bfchar> table according to PDF spec 1.4 and Adobe Technote 5014.
static void append_cmap_bfchar_sections(
                const SkTDArray<SkUnichar>& glyphUnicode,
                SkDynamicMemoryWStream* cmap) {
    // PDF spec defines that every bf* list can have at most 100 entries.
    const size_t kMaxEntries = 100;
    uint16_t glyphId[kMaxEntries];
    SkUnichar unicode[kMaxEntries];
    size_t index = 0;
    for (int i = 0; i < glyphUnicode.count(); i++) {
        if (glyphUnicode[i]) {
            glyphId[index] = i;
            unicode[index] = glyphUnicode[i];
            ++index;
        }
        if (index == kMaxEntries) {
            append_cmap_bfchar_table(glyphId, unicode, index, cmap);
            index = 0;
        }
    }

    if (index) {
        append_cmap_bfchar_table(glyphId, unicode, index, cmap);
    }
}

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
    SkAutoMutexAcquire lock(canonicalFontsMutex());
    int index;
    if (find(SkTypeface::UniqueID(fTypeface.get()), fFirstGlyphID, &index)) {
        canonicalFonts().removeShuffle(index);
#ifdef SK_DEBUG
        SkASSERT(!fDescendant);
    } else {
        SkASSERT(fDescendant);
#endif
    }
    fResources.unrefAll();
}

void SkPDFFont::getResources(SkTDArray<SkPDFObject*>* resourceList) {
    resourceList->setReserve(resourceList->count() + fResources.count());
    for (int i = 0; i < fResources.count(); i++) {
        resourceList->push(fResources[i]);
        fResources[i]->ref();
        fResources[i]->getResources(resourceList);
    }
}

SkTypeface* SkPDFFont::typeface() {
    return fTypeface.get();
}

SkAdvancedTypefaceMetrics::FontType SkPDFFont::getType() {
    return fType;
}

bool SkPDFFont::hasGlyph(uint16_t id) {
    return (id >= fFirstGlyphID && id <= fLastGlyphID) || id == 0;
}

bool SkPDFFont::multiByteGlyphs() {
    return fMultiByteGlyphs;
}

size_t SkPDFFont::glyphsToPDFFontEncoding(uint16_t* glyphIDs,
                                          size_t numGlyphs) {
    // A font with multibyte glyphs will support all glyph IDs in a single font.
    if (fMultiByteGlyphs) {
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
SkPDFFont* SkPDFFont::getFontResource(SkTypeface* typeface, uint16_t glyphID) {
    SkAutoMutexAcquire lock(canonicalFontsMutex());
    const uint32_t fontID = SkTypeface::UniqueID(typeface);
    int index;
    if (find(fontID, glyphID, &index)) {
        canonicalFonts()[index].fFont->ref();
        return canonicalFonts()[index].fFont;
    }

    SkRefPtr<SkAdvancedTypefaceMetrics> fontInfo;
    SkPDFDict* fontDescriptor = NULL;
    if (index >= 0) {
        SkPDFFont* relatedFont = canonicalFonts()[index].fFont;
        SkASSERT(relatedFont->fFontInfo.get());
        fontInfo = relatedFont->fFontInfo;
        fontDescriptor = relatedFont->fDescriptor.get();
    } else {
        SkAdvancedTypefaceMetrics::PerGlyphInfo info;
        info = SkAdvancedTypefaceMetrics::kHAdvance_PerGlyphInfo;
        info = SkTBitOr<SkAdvancedTypefaceMetrics::PerGlyphInfo>(
                  info, SkAdvancedTypefaceMetrics::kGlyphNames_PerGlyphInfo);
        info = SkTBitOr<SkAdvancedTypefaceMetrics::PerGlyphInfo>(
                  info, SkAdvancedTypefaceMetrics::kToUnicode_PerGlyphInfo);
        fontInfo = SkFontHost::GetAdvancedTypefaceMetrics(fontID, info);
        SkSafeUnref(fontInfo.get());  // SkRefPtr and Get both took a reference.
    }

    SkPDFFont* font = new SkPDFFont(fontInfo.get(), typeface, glyphID, false,
                                    fontDescriptor);
    FontRec newEntry(font, fontID, font->fFirstGlyphID);
    index = canonicalFonts().count();
    canonicalFonts().push(newEntry);
    return font;  // Return the reference new SkPDFFont() created.
}

// static
SkTDArray<SkPDFFont::FontRec>& SkPDFFont::canonicalFonts() {
    // This initialization is only thread safe with gcc.
    static SkTDArray<FontRec> gCanonicalFonts;
    return gCanonicalFonts;
}

// static
SkMutex& SkPDFFont::canonicalFontsMutex() {
    // This initialization is only thread safe with gcc.
    static SkMutex gCanonicalFontsMutex;
    return gCanonicalFontsMutex;
}

// static
bool SkPDFFont::find(uint32_t fontID, uint16_t glyphID, int* index) {
    // TODO(vandebo) optimize this, do only one search?
    FontRec search(NULL, fontID, glyphID);
    *index = canonicalFonts().find(search);
    if (*index >= 0)
        return true;
    search.fGlyphID = 0;
    *index = canonicalFonts().find(search);
    return false;
}

SkPDFFont::SkPDFFont(class SkAdvancedTypefaceMetrics* fontInfo,
                     SkTypeface* typeface,
                     uint16_t glyphID,
                     bool descendantFont,
                     SkPDFDict* fontDescriptor)
        : SkPDFDict("Font"),
          fTypeface(typeface),
          fType(fontInfo ? fontInfo->fType :
                           SkAdvancedTypefaceMetrics::kNotEmbeddable_Font),
#ifdef SK_DEBUG
          fDescendant(descendantFont),
#endif
          fMultiByteGlyphs(false),
          fFirstGlyphID(1),
          fLastGlyphID(fontInfo ? fontInfo->fLastGlyphID : 0),
          fFontInfo(fontInfo),
          fDescriptor(fontDescriptor) {
    if (fontInfo && fontInfo->fMultiMaster) {
        NOT_IMPLEMENTED(true, true);
        fType = SkAdvancedTypefaceMetrics::kOther_Font;
    }
    if (fType == SkAdvancedTypefaceMetrics::kType1CID_Font ||
        fType == SkAdvancedTypefaceMetrics::kTrueType_Font) {
        if (descendantFont) {
            populateCIDFont();
        } else {
            populateType0Font();
        }
        // No need to hold onto the font info for fonts types that
        // support multibyte glyphs.
        fFontInfo = NULL;
        return;
    }

    if (fType == SkAdvancedTypefaceMetrics::kType1_Font &&
        populateType1Font(glyphID)) {
        return;
    }

    SkASSERT(fType == SkAdvancedTypefaceMetrics::kType1_Font ||
             fType == SkAdvancedTypefaceMetrics::kCFF_Font ||
             fType == SkAdvancedTypefaceMetrics::kOther_Font ||
             fType == SkAdvancedTypefaceMetrics::kNotEmbeddable_Font);
    populateType3Font(glyphID);
}

void SkPDFFont::populateType0Font() {
    fMultiByteGlyphs = true;

    insert("Subtype", new SkPDFName("Type0"))->unref();
    insert("BaseFont", new SkPDFName(fFontInfo->fFontName))->unref();
    insert("Encoding",  new SkPDFName("Identity-H"))->unref();

    SkRefPtr<SkPDFArray> descendantFonts = new SkPDFArray();
    descendantFonts->unref();  // SkRefPtr and new took a reference.

    // Pass ref new created to fResources.
    fResources.push(
        new SkPDFFont(fFontInfo.get(), fTypeface.get(), 1, true, NULL));
    descendantFonts->append(new SkPDFObjRef(fResources.top()))->unref();
    insert("DescendantFonts", descendantFonts.get());

    populateToUnicodeTable();
}

void SkPDFFont::populateToUnicodeTable() {
    if (fFontInfo.get() == NULL ||
        fFontInfo->fGlyphToUnicode.begin() == NULL) {
        return;
    }

    SkDynamicMemoryWStream cmap;
    append_tounicode_header(&cmap);
    append_cmap_bfchar_sections(fFontInfo->fGlyphToUnicode, &cmap);
    append_cmap_footer(&cmap);
    SkRefPtr<SkMemoryStream> cmapStream = new SkMemoryStream();
    cmapStream->unref();  // SkRefPtr and new took a reference.
    cmapStream->setData(cmap.copyToData())->unref();
    SkRefPtr<SkPDFStream> pdfCmap = new SkPDFStream(cmapStream.get());
    fResources.push(pdfCmap.get());  // Pass reference from new.
    insert("ToUnicode", new SkPDFObjRef(pdfCmap.get()))->unref();
}

void SkPDFFont::populateCIDFont() {
    fMultiByteGlyphs = true;
    insert("BaseFont", new SkPDFName(fFontInfo->fFontName))->unref();

    if (fFontInfo->fType == SkAdvancedTypefaceMetrics::kType1CID_Font) {
        insert("Subtype", new SkPDFName("CIDFontType0"))->unref();
    } else if (fFontInfo->fType == SkAdvancedTypefaceMetrics::kTrueType_Font) {
        insert("Subtype", new SkPDFName("CIDFontType2"))->unref();
        insert("CIDToGIDMap", new SkPDFName("Identity"))->unref();
    } else {
        SkASSERT(false);
    }

    SkRefPtr<SkPDFDict> sysInfo = new SkPDFDict;
    sysInfo->unref();  // SkRefPtr and new both took a reference.
    sysInfo->insert("Registry", new SkPDFString("Adobe"))->unref();
    sysInfo->insert("Ordering", new SkPDFString("Identity"))->unref();
    sysInfo->insert("Supplement", new SkPDFInt(0))->unref();
    insert("CIDSystemInfo", sysInfo.get());

    addFontDescriptor(0);

    if (fFontInfo->fGlyphWidths.get()) {
        int16_t defaultWidth = 0;
        SkRefPtr<SkPDFArray> widths =
            composeAdvanceData(fFontInfo->fGlyphWidths.get(),
                               fFontInfo->fEmSize, &appendWidth, &defaultWidth);
        widths->unref();  // SkRefPtr and compose both took a reference.
        if (widths->size())
            insert("W", widths.get());
        if (defaultWidth != 0) {
            insert("DW", new SkPDFScalar(scaleFromFontUnits(
                    defaultWidth, fFontInfo->fEmSize)))->unref();
        }
    }
    if (fFontInfo->fVerticalMetrics.get()) {
        struct SkAdvancedTypefaceMetrics::VerticalMetric defaultAdvance;
        defaultAdvance.fVerticalAdvance = 0;
        defaultAdvance.fOriginXDisp = 0;
        defaultAdvance.fOriginYDisp = 0;
        SkRefPtr<SkPDFArray> advances =
            composeAdvanceData(fFontInfo->fVerticalMetrics.get(),
                               fFontInfo->fEmSize, &appendVerticalAdvance,
                               &defaultAdvance);
        advances->unref();  // SkRefPtr and compose both took a ref.
        if (advances->size())
            insert("W2", advances.get());
        if (defaultAdvance.fVerticalAdvance ||
                defaultAdvance.fOriginXDisp ||
                defaultAdvance.fOriginYDisp) {
            insert("DW2", appendVerticalAdvance(defaultAdvance,
                                                fFontInfo->fEmSize,
                                                new SkPDFArray))->unref();
        }
    }
}

bool SkPDFFont::populateType1Font(int16_t glyphID) {
    SkASSERT(!fFontInfo->fVerticalMetrics.get());
    SkASSERT(fFontInfo->fGlyphWidths.get());

    adjustGlyphRangeForSingleByteEncoding(glyphID);

    int16_t defaultWidth = 0;
    const SkAdvancedTypefaceMetrics::WidthRange* widthRangeEntry = NULL;
    const SkAdvancedTypefaceMetrics::WidthRange* widthEntry;
    for (widthEntry = fFontInfo.get()->fGlyphWidths.get();
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

    if (!addFontDescriptor(defaultWidth))
        return false;

    insert("Subtype", new SkPDFName("Type1"))->unref();
    insert("BaseFont", new SkPDFName(fFontInfo->fFontName))->unref();

    addWidthInfoFromRange(defaultWidth, widthRangeEntry);

    SkRefPtr<SkPDFDict> encoding = new SkPDFDict("Encoding");
    encoding->unref();  // SkRefPtr and new both took a reference.
    insert("Encoding", encoding.get());

    SkRefPtr<SkPDFArray> encDiffs = new SkPDFArray;
    encDiffs->unref();  // SkRefPtr and new both took a reference.
    encoding->insert("Differences", encDiffs.get());

    encDiffs->reserve(fLastGlyphID - fFirstGlyphID + 2);
    encDiffs->append(new SkPDFInt(1))->unref();
    for (int gID = fFirstGlyphID; gID <= fLastGlyphID; gID++) {
        encDiffs->append(
            new SkPDFName(fFontInfo->fGlyphNames->get()[gID]))->unref();
    }

    if (fFontInfo->fLastGlyphID <= 255)
        fFontInfo = NULL;
    return true;
}

void SkPDFFont::populateType3Font(int16_t glyphID) {
    SkPaint paint;
    paint.setTypeface(fTypeface.get());
    paint.setTextSize(1000);
    SkAutoGlyphCache autoCache(paint, NULL);
    SkGlyphCache* cache = autoCache.getCache();
    // If fLastGlyphID isn't set (because there is not fFontInfo), look it up.
    if (fLastGlyphID == 0) {
        fLastGlyphID = cache->getGlyphCount() - 1;
    }

    adjustGlyphRangeForSingleByteEncoding(glyphID);

    insert("Subtype", new SkPDFName("Type3"))->unref();
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
    encDiffs->reserve(fLastGlyphID - fFirstGlyphID + 2);
    encDiffs->append(new SkPDFInt(1))->unref();

    SkRefPtr<SkPDFArray> widthArray = new SkPDFArray();
    widthArray->unref();  // SkRefPtr and new both took a ref.

    SkIRect bbox = SkIRect::MakeEmpty();
    for (int gID = fFirstGlyphID; gID <= fLastGlyphID; gID++) {
        SkString characterName;
        characterName.printf("gid%d", gID);
        encDiffs->append(new SkPDFName(characterName))->unref();

        const SkGlyph& glyph = cache->getGlyphIDMetrics(gID);
        widthArray->append(new SkPDFScalar(SkFixedToScalar(glyph.fAdvanceX)))->unref();
        SkIRect glyphBBox = SkIRect::MakeXYWH(glyph.fLeft, glyph.fTop,
                                              glyph.fWidth, glyph.fHeight);
        bbox.join(glyphBBox);

        SkDynamicMemoryWStream content;
        setGlyphWidthAndBoundingBox(SkFixedToScalar(glyph.fAdvanceX), glyphBBox,
                                    &content);
        const SkPath* path = cache->findPath(glyph);
        if (path) {
            SkPDFUtils::EmitPath(*path, &content);
            SkPDFUtils::PaintPath(paint.getStyle(), path->getFillType(),
                                  &content);
        }
        SkRefPtr<SkMemoryStream> glyphStream = new SkMemoryStream();
        glyphStream->unref();  // SkRefPtr and new both took a ref.
        glyphStream->setData(content.copyToData())->unref();

        SkRefPtr<SkPDFStream> glyphDescription =
            new SkPDFStream(glyphStream.get());
        // SkRefPtr and new both ref()'d charProcs, pass one.
        fResources.push(glyphDescription.get());
        charProcs->insert(characterName.c_str(),
                          new SkPDFObjRef(glyphDescription.get()))->unref();
    }

    insert("FontBBox", makeFontBBox(bbox, 1000))->unref();
    insert("FirstChar", new SkPDFInt(fFirstGlyphID))->unref();
    insert("LastChar", new SkPDFInt(fLastGlyphID))->unref();
    insert("Widths", widthArray.get());
    insert("CIDToGIDMap", new SkPDFName("Identity"))->unref();

    if (fFontInfo && fFontInfo->fLastGlyphID <= 255)
        fFontInfo = NULL;

    populateToUnicodeTable();
}

bool SkPDFFont::addFontDescriptor(int16_t defaultWidth) {
    if (fDescriptor.get() != NULL) {
        fResources.push(fDescriptor.get());
        fDescriptor->ref();
        insert("FontDescriptor", new SkPDFObjRef(fDescriptor.get()))->unref();
        return true;
    }

    fDescriptor = new SkPDFDict("FontDescriptor");
    fDescriptor->unref();  // SkRefPtr and new both took a ref.

    switch (fFontInfo->fType) {
        case SkAdvancedTypefaceMetrics::kType1_Font: {
            size_t header SK_INIT_TO_AVOID_WARNING;
            size_t data SK_INIT_TO_AVOID_WARNING;
            size_t trailer SK_INIT_TO_AVOID_WARNING;
            SkRefPtr<SkStream> rawFontData =
                SkFontHost::OpenStream(SkTypeface::UniqueID(fTypeface.get()));
            rawFontData->unref();  // SkRefPtr and OpenStream both took a ref.
            SkStream* fontData = handleType1Stream(rawFontData.get(), &header,
                                                   &data, &trailer);
            if (fontData == NULL)
                return false;
            SkRefPtr<SkPDFStream> fontStream = new SkPDFStream(fontData);
            // SkRefPtr and new both ref()'d fontStream, pass one.
            fResources.push(fontStream.get());
            fontStream->insert("Length1", new SkPDFInt(header))->unref();
            fontStream->insert("Length2", new SkPDFInt(data))->unref();
            fontStream->insert("Length3", new SkPDFInt(trailer))->unref();
            fDescriptor->insert("FontFile",
                                new SkPDFObjRef(fontStream.get()))->unref();
            break;
        }
        case SkAdvancedTypefaceMetrics::kTrueType_Font: {
            SkRefPtr<SkStream> fontData =
                SkFontHost::OpenStream(SkTypeface::UniqueID(fTypeface.get()));
            fontData->unref();  // SkRefPtr and OpenStream both took a ref.
            SkRefPtr<SkPDFStream> fontStream = new SkPDFStream(fontData.get());
            // SkRefPtr and new both ref()'d fontStream, pass one.
            fResources.push(fontStream.get());

            fontStream->insert("Length1",
                               new SkPDFInt(fontData->getLength()))->unref();
            fDescriptor->insert("FontFile2",
                                new SkPDFObjRef(fontStream.get()))->unref();
            break;
        }
        case SkAdvancedTypefaceMetrics::kCFF_Font:
        case SkAdvancedTypefaceMetrics::kType1CID_Font: {
            SkRefPtr<SkStream> fontData =
                SkFontHost::OpenStream(SkTypeface::UniqueID(fTypeface.get()));
            fontData->unref();  // SkRefPtr and OpenStream both took a ref.
            SkRefPtr<SkPDFStream> fontStream = new SkPDFStream(fontData.get());
            // SkRefPtr and new both ref()'d fontStream, pass one.
            fResources.push(fontStream.get());

            if (fFontInfo->fType == SkAdvancedTypefaceMetrics::kCFF_Font) {
                fontStream->insert("Subtype", new SkPDFName("Type1C"))->unref();
            } else {
                fontStream->insert("Subtype",
                        new SkPDFName("CIDFontType0c"))->unref();
            }
            fDescriptor->insert("FontFile3",
                                new SkPDFObjRef(fontStream.get()))->unref();
            break;
        }
        default:
            SkASSERT(false);
    }

    const uint16_t emSize = fFontInfo->fEmSize;
    fResources.push(fDescriptor.get());
    fDescriptor->ref();
    insert("FontDescriptor", new SkPDFObjRef(fDescriptor.get()))->unref();

    fDescriptor->insert("FontName", new SkPDFName(
            fFontInfo->fFontName))->unref();
    fDescriptor->insert("Flags", new SkPDFInt(fFontInfo->fStyle))->unref();
    fDescriptor->insert("Ascent", new SkPDFScalar(
            scaleFromFontUnits(fFontInfo->fAscent, emSize)))->unref();
    fDescriptor->insert("Descent", new SkPDFScalar(
            scaleFromFontUnits(fFontInfo->fDescent, emSize)))->unref();
    fDescriptor->insert("StemV", new SkPDFScalar(
            scaleFromFontUnits(fFontInfo->fStemV, emSize)))->unref();
    fDescriptor->insert("CapHeight", new SkPDFScalar(
            scaleFromFontUnits(fFontInfo->fCapHeight, emSize)))->unref();
    fDescriptor->insert("ItalicAngle", new SkPDFInt(
            fFontInfo->fItalicAngle))->unref();
    fDescriptor->insert("FontBBox", makeFontBBox(fFontInfo->fBBox,
                                                 fFontInfo->fEmSize))->unref();

    if (defaultWidth > 0) {
        fDescriptor->insert("MissingWidth", new SkPDFScalar(
                scaleFromFontUnits(defaultWidth, emSize)))->unref();
    }
    return true;
}
void SkPDFFont::addWidthInfoFromRange(
        int16_t defaultWidth,
        const SkAdvancedTypefaceMetrics::WidthRange* widthRangeEntry) {
    SkRefPtr<SkPDFArray> widthArray = new SkPDFArray();
    widthArray->unref();  // SkRefPtr and new both took a ref.
    int firstChar = 0;
    if (widthRangeEntry) {
        const uint16_t emSize = fFontInfo->fEmSize;
        int startIndex = fFirstGlyphID - widthRangeEntry->fStartId;
        int endIndex = startIndex + fLastGlyphID - fFirstGlyphID + 1;
        if (startIndex < 0)
            startIndex = 0;
        if (endIndex > widthRangeEntry->fAdvance.count())
            endIndex = widthRangeEntry->fAdvance.count();
        if (widthRangeEntry->fStartId == 0) {
            appendWidth(widthRangeEntry->fAdvance[0], emSize, widthArray.get());
        } else {
            firstChar = startIndex + widthRangeEntry->fStartId;
        }
        for (int i = startIndex; i < endIndex; i++)
            appendWidth(widthRangeEntry->fAdvance[i], emSize, widthArray.get());
    } else {
        appendWidth(defaultWidth, 1000, widthArray.get());
    }
    insert("FirstChar", new SkPDFInt(firstChar))->unref();
    insert("LastChar",
           new SkPDFInt(firstChar + widthArray->size() - 1))->unref();
    insert("Widths", widthArray.get());
}

void SkPDFFont::adjustGlyphRangeForSingleByteEncoding(int16_t glyphID) {
    // Single byte glyph encoding supports a max of 255 glyphs.
    fFirstGlyphID = glyphID - (glyphID - 1) % 255;
    if (fLastGlyphID > fFirstGlyphID + 255 - 1) {
        fLastGlyphID = fFirstGlyphID + 255 - 1;
    }
}


bool SkPDFFont::FontRec::operator==(const SkPDFFont::FontRec& b) const {
    if (fFontID != b.fFontID)
        return false;
    if (fFont != NULL && b.fFont != NULL) {
        return fFont->fFirstGlyphID == b.fFont->fFirstGlyphID &&
            fFont->fLastGlyphID == b.fFont->fLastGlyphID;
    }
    if (fGlyphID == 0 || b.fGlyphID == 0)
        return true;

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
