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

#include "SkFontHost.h"
#include "SkPaint.h"
#include "SkPDFFont.h"
#include "SkPDFStream.h"
#include "SkPDFTypefaceInfo.h"
#include "SkPDFTypes.h"
#include "SkStream.h"
#include "SkTypeface.h"
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

    *size = buf[2] | (buf[3] << 8) | (buf[4] << 16) | (buf[5] << 24);
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
        // getStream makes another copy, but we couldn't do any better.
        src = (const uint8_t*)dynamicStream.getStream();
        srcLen = dynamicStream.getOffset() - 1;
    }

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
        uint8_t dataByte;
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

void appendWidth(const int& width, SkPDFArray* array) {
    SkRefPtr<SkPDFInt> widthInt = new SkPDFInt(width);
    widthInt->unref();  // SkRefPtr and new both took a reference.
    array->append(widthInt.get());
}

void appendVerticalAdvance(const SkPDFTypefaceInfo::VerticalMetric& advance,
                           SkPDFArray* array) {
    appendWidth(advance.fVerticalAdvance, array);
    appendWidth(advance.fOriginXDisp, array);
    appendWidth(advance.fOriginYDisp, array);
}

template <typename Data>
SkPDFArray* composeAdvanceData(
        SkPDFTypefaceInfo::AdvanceMetric<Data>* advanceInfo,
        void (*appendAdvance)(const Data& advance, SkPDFArray* array),
        Data* defaultAdvance) {
    SkPDFArray* result = new SkPDFArray();
    for (; advanceInfo != NULL; advanceInfo = advanceInfo->fNext.get()) {
        switch (advanceInfo->fType) {
            case SkPDFTypefaceInfo::WidthRange::kDefault: {
                SkASSERT(advanceInfo->fAdvance.count() == 1);
                *defaultAdvance = advanceInfo->fAdvance[0];
                break;
            }
            case SkPDFTypefaceInfo::WidthRange::kRange: {
                SkRefPtr<SkPDFArray> advanceArray = new SkPDFArray();
                advanceArray->unref();  // SkRefPtr and new both took a ref.
                for (int j = 0; j < advanceInfo->fAdvance.count(); j++)
                    appendAdvance(advanceInfo->fAdvance[j], advanceArray.get());
                SkRefPtr<SkPDFInt> rangeStart =
                    new SkPDFInt(advanceInfo->fStartId);
                rangeStart->unref();  // SkRefPtr and new both took a reference.
                result->append(rangeStart.get());
                result->append(advanceArray.get());
                break;
            }
            case SkPDFTypefaceInfo::WidthRange::kRun: {
                SkASSERT(advanceInfo->fAdvance.count() == 1);
                SkRefPtr<SkPDFInt> rangeStart =
                    new SkPDFInt(advanceInfo->fStartId);
                rangeStart->unref();  // SkRefPtr and new both took a reference.
                result->append(rangeStart.get());

                SkRefPtr<SkPDFInt> rangeEnd = new SkPDFInt(advanceInfo->fEndId);
                rangeEnd->unref();  // SkRefPtr and new both took a reference.
                result->append(rangeEnd.get());

                appendAdvance(advanceInfo->fAdvance[0], result);
                break;
            }
        }
    }
    return result;
}

}  // namespace

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
    if (find(fFontID, fFirstGlyphID, &index)) {
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

uint32_t SkPDFFont::fontID() {
    return fFontID;
}

bool SkPDFFont::hasGlyph(uint16_t id) {
    return (id >= fFirstGlyphID && id <= fLastGlyphID) || id == 0;
}

bool SkPDFFont::multiByteGlyphs() {
    return fMultiByteGlyphs;
}

size_t SkPDFFont::glyphsToPDFFontEncoding(const uint16_t* glyphIDs,
                                          size_t numGlyphs, void* encodedValues,
                                          size_t* encodedLength) {
    if (numGlyphs * 2 > *encodedLength)
        numGlyphs = *encodedLength / 2;

    // A font with multibyte glyphs will support all glyph IDs in a single font,
    // shortcut if we can.
    if (fMultiByteGlyphs) {
        *encodedLength = numGlyphs * 2;
        memcpy(encodedValues, glyphIDs, *encodedLength);
    } else {
        char* output = (char*) encodedValues;
        for (size_t i = 0; i < numGlyphs; i++) {
            if (glyphIDs[i] == 0) {
                output[i] = 0;
                continue;
            }
            if (glyphIDs[i] < fFirstGlyphID || glyphIDs[i] > fLastGlyphID) {
                numGlyphs = i;
                break;
            }
            output[i] = glyphIDs[i] - fFirstGlyphID + 1;
        }
    }

    return numGlyphs;
}

// static
SkPDFFont* SkPDFFont::getFontResource(uint32_t fontID, uint16_t glyphID) {
    SkAutoMutexAcquire lock(canonicalFontsMutex());
    int index;
    if (find(fontID, glyphID, &index)) {
        canonicalFonts()[index].fFont->ref();
        return canonicalFonts()[index].fFont;
    }

    SkRefPtr<SkPDFTypefaceInfo> fontInfo;
    SkPDFDict* fontDescriptor = NULL;
    if (index >= 0) {
        SkPDFFont* relatedFont = canonicalFonts()[index].fFont;
        SkASSERT(relatedFont->fFontInfo.get());
        fontInfo = relatedFont->fFontInfo;
        fontDescriptor = relatedFont->fDescriptor.get();
    } else {
        fontInfo = SkFontHost::GetPDFTypefaceInfo(fontID);
        fontInfo->unref();  // SkRefPtr and get info both took a reference.
    }

    SkPDFFont* font = new SkPDFFont(fontInfo.get(), fontID, glyphID, false,
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

SkPDFFont::SkPDFFont(class SkPDFTypefaceInfo* fontInfo, uint32_t fontID,
                     uint16_t glyphID, bool descendantFont,
                     SkPDFDict* fontDescriptor)
        : SkPDFDict("Font"),
          fFontID(fontID),
#ifdef SK_DEBUG
          fDescendant(descendantFont),
#endif
          fMultiByteGlyphs(false),
          fFirstGlyphID(1),
          fLastGlyphID(fontInfo->fLastGlyphID),
          fFontInfo(fontInfo),
          fDescriptor(fontDescriptor) {

    if (fontInfo->fMultiMaster) {
        populateType3Font();
    } else {
        switch (fontInfo->fType) {
            case SkPDFTypefaceInfo::kType1CID_Font:
            case SkPDFTypefaceInfo::kTrueType_Font:
                if (descendantFont)
                    populateCIDFont();
                else
                    populateType0Font();
                break;
            case SkPDFTypefaceInfo::kType1_Font: {
                uint16_t firstGlyphID = glyphID - (glyphID - 1) % 255;
                uint16_t lastGlyphID = firstGlyphID + 255 - 1;
                if (lastGlyphID > fLastGlyphID)
                    lastGlyphID = fLastGlyphID;
                if (populateType1Font(firstGlyphID, lastGlyphID))
                    break;
                // else, fall through.
            }
            case SkPDFTypefaceInfo::kOther_Font:
            case SkPDFTypefaceInfo::kNotEmbeddable_Font:
                populateType3Font();
                break;
            case SkPDFTypefaceInfo::kCFF_Font:
                SkASSERT(false);  // Not supported yet.
        }
    }

    // Type1 fonts may hold on to the font info, otherwise we are done with it.
    if (fontInfo->fType != SkPDFTypefaceInfo::kType1_Font)
        fFontInfo = NULL;
}

void SkPDFFont::populateType0Font() {
    fMultiByteGlyphs = true;

    SkRefPtr<SkPDFName> subType = new SkPDFName("Type0");
    subType->unref();  // SkRefPtr and new both took a reference.
    insert("Subtype", subType.get());

    SkRefPtr<SkPDFName> baseFont = new SkPDFName(fFontInfo.get()->fFontName);
    baseFont->unref();  // SkRefPtr and new both took a reference.
    insert("BaseFont", baseFont.get());

    SkRefPtr<SkPDFName> encoding = new SkPDFName("Identity-H");
    encoding->unref();  // SkRefPtr and new both took a reference.
    insert("Encoding", encoding.get());

    // TODO(vandebo) add a ToUnicode mapping.

    SkRefPtr<SkPDFFont> cidFont = new SkPDFFont(fFontInfo.get(),
                                                fFontID, 1, true, NULL);
    fResources.push(cidFont.get());  // 2 refs: SkRefPtr, new. Pass one.

    SkRefPtr<SkPDFArray> descendantFonts = new SkPDFArray();
    descendantFonts->unref();  // SkRefPtr and new took a reference.
    SkRefPtr<SkPDFObjRef> cidFontRef = new SkPDFObjRef(cidFont.get());
    cidFontRef->unref();  // SkRefPtr and new both took a reference.
    descendantFonts->append(cidFontRef.get());
    insert("DescendantFonts", descendantFonts.get());
}

void SkPDFFont::populateCIDFont() {
    fMultiByteGlyphs = true;

    SkRefPtr<SkPDFName> subType;
    if (fFontInfo.get()->fType == SkPDFTypefaceInfo::kType1CID_Font)
        subType = new SkPDFName("CIDFontType0");
    else if (fFontInfo.get()->fType == SkPDFTypefaceInfo::kTrueType_Font)
        subType = new SkPDFName("CIDFontType2");
    else
        SkASSERT(false);
    subType->unref();  // SkRefPtr and new both took a reference.
    insert("Subtype", subType.get());

    SkRefPtr<SkPDFName> baseFont = new SkPDFName(fFontInfo.get()->fFontName);
    baseFont->unref();  // SkRefPtr and new both took a reference.
    insert("BaseFont", baseFont.get());

    SkRefPtr<SkPDFDict> sysInfo = new SkPDFDict;
    sysInfo->unref();  // SkRefPtr and new both took a reference.

    SkRefPtr<SkPDFString> adobeString = new SkPDFString("Adobe");
    adobeString->unref();  // SkRefPtr and new both took a reference.
    sysInfo->insert("Registry", adobeString.get());

    SkRefPtr<SkPDFString> identityString = new SkPDFString("Identity");
    identityString->unref();  // SkRefPtr and new both took a reference.
    sysInfo->insert("Ordering", identityString.get());

    SkRefPtr<SkPDFInt> supplement = new SkPDFInt(0);
    supplement->unref();  // SkRefPtr and new both took a reference.
    sysInfo->insert("Supplement", supplement.get());

    insert("CIDSystemInfo", sysInfo.get());

    addFontDescriptor(0);

    if (fFontInfo.get()->fGlyphWidths.get()) {
        int defaultWidth = 0;
        SkRefPtr<SkPDFArray> widths =
            composeAdvanceData(fFontInfo.get()->fGlyphWidths.get(),
                               &appendWidth, &defaultWidth);
        widths->unref();  // SkRefPtr and compose both took a reference.
        if (widths->size())
            insert("W", widths.get());
        if (defaultWidth != 0) {
            SkRefPtr<SkPDFInt> defaultWidthInt =
                new SkPDFInt(defaultWidth);
            // SkRefPtr and compose both took a reference.
            defaultWidthInt->unref();
            insert("DW", defaultWidthInt.get());
        }
    }
    if (fFontInfo.get()->fVerticalMetrics.get()) {
        struct SkPDFTypefaceInfo::VerticalMetric defaultAdvance;
        defaultAdvance.fVerticalAdvance = 0;
        defaultAdvance.fOriginXDisp = 0;
        defaultAdvance.fOriginYDisp = 0;
        SkRefPtr<SkPDFArray> advances =
            composeAdvanceData(fFontInfo.get()->fVerticalMetrics.get(),
                               &appendVerticalAdvance, &defaultAdvance);
        advances->unref();  // SkRefPtr and compose both took a ref.
        if (advances->size())
            insert("W2", advances.get());
        if (defaultAdvance.fVerticalAdvance ||
                defaultAdvance.fOriginXDisp ||
                defaultAdvance.fOriginYDisp) {
            SkRefPtr<SkPDFArray> defaultAdvanceArray = new SkPDFArray;
            // SkRefPtr and compose both took a reference.
            defaultAdvanceArray->unref();
            appendVerticalAdvance(defaultAdvance,
                                  defaultAdvanceArray.get());
            insert("DW2", defaultAdvanceArray.get());
        }
    }
}

bool SkPDFFont::populateType1Font(uint16_t firstGlyphID, uint16_t lastGlyphID) {
    SkASSERT(!fFontInfo.get()->fVerticalMetrics.get());
    SkASSERT(fFontInfo.get()->fGlyphWidths.get());

    int defaultWidth = 0;
    const SkPDFTypefaceInfo::WidthRange* widthRangeEntry = NULL;
    const SkPDFTypefaceInfo::WidthRange* widthEntry;
    for (widthEntry = fFontInfo.get()->fGlyphWidths.get();
            widthEntry != NULL;
            widthEntry = widthEntry->fNext.get()) {
        switch (widthEntry->fType) {
            case SkPDFTypefaceInfo::WidthRange::kDefault:
                defaultWidth = widthEntry->fAdvance[0];
                break;
            case SkPDFTypefaceInfo::WidthRange::kRun:
                SkASSERT(false);
                break;
            case SkPDFTypefaceInfo::WidthRange::kRange:
                SkASSERT(widthRangeEntry == NULL);
                widthRangeEntry = widthEntry;
                break;
        }
    }

    if (!addFontDescriptor(defaultWidth))
        return false;

    fFirstGlyphID = firstGlyphID;
    fLastGlyphID = lastGlyphID;

    SkRefPtr<SkPDFName> subType = new SkPDFName("Type1");
    subType->unref();  // SkRefPtr and new both took a reference.
    insert("Subtype", subType.get());

    SkRefPtr<SkPDFName> baseFont = new SkPDFName(fFontInfo.get()->fFontName);
    baseFont->unref();  // SkRefPtr and new both took a reference.
    insert("BaseFont", baseFont.get());

    SkRefPtr<SkPDFArray> widthArray = new SkPDFArray();
    widthArray->unref();  // SkRefPtr and new both took a ref.
    int firstChar = 0;
    if (widthRangeEntry) {
        int startIndex = firstGlyphID - widthRangeEntry->fStartId;
        int endIndex = startIndex + lastGlyphID - firstGlyphID + 1;
        if (startIndex < 0)
            startIndex = 0;
        if (endIndex > widthRangeEntry->fAdvance.count())
            endIndex = widthRangeEntry->fAdvance.count();
        if (widthRangeEntry->fStartId == 0) {
            appendWidth(widthRangeEntry->fAdvance[0], widthArray.get());
        } else {
            firstChar = startIndex + widthRangeEntry->fStartId;
        }
        for (int i = startIndex; i < endIndex; i++)
            appendWidth(widthRangeEntry->fAdvance[i], widthArray.get());
    } else {
        appendWidth(defaultWidth, widthArray.get());
    }
    insert("Widths", widthArray.get());

    SkRefPtr<SkPDFInt> firstCharInt = new SkPDFInt(firstChar);
    firstCharInt->unref();  // SkRefPtr and new both took a reference.
    insert("FirstChar", firstCharInt.get());

    SkRefPtr<SkPDFInt> lastChar =
        new SkPDFInt(firstChar + widthArray->size() - 1);
    lastChar->unref();  // SkRefPtr and new both took a reference.
    insert("LastChar", lastChar.get());

    SkRefPtr<SkPDFDict> encoding = new SkPDFDict("Encoding");
    encoding->unref();  // SkRefPtr and new both took a reference.
    insert("Encoding", encoding.get());

    SkRefPtr<SkPDFArray> encDiffs = new SkPDFArray;
    encDiffs->unref();  // SkRefPtr and new both took a reference.
    encoding->insert("Differences", encDiffs.get());

    encDiffs->reserve(fLastGlyphID - fFirstGlyphID + 2);
    SkRefPtr<SkPDFInt> startID = new SkPDFInt(1);
    startID->unref();  // SkRefPtr and new both took a reference.
    encDiffs->append(startID.get());
    for (int gID = fFirstGlyphID; gID <= fLastGlyphID; gID++) {
        SkRefPtr<SkPDFName> glyphName =
            new SkPDFName(fFontInfo.get()->fGlyphNames->get()[gID]);
        glyphName->unref();  // SkRefPtr and new both took a reference.
        encDiffs->append(glyphName.get());
    }

    if (fFontInfo.get()->fLastGlyphID <= 255)
        fFontInfo = NULL;
    return true;
}

void SkPDFFont::populateType3Font() {
    // TODO(vandebo)
    SkASSERT(false);
}

bool SkPDFFont::addFontDescriptor(int defaultWidth) {
    if (fDescriptor.get() != NULL) {
        fResources.push(fDescriptor.get());
        fDescriptor->ref();
        SkRefPtr<SkPDFObjRef> descRef = new SkPDFObjRef(fDescriptor.get());
        descRef->unref();  // SkRefPtr and new both took a reference.
        insert("FontDescriptor", descRef.get());
        return true;
    }

    fDescriptor = new SkPDFDict("FontDescriptor");
    fDescriptor->unref();  // SkRefPtr and new both took a ref.

    switch (fFontInfo.get()->fType) {
        case SkPDFTypefaceInfo::kType1_Font: {
            size_t header, data, trailer;
            SkRefPtr<SkStream> rawFontData =
                SkFontHost::OpenStream(fFontID);
            rawFontData->unref();  // SkRefPtr and OpenStream both took a ref.
            SkStream* fontData = handleType1Stream(rawFontData.get(), &header,
                                                   &data, &trailer);
            if (fontData == NULL)
                return false;
            SkRefPtr<SkPDFStream> fontStream = new SkPDFStream(fontData);
            // SkRefPtr and new both ref()'d fontStream, pass one.
            fResources.push(fontStream.get());

            SkRefPtr<SkPDFInt> headerLen = new SkPDFInt(header);
            headerLen->unref();  // SkRefPtr and new both took a reference.
            fontStream->insert("Length1", headerLen.get());
            SkRefPtr<SkPDFInt> dataLen = new SkPDFInt(data);
            dataLen->unref();  // SkRefPtr and new both took a reference.
            fontStream->insert("Length2", dataLen.get());
            SkRefPtr<SkPDFInt> trailerLen = new SkPDFInt(trailer);
            trailerLen->unref();  // SkRefPtr and new both took a reference.
            fontStream->insert("Length3", trailerLen.get());

            SkRefPtr<SkPDFObjRef> streamRef = new SkPDFObjRef(fontStream.get());
            streamRef->unref();  // SkRefPtr and new both took a reference.
            fDescriptor->insert("FontFile", streamRef.get());
            break;
        }
        case SkPDFTypefaceInfo::kTrueType_Font: {
            SkRefPtr<SkStream> fontData = SkFontHost::OpenStream(fFontID);
            fontData->unref();  // SkRefPtr and OpenStream both took a ref.
            SkRefPtr<SkPDFStream> fontStream = new SkPDFStream(fontData.get());
            // SkRefPtr and new both ref()'d fontStream, pass one.
            fResources.push(fontStream.get());

            SkRefPtr<SkPDFInt> length = new SkPDFInt(fontData->getLength());
            length->unref();  // SkRefPtr and new both took a reference.
            fontStream->insert("Length1", length.get());

            SkRefPtr<SkPDFObjRef> streamRef = new SkPDFObjRef(fontStream.get());
            streamRef->unref();  // SkRefPtr and new both took a reference.
            fDescriptor->insert("FontFile2", streamRef.get());
            break;
        }
        case SkPDFTypefaceInfo::kCFF_Font:
        case SkPDFTypefaceInfo::kType1CID_Font: {
            SkRefPtr<SkStream> fontData = SkFontHost::OpenStream(fFontID);
            fontData->unref();  // SkRefPtr and OpenStream both took a ref.
            SkRefPtr<SkPDFStream> fontStream = new SkPDFStream(fontData.get());
            // SkRefPtr and new both ref()'d fontStream, pass one.
            fResources.push(fontStream.get());

            SkRefPtr<SkPDFName> subtype;
            if (fFontInfo.get()->fType == SkPDFTypefaceInfo::kCFF_Font)
                subtype = new SkPDFName("Type1C");
            else
                subtype = new SkPDFName("CIDFontType0c");
            subtype->unref();  // SkRefPtr and new both took a reference.
            fontStream->insert("Subtype", subtype.get());

            SkRefPtr<SkPDFObjRef> streamRef = new SkPDFObjRef(fontStream.get());
            streamRef->unref();  // SkRefPtr and new both took a reference.
            fDescriptor->insert("FontFile3", streamRef.get());
            break;
        }
        default:
            SkASSERT(false);
    }

    fResources.push(fDescriptor.get());
    fDescriptor->ref();
    SkRefPtr<SkPDFObjRef> descRef = new SkPDFObjRef(fDescriptor.get());
    descRef->unref();  // SkRefPtr and new both took a reference.
    insert("FontDescriptor", descRef.get());

    SkRefPtr<SkPDFName> fontName =
        new SkPDFName(fFontInfo.get()->fFontName);
    fontName->unref();  // SkRefPtr and new both took a reference.
    fDescriptor->insert("FontName", fontName.get());

    SkRefPtr<SkPDFInt> flags = new SkPDFInt(fFontInfo.get()->fStyle);
    flags->unref();  // SkRefPtr and new both took a reference.
    fDescriptor->insert("Flags", flags.get());

    SkIRect glyphBBox = fFontInfo.get()->fBBox;
    SkRefPtr<SkPDFArray> bbox = new SkPDFArray;
    bbox->unref();  // SkRefPtr and new both took a reference.
    bbox->reserve(4);
    SkRefPtr<SkPDFInt> bboxXMin = new SkPDFInt(glyphBBox.fLeft);
    bboxXMin->unref();  // SkRefPtr and new both took a reference.
    bbox->append(bboxXMin.get());
    SkRefPtr<SkPDFInt> bboxYMin = new SkPDFInt(glyphBBox.fBottom);
    bboxYMin->unref();  // SkRefPtr and new both took a reference.
    bbox->append(bboxYMin.get());
    SkRefPtr<SkPDFInt> bboxXMax = new SkPDFInt(glyphBBox.fRight);
    bboxXMax->unref();  // SkRefPtr and new both took a reference.
    bbox->append(bboxXMax.get());
    SkRefPtr<SkPDFInt> bboxYMax = new SkPDFInt(glyphBBox.fTop);
    bboxYMax->unref();  // SkRefPtr and new both took a reference.
    bbox->append(bboxYMax.get());
    fDescriptor->insert("FontBBox", bbox.get());

    SkRefPtr<SkPDFInt> italicAngle =
        new SkPDFInt(fFontInfo.get()->fItalicAngle);
    italicAngle->unref();  // SkRefPtr and new both took a reference.
    fDescriptor->insert("ItalicAngle", italicAngle.get());

    SkRefPtr<SkPDFScalar> ascent = new SkPDFScalar(fFontInfo.get()->fAscent);
    ascent->unref();  // SkRefPtr and new both took a reference.
    fDescriptor->insert("Ascent", ascent.get());

    SkRefPtr<SkPDFScalar> descent = new SkPDFScalar(fFontInfo.get()->fDescent);
    descent->unref();  // SkRefPtr and new both took a reference.
    fDescriptor->insert("Descent", descent.get());

    SkRefPtr<SkPDFScalar> capHeight =
        new SkPDFScalar(fFontInfo.get()->fCapHeight);
    capHeight->unref();  // SkRefPtr and new both took a reference.
    fDescriptor->insert("CapHeight", capHeight.get());

    SkRefPtr<SkPDFScalar> stemV = new SkPDFScalar(fFontInfo.get()->fStemV);
    stemV->unref();  // SkRefPtr and new both took a reference.
    fDescriptor->insert("StemV", stemV.get());

    if (defaultWidth > 0) {
        SkRefPtr<SkPDFInt> defaultWidthInt = new SkPDFInt(defaultWidth);
        defaultWidthInt->unref();  // SkRefPtr and new both took a reference.
        fDescriptor->insert("MissingWidth", defaultWidthInt.get());
    }
    return true;
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
