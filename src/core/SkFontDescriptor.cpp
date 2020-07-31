/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "src/core/SkFontDescriptor.h"

enum {
    kInvalid        = 0x00,

    // these must match the sfnt 'name' enums
    kFontFamilyName = 0x01,
    kFullName       = 0x04,
    kPostscriptName = 0x06,

    // These count backwards from 0xFF, so as not to collide with the SFNT
    // defines for names in its 'name' table.
    kFontVariation  = 0xFA,
    kFontAxes       = 0xFB,
    kFontIndex      = 0xFD,
    kSentinel       = 0xFF,
};

SkFontDescriptor::SkFontDescriptor() { }

static bool SK_WARN_UNUSED_RESULT read_string(SkStream* stream, SkString* string) {
    size_t length;
    if (!stream->readPackedUInt(&length)) { return false; }
    if (length > 0) {
        string->resize(length);
        if (stream->read(string->writable_str(), length) != length) { return false; }
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

static size_t SK_WARN_UNUSED_RESULT read_id(SkStream* stream) {
    size_t i;
    if (!stream->readPackedUInt(&i)) { return kInvalid; }
    return i;
}

std::unique_ptr<SkFontData> SkFontDescriptor::maybeAsSkFontData() {
    if (!fVariationIsOrderedNotNamed) {
        return nullptr;
    }
    SkFontArguments args;
    args.setCollectionIndex(this->getCollectionIndex());
    args.setVariationDesignPosition({this->getVariation(), this->getVariationCoordinateCount()});
    return std::make_unique<SkFontData>(this->dupStream(), args);
}

bool SkFontDescriptor::Deserialize(SkStream* stream, SkFontDescriptor* result) {
    size_t styleBits;
    if (!stream->readPackedUInt(&styleBits)) { return false; }
    result->fStyle = SkFontStyle((styleBits >> 16) & 0xFFFF,
                                 (styleBits >> 8 ) & 0xFF,
                                 static_cast<SkFontStyle::Slant>(styleBits & 0xFF));

    SkAutoSTMalloc<4, SkFixed> axis;
    Coordinates variation;
    size_t axisCount = 0;
    size_t coordinateCount = 0;
    size_t index = 0;
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
            case kFontAxes:
                if (!stream->readPackedUInt(&axisCount)) { return false; }
                axis.reset(axisCount);
                for (size_t i = 0; i < axisCount; ++i) {
                    if (!stream->readS32(&axis[i])) { return false; }
                }
                break;
            case kFontVariation:
                if (!stream->readPackedUInt(&coordinateCount)) { return false; }
                variation.reset(coordinateCount);
                for (size_t i = 0; i < coordinateCount; ++i) {
                    if (!stream->readU32(&variation[i].axis)) { return false; }
                    if (!stream->readScalar(&variation[i].value)) { return false; }
                }
                break;
            case kFontIndex:
                if (!stream->readPackedUInt(&index)) { return false; }
                break;
            default:
                SkDEBUGFAIL("Unknown id used by a font descriptor");
                return false;
        }
    }

    result->fCollectionIndex = index;

    result->fVariationIsOrderedNotNamed = false;
    if (coordinateCount) {
        result->fCoordinateCount = coordinateCount;
        result->fVariation = std::move(variation);
    } else if (axisCount) {
        // This path only taken with old skps.
        result->fVariationIsOrderedNotNamed = true;
        result->fCoordinateCount = axisCount;
        variation.reset(axisCount);
        for (size_t i = 0; i < axisCount; ++i) {
            variation[i].axis = 0;
            variation[i].value = SkFixedToScalar(axis[i]);
        }
        result->fVariation = std::move(variation);
    }

    size_t length;
    if (!stream->readPackedUInt(&length)) { return false; }
    if (length > 0) {
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

    if (fCollectionIndex) {
        write_uint(stream, fCollectionIndex, kFontIndex);
    }
    if (fCoordinateCount) {
        write_uint(stream, fCoordinateCount, kFontVariation);
        for (int i = 0; i < fCoordinateCount; ++i) {
            stream->write32(fVariation[i].axis);
            stream->writeScalar(fVariation[i].value);
        }
    }

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
