/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"
#include "SkDecodingImageGenerator.h"
#include "SkImageDecoder.h"
#include "SkImageInfo.h"
#include "SkImageGenerator.h"
#include "SkImagePriv.h"
#include "SkStream.h"
#include "SkUtils.h"

namespace {
/**
 *  Special allocator used by getPixels(). Uses preallocated memory
 *  provided.
 */
class TargetAllocator : public SkBitmap::Allocator {
public:
    TargetAllocator(void* target,
                    size_t rowBytes,
                    int width,
                    int height,
                    SkBitmap::Config config)
        : fTarget(target)
        , fRowBytes(rowBytes)
        , fWidth(width)
        , fHeight(height)
        , fConfig(config) { }

    bool isReady() { return (fTarget != NULL); }

    virtual bool allocPixelRef(SkBitmap* bm, SkColorTable* ct) {
        if ((NULL == fTarget)
            || (fConfig != bm->config())
            || (fWidth != bm->width())
            || (fHeight != bm->height())
            || (ct != NULL)) {
            // Call default allocator.
            return bm->allocPixels(NULL, ct);
        }
        // make sure fRowBytes is correct.
        bm->setConfig(fConfig, fWidth, fHeight, fRowBytes, bm->alphaType());
        // TODO(halcanary): verify that all callers of this function
        // will respect new RowBytes.  Will be moot once rowbytes belongs
        // to PixelRef.
        bm->setPixels(fTarget, NULL);
        fTarget = NULL;  // never alloc same pixels twice!
        return true;
    }

private:
    void* fTarget;  // Block of memory to be supplied as pixel memory
                    // in allocPixelRef.  Must be large enough to hold
                    // a bitmap described by fWidth, fHeight, and
                    // fRowBytes.
    size_t fRowBytes;  // rowbytes for the destination bitmap
    int fWidth;   // Along with fHeight and fConfig, the information
    int fHeight;  // about the bitmap whose pixels this allocator is
                  // expected to allocate. If they do not match the
                  // bitmap passed to allocPixelRef, it is assumed
                  // that the bitmap will be copied to a bitmap with
                  // the correct info using this allocator, so the
                  // default allocator will be used instead of
                  // fTarget.
    SkBitmap::Config fConfig;
    typedef SkBitmap::Allocator INHERITED;
};

// TODO(halcanary): Give this macro a better name and move it into SkTypes.h
#ifdef SK_DEBUG
    #define SkCheckResult(expr, value)  SkASSERT((value) == (expr))
#else
    #define SkCheckResult(expr, value)  (void)(expr)
#endif

#ifdef SK_DEBUG
inline bool check_alpha(SkAlphaType reported, SkAlphaType actual) {
    return ((reported == actual)
            || ((reported == kPremul_SkAlphaType)
                && (actual == kOpaque_SkAlphaType)));
}
#endif  // SK_DEBUG

}  // namespace
////////////////////////////////////////////////////////////////////////////////

SkDecodingImageGenerator::SkDecodingImageGenerator(
        SkData* data,
        SkStreamRewindable* stream,
        const SkImageInfo& info,
        int sampleSize,
        bool ditherImage,
        SkBitmap::Config requestedConfig)
    : fData(data)
    , fStream(stream)
    , fInfo(info)
    , fSampleSize(sampleSize)
    , fDitherImage(ditherImage)
    , fRequestedConfig(requestedConfig) {
    SkASSERT(stream != NULL);
    SkSafeRef(fData);  // may be NULL.
}

SkDecodingImageGenerator::~SkDecodingImageGenerator() {
    SkSafeUnref(fData);
    fStream->unref();
}

bool SkDecodingImageGenerator::getInfo(SkImageInfo* info) {
    if (info != NULL) {
        *info = fInfo;
    }
    return true;
}

