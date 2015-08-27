/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkEndian.h"
#include "SkColorPriv.h"
#include "SkImageDecoder.h"
#include "SkScaledBitmapSampler.h"
#include "SkStream.h"
#include "SkStreamPriv.h"
#include "SkTypes.h"

#include "SkTextureCompressor.h"

class SkASTCImageDecoder : public SkImageDecoder {
public:
    SkASTCImageDecoder() { }

    Format getFormat() const override {
        return kASTC_Format;
    }

protected:
    Result onDecode(SkStream* stream, SkBitmap* bm, Mode) override;

private:
    typedef SkImageDecoder INHERITED;
};

/////////////////////////////////////////////////////////////////////////////////////////

static const uint32_t kASTCMagicNumber = 0x5CA1AB13;

static inline int read_24bit(const uint8_t* buf) {
    // Assume everything is little endian...
    return
        static_cast<int>(buf[0]) |
        (static_cast<int>(buf[1]) << 8) |
        (static_cast<int>(buf[2]) << 16);
}

SkImageDecoder::Result SkASTCImageDecoder::onDecode(SkStream* stream, SkBitmap* bm, Mode mode) {
    SkAutoMalloc autoMal;
    const size_t length = SkCopyStreamToStorage(&autoMal, stream);
    if (0 == length) {
        return kFailure;
    }

    unsigned char* buf = (unsigned char*)autoMal.get();

    // Make sure that the magic header is there...
    SkASSERT(SkEndian_SwapLE32(*(reinterpret_cast<uint32_t*>(buf))) == kASTCMagicNumber);

    // Advance past the magic header
    buf += 4;

    const int blockDimX = buf[0];
    const int blockDimY = buf[1];
    const int blockDimZ = buf[2];

    if (1 != blockDimZ) {
        // We don't support decoding 3D
        return kFailure;
    }

    // Choose the proper ASTC format
    SkTextureCompressor::Format astcFormat;
    if (4 == blockDimX && 4 == blockDimY) {
        astcFormat = SkTextureCompressor::kASTC_4x4_Format;
    } else if (5 == blockDimX && 4 == blockDimY) {
        astcFormat = SkTextureCompressor::kASTC_5x4_Format;
    } else if (5 == blockDimX && 5 == blockDimY) {
        astcFormat = SkTextureCompressor::kASTC_5x5_Format;
    } else if (6 == blockDimX && 5 == blockDimY) {
        astcFormat = SkTextureCompressor::kASTC_6x5_Format;
    } else if (6 == blockDimX && 6 == blockDimY) {
        astcFormat = SkTextureCompressor::kASTC_6x6_Format;
    } else if (8 == blockDimX && 5 == blockDimY) {
        astcFormat = SkTextureCompressor::kASTC_8x5_Format;
    } else if (8 == blockDimX && 6 == blockDimY) {
        astcFormat = SkTextureCompressor::kASTC_8x6_Format;
    } else if (8 == blockDimX && 8 == blockDimY) {
        astcFormat = SkTextureCompressor::kASTC_8x8_Format;
    } else if (10 == blockDimX && 5 == blockDimY) {
        astcFormat = SkTextureCompressor::kASTC_10x5_Format;
    } else if (10 == blockDimX && 6 == blockDimY) {
        astcFormat = SkTextureCompressor::kASTC_10x6_Format;
    } else if (10 == blockDimX && 8 == blockDimY) {
        astcFormat = SkTextureCompressor::kASTC_10x8_Format;
    } else if (10 == blockDimX && 10 == blockDimY) {
        astcFormat = SkTextureCompressor::kASTC_10x10_Format;
    } else if (12 == blockDimX && 10 == blockDimY) {
        astcFormat = SkTextureCompressor::kASTC_12x10_Format;
    } else if (12 == blockDimX && 12 == blockDimY) {
        astcFormat = SkTextureCompressor::kASTC_12x12_Format;
    } else {
        // We don't support any other block dimensions..
        return kFailure;
    }

    // Advance buf past the block dimensions
    buf += 3;

    // Read the width/height/depth from the buffer...
    const int width = read_24bit(buf);
    const int height = read_24bit(buf + 3);
    const int depth = read_24bit(buf + 6);

    if (1 != depth) {
        // We don't support decoding 3D.
        return kFailure;
    }

    // Advance the buffer past the image dimensions
    buf += 9;

    // Setup the sampler...
    SkScaledBitmapSampler sampler(width, height, this->getSampleSize());

    // Determine the alpha of the bitmap...
    SkAlphaType alphaType = kOpaque_SkAlphaType;
    if (this->getRequireUnpremultipliedColors()) {
        alphaType = kUnpremul_SkAlphaType;
    } else {
        alphaType = kPremul_SkAlphaType;
    }

    // Set the config...
    bm->setInfo(SkImageInfo::MakeN32(sampler.scaledWidth(), sampler.scaledHeight(), alphaType));

    if (SkImageDecoder::kDecodeBounds_Mode == mode) {
        return kSuccess;
    }

    if (!this->allocPixelRef(bm, nullptr)) {
        return kFailure;
    }

    // Lock the pixels, since we're about to write to them...
    SkAutoLockPixels alp(*bm);

    if (!sampler.begin(bm, SkScaledBitmapSampler::kRGBA, *this)) {
        return kFailure;
    }

    // ASTC Data is encoded as RGBA pixels, so we should extract it as such
    int nPixels = width * height;
    SkAutoMalloc outRGBAData(nPixels * 4);
    uint8_t *outRGBADataPtr = reinterpret_cast<uint8_t *>(outRGBAData.get());

    // Decode ASTC
    if (!SkTextureCompressor::DecompressBufferFromFormat(
            outRGBADataPtr, width*4, buf, width, height, astcFormat)) {
        return kFailure;
    }

    // Set each of the pixels...
    const int srcRowBytes = width * 4;
    const int dstHeight = sampler.scaledHeight();
    const uint8_t *srcRow = reinterpret_cast<uint8_t *>(outRGBADataPtr);
    srcRow += sampler.srcY0() * srcRowBytes;
    for (int y = 0; y < dstHeight; ++y) {
        sampler.next(srcRow);
        srcRow += sampler.srcDY() * srcRowBytes;
    }

    return kSuccess;
}

/////////////////////////////////////////////////////////////////////////////////////////
DEFINE_DECODER_CREATOR(ASTCImageDecoder);
/////////////////////////////////////////////////////////////////////////////////////////

static bool is_astc(SkStreamRewindable* stream) {
    // Read the ASTC header and make sure it's valid.
    uint32_t magic;
    if (stream->read((void*)&magic, 4) != 4) {
        return false;
    }

    return kASTCMagicNumber == SkEndian_SwapLE32(magic);
}

static SkImageDecoder* sk_libastc_dfactory(SkStreamRewindable* stream) {
    if (is_astc(stream)) {
        return new SkASTCImageDecoder;
    }
    return nullptr;
}

static SkImageDecoder_DecodeReg gReg(sk_libastc_dfactory);

static SkImageDecoder::Format get_format_astc(SkStreamRewindable* stream) {
    if (is_astc(stream)) {
        return SkImageDecoder::kASTC_Format;
    }
    return SkImageDecoder::kUnknown_Format;
}

static SkImageDecoder_FormatReg gFormatReg(get_format_astc);
