/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec.h"
#include "SkColorPriv.h"
#include "SkStream.h"
#include "SkCodec_wbmp.h"

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
    if (!read_mbf(stream, &height) || width > 0xFFFF || !height) {
        return false;
    }
    if (size) {
        *size = SkISize::Make(SkToS32(width), SkToS32(height));
    }
    return true;
}

#define BLACK SkPackARGB32NoCheck(0xFF, 0, 0, 0)
#define WHITE SkPackARGB32NoCheck(0xFF, 0xFF, 0xFF, 0xFF)

#define GRAYSCALE_BLACK 0
#define GRAYSCALE_WHITE 0xFF

#define RGB565_BLACK 0
#define RGB565_WHITE 0xFFFF

static SkPMColor bit_to_pmcolor(U8CPU bit) { return bit ? WHITE : BLACK; }

static uint8_t bit_to_bit(U8CPU bit) { return bit; }

static uint8_t bit_to_grayscale(U8CPU bit) {
    return bit ? GRAYSCALE_WHITE : GRAYSCALE_BLACK;
}

static uint16_t bit_to_rgb565(U8CPU bit) {
    return bit ? RGB565_WHITE : RGB565_BLACK;
}

typedef void (*ExpandProc)(uint8_t*, const uint8_t*, int);

// TODO(halcanary): Add this functionality (grayscale and indexed output) to
//                  SkSwizzler and use it here.
template <typename T, T (*TRANSFORM)(U8CPU)>
static void expand_bits_to_T(uint8_t* dstptr, const uint8_t* src, int bits) {
    T* dst = reinterpret_cast<T*>(dstptr);
    int bytes = bits >> 3;
    for (int i = 0; i < bytes; i++) {
        U8CPU mask = *src++;
        for (int j = 0; j < 8; j++) {
            dst[j] = TRANSFORM((mask >> (7 - j)) & 1);
        }
        dst += 8;
    }
    bits &= 7;
    if (bits > 0) {
        U8CPU mask = *src;
        do {
            *dst++ = TRANSFORM((mask >> 7) & 1);
            mask <<= 1;
        } while (--bits != 0);
    }
}

SkWbmpCodec::SkWbmpCodec(const SkImageInfo& info, SkStream* stream)
    : INHERITED(info, stream) {}

SkEncodedFormat SkWbmpCodec::onGetEncodedFormat() const {
    return kWBMP_SkEncodedFormat;
}

SkImageGenerator::Result SkWbmpCodec::onGetPixels(const SkImageInfo& info,
                                                  void* pixels,
                                                  size_t rowBytes,
                                                  const Options&,
                                                  SkPMColor ctable[],
                                                  int* ctableCount) {
    SkCodec::RewindState rewindState = this->rewindIfNeeded();
    if (rewindState == kCouldNotRewind_RewindState) {
        return SkImageGenerator::kCouldNotRewind;
    } else if (rewindState == kRewound_RewindState) {
        (void)read_header(this->stream(), NULL);
    }
    if (info.dimensions() != this->getInfo().dimensions()) {
        return SkImageGenerator::kInvalidScale;
    }
    ExpandProc proc = NULL;
    switch (info.colorType()) {
        case kGray_8_SkColorType:
            proc = expand_bits_to_T<uint8_t, bit_to_grayscale>;
            break;
        case kN32_SkColorType:
            proc = expand_bits_to_T<SkPMColor, bit_to_pmcolor>;
            break;
        case kIndex_8_SkColorType:
            ctable[0] = BLACK;
            ctable[1] = WHITE;
            *ctableCount = 2;
            proc = expand_bits_to_T<uint8_t, bit_to_bit>;
            break;
        case kRGB_565_SkColorType:
            proc = expand_bits_to_T<uint16_t, bit_to_rgb565>;
            break;
        default:
            return SkImageGenerator::kInvalidConversion;
    }
    SkISize size = info.dimensions();
    uint8_t* dst = static_cast<uint8_t*>(pixels);
    size_t srcRowBytes = SkAlign8(size.width()) >> 3;
    SkAutoTMalloc<uint8_t> src(srcRowBytes);
    for (int y = 0; y < size.height(); ++y) {
        if (this->stream()->read(src.get(), srcRowBytes) != srcRowBytes) {
            return SkImageGenerator::kIncompleteInput;
        }
        proc(dst, src.get(), size.width());
        dst += rowBytes;
    }
    return SkImageGenerator::kSuccess;
}

bool SkWbmpCodec::IsWbmp(SkStream* stream) {
    return read_header(stream, NULL);
}

SkCodec* SkWbmpCodec::NewFromStream(SkStream* stream) {
    SkAutoTDelete<SkStream> streamDeleter(stream);
    SkISize size;
    if (!read_header(stream, &size)) {
        return NULL;
    }
    SkImageInfo info =
            SkImageInfo::Make(size.width(), size.height(), kGray_8_SkColorType,
                              kOpaque_SkAlphaType);
    return SkNEW_ARGS(SkWbmpCodec, (info, streamDeleter.detach()));
}
