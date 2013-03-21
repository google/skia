
/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "bmpdecoderhelper.h"
#include "SkImageDecoder.h"
#include "SkScaledBitmapSampler.h"
#include "SkStream.h"
#include "SkColorPriv.h"
#include "SkTDArray.h"
#include "SkTRegistry.h"

class SkBMPImageDecoder : public SkImageDecoder {
public:
    SkBMPImageDecoder() {}

    virtual Format getFormat() const SK_OVERRIDE {
        return kBMP_Format;
    }

protected:
    virtual bool onDecode(SkStream* stream, SkBitmap* bm, Mode mode) SK_OVERRIDE;

private:
    typedef SkImageDecoder INHERITED;
};

///////////////////////////////////////////////////////////////////////////////
DEFINE_DECODER_CREATOR(BMPImageDecoder);
///////////////////////////////////////////////////////////////////////////////

static SkImageDecoder* sk_libbmp_dfactory(SkStream* stream) {
    static const char kBmpMagic[] = { 'B', 'M' };


    char buffer[sizeof(kBmpMagic)];

    if (stream->read(buffer, sizeof(kBmpMagic)) == sizeof(kBmpMagic) &&
        !memcmp(buffer, kBmpMagic, sizeof(kBmpMagic))) {
        return SkNEW(SkBMPImageDecoder);
    }
    return NULL;
}

static SkTRegistry<SkImageDecoder*, SkStream*> gReg(sk_libbmp_dfactory);

///////////////////////////////////////////////////////////////////////////////

class SkBmpDecoderCallback : public image_codec::BmpDecoderCallback {
public:
    // we don't copy the bitmap, just remember the pointer
    SkBmpDecoderCallback(bool justBounds) : fJustBounds(justBounds) {}

    // override from BmpDecoderCallback
    virtual uint8* SetSize(int width, int height) {
        fWidth = width;
        fHeight = height;
        if (fJustBounds) {
            return NULL;
        }

        fRGB.setCount(width * height * 3);  // 3 == r, g, b
        return fRGB.begin();
    }

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    const uint8_t* rgb() const { return fRGB.begin(); }

private:
    SkTDArray<uint8_t> fRGB;
    int fWidth;
    int fHeight;
    bool fJustBounds;
};

bool SkBMPImageDecoder::onDecode(SkStream* stream, SkBitmap* bm, Mode mode) {

    size_t length = stream->getLength();
    SkAutoMalloc storage(length);

    if (stream->read(storage.get(), length) != length) {
        return false;
    }

    const bool justBounds = SkImageDecoder::kDecodeBounds_Mode == mode;
    SkBmpDecoderCallback callback(justBounds);

    // Now decode the BMP into callback's rgb() array [r,g,b, r,g,b, ...]
    {
        image_codec::BmpDecoderHelper helper;
        const int max_pixels = 16383*16383; // max width*height
        if (!helper.DecodeImage((const char*)storage.get(), length,
                                max_pixels, &callback)) {
            return false;
        }
    }

    // we don't need this anymore, so free it now (before we try to allocate
    // the bitmap's pixels) rather than waiting for its destructor
    storage.free();

    int width = callback.width();
    int height = callback.height();
    SkBitmap::Config config = this->getPrefConfig(k32Bit_SrcDepth, false);

    // only accept prefConfig if it makes sense for us
    if (SkBitmap::kARGB_4444_Config != config &&
            SkBitmap::kRGB_565_Config != config) {
        config = SkBitmap::kARGB_8888_Config;
    }

    SkScaledBitmapSampler sampler(width, height, getSampleSize());

    if (justBounds) {
        bm->setConfig(config, sampler.scaledWidth(), sampler.scaledHeight());
        bm->setIsOpaque(true);
        return true;
    }
    // No Bitmap reuse supported for this format
    if (!bm->isNull()) {
        return false;
    }

    bm->setConfig(config, sampler.scaledWidth(), sampler.scaledHeight());
    bm->setIsOpaque(true);

    if (!this->allocPixelRef(bm, NULL)) {
        return false;
    }

    SkAutoLockPixels alp(*bm);

    if (!sampler.begin(bm, SkScaledBitmapSampler::kRGB, getDitherImage())) {
        return false;
    }

    const int srcRowBytes = width * 3;
    const int dstHeight = sampler.scaledHeight();
    const uint8_t* srcRow = callback.rgb();

    srcRow += sampler.srcY0() * srcRowBytes;
    for (int y = 0; y < dstHeight; y++) {
        sampler.next(srcRow);
        srcRow += sampler.srcDY() * srcRowBytes;
    }
    return true;
}
