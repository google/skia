/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec.h"
#include "SkCodecPriv.h"
#include "SkColorPriv.h"
#include "SkColorTable.h"
#include "SkData.h"
#include "SkStream.h"
#include "SkWbmpCodec.h"

// Each bit represents a pixel, so width is actually a number of bits.
// A row will always be stored in bytes, so we round width up to the
// nearest multiple of 8 to get the number of bits actually in the row.
// We then divide by 8 to convert to bytes.
static inline size_t get_src_row_bytes(int width) {
    return SkAlign8(width) >> 3;
}

static inline void setup_color_table(SkColorType colorType,
        SkPMColor* colorPtr, int* colorCount) {
    if (kIndex_8_SkColorType == colorType) {
        colorPtr[0] = SK_ColorBLACK;
        colorPtr[1] = SK_ColorWHITE;
        *colorCount = 2;
    }
}

static inline bool valid_color_type(SkColorType colorType, SkAlphaType alphaType) {
    switch (colorType) {
        case kN32_SkColorType:
        case kIndex_8_SkColorType:
            return true;
        case kGray_8_SkColorType:
        case kRGB_565_SkColorType:
            return kOpaque_SkAlphaType == alphaType;
        default:
            return false;
    }
}

static bool read_byte(SkStream* stream, uint8_t* data)
{
    return stream->read(data, 1) == 1;
}

// http://en.wikipedia.org/wiki/Variable-length_quantity
static bool read_mbf(SkStream* stream, uint64_t* value) {
    uint64_t n = 0;
    uint8_t data;
    const uint64_t kLimit = 0xFE00000000000000;
    SkASSERT(kLimit == ~((~static_cast<uint64_t>(0)) >> 7));
    do {
        if (n & kLimit) { // Will overflow on shift by 7.
            return false;
        }
        if (stream->read(&data, 1) != 1) {
            return false;
        }
        n = (n << 7) | (data & 0x7F);
    } while (data & 0x80);
    *value = n;
    return true;
}

static bool read_header(SkStream* stream, SkISize* size) {
    {
        uint8_t data;
        if (!read_byte(stream, &data) || data != 0) { // unknown type
            return false;
        }
        if (!read_byte(stream, &data) || (data & 0x9F)) { // skip fixed header
            return false;
        }
    }

    uint64_t width, height;
    if (!read_mbf(stream, &width) || width > 0xFFFF || !width) {
        return false;
    }
    if (!read_mbf(stream, &height) || height > 0xFFFF || !height) {
        return false;
    }
    if (size) {
        *size = SkISize::Make(SkToS32(width), SkToS32(height));
    }
    return true;
}

bool SkWbmpCodec::onRewind() {
    return read_header(this->stream(), nullptr);
}

SkSwizzler* SkWbmpCodec::initializeSwizzler(const SkImageInfo& info, const SkPMColor* ctable,
        const Options& opts) {
    return SkSwizzler::CreateSwizzler(SkSwizzler::kBit, ctable, info, opts);
}

bool SkWbmpCodec::readRow(uint8_t* row) {
    return this->stream()->read(row, fSrcRowBytes) == fSrcRowBytes;
}

SkWbmpCodec::SkWbmpCodec(const SkImageInfo& info, SkStream* stream)
    : INHERITED(info, stream)
    , fSrcRowBytes(get_src_row_bytes(this->getInfo().width()))
    , fSwizzler(nullptr)
    , fColorTable(nullptr)
{}

SkEncodedFormat SkWbmpCodec::onGetEncodedFormat() const {
    return kWBMP_SkEncodedFormat;
}

SkCodec::Result SkWbmpCodec::onGetPixels(const SkImageInfo& info,
                                         void* dst,
                                         size_t rowBytes,
                                         const Options& options,
                                         SkPMColor ctable[],
                                         int* ctableCount,
                                         int* rowsDecoded) {
    if (options.fSubset) {
        // Subsets are not supported.
        return kUnimplemented;
    }

    if (!valid_color_type(info.colorType(), info.alphaType()) ||
            !valid_alpha(info.alphaType(), this->getInfo().alphaType())) {
        return kInvalidConversion;
    }

    // Prepare a color table if necessary
    setup_color_table(info.colorType(), ctable, ctableCount);

    // Initialize the swizzler
    SkAutoTDelete<SkSwizzler> swizzler(this->initializeSwizzler(info, ctable, options));
    SkASSERT(swizzler);

    // Perform the decode
    SkISize size = info.dimensions();
    SkAutoTMalloc<uint8_t> src(fSrcRowBytes);
    void* dstRow = dst;
    for (int y = 0; y < size.height(); ++y) {
        if (!this->readRow(src.get())) {
            *rowsDecoded = y;
            return kIncompleteInput;
        }
        swizzler->swizzle(dstRow, src.get());
        dstRow = SkTAddOffset<void>(dstRow, rowBytes);
    }
    return kSuccess;
}

bool SkWbmpCodec::IsWbmp(const void* buffer, size_t bytesRead) {
    SkAutoTUnref<SkData> data(SkData::NewWithoutCopy(buffer, bytesRead));
    SkMemoryStream stream(data);
    return read_header(&stream, nullptr);
}

SkCodec* SkWbmpCodec::NewFromStream(SkStream* stream) {
    SkAutoTDelete<SkStream> streamDeleter(stream);
    SkISize size;
    if (!read_header(stream, &size)) {
        return nullptr;
    }
    SkImageInfo info = SkImageInfo::Make(size.width(), size.height(),
            kGray_8_SkColorType, kOpaque_SkAlphaType);
    return new SkWbmpCodec(info, streamDeleter.release());
}

int SkWbmpCodec::onGetScanlines(void* dst, int count, size_t dstRowBytes) {
    void* dstRow = dst;
    for (int y = 0; y < count; ++y) {
        if (!this->readRow(fSrcBuffer.get())) {
            return y;
        }
        fSwizzler->swizzle(dstRow, fSrcBuffer.get());
        dstRow = SkTAddOffset<void>(dstRow, dstRowBytes);
    }
    return count;
}

bool SkWbmpCodec::onSkipScanlines(int count) {
    const size_t bytesToSkip = count * fSrcRowBytes;
    return this->stream()->skip(bytesToSkip) == bytesToSkip;
}

SkCodec::Result SkWbmpCodec::onStartScanlineDecode(const SkImageInfo& dstInfo,
        const Options& options, SkPMColor inputColorTable[], int* inputColorCount) {
    if (options.fSubset) {
        // Subsets are not supported.
        return kUnimplemented;
    }

    if (!valid_color_type(dstInfo.colorType(), dstInfo.alphaType()) ||
            !valid_alpha(dstInfo.alphaType(), this->getInfo().alphaType())) {
        return kInvalidConversion;
    }

    // Fill in the color table
    setup_color_table(dstInfo.colorType(), inputColorTable, inputColorCount);

    // Copy the color table to a pointer that can be owned by the scanline decoder
    if (kIndex_8_SkColorType == dstInfo.colorType()) {
        fColorTable.reset(new SkColorTable(inputColorTable, 2));
    }

    // Initialize the swizzler
    fSwizzler.reset(this->initializeSwizzler(dstInfo, get_color_ptr(fColorTable.get()), options));
    SkASSERT(fSwizzler);

    fSrcBuffer.reset(fSrcRowBytes);

    return kSuccess;
}
