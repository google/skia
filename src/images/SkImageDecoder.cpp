/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkImageDecoder.h"
#include "SkBitmap.h"
#include "SkImagePriv.h"
#include "SkPixelRef.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkCanvas.h"

SK_DEFINE_INST_COUNT(SkImageDecoder::Peeker)
SK_DEFINE_INST_COUNT(SkImageDecoder::Chooser)
SK_DEFINE_INST_COUNT(SkImageDecoderFactory)

const char *SkImageDecoder::sFormatName[] = {
    "Unknown Format",
    "BMP",
    "GIF",
    "ICO",
    "JPEG",
    "PNG",
    "WBMP",
    "WEBP",
};

static SkBitmap::Config gDeviceConfig = SkBitmap::kNo_Config;

SkBitmap::Config SkImageDecoder::GetDeviceConfig()
{
    return gDeviceConfig;
}

void SkImageDecoder::SetDeviceConfig(SkBitmap::Config config)
{
    gDeviceConfig = config;
}

///////////////////////////////////////////////////////////////////////////////

SkImageDecoder::SkImageDecoder()
    : fPeeker(NULL), fChooser(NULL), fAllocator(NULL), fSampleSize(1),
      fDefaultPref(SkBitmap::kNo_Config), fDitherImage(true),
      fUsePrefTable(false),fPreferQualityOverSpeed(false) {
}

SkImageDecoder::~SkImageDecoder() {
    SkSafeUnref(fPeeker);
    SkSafeUnref(fChooser);
    SkSafeUnref(fAllocator);
}

SkImageDecoder::Format SkImageDecoder::getFormat() const {
    return kUnknown_Format;
}

const char* SkImageDecoder::getFormatName() const {
    SkASSERT(SK_ARRAY_COUNT(sFormatName) == kLastKnownFormat);
    return sFormatName[this->getFormat()];
}

SkImageDecoder::Peeker* SkImageDecoder::setPeeker(Peeker* peeker) {
    SkRefCnt_SafeAssign(fPeeker, peeker);
    return peeker;
}

SkImageDecoder::Chooser* SkImageDecoder::setChooser(Chooser* chooser) {
    SkRefCnt_SafeAssign(fChooser, chooser);
    return chooser;
}

SkBitmap::Allocator* SkImageDecoder::setAllocator(SkBitmap::Allocator* alloc) {
    SkRefCnt_SafeAssign(fAllocator, alloc);
    return alloc;
}

void SkImageDecoder::setSampleSize(int size) {
    if (size < 1) {
        size = 1;
    }
    fSampleSize = size;
}

bool SkImageDecoder::chooseFromOneChoice(SkBitmap::Config config, int width,
                                         int height) const {
    Chooser* chooser = fChooser;

    if (NULL == chooser) {    // no chooser, we just say YES to decoding :)
        return true;
    }
    chooser->begin(1);
    chooser->inspect(0, config, width, height);
    return chooser->choose() == 0;
}

bool SkImageDecoder::allocPixelRef(SkBitmap* bitmap,
                                   SkColorTable* ctable) const {
    return bitmap->allocPixels(fAllocator, ctable);
}

///////////////////////////////////////////////////////////////////////////////

void SkImageDecoder::setPrefConfigTable(const SkBitmap::Config pref[6]) {
    if (NULL == pref) {
        fUsePrefTable = false;
    } else {
        fUsePrefTable = true;
        memcpy(fPrefTable, pref, sizeof(fPrefTable));
    }
}

SkBitmap::Config SkImageDecoder::getPrefConfig(SrcDepth srcDepth,
                                               bool srcHasAlpha) const {
    SkBitmap::Config config;

    if (fUsePrefTable) {
        int index = 0;
        switch (srcDepth) {
            case kIndex_SrcDepth:
                index = 0;
                break;
            case k16Bit_SrcDepth:
                index = 2;
                break;
            case k32Bit_SrcDepth:
                index = 4;
                break;
        }
        if (srcHasAlpha) {
            index += 1;
        }
        config = fPrefTable[index];
    } else {
        config = fDefaultPref;
    }

    if (SkBitmap::kNo_Config == config) {
        config = SkImageDecoder::GetDeviceConfig();
    }
    return config;
}

bool SkImageDecoder::decode(SkStream* stream, SkBitmap* bm,
                            SkBitmap::Config pref, Mode mode, bool reuseBitmap) {
    // we reset this to false before calling onDecode
    fShouldCancelDecode = false;
    // assign this, for use by getPrefConfig(), in case fUsePrefTable is false
    fDefaultPref = pref;

    if (reuseBitmap) {
        SkAutoLockPixels alp(*bm);
        if (NULL != bm->getPixels()) {
            return this->onDecode(stream, bm, mode);
        }
    }

    // pass a temporary bitmap, so that if we return false, we are assured of
    // leaving the caller's bitmap untouched.
    SkBitmap    tmp;
    if (!this->onDecode(stream, &tmp, mode)) {
        return false;
    }
    bm->swap(tmp);
    return true;
}

