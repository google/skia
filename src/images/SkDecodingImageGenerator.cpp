/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDecodingImageGenerator.h"
#include "SkData.h"
#include "SkImageDecoder.h"
#include "SkImageGenerator.h"
#include "SkImagePriv.h"
#include "SkStream.h"


namespace {
/**
 *  Special allocator used by getPixels(). Uses preallocated memory
 *  provided.
 */
class TargetAllocator : public SkBitmap::Allocator {
public:
    TargetAllocator(void* target, size_t rowBytes, const SkImageInfo& info)
        : fTarget(target)
        , fRowBytes(rowBytes)
        , fInfo(info) { }

    virtual bool allocPixelRef(SkBitmap* bm, SkColorTable* ct) SK_OVERRIDE {
        if ((SkImageInfoToBitmapConfig(fInfo) != bm->config())
            || (bm->width() != fInfo.fWidth)
            || (bm->height() != fInfo.fHeight)) {
            return false;
        }
        bm->setConfig(bm->config(), bm->width(), bm->height(),
                      fRowBytes, bm->alphaType());
        bm->setPixels(fTarget, ct);
        return true;
    }

private:
    void* fTarget;
    size_t fRowBytes;
    SkImageInfo fInfo;
    typedef SkBitmap::Allocator INHERITED;
};
}  // namespace
////////////////////////////////////////////////////////////////////////////////

SkDecodingImageGenerator::SkDecodingImageGenerator(SkData* data)
    : fData(data)
    , fHasInfo(false)
    , fDoCopyTo(false) {
    SkASSERT(fData != NULL);
    fStream = SkNEW_ARGS(SkMemoryStream, (fData));
    SkASSERT(fStream != NULL);
    SkASSERT(fStream->unique());
    fData->ref();
}

SkDecodingImageGenerator::SkDecodingImageGenerator(SkStreamRewindable* stream)
    : fData(NULL)
    , fStream(stream)
    , fHasInfo(false)
    , fDoCopyTo(false) {
    SkASSERT(fStream != NULL);
    SkASSERT(fStream->unique());
}

SkDecodingImageGenerator::~SkDecodingImageGenerator() {
    SkSafeUnref(fData);
    fStream->unref();
}

// TODO(halcanary): Give this macro a better name and move it into SkTypes.h
#ifdef SK_DEBUG
    #define SkCheckResult(expr, value)  SkASSERT((value) == (expr))
#else
    #define SkCheckResult(expr, value)  (void)(expr)
#endif

SkData* SkDecodingImageGenerator::refEncodedData() {
    // This functionality is used in `gm --serialize`
    if (fData != NULL) {
        return SkSafeRef(fData);
    }
    // TODO(halcanary): SkStreamRewindable needs a refData() function
    // which returns a cheap copy of the underlying data.
    if (!fStream->rewind()) {
        return NULL;
    }
    size_t length = fStream->getLength();
    if (0 == length) {
        return NULL;
    }
    void* buffer = sk_malloc_flags(length, 0);
    SkCheckResult(fStream->read(buffer, length), length);
    fData = SkData::NewFromMalloc(buffer, length);
    return SkSafeRef(fData);
}

bool SkDecodingImageGenerator::getInfo(SkImageInfo* info) {
    // info can be NULL.  If so, will update fInfo, fDoCopyTo, and fHasInfo.
    if (fHasInfo) {
        if (info != NULL) {
            *info = fInfo;
        }
        return true;
    }
    SkAssertResult(fStream->rewind());
    SkAutoTDelete<SkImageDecoder> decoder(SkImageDecoder::Factory(fStream));
    if (NULL == decoder.get()) {
        return false;
    }
    SkBitmap bitmap;
    if (!decoder->decode(fStream, &bitmap,
                         SkImageDecoder::kDecodeBounds_Mode)) {
        return false;
    }
    if (bitmap.config() == SkBitmap::kNo_Config) {
        return false;
    }
    if (!bitmap.asImageInfo(&fInfo)) {
        // We can't use bitmap.config() as is.
        if (!bitmap.canCopyTo(SkBitmap::kARGB_8888_Config)) {
            SkDEBUGFAIL("!bitmap->canCopyTo(SkBitmap::kARGB_8888_Config)");
            return false;
        }
        fDoCopyTo = true;
        fInfo.fWidth = bitmap.width();
        fInfo.fHeight = bitmap.height();
        fInfo.fColorType = kPMColor_SkColorType;
        fInfo.fAlphaType = bitmap.alphaType();
    }
    if (info != NULL) {
        *info = fInfo;
    }
    fHasInfo = true;
    return true;
}

bool SkDecodingImageGenerator::getPixels(const SkImageInfo& info,
                                         void* pixels,
                                         size_t rowBytes) {
    if (NULL == pixels) {
        return false;
    }
    if (!this->getInfo(NULL)) {
        return false;
    }
    if (SkImageInfoToBitmapConfig(info) == SkBitmap::kNo_Config) {
        return false;  // Unsupported SkColorType.
    }
    SkAssertResult(fStream->rewind());
    SkAutoTDelete<SkImageDecoder> decoder(SkImageDecoder::Factory(fStream));
    if (NULL == decoder.get()) {
        return false;
    }
    if (fInfo != info) {
        // The caller has specified a different info.  For now, this
        // is an error.  In the future, we will check to see if we can
        // convert.
        return false;
    }
    int bpp = SkBitmap::ComputeBytesPerPixel(SkImageInfoToBitmapConfig(info));
    if (static_cast<size_t>(bpp * info.fWidth) > rowBytes) {
        return false;
    }
    SkBitmap bitmap;
    if (!bitmap.setConfig(info, rowBytes)) {
        return false;
    }

    TargetAllocator allocator(pixels, rowBytes, info);
    if (!fDoCopyTo) {
        decoder->setAllocator(&allocator);
    }
    bool success = decoder->decode(fStream, &bitmap,
                                   SkImageDecoder::kDecodePixels_Mode);
    decoder->setAllocator(NULL);
    if (!success) {
        return false;
    }
    if (fDoCopyTo) {
        SkBitmap bm8888;
        bitmap.copyTo(&bm8888, SkBitmap::kARGB_8888_Config, &allocator);
    }
    return true;
}
bool SkDecodingImageGenerator::Install(SkData* data, SkBitmap* dst,
                                       SkDiscardableMemory::Factory* factory) {
    SkASSERT(data != NULL);
    SkASSERT(dst != NULL);
    SkImageGenerator* gen(SkNEW_ARGS(SkDecodingImageGenerator, (data)));
    return SkInstallDiscardablePixelRef(gen, dst, factory);
}

bool SkDecodingImageGenerator::Install(SkStreamRewindable* stream,
                                       SkBitmap* dst,
                                       SkDiscardableMemory::Factory* factory) {
    SkASSERT(stream != NULL);
    SkASSERT(dst != NULL);
    if ((stream == NULL) || !stream->unique()) {
        SkSafeUnref(stream);
        return false;
    }
    SkImageGenerator* gen(SkNEW_ARGS(SkDecodingImageGenerator, (stream)));
    return SkInstallDiscardablePixelRef(gen, dst, factory);
}
