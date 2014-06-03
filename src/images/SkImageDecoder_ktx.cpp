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
#include "SkStreamHelpers.h"
#include "SkTypes.h"

#include "ktx.h"
#include "etc1.h"

/////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////

// KTX Image decoder
// ---
// KTX is a general texture data storage file format ratified by the Khronos Group. As an
// overview, a KTX file contains all of the appropriate values needed to fully specify a
// texture in an OpenGL application, including the use of compressed data.
//
// This decoder is meant to be used with an SkDiscardablePixelRef so that GPU backends
// can sniff the data before creating a texture. If they encounter a compressed format
// that they understand, they can then upload the data directly to the GPU. Otherwise,
// they will decode the data into a format that Skia supports.

class SkKTXImageDecoder : public SkImageDecoder {
public:
    SkKTXImageDecoder() { }

    virtual Format getFormat() const SK_OVERRIDE {
        return kKTX_Format;
    }

protected:
    virtual bool onDecode(SkStream* stream, SkBitmap* bm, Mode) SK_OVERRIDE;

private:
    typedef SkImageDecoder INHERITED;
};

bool SkKTXImageDecoder::onDecode(SkStream* stream, SkBitmap* bm, Mode mode) {
    // TODO: Implement SkStream::copyToData() that's cheap for memory and file streams
    SkAutoDataUnref data(CopyStreamToData(stream));
    if (NULL == data) {
        return false;
    }

    SkKTXFile ktxFile(data);
    if (!ktxFile.valid()) {
        return false;
    }

    const unsigned short width = ktxFile.width();
    const unsigned short height = ktxFile.height();

    // should we allow the Chooser (if present) to pick a config for us???
    if (!this->chooseFromOneChoice(SkBitmap::kARGB_8888_Config, width, height)) {
        return false;
    }

    // Setup the sampler...
    SkScaledBitmapSampler sampler(width, height, this->getSampleSize());

    // Set the config...
    bm->setConfig(SkBitmap::kARGB_8888_Config,
                  sampler.scaledWidth(), sampler.scaledHeight(),
                  0,
                  ktxFile.isRGBA8()? kUnpremul_SkAlphaType : kOpaque_SkAlphaType);
    if (SkImageDecoder::kDecodeBounds_Mode == mode) {
        return true;
    }
    
    // If we've made it this far, then we know how to grok the data.
    if (!this->allocPixelRef(bm, NULL)) {
        return false;
    }

    // Lock the pixels, since we're about to write to them...
    SkAutoLockPixels alp(*bm);

    if (ktxFile.isETC1()) {
        if (!sampler.begin(bm, SkScaledBitmapSampler::kRGB, *this)) {
            return false;
        }

        // ETC1 Data is encoded as RGB pixels, so we should extract it as such
        int nPixels = width * height;
        SkAutoMalloc outRGBData(nPixels * 3);
        etc1_byte *outRGBDataPtr = reinterpret_cast<etc1_byte *>(outRGBData.get());

        // Decode ETC1
        const etc1_byte *buf = reinterpret_cast<const etc1_byte *>(ktxFile.pixelData());
        if (etc1_decode_image(buf, outRGBDataPtr, width, height, 3, width*3)) {
            return false;
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

        return true;

    } else if (ktxFile.isRGB8()) {

        // Uncompressed RGB data (without alpha)
        if (!sampler.begin(bm, SkScaledBitmapSampler::kRGB, *this)) {
            return false;
        }

        // Just need to read RGB pixels
        const int srcRowBytes = width * 3;
        const int dstHeight = sampler.scaledHeight();
        const uint8_t *srcRow = reinterpret_cast<const uint8_t *>(ktxFile.pixelData());
        srcRow += sampler.srcY0() * srcRowBytes;
        for (int y = 0; y < dstHeight; ++y) {
            sampler.next(srcRow);
            srcRow += sampler.srcDY() * srcRowBytes;
        }

        return true;

    } else if (ktxFile.isRGBA8()) {

        // Uncompressed RGBA data
        if (!sampler.begin(bm, SkScaledBitmapSampler::kRGBA, *this)) {
            return false;
        }

        // Just need to read RGBA pixels
        const int srcRowBytes = width * 4;
        const int dstHeight = sampler.scaledHeight();
        const uint8_t *srcRow = reinterpret_cast<const uint8_t *>(ktxFile.pixelData());
        srcRow += sampler.srcY0() * srcRowBytes;
        for (int y = 0; y < dstHeight; ++y) {
            sampler.next(srcRow);
            srcRow += sampler.srcDY() * srcRowBytes;
        }

        return true;
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
DEFINE_DECODER_CREATOR(KTXImageDecoder);
/////////////////////////////////////////////////////////////////////////////////////////

static SkImageDecoder* sk_libktx_dfactory(SkStreamRewindable* stream) {
    if (SkKTXFile::is_ktx(stream)) {
        return SkNEW(SkKTXImageDecoder);
    }
    return NULL;
}

static SkImageDecoder_DecodeReg gReg(sk_libktx_dfactory);

static SkImageDecoder::Format get_format_ktx(SkStreamRewindable* stream) {
    if (SkKTXFile::is_ktx(stream)) {
        return SkImageDecoder::kKTX_Format;
    }
    return SkImageDecoder::kUnknown_Format;
}

static SkImageDecoder_FormatReg gFormatReg(get_format_ktx);
