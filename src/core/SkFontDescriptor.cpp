/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkFontDescriptor.h"

#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTFitsIn.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkStreamPriv.h"

#include <cstddef>
#include <cstdint>
enum {
    kInvalid        = 0x00,

    // Related to a font request.
    kFontFamilyName = 0x01, // int length, data[length]
    kFullName       = 0x04, // int length, data[length]
    kPostscriptName = 0x06, // int length, data[length]
    kWeight         = 0x10, // scalar (1 - 1000)
    kWidth          = 0x11, // scalar (percentage, 100 is 'normal')
    kSlant          = 0x12, // scalar (cw angle, 14 is a normal right leaning oblique)
    kItalic         = 0x13, // scalar (0 is Roman, 1 is fully Italic)

    // Related to font data. Can also be used with a requested font.
    kPaletteIndex   = 0xF8, // int
    kPaletteEntryOverrides = 0xF9, // int count, (int, u32)[count]
    kFontVariation  = 0xFA, // int count, (u32, scalar)[count]

    // Related to font data.
    kFactoryId      = 0xFC, // int
    kFontIndex      = 0xFD, // int
    kSentinel       = 0xFF, // no data
};

SkFontDescriptor::SkFontDescriptor() { }

[[nodiscard]] static bool read_string(SkStream* stream, SkString* string) {
    size_t length;
    if (!stream->readPackedUInt(&length)) { return false; }
    if (length > 0) {
        if (StreamRemainingLengthIsBelow(stream, length)) {
            return false;
        }
        string->resize(length);
        if (stream->read(string->data(), length) != length) { return false; }
    }
    return true;
}

static bool write_string(SkWStream* stream, const SkString& string, uint32_t id) {
    if (string.isEmpty()) { return true; }
    return stream->writePackedUInt(id) &&
           stream->writePackedUInt(string.size()) &&
           stream->write(string.c_str(), string.size());
}

static bool write_uint(SkWStream* stream, size_t n, uint32_t id) {
    return stream->writePackedUInt(id) &&
           stream->writePackedUInt(n);
}

static bool write_scalar(SkWStream* stream, SkScalar n, uint32_t id) {
    return stream->writePackedUInt(id) &&
           stream->writeScalar(n);
}

[[nodiscard]] static size_t read_id(SkStream* stream) {
    size_t i;
    if (!stream->readPackedUInt(&i)) { return kInvalid; }
    return i;
}

static constexpr SkScalar usWidths[9] {
    1, 2, 3, 4, 5, 6, 7, 8, 9
};
static constexpr SkScalar width_for_usWidth[0x10] = {
    50,
    50, 62.5, 75, 87.5, 100, 112.5, 125, 150, 200,
    200, 200, 200, 200, 200, 200
};