bool SkImageDecoder::decodeRegion(SkBitmap* bm, const SkIRect& rect,
                                  SkBitmap::Config pref) {
    // we reset this to false before calling onDecodeRegion
    fShouldCancelDecode = false;
    // assign this, for use by getPrefConfig(), in case fUsePrefTable is false
    fDefaultPref = pref;

    return this->onDecodeRegion(bm, rect);
}

bool SkImageDecoder::buildTileIndex(SkStream* stream,
                                int *width, int *height) {
    // we reset this to false before calling onBuildTileIndex
    fShouldCancelDecode = false;

    return this->onBuildTileIndex(stream, width, height);
}

void SkImageDecoder::cropBitmap(SkBitmap *dst, SkBitmap *src, int sampleSize,
                int dstX, int dstY, int width, int height,
                int srcX, int srcY) {
    int w = width / sampleSize;
    int h = height / sampleSize;
    // if the destination has no pixels then we must allocate them.
    if (dst->isNull()) {
        dst->setConfig(src->getConfig(), w, h);
        dst->setIsOpaque(src->isOpaque());

        if (!this->allocPixelRef(dst, NULL)) {
            SkDEBUGF(("failed to allocate pixels needed to crop the bitmap"));
            return;
        }
    }
    // check to see if the destination is large enough to decode the desired
    // region. If this assert fails we will just draw as much of the source
    // into the destination that we can.
    SkASSERT(dst->width() >= w && dst->height() >= h);

    // Set the Src_Mode for the paint to prevent transparency issue in the
    // dest in the event that the dest was being re-used.
    SkPaint paint;
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);

    SkCanvas canvas(*dst);
    canvas.drawSprite(*src, (srcX - dstX) / sampleSize,
                            (srcY - dstY) / sampleSize,
                            &paint);
}

///////////////////////////////////////////////////////////////////////////////

bool SkImageDecoder::DecodeFile(const char file[], SkBitmap* bm,
                            SkBitmap::Config pref,  Mode mode, Format* format) {
    SkASSERT(file);
    SkASSERT(bm);

    SkAutoTUnref<SkStream> stream(SkStream::NewFromFile(file));
    if (stream.get()) {
        if (SkImageDecoder::DecodeStream(stream, bm, pref, mode, format)) {
            bm->pixelRef()->setURI(file);
            return true;
        }
    }
    return false;
}

bool SkImageDecoder::DecodeMemory(const void* buffer, size_t size, SkBitmap* bm,
                          SkBitmap::Config pref, Mode mode, Format* format) {
    if (0 == size) {
        return false;
    }
    SkASSERT(buffer);

    SkMemoryStream  stream(buffer, size);
    return SkImageDecoder::DecodeStream(&stream, bm, pref, mode, format);
}

class TargetAllocator : public SkBitmap::Allocator {

public:
    TargetAllocator(void* target)
        : fTarget(target) {}

    virtual bool allocPixelRef(SkBitmap* bm, SkColorTable* ct) SK_OVERRIDE {
        // SkColorTable is not supported by Info/Target model.
        SkASSERT(NULL == ct);
        bm->setPixels(fTarget);
        return true;
    }

private:
    void* fTarget;
};

bool SkImageDecoder::DecodeMemoryToTarget(const void* buffer, size_t size,
                                          SkImage::Info* info,
                                          const SkBitmapFactory::Target* target) {
    if (NULL == info) {
        return false;
    }
    // FIXME: Just to get this working, implement in terms of existing
    // ImageDecoder calls.
    SkBitmap bm;
    SkMemoryStream stream(buffer, size);
    SkAutoTDelete<SkImageDecoder> decoder(SkImageDecoder::Factory(&stream));
    if (decoder.get() != NULL && decoder->decode(&stream, &bm, kDecodeBounds_Mode)) {
        // Now set info properly
        if (!SkBitmapToImageInfo(bm, info)) {
            return false;
        }

        // SkBitmapToImageInfo will return false if Index8 is used. kIndex8
        // is not supported by the Info/Target model, since kIndex8 requires
        // an SkColorTable, which this model does not keep track of.
        SkASSERT(bm.config() != SkBitmap::kIndex8_Config);

        if (NULL == target) {
            return true;
        }

        if (target->fRowBytes != (uint32_t) bm.rowBytes()) {
            if (target->fRowBytes < SkImageMinRowBytes(*info)) {
                SkASSERT(!"Desired row bytes is too small");
                return false;
            }
            bm.setConfig(bm.config(), bm.width(), bm.height(), target->fRowBytes);
        }

        TargetAllocator allocator(target->fAddr);
        decoder->setAllocator(&allocator);
        stream.rewind();
        bool success = decoder->decode(&stream, &bm, kDecodePixels_Mode);
        // Remove the allocator, since it's on the stack.
        decoder->setAllocator(NULL);
        return success;
    }
    return false;
}


bool SkImageDecoder::DecodeStream(SkStream* stream, SkBitmap* bm,
                          SkBitmap::Config pref, Mode mode, Format* format) {
    SkASSERT(stream);
    SkASSERT(bm);

    bool success = false;
    SkImageDecoder* codec = SkImageDecoder::Factory(stream);

    if (NULL != codec) {
        success = codec->decode(stream, bm, pref, mode);
        if (success && format) {
            *format = codec->getFormat();
        }
        delete codec;
    }
    return success;
}
