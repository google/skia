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

SkPDFArray* appendWidth(const int& width, SkPDFArray* array) {
    array->append(new SkPDFInt(width))->unref();
    return array;
}

SkPDFArray* appendVerticalAdvance(
        const SkPDFTypefaceInfo::VerticalMetric& advance, SkPDFArray* array) {
    appendWidth(advance.fVerticalAdvance, array);
    appendWidth(advance.fOriginXDisp, array);
    appendWidth(advance.fOriginYDisp, array);
    return array;
}

template <typename Data>
SkPDFArray* composeAdvanceData(
        SkPDFTypefaceInfo::AdvanceMetric<Data>* advanceInfo,
        SkPDFArray* (*appendAdvance)(const Data& advance, SkPDFArray* array),
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
                result->append(new SkPDFInt(advanceInfo->fStartId))->unref();
                result->append(advanceArray.get());
                break;
            }
            case SkPDFTypefaceInfo::WidthRange::kRun: {
                SkASSERT(advanceInfo->fAdvance.count() == 1);
                result->append(new SkPDFInt(advanceInfo->fStartId))->unref();
                result->append(new SkPDFInt(advanceInfo->fEndId))->unref();
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
    // TODO(vandebo) add a ToUnicode mapping.
    fMultiByteGlyphs = true;

    insert("Subtype", new SkPDFName("Type0"))->unref();
    insert("BaseFont", new SkPDFName(fFontInfo.get()->fFontName))->unref();
    insert("Encoding",  new SkPDFName("Identity-H"))->unref();

    SkRefPtr<SkPDFArray> descendantFonts = new SkPDFArray();
    descendantFonts->unref();  // SkRefPtr and new took a reference.

    // Pass ref new created to fResources.
    fResources.push(new SkPDFFont(fFontInfo.get(), fFontID, 1, true, NULL));
    descendantFonts->append(new SkPDFObjRef(fResources.top()))->unref();
    insert("DescendantFonts", descendantFonts.get());
}

void SkPDFFont::populateCIDFont() {
    fMultiByteGlyphs = true;

    insert("BaseFont", new SkPDFName(fFontInfo.get()->fFontName))->unref();

    if (fFontInfo.get()->fType == SkPDFTypefaceInfo::kType1CID_Font)
        insert("Subtype", new SkPDFName("CIDFontType0"))->unref();
    else if (fFontInfo.get()->fType == SkPDFTypefaceInfo::kTrueType_Font)
        insert("Subtype", new SkPDFName("CIDFontType2"))->unref();
    else
        SkASSERT(false);

    SkRefPtr<SkPDFDict> sysInfo = new SkPDFDict;
    sysInfo->unref();  // SkRefPtr and new both took a reference.
    sysInfo->insert("Registry", new SkPDFString("Adobe"))->unref();
    sysInfo->insert("Ordering", new SkPDFString("Identity"))->unref();
    sysInfo->insert("Supplement", new SkPDFInt(0))->unref();
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
            insert("DW", new SkPDFInt(defaultWidth))->unref();
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
            insert("DW2", appendVerticalAdvance(defaultAdvance,
                                                new SkPDFArray))->unref();
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

    insert("Subtype", new SkPDFName("Type1"))->unref();
    insert("BaseFont", new SkPDFName(fFontInfo.get()->fFontName))->unref();

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
    insert("FirstChar", new SkPDFInt(firstChar))->unref();
    insert("LastChar",
           new SkPDFInt(firstChar + widthArray->size() - 1))->unref();

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
            new SkPDFName(fFontInfo.get()->fGlyphNames->get()[gID]))->unref();
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
        insert("FontDescriptor", new SkPDFObjRef(fDescriptor.get()))->unref();
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
            fontStream->insert("Length1", new SkPDFInt(header))->unref();
            fontStream->insert("Length2", new SkPDFInt(data))->unref();
            fontStream->insert("Length3", new SkPDFInt(trailer))->unref();
            fDescriptor->insert("FontFile",
                                new SkPDFObjRef(fontStream.get()))->unref();
            break;
        }
        case SkPDFTypefaceInfo::kTrueType_Font: {
            SkRefPtr<SkStream> fontData = SkFontHost::OpenStream(fFontID);
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
        case SkPDFTypefaceInfo::kCFF_Font:
        case SkPDFTypefaceInfo::kType1CID_Font: {
            SkRefPtr<SkStream> fontData = SkFontHost::OpenStream(fFontID);
            fontData->unref();  // SkRefPtr and OpenStream both took a ref.
            SkRefPtr<SkPDFStream> fontStream = new SkPDFStream(fontData.get());
            // SkRefPtr and new both ref()'d fontStream, pass one.
            fResources.push(fontStream.get());

            if (fFontInfo.get()->fType == SkPDFTypefaceInfo::kCFF_Font) {
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

    fResources.push(fDescriptor.get());
    fDescriptor->ref();
    insert("FontDescriptor", new SkPDFObjRef(fDescriptor.get()))->unref();

    fDescriptor->insert("FontName",
                        new SkPDFName(fFontInfo.get()->fFontName))->unref();
    fDescriptor->insert("Flags",
                        new SkPDFInt(fFontInfo.get()->fStyle))->unref();
    fDescriptor->insert("Ascent",
                        new SkPDFScalar(fFontInfo.get()->fAscent))->unref();
    fDescriptor->insert("Descent",
                        new SkPDFScalar(fFontInfo.get()->fDescent))->unref();
    fDescriptor->insert("CapHeight",
                        new SkPDFScalar(fFontInfo.get()->fCapHeight))->unref();
    fDescriptor->insert("StemV",
                        new SkPDFScalar(fFontInfo.get()->fStemV))->unref();
    fDescriptor->insert("ItalicAngle",
                        new SkPDFInt(fFontInfo.get()->fItalicAngle))->unref();

    SkIRect glyphBBox = fFontInfo.get()->fBBox;
    SkRefPtr<SkPDFArray> bbox = new SkPDFArray;
    bbox->unref();  // SkRefPtr and new both took a reference.
    bbox->reserve(4);
    bbox->append(new SkPDFInt(glyphBBox.fLeft))->unref();
    bbox->append(new SkPDFInt(glyphBBox.fBottom))->unref();
    bbox->append(new SkPDFInt(glyphBBox.fRight))->unref();
    bbox->append(new SkPDFInt(glyphBBox.fTop))->unref();
    fDescriptor->insert("FontBBox", bbox.get());

    if (defaultWidth > 0) {
        fDescriptor->insert("MissingWidth",
                            new SkPDFInt(defaultWidth))->unref();
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
