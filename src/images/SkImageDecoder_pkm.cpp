/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorPriv.h"
#include "SkImageDecoder.h"
#include "SkScaledBitmapSampler.h"
#include "SkStream.h"
#include "SkStreamPriv.h"
#include "SkTextureCompressor.h"
#include "SkTypes.h"

#include "etc1.h"

class SkPKMImageDecoder : public SkImageDecoder {
public:
    SkPKMImageDecoder() { }

    Format getFormat() const override {
        return kPKM_Format;
    }

protected:
    Result onDecode(SkStream* stream, SkBitmap* bm, Mode) override;

private:
    typedef SkImageDecoder INHERITED;
};

/////////////////////////////////////////////////////////////////////////////////////////

SkImageDecoder::Result SkPKMImageDecoder::onDecode(SkStream* stream, SkBitmap* bm, Mode mode) {
    SkAutoMalloc autoMal;
    const size_t length = SkCopyStreamToStorage(&autoMal, stream);
    if (0 == length) {
        return kFailure;
    }

    unsigned char* buf = (unsigned char*)autoMal.get();

    // Make sure original PKM header is there...
    SkASSERT(etc1_pkm_is_valid(buf));

    const unsigned short width = etc1_pkm_get_width(buf);
    const unsigned short height = etc1_pkm_get_height(buf);

    // Setup the sampler...
    SkScaledBitmapSampler sampler(width, height, this->getSampleSize());

    // Set the config...
    bm->setInfo(SkImageInfo::MakeN32(sampler.scaledWidth(), sampler.scaledHeight(),
                                     kOpaque_SkAlphaType));
    if (SkImageDecoder::kDecodeBounds_Mode == mode) {
        return kSuccess;
    }

    if (!this->allocPixelRef(bm, nullptr)) {
        return kFailure;
    }

    // Lock the pixels, since we're about to write to them...
    SkAutoLockPixels alp(*bm);

    if (!sampler.begin(bm, SkScaledBitmapSampler::kRGB, *this)) {
        return kFailure;
    }

    // Advance buffer past the header
    buf += ETC_PKM_HEADER_SIZE;

    // ETC1 Data is encoded as RGB pixels, so we should extract it as such
    int nPixels = width * height;
    SkAutoMalloc outRGBData(nPixels * 3);
    uint8_t *outRGBDataPtr = reinterpret_cast<uint8_t *>(outRGBData.get());

    // Decode ETC1
    if (!SkTextureCompressor::DecompressBufferFromFormat(
            outRGBDataPtr, width*3, buf, width, height, SkTextureCompressor::kETC1_Format)) {
        return kFailure;
    }

    // Set each of the pixels...
    const int srcRowBytes = width * 3;
    const int dstHeight = sampler.scaledHeight();
    const uint8_t *srcRow = reinterpret_cast<uint8_t *>(outRGBDataPtr);
    srcRow += sampler.srcY0() * srcRowBytes;
    for (int y = 0; y < dstHeight; ++y) {
        sampler.next(srcRow);
        srcRow += sampler.srcDY() * srcRowBytes;
    }

    return kSuccess;
}

/////////////////////////////////////////////////////////////////////////////////////////
DEFINE_DECODER_CREATOR(PKMImageDecoder);
/////////////////////////////////////////////////////////////////////////////////////////

static bool is_pkm(SkStreamRewindable* stream) {
    // Read the PKM header and make sure it's valid.
    unsigned char buf[ETC_PKM_HEADER_SIZE];
    if (stream->read((void*)buf, ETC_PKM_HEADER_SIZE) != ETC_PKM_HEADER_SIZE) {
        return false;
    }

    return SkToBool(etc1_pkm_is_valid(buf));
}

static SkImageDecoder* sk_libpkm_dfactory(SkStreamRewindable* stream) {
    if (is_pkm(stream)) {
        return new SkPKMImageDecoder;
    }
    return nullptr;
}

static SkImageDecoder_DecodeReg gReg(sk_libpkm_dfactory);

static SkImageDecoder::Format get_format_pkm(SkStreamRewindable* stream) {
    if (is_pkm(stream)) {
        return SkImageDecoder::kPKM_Format;
    }
    return SkImageDecoder::kUnknown_Format;
}

static SkImageDecoder_FormatReg gFormatReg(get_format_pkm);
