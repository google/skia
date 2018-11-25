/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec.h"
#include "SkCodecPriv.h"
#include "SkColorData.h"
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

static inline bool valid_color_type(const SkImageInfo& dstInfo) {
    switch (dstInfo.colorType()) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
        case kGray_8_SkColorType:
        case kRGB_565_SkColorType:
            return true;
        case kRGBA_F16_SkColorType:
            return dstInfo.colorSpace() && dstInfo.colorSpace()->gammaIsLinear();
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

SkSwizzler* SkWbmpCodec::initializeSwizzler(const SkImageInfo& info, const Options& opts) {
    return SkSwizzler::CreateSwizzler(this->getEncodedInfo(), nullptr, info, opts);
}

bool SkWbmpCodec::readRow(uint8_t* row) {
    return this->stream()->read(row, fSrcRowBytes) == fSrcRowBytes;
}

SkWbmpCodec::SkWbmpCodec(int width, int height, const SkEncodedInfo& info,
                         std::unique_ptr<SkStream> stream)
    // Wbmp does not need a colorXform, so choose an arbitrary srcFormat.
    : INHERITED(width, height, info, SkColorSpaceXform::ColorFormat(),
                std::move(stream), SkColorSpace::MakeSRGB())
    , fSrcRowBytes(get_src_row_bytes(this->getInfo().width()))
    , fSwizzler(nullptr)
{}

SkEncodedImageFormat SkWbmpCodec::onGetEncodedFormat() const {
    return SkEncodedImageFormat::kWBMP;
}

bool SkWbmpCodec::conversionSupported(const SkImageInfo& dst, SkColorType /*srcColor*/,
                                      bool srcIsOpaque, const SkColorSpace* srcCS) const {
    return valid_color_type(dst) && valid_alpha(dst.alphaType(), srcIsOpaque);
}

SkCodec::Result SkWbmpCodec::onGetPixels(const SkImageInfo& info,
                                         void* dst,
                                         size_t rowBytes,
                                         const Options& options,
                                         int* rowsDecoded) {
    if (options.fSubset) {
        // Subsets are not supported.
        return kUnimplemented;
    }

    // Initialize the swizzler
    std::unique_ptr<SkSwizzler> swizzler(this->initializeSwizzler(info, options));
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
    SkMemoryStream stream(buffer, bytesRead, false);
    return read_header(&stream, nullptr);
}

std::unique_ptr<SkCodec> SkWbmpCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                     Result* result) {
    SkISize size;
    if (!read_header(stream.get(), &size)) {
        // This already succeeded in IsWbmp, so this stream was corrupted in/
        // after rewind.
        *result = kCouldNotRewind;
        return nullptr;
    }
    *result = kSuccess;
    SkEncodedInfo info = SkEncodedInfo::Make(SkEncodedInfo::kGray_Color,
            SkEncodedInfo::kOpaque_Alpha, 1);
    return std::unique_ptr<SkCodec>(new SkWbmpCodec(size.width(), size.height(), info,
                                                    std::move(stream)));
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
        const Options& options) {
    if (options.fSubset) {
        // Subsets are not supported.
        return kUnimplemented;
    }

    // Initialize the swizzler
    fSwizzler.reset(this->initializeSwizzler(dstInfo, options));
    SkASSERT(fSwizzler);

    fSrcBuffer.reset(fSrcRowBytes);

    return kSuccess;
}