SkData* SkDecodingImageGenerator::refEncodedData() {
    // This functionality is used in `gm --serialize`
    // Does not encode options.
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

bool SkDecodingImageGenerator::getPixels(const SkImageInfo& info,
                                         void* pixels,
                                         size_t rowBytes) {
    if (NULL == pixels) {
        return false;
    }
    if (fInfo != info) {
        // The caller has specified a different info.  This is an
        // error for this kind of SkImageGenerator.  Use the Options
        // to change the settings.
        return false;
    }
    int bpp = SkBitmap::ComputeBytesPerPixel(fRequestedConfig);
    if (static_cast<size_t>(bpp * info.fWidth) > rowBytes) {
        // The caller has specified a bad rowBytes.
        return false;
    }

    SkAssertResult(fStream->rewind());
    SkAutoTDelete<SkImageDecoder> decoder(SkImageDecoder::Factory(fStream));
    if (NULL == decoder.get()) {
        return false;
    }
    decoder->setDitherImage(fDitherImage);
    decoder->setSampleSize(fSampleSize);

    SkBitmap bitmap;
    TargetAllocator allocator(pixels, rowBytes, info.fWidth,
                              info.fHeight, fRequestedConfig);
    decoder->setAllocator(&allocator);
    bool success = decoder->decode(fStream, &bitmap, fRequestedConfig,
                                   SkImageDecoder::kDecodePixels_Mode);
    decoder->setAllocator(NULL);
    if (!success) {
        return false;
    }
    if (allocator.isReady()) {  // Did not use pixels!
        SkBitmap bm;
        SkASSERT(bitmap.canCopyTo(fRequestedConfig));
        if (!bitmap.copyTo(&bm, fRequestedConfig, &allocator)
            || allocator.isReady()) {
            SkDEBUGFAIL("bitmap.copyTo(requestedConfig) failed.");
            // Earlier we checked canCopyto(); we expect consistency.
            return false;
        }
        SkASSERT(check_alpha(fInfo.fAlphaType, bm.alphaType()));
    } else {
        SkASSERT(check_alpha(fInfo.fAlphaType, bitmap.alphaType()));
    }
    return true;
}

SkImageGenerator* SkDecodingImageGenerator::Create(
        SkData* data,
        const SkDecodingImageGenerator::Options& opts) {
    SkASSERT(data != NULL);
    if (NULL == data) {
        return NULL;
    }
    SkStreamRewindable* stream = SkNEW_ARGS(SkMemoryStream, (data));
    SkASSERT(stream != NULL);
    SkASSERT(stream->unique());
    return SkDecodingImageGenerator::Create(data, stream, opts);
}

SkImageGenerator* SkDecodingImageGenerator::Create(
        SkStreamRewindable* stream,
        const SkDecodingImageGenerator::Options& opts) {
    SkASSERT(stream != NULL);
    SkASSERT(stream->unique());
    if ((stream == NULL) || !stream->unique()) {
        SkSafeUnref(stream);
        return NULL;
    }
    return SkDecodingImageGenerator::Create(NULL, stream, opts);
}

// A contructor-type function that returns NULL on failure.  This
// prevents the returned SkImageGenerator from ever being in a bad
// state.  Called by both Create() functions
SkImageGenerator* SkDecodingImageGenerator::Create(
        SkData* data,
        SkStreamRewindable* stream,
        const SkDecodingImageGenerator::Options& opts) {
    SkASSERT(stream);
    SkAutoTUnref<SkStreamRewindable> autoStream(stream);  // always unref this.
    if (opts.fUseRequestedColorType &&
        (kIndex_8_SkColorType == opts.fRequestedColorType)) {
        // We do not support indexed color with SkImageGenerators,
        return NULL;
    }
    SkAssertResult(autoStream->rewind());
    SkAutoTDelete<SkImageDecoder> decoder(SkImageDecoder::Factory(autoStream));
    if (NULL == decoder.get()) {
        return NULL;
    }
    SkBitmap bitmap;
    decoder->setSampleSize(opts.fSampleSize);
    if (!decoder->decode(stream, &bitmap,
                         SkImageDecoder::kDecodeBounds_Mode)) {
        return NULL;
    }
    if (bitmap.config() == SkBitmap::kNo_Config) {
        return NULL;
    }

    SkImageInfo info;
    SkBitmap::Config config;

    if (!opts.fUseRequestedColorType) {
        // Use default config.
        if (SkBitmap::kIndex8_Config == bitmap.config()) {
            // We don't support kIndex8 because we don't support
            // colortables in this workflow.
            config = SkBitmap::kARGB_8888_Config;
            info.fWidth = bitmap.width();
            info.fHeight = bitmap.height();
            info.fColorType = kPMColor_SkColorType;
            info.fAlphaType = bitmap.alphaType();
        } else {
            config = bitmap.config();  // Save for later!
            if (!bitmap.asImageInfo(&info)) {
                SkDEBUGFAIL("Getting SkImageInfo from bitmap failed.");
                return NULL;
            }
        }
    } else {
        config = SkColorTypeToBitmapConfig(opts.fRequestedColorType);
        if (!bitmap.canCopyTo(config)) {
            SkASSERT(bitmap.config() != config);
            return NULL;  // Can not translate to needed config.
        }
        info.fWidth = bitmap.width();
        info.fHeight = bitmap.height();
        info.fColorType = opts.fRequestedColorType;
        info.fAlphaType = bitmap.alphaType();
    }
    return SkNEW_ARGS(SkDecodingImageGenerator,
                      (data, autoStream.detach(), info,
                       opts.fSampleSize, opts.fDitherImage, config));
}