bool SkFontDescriptor::Deserialize(SkStream* stream, SkFontDescriptor* result) {
    size_t factoryId;
    using FactoryIdType = decltype(result->fFactoryId);

    size_t coordinateCount;
    using CoordinateCountType = decltype(result->fCoordinateCount);

    size_t index;
    using CollectionIndexType = decltype(result->fCollectionIndex);

    size_t paletteIndex;
    using PaletteIndexType = decltype(result->fPaletteIndex);

    size_t paletteEntryOverrideCount;
    using PaletteEntryOverrideCountType = decltype(result->fPaletteEntryOverrideCount);

    size_t paletteEntryOverrideIndex;
    using PaletteEntryOverrideIndexType = decltype(result->fPaletteEntryOverrides[0].index);

    SkScalar weight = SkFontStyle::kNormal_Weight;
    SkScalar width = SkFontStyle::kNormal_Width;
    SkScalar slant = 0;
    SkScalar italic = 0;

    size_t styleBits;
    if (!stream->readPackedUInt(&styleBits)) { return false; }
    weight = ((styleBits >> 16) & 0xFFFF);
    width  = ((styleBits >>  8) & 0x000F)[width_for_usWidth];
    slant  = ((styleBits >>  0) & 0x000F) != SkFontStyle::kUpright_Slant ? 14 : 0;
    italic = ((styleBits >>  0) & 0x000F) == SkFontStyle::kItalic_Slant ? 1 : 0;

    for (size_t id; (id = read_id(stream)) != kSentinel;) {
        switch (id) {
            case kFontFamilyName:
                if (!read_string(stream, &result->fFamilyName)) { return false; }
                break;
            case kFullName:
                if (!read_string(stream, &result->fFullName)) { return false; }
                break;
            case kPostscriptName:
                if (!read_string(stream, &result->fPostscriptName)) { return false; }
                break;
            case kWeight:
                if (!stream->readScalar(&weight)) { return false; }
                break;
            case kWidth:
                if (!stream->readScalar(&width)) { return false; }
                break;
            case kSlant:
                if (!stream->readScalar(&slant)) { return false; }
                break;
            case kItalic:
                if (!stream->readScalar(&italic)) { return false; }
                break;
            case kFontVariation:
                if (!stream->readPackedUInt(&coordinateCount)) { return false; }
                if (!SkTFitsIn<CoordinateCountType>(coordinateCount)) { return false; }
                if (StreamRemainingLengthIsBelow(stream, coordinateCount)) {
                    return false;
                }
                result->fCoordinateCount = SkTo<CoordinateCountType>(coordinateCount);

                result->fVariation.reset(coordinateCount);
                for (size_t i = 0; i < coordinateCount; ++i) {
                    if (!stream->readU32(&result->fVariation[i].axis)) { return false; }
                    if (!stream->readScalar(&result->fVariation[i].value)) { return false; }
                }
                break;
            case kFontIndex:
                if (!stream->readPackedUInt(&index)) { return false; }
                if (!SkTFitsIn<CollectionIndexType>(index)) { return false; }
                result->fCollectionIndex = SkTo<CollectionIndexType>(index);
                break;
            case kPaletteIndex:
                if (!stream->readPackedUInt(&paletteIndex)) { return false; }
                if (!SkTFitsIn<PaletteIndexType>(paletteIndex)) { return false; }
                result->fPaletteIndex = SkTo<PaletteIndexType>(paletteIndex);
                break;
            case kPaletteEntryOverrides:
                if (!stream->readPackedUInt(&paletteEntryOverrideCount)) { return false; }
                if (!SkTFitsIn<PaletteEntryOverrideCountType>(paletteEntryOverrideCount)) {
                    return false;
                }
                if (StreamRemainingLengthIsBelow(stream, paletteEntryOverrideCount)) {
                    return false;
                }
                result->fPaletteEntryOverrideCount =
                        SkTo<PaletteEntryOverrideCountType>(paletteEntryOverrideCount);

                result->fPaletteEntryOverrides.reset(paletteEntryOverrideCount);
                for (size_t i = 0; i < paletteEntryOverrideCount; ++i) {
                    if (!stream->readPackedUInt(&paletteEntryOverrideIndex)) { return false; }
                    if (!SkTFitsIn<PaletteEntryOverrideIndexType>(paletteEntryOverrideIndex)) {
                        return false;
                    }
                    result->fPaletteEntryOverrides[i].index =
                            SkTo<PaletteEntryOverrideIndexType>(paletteEntryOverrideIndex);
                    if (!stream->readU32(&result->fPaletteEntryOverrides[i].color)) {
                        return false;
                    }
                }
                break;
            case kFactoryId:
                if (!stream->readPackedUInt(&factoryId)) { return false; }
                if (!SkTFitsIn<FactoryIdType>(factoryId)) { return false; }
                result->fFactoryId = SkTo<FactoryIdType>(factoryId);
                break;
            default:
                SkDEBUGFAIL("Unknown id used by a font descriptor");
                return false;
        }
    }

    SkFontStyle::Slant slantEnum = SkFontStyle::kUpright_Slant;
    if (slant != 0) { slantEnum = SkFontStyle::kOblique_Slant; }
    if (0 < italic) { slantEnum = SkFontStyle::kItalic_Slant; }
    SkFontStyle::Width widthEnum = SkFontStyleWidthForWidthAxisValue(width);
    result->fStyle = SkFontStyle(SkScalarRoundToInt(weight), widthEnum, slantEnum);

    size_t length;
    if (!stream->readPackedUInt(&length)) { return false; }
    if (length > 0) {
        if (StreamRemainingLengthIsBelow(stream, length)) {
            return false;
        }
        sk_sp<SkData> data(SkData::MakeUninitialized(length));
        if (stream->read(data->writable_data(), length) != length) {
            SkDEBUGFAIL("Could not read font data");
            return false;
        }
        result->fStream = SkMemoryStream::Make(std::move(data));
    }
    return true;
}

void SkFontDescriptor::serialize(SkWStream* stream) const {
    uint32_t styleBits = (fStyle.weight() << 16) | (fStyle.width() << 8) | (fStyle.slant());
    stream->writePackedUInt(styleBits);

    write_string(stream, fFamilyName, kFontFamilyName);
    write_string(stream, fFullName, kFullName);
    write_string(stream, fPostscriptName, kPostscriptName);

    write_scalar(stream, fStyle.weight(), kWeight);
    write_scalar(stream, fStyle.width()[width_for_usWidth], kWidth);
    write_scalar(stream, fStyle.slant() == SkFontStyle::kUpright_Slant ? 0 : 14, kSlant);
    write_scalar(stream, fStyle.slant() == SkFontStyle::kItalic_Slant ? 1 : 0, kItalic);

    if (fCollectionIndex > 0) {
        write_uint(stream, fCollectionIndex, kFontIndex);
    }
    if (fPaletteIndex > 0) {
        write_uint(stream, fPaletteIndex, kPaletteIndex);
    }
    if (fCoordinateCount > 0) {
        write_uint(stream, fCoordinateCount, kFontVariation);
        for (int i = 0; i < fCoordinateCount; ++i) {
            stream->write32(fVariation[i].axis);
            stream->writeScalar(fVariation[i].value);
        }
    }
    if (fPaletteEntryOverrideCount > 0) {
        write_uint(stream, fPaletteEntryOverrideCount, kPaletteEntryOverrides);
        for (int i = 0; i < fPaletteEntryOverrideCount; ++i) {
            stream->writePackedUInt(fPaletteEntryOverrides[i].index);
            stream->write32(fPaletteEntryOverrides[i].color);
        }
    }

    write_uint(stream, fFactoryId, kFactoryId);

    stream->writePackedUInt(kSentinel);

    if (fStream) {
        std::unique_ptr<SkStreamAsset> fontStream = fStream->duplicate();
        size_t length = fontStream->getLength();
        stream->writePackedUInt(length);
        stream->writeStream(fontStream.get(), length);
    } else {
        stream->writePackedUInt(0);
    }
}

SkFontStyle::Width SkFontDescriptor::SkFontStyleWidthForWidthAxisValue(SkScalar width) {
    int usWidth = SkScalarRoundToInt(SkScalarInterpFunc(width, &width_for_usWidth[1], usWidths, 9));
    return static_cast<SkFontStyle::Width>(usWidth);
}
