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
bool equal_modulo_alpha(const SkImageInfo& a, const SkImageInfo& b) {
    return a.width() == b.width() && a.height() == b.height() &&
           a.colorType() == b.colorType();
}

class DecodingImageGenerator : public SkImageGenerator {
public:
    virtual ~DecodingImageGenerator();

    SkData*                fData;
    SkStreamRewindable*    fStream;
    const SkImageInfo      fInfo;
    const int              fSampleSize;
    const bool             fDitherImage;

    DecodingImageGenerator(SkData* data,
                           SkStreamRewindable* stream,
                           const SkImageInfo& info,
                           int sampleSize,
                           bool ditherImage);

protected:
    virtual SkData* onRefEncodedData() SK_OVERRIDE;
    virtual bool onGetInfo(SkImageInfo* info) SK_OVERRIDE {
        *info = fInfo;
        return true;
    }
    virtual bool onGetPixels(const SkImageInfo& info,
                             void* pixels, size_t rowBytes,
                             SkPMColor ctable[], int* ctableCount) SK_OVERRIDE;

private:
    typedef SkImageGenerator INHERITED;
};

/**
 *  Special allocator used by getPixels(). Uses preallocated memory
 *  provided if possible, else fall-back on the default allocator
 */
class TargetAllocator : public SkBitmap::Allocator {
public:
    TargetAllocator(const SkImageInfo& info,
                    void* target,
                    size_t rowBytes)
        : fInfo(info)
        , fTarget(target)
        , fRowBytes(rowBytes)
    {}

    bool isReady() { return (fTarget != NULL); }

    virtual bool allocPixelRef(SkBitmap* bm, SkColorTable* ct) {
        if (NULL == fTarget || !equal_modulo_alpha(fInfo, bm->info())) {
            // Call default allocator.
            return bm->allocPixels(NULL, ct);
        }

        // TODO(halcanary): verify that all callers of this function
        // will respect new RowBytes.  Will be moot once rowbytes belongs
        // to PixelRef.
        bm->installPixels(fInfo, fTarget, fRowBytes, ct, NULL, NULL);

        fTarget = NULL;  // never alloc same pixels twice!
        return true;
    }

private:
    const SkImageInfo fInfo;
    void* fTarget;  // Block of memory to be supplied as pixel memory
                    // in allocPixelRef.  Must be large enough to hold
                    // a bitmap described by fInfo and fRowBytes
    const size_t fRowBytes;  // rowbytes for the destination bitmap

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

////////////////////////////////////////////////////////////////////////////////

DecodingImageGenerator::DecodingImageGenerator(
        SkData* data,
        SkStreamRewindable* stream,
        const SkImageInfo& info,
        int sampleSize,
        bool ditherImage)
    : fData(data)
    , fStream(stream)
    , fInfo(info)
    , fSampleSize(sampleSize)
    , fDitherImage(ditherImage)
{
    SkASSERT(stream != NULL);
    SkSafeRef(fData);  // may be NULL.
}

DecodingImageGenerator::~DecodingImageGenerator() {
    SkSafeUnref(fData);
    fStream->unref();
}

SkData* DecodingImageGenerator::onRefEncodedData() {
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

bool DecodingImageGenerator::onGetPixels(const SkImageInfo& info,
                                         void* pixels, size_t rowBytes,
                                         SkPMColor ctableEntries[], int* ctableCount) {
    if (fInfo != info) {
        // The caller has specified a different info.  This is an
        // error for this kind of SkImageGenerator.  Use the Options
        // to change the settings.
        return false;
    }

    SkAssertResult(fStream->rewind());
    SkAutoTDelete<SkImageDecoder> decoder(SkImageDecoder::Factory(fStream));
    if (NULL == decoder.get()) {
        return false;
    }
    decoder->setDitherImage(fDitherImage);
    decoder->setSampleSize(fSampleSize);
    decoder->setRequireUnpremultipliedColors(
            info.fAlphaType == kUnpremul_SkAlphaType);

    SkBitmap bitmap;
    TargetAllocator allocator(fInfo, pixels, rowBytes);
    decoder->setAllocator(&allocator);
    bool success = decoder->decode(fStream, &bitmap, info.colorType(),
                                   SkImageDecoder::kDecodePixels_Mode);
    decoder->setAllocator(NULL);
    if (!success) {
        return false;
    }
    if (allocator.isReady()) {  // Did not use pixels!
        SkBitmap bm;
        SkASSERT(bitmap.canCopyTo(info.colorType()));
        bool copySuccess = bitmap.copyTo(&bm, info.colorType(), &allocator);
        if (!copySuccess || allocator.isReady()) {
            SkDEBUGFAIL("bitmap.copyTo(requestedConfig) failed.");
            // Earlier we checked canCopyto(); we expect consistency.
            return false;
        }
        SkASSERT(check_alpha(info.alphaType(), bm.alphaType()));
    } else {
        SkASSERT(check_alpha(info.alphaType(), bitmap.alphaType()));
    }

    if (kIndex_8_SkColorType == info.colorType()) {
        if (kIndex_8_SkColorType != bitmap.colorType()) {
            return false;   // they asked for Index8, but we didn't receive that from decoder
        }
        SkColorTable* ctable = bitmap.getColorTable();
        if (NULL == ctable) {
            return false;
        }
        const int count = ctable->count();
        memcpy(ctableEntries, ctable->lockColors(), count * sizeof(SkPMColor));
        ctable->unlockColors();
        *ctableCount = count;
    }
    return true;
}

// A contructor-type function that returns NULL on failure.  This
// prevents the returned SkImageGenerator from ever being in a bad
// state.  Called by both Create() functions
SkImageGenerator* CreateDecodingImageGenerator(
        SkData* data,
        SkStreamRewindable* stream,
        const SkDecodingImageGenerator::Options& opts) {
    SkASSERT(stream);
    SkAutoTUnref<SkStreamRewindable> autoStream(stream);  // always unref this.
    SkAssertResult(autoStream->rewind());
    SkAutoTDelete<SkImageDecoder> decoder(SkImageDecoder::Factory(autoStream));
    if (NULL == decoder.get()) {
        return NULL;
    }
    SkBitmap bitmap;
    decoder->setSampleSize(opts.fSampleSize);
    decoder->setRequireUnpremultipliedColors(opts.fRequireUnpremul);
    if (!decoder->decode(stream, &bitmap, SkImageDecoder::kDecodeBounds_Mode)) {
        return NULL;
    }
    if (kUnknown_SkColorType == bitmap.colorType()) {
        return NULL;
    }

    SkImageInfo info = bitmap.info();

    if (opts.fUseRequestedColorType && (opts.fRequestedColorType != info.colorType())) {
        if (!bitmap.canCopyTo(opts.fRequestedColorType)) {
            SkASSERT(bitmap.colorType() != opts.fRequestedColorType);
            return NULL;  // Can not translate to needed config.
        }
        info.fColorType = opts.fRequestedColorType;
    }

    if (opts.fRequireUnpremul && info.fAlphaType != kOpaque_SkAlphaType) {
        info.fAlphaType = kUnpremul_SkAlphaType;
    }

    if (!SkColorTypeValidateAlphaType(info.fColorType, info.fAlphaType, &info.fAlphaType)) {
        return NULL;
    }

    return SkNEW_ARGS(DecodingImageGenerator,
                      (data, autoStream.detach(), info,
                       opts.fSampleSize, opts.fDitherImage));
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////

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
    return CreateDecodingImageGenerator(data, stream, opts);
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
    return CreateDecodingImageGenerator(NULL, stream, opts);
}
