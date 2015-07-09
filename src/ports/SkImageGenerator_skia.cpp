/*
 * Copyright 2015 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"
#include "SkImageDecoder.h"
#include "SkImageGenerator.h"
#include "SkStream.h"

class BareMemoryAllocator : public SkBitmap::Allocator {
    const SkImageInfo   fInfo;
    void* const         fMemory;
    const size_t        fRowBytes;

public:
    BareMemoryAllocator(const SkImageInfo& info, void* memory, size_t rowBytes)
        : fInfo(info), fMemory(memory), fRowBytes(rowBytes)
    {}

protected:
    bool allocPixelRef(SkBitmap* bm, SkColorTable* ctable) override {
        const SkImageInfo bmi = bm->info();
        if (bmi.width() != fInfo.width() || bmi.height() != fInfo.height() ||
            bmi.colorType() != fInfo.colorType())
        {
            return false;
        }
        return bm->installPixels(bmi, fMemory, fRowBytes, ctable, NULL, NULL);
    }
};

class SkImageDecoderGenerator : public SkImageGenerator {
    const SkImageInfo               fInfo;
    SkAutoTDelete<SkImageDecoder>   fDecoder;
    SkAutoTUnref<SkData>            fData;

public:
    SkImageDecoderGenerator(const SkImageInfo& info, SkImageDecoder* decoder, SkData* data)
        : INHERITED(info), fInfo(info), fDecoder(decoder), fData(SkRef(data))
    {}

protected:
    SkData* onRefEncodedData() override {
        return SkRef(fData.get());
    }
#ifdef SK_LEGACY_IMAGE_GENERATOR_ENUMS_AND_OPTIONS
    Result onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                       const Options&,
                       SkPMColor ctableEntries[], int* ctableCount) override {
#else
    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                     SkPMColor ctableEntries[], int* ctableCount) override {
#endif
        SkMemoryStream stream(fData->data(), fData->size(), false);
        SkAutoTUnref<BareMemoryAllocator> allocator(SkNEW_ARGS(BareMemoryAllocator,
                                                               (info, pixels, rowBytes)));
        fDecoder->setAllocator(allocator);
        fDecoder->setRequireUnpremultipliedColors(kUnpremul_SkAlphaType == info.alphaType());

        SkBitmap bm;
        const SkImageDecoder::Result result = fDecoder->decode(&stream, &bm, info.colorType(),
                                                               SkImageDecoder::kDecodePixels_Mode);
        if (SkImageDecoder::kFailure == result) {
#ifdef SK_LEGACY_IMAGE_GENERATOR_ENUMS_AND_OPTIONS
            return kInvalidInput;
#else
            return false;
#endif
        }

        SkASSERT(info.colorType() == bm.info().colorType());

        if (kIndex_8_SkColorType == info.colorType()) {
            SkASSERT(ctableEntries);

            SkColorTable* ctable = bm.getColorTable();
            if (NULL == ctable) {
#ifdef SK_LEGACY_IMAGE_GENERATOR_ENUMS_AND_OPTIONS
                return kInvalidConversion;
#else
                return false;
#endif
            }
            const int count = ctable->count();
            memcpy(ctableEntries, ctable->readColors(), count * sizeof(SkPMColor));
            *ctableCount = count;
        }
#ifdef SK_LEGACY_IMAGE_GENERATOR_ENUMS_AND_OPTIONS
        if (SkImageDecoder::kPartialSuccess == result) {
            return kIncompleteInput;
        }
        return kSuccess;
#else
        return true;
#endif
    }

    bool onGetYUV8Planes(SkISize sizes[3], void* planes[3], size_t rowBytes[3],
                         SkYUVColorSpace* colorSpace) override {
        SkMemoryStream stream(fData->data(), fData->size(), false);
        return fDecoder->decodeYUV8Planes(&stream, sizes, planes, rowBytes, colorSpace);
    }

private:
    typedef SkImageGenerator INHERITED;
};

SkImageGenerator* SkImageGenerator::NewFromEncodedImpl(SkData* data) {
    SkMemoryStream stream(data->data(), data->size(), false);
    SkImageDecoder* decoder = SkImageDecoder::Factory(&stream);
    if (NULL == decoder) {
        return NULL;
    }

    SkBitmap bm;
    stream.rewind();
    if (!decoder->decode(&stream, &bm, kUnknown_SkColorType, SkImageDecoder::kDecodeBounds_Mode)) {
        SkDELETE(decoder);
        return NULL;
    }

    return SkNEW_ARGS(SkImageDecoderGenerator, (bm.info(), decoder, data));
}
