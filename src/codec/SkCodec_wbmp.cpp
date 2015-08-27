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
#include "SkScaledCodec.h"
#include "SkStream.h"
#include "SkCodec_wbmp.h"

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
    uint64_t width, height;
    uint16_t data;
    if (stream->read(&data, 2) != 2 || data != 0) {
        return false;
    }
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

SkSwizzler* SkWbmpCodec::initializeSwizzler(const SkImageInfo& info,
        const SkPMColor* ctable, const Options& opts) {
    // Create the swizzler based on the desired color type
    switch (info.colorType()) {
        case kIndex_8_SkColorType:
        case kN32_SkColorType:
        case kRGB_565_SkColorType:
        case kGray_8_SkColorType:
            return SkSwizzler::CreateSwizzler(SkSwizzler::kBit, ctable, info, opts.fZeroInitialized, 
                                              this->getInfo());
        default:
            return nullptr;
    }
}

SkCodec::Result SkWbmpCodec::readRow(uint8_t* row) {
    if (this->stream()->read(row, fSrcRowBytes) != fSrcRowBytes) {
        return kIncompleteInput;
    }
    return kSuccess;
}

SkWbmpCodec::SkWbmpCodec(const SkImageInfo& info, SkStream* stream)
    : INHERITED(info, stream)
    , fSrcRowBytes(get_src_row_bytes(this->getInfo().width()))
{}

SkEncodedFormat SkWbmpCodec::onGetEncodedFormat() const {
    return kWBMP_SkEncodedFormat;
}

SkCodec::Result SkWbmpCodec::onGetPixels(const SkImageInfo& info,
                                         void* dst,
                                         size_t rowBytes,
                                         const Options& options,
                                         SkPMColor ctable[],
                                         int* ctableCount) {
    if (!this->rewindIfNeeded()) {
        return kCouldNotRewind;
    }
    if (options.fSubset) {
        // Subsets are not supported.
        return kUnimplemented;
    }
    if (info.dimensions() != this->getInfo().dimensions()) {
        return kInvalidScale;
    }

    if (!valid_alpha(info.alphaType(), this->getInfo().alphaType())) {
        return SkCodec::kInvalidConversion;
    }

    // Prepare a color table if necessary
    setup_color_table(info.colorType(), ctable, ctableCount);


    // Initialize the swizzler
    SkAutoTDelete<SkSwizzler> swizzler(this->initializeSwizzler(info, ctable, options));
    if (nullptr == swizzler.get()) {
        return kInvalidConversion;
    }

    // Perform the decode
    SkISize size = info.dimensions();
    SkAutoTMalloc<uint8_t> src(fSrcRowBytes);
    void* dstRow = dst;
    for (int y = 0; y < size.height(); ++y) {
        Result rowResult = this->readRow(src.get());
        if (kSuccess != rowResult) {
            return rowResult;
        }
        swizzler->swizzle(dstRow, src.get());
        dstRow = SkTAddOffset<void>(dstRow, rowBytes);
    }
    return kSuccess;
}

bool SkWbmpCodec::IsWbmp(SkStream* stream) {
    return read_header(stream, nullptr);
}

SkCodec* SkWbmpCodec::NewFromStream(SkStream* stream) {
    SkAutoTDelete<SkStream> streamDeleter(stream);
    SkISize size;
    if (!read_header(stream, &size)) {
        return nullptr;
    }
    SkImageInfo info = SkImageInfo::Make(size.width(), size.height(),
            kGray_8_SkColorType, kOpaque_SkAlphaType);
    return new SkWbmpCodec(info, streamDeleter.detach());
}

class SkWbmpScanlineDecoder : public SkScanlineDecoder {
public:
    /*
     * Takes ownership of all pointer paramters.
     */
    SkWbmpScanlineDecoder(SkWbmpCodec* codec)
        : INHERITED(codec->getInfo())
        , fCodec(codec)
        , fColorTable(nullptr)
        , fSwizzler(nullptr)
        , fSrcBuffer(codec->fSrcRowBytes)
    {}

    SkCodec::Result onGetScanlines(void* dst, int count, size_t dstRowBytes) override {
        void* dstRow = dst;
        for (int y = 0; y < count; ++y) {
            SkCodec::Result rowResult = fCodec->readRow(fSrcBuffer.get());
            if (SkCodec::kSuccess != rowResult) {
                return rowResult;
            }
            fSwizzler->swizzle(dstRow, fSrcBuffer.get());
            dstRow = SkTAddOffset<void>(dstRow, dstRowBytes);
        }
        return SkCodec::kSuccess;
    }

    SkCodec::Result onStart(const SkImageInfo& dstInfo,
            const SkCodec::Options& options, SkPMColor inputColorTable[],
            int* inputColorCount) {
        if (!fCodec->rewindIfNeeded()) {
            return SkCodec::kCouldNotRewind;
        }
        if (options.fSubset) {
            // Subsets are not supported.
            return SkCodec::kUnimplemented;
        }
        if (dstInfo.dimensions() != this->getInfo().dimensions()) {
            if (!SkScaledCodec::DimensionsSupportedForSampling(this->getInfo(), dstInfo)) {
                return SkCodec::kInvalidScale;
            }
        }

        if (!valid_alpha(dstInfo.alphaType(), this->getInfo().alphaType())) {
            return SkCodec::kInvalidConversion;
        }

        // Fill in the color table
        setup_color_table(dstInfo.colorType(), inputColorTable, inputColorCount);

        // Copy the color table to a pointer that can be owned by the scanline decoder
        if (kIndex_8_SkColorType == dstInfo.colorType()) {
            fColorTable.reset(new SkColorTable(inputColorTable, 2));
        }

        // Initialize the swizzler
        fSwizzler.reset(fCodec->initializeSwizzler(dstInfo,
                get_color_ptr(fColorTable.get()), options));
        if (nullptr == fSwizzler.get()) {
            return SkCodec::kInvalidConversion;
        }

        return SkCodec::kSuccess;
    }

    SkEncodedFormat onGetEncodedFormat() const {
        return kWBMP_SkEncodedFormat;
    }

private:
    SkAutoTDelete<SkWbmpCodec>   fCodec;
    SkAutoTUnref<SkColorTable>   fColorTable;
    SkAutoTDelete<SkSwizzler>    fSwizzler;
    SkAutoTMalloc<uint8_t>       fSrcBuffer;

    typedef SkScanlineDecoder INHERITED;
};

SkScanlineDecoder* SkWbmpCodec::NewSDFromStream(SkStream* stream) {
    SkAutoTDelete<SkWbmpCodec> codec(static_cast<SkWbmpCodec*>(
            SkWbmpCodec::NewFromStream(stream)));
    if (!codec) {
        return nullptr;
    }

    // Return the new scanline decoder
    return new SkWbmpScanlineDecoder(codec.detach());
}
