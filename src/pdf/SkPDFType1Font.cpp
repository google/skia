// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "src/pdf/SkPDFType1Font.h"

#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "src/core/SkStrikeSpec.h"

#include <ctype.h>

/*
  "A standard Type 1 font program, as described in the Adobe Type 1
  Font Format specification, consists of three parts: a clear-text
  portion (written using PostScript syntax), an encrypted portion, and
  a fixed-content portion.  The fixed-content portion contains 512
  ASCII zeros followed by a cleartomark operator, and perhaps followed
  by additional data. Although the encrypted portion of a standard
  Type 1 font may be in binary or ASCII hexadecimal format, PDF
  supports only the binary format."
*/
static bool parsePFBSection(const uint8_t** src, size_t* len, int sectionType,
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

static bool parsePFB(const uint8_t* src, size_t size, size_t* headerLen,
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
static bool parsePFA(const char* src, size_t size, size_t* headerLen,
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
        // isxdigit() is locale-sensitive https://bugs.skia.org/8285
        if (nullptr == strchr("0123456789abcdefABCDEF", *dataPos)) {
            return false;
        }
        nibbles++;
    }
    *dataLen = (nibbles + 1) / 2;

    return true;
}

static int8_t hexToBin(uint8_t c) {
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

static sk_sp<SkData> convert_type1_font_stream(std::unique_ptr<SkStreamAsset> srcStream,
                                               size_t* headerLen,
                                               size_t* dataLen,
                                               size_t* trailerLen) {
    size_t srcLen = srcStream ? srcStream->getLength() : 0;
    SkASSERT(srcLen);
    if (!srcLen) {
        return nullptr;
    }
    // Flatten and Nul-terminate the source stream so that we can use
    // strstr() to search it.
    SkAutoTMalloc<uint8_t> sourceBuffer(SkToInt(srcLen + 1));
    (void)srcStream->read(sourceBuffer.get(), srcLen);
    sourceBuffer[SkToInt(srcLen)] = 0;
    const uint8_t* src = sourceBuffer.get();

    if (parsePFB(src, srcLen, headerLen, dataLen, trailerLen)) {
        static const int kPFBSectionHeaderLength = 6;
        const size_t length = *headerLen + *dataLen + *trailerLen;
        SkASSERT(length > 0);
        SkASSERT(length + (2 * kPFBSectionHeaderLength) <= srcLen);

        sk_sp<SkData> data(SkData::MakeUninitialized(length));

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
    if (!parsePFA((const char*)src, srcLen, headerLen, &hexDataLen, dataLen,
                 trailerLen)) {
        return nullptr;
    }
    const size_t length = *headerLen + *dataLen + *trailerLen;
    SkASSERT(length > 0);
    auto data = SkData::MakeUninitialized(length);
    uint8_t* buffer = (uint8_t*)data->writable_data();

    memcpy(buffer, src, *headerLen);
    uint8_t* const resultData = &(buffer[*headerLen]);

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
    return data;
}

inline static bool can_embed(const SkAdvancedTypefaceMetrics& metrics) {
    return !SkToBool(metrics.fFlags & SkAdvancedTypefaceMetrics::kNotEmbeddable_FontFlag);
}

inline static SkScalar from_font_units(SkScalar scaled, uint16_t emSize) {
    return emSize == 1000 ? scaled : scaled * 1000 / emSize;
}

static SkPDFIndirectReference make_type1_font_descriptor(SkPDFDocument* doc,
                                                         const SkTypeface* typeface,
                                                         const SkAdvancedTypefaceMetrics* info) {
    SkPDFDict descriptor("FontDescriptor");
    uint16_t emSize = SkToU16(typeface->getUnitsPerEm());
    if (info) {
        SkPDFFont::PopulateCommonFontDescriptor(&descriptor, *info, emSize, 0);
        if (can_embed(*info)) {
            int ttcIndex;
            size_t header SK_INIT_TO_AVOID_WARNING;
            size_t data SK_INIT_TO_AVOID_WARNING;
            size_t trailer SK_INIT_TO_AVOID_WARNING;
            std::unique_ptr<SkStreamAsset> rawFontData = typeface->openStream(&ttcIndex);
            sk_sp<SkData> fontData = convert_type1_font_stream(std::move(rawFontData),
                                                               &header, &data, &trailer);
            if (fontData) {
                std::unique_ptr<SkPDFDict> dict = SkPDFMakeDict();
                dict->insertInt("Length1", header);
                dict->insertInt("Length2", data);
                dict->insertInt("Length3", trailer);
                auto fontStream = SkMemoryStream::Make(std::move(fontData));
                descriptor.insertRef("FontFile", SkPDFStreamOut(std::move(dict),
                                                                std::move(fontStream), doc, true));
            }
        }
    }
    return doc->emit(descriptor);
}


static const std::vector<SkString>& type_1_glyphnames(SkPDFDocument* canon,
                                                      const SkTypeface* typeface) {
    SkFontID fontID = typeface->uniqueID();
    const std::vector<SkString>* glyphNames = canon->fType1GlyphNames.find(fontID);
    if (!glyphNames) {
        std::vector<SkString> names(typeface->countGlyphs());
        SkPDFFont::GetType1GlyphNames(*typeface, names.data());
        glyphNames = canon->fType1GlyphNames.set(fontID, std::move(names));
    }
    SkASSERT(glyphNames);
    return *glyphNames;
}

static SkPDFIndirectReference type1_font_descriptor(SkPDFDocument* doc,
                                                    const SkTypeface* typeface) {
    SkFontID fontID = typeface->uniqueID();
    if (SkPDFIndirectReference* ptr = doc->fFontDescriptors.find(fontID)) {
        return *ptr;
    }
    const SkAdvancedTypefaceMetrics* info = SkPDFFont::GetMetrics(typeface, doc);
    auto fontDescriptor = make_type1_font_descriptor(doc, typeface, info);
    doc->fFontDescriptors.set(fontID, fontDescriptor);
    return fontDescriptor;
}


void SkPDFEmitType1Font(const SkPDFFont& pdfFont, SkPDFDocument* doc) {
    SkTypeface* typeface = pdfFont.typeface();
    const std::vector<SkString>& glyphNames = type_1_glyphnames(doc, typeface);
    SkGlyphID firstGlyphID = pdfFont.firstGlyphID();
    SkGlyphID lastGlyphID = pdfFont.lastGlyphID();

    SkPDFDict font("Font");
    font.insertRef("FontDescriptor", type1_font_descriptor(doc, typeface));
    font.insertName("Subtype", "Type1");
    if (const SkAdvancedTypefaceMetrics* info = SkPDFFont::GetMetrics(typeface, doc)) {
        font.insertName("BaseFont", info->fPostScriptName);
    }

    // glyphCount not including glyph 0
    unsigned glyphCount = 1 + lastGlyphID - firstGlyphID;
    SkASSERT(glyphCount > 0 && glyphCount <= 255);
    font.insertInt("FirstChar", (size_t)0);
    font.insertInt("LastChar", (size_t)glyphCount);
    {
        int emSize;
        auto widths = SkPDFMakeArray();

        int glyphRangeSize = lastGlyphID - firstGlyphID + 2;
        SkAutoTArray<SkGlyphID> glyphIDs{glyphRangeSize};
        glyphIDs[0] = 0;
        for (unsigned gId = firstGlyphID; gId <= lastGlyphID; gId++) {
            glyphIDs[gId - firstGlyphID + 1] = gId;
        }
        SkStrikeSpec strikeSpec = SkStrikeSpec::MakePDFVector(*typeface, &emSize);
        SkBulkGlyphMetrics metrics{strikeSpec};
        auto glyphs = metrics.glyphs(SkSpan(glyphIDs.get(), glyphRangeSize));
        for (int i = 0; i < glyphRangeSize; ++i) {
            widths->appendScalar(from_font_units(glyphs[i]->advanceX(), SkToU16(emSize)));
        }
        font.insertObject("Widths", std::move(widths));
    }
    auto encDiffs = SkPDFMakeArray();
    encDiffs->reserve(lastGlyphID - firstGlyphID + 3);
    encDiffs->appendInt(0);

    SkASSERT(glyphNames.size() > lastGlyphID);
    const SkString unknown("UNKNOWN");
    encDiffs->appendName(glyphNames[0].isEmpty() ? unknown : glyphNames[0]);
    for (int gID = firstGlyphID; gID <= lastGlyphID; gID++) {
        encDiffs->appendName(glyphNames[gID].isEmpty() ? unknown : glyphNames[gID]);
    }

    auto encoding = SkPDFMakeDict("Encoding");
    encoding->insertObject("Differences", std::move(encDiffs));
    font.insertObject("Encoding", std::move(encoding));

    doc->emit(font, pdfFont.indirectReference());
}
