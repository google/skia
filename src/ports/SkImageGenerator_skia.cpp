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
    bool allocPixelRef(SkBitmap* bm, SkColorTable* ctable) SK_OVERRIDE {
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
        : fInfo(info), fDecoder(decoder), fData(SkRef(data))
    {}

protected:
    SkData* onRefEncodedData() SK_OVERRIDE {
        return SkRef(fData.get());
    }

    virtual bool onGetInfo(SkImageInfo* info) SK_OVERRIDE {
        *info = fInfo;
        return true;
    }

    virtual Result onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                               SkPMColor ctableEntries[], int* ctableCount) SK_OVERRIDE {
        SkMemoryStream stream(fData->data(), fData->size(), false);
        SkAutoTUnref<BareMemoryAllocator> allocator(SkNEW_ARGS(BareMemoryAllocator,
                                                               (info, pixels, rowBytes)));
        fDecoder->setAllocator(allocator);
        fDecoder->setRequireUnpremultipliedColors(kUnpremul_SkAlphaType == info.alphaType());

        SkBitmap bm;
        const SkImageDecoder::Result result = fDecoder->decode(&stream, &bm, info.colorType(),
                                                               SkImageDecoder::kDecodePixels_Mode);
        if (SkImageDecoder::kFailure == result) {
            return kInvalidInput;
        }

        SkASSERT(info.colorType() == bm.info().colorType());

        if (kIndex_8_SkColorType == info.colorType()) {
            SkASSERT(ctableEntries);

            SkColorTable* ctable = bm.getColorTable();
            if (NULL == ctable) {
                return kInvalidConversion;
            }
            const int count = ctable->count();
            memcpy(ctableEntries, ctable->readColors(), count * sizeof(SkPMColor));
            *ctableCount = count;
        }
        if (SkImageDecoder::kPartialSuccess == result) {
            return kIncompleteInput;
        }
        return kSuccess;
    }

    bool onGetYUV8Planes(SkISize sizes[3], void* planes[3], size_t rowBytes[3],
                         SkYUVColorSpace* colorSpace) SK_OVERRIDE {
        SkMemoryStream stream(fData->data(), fData->size(), false);
        return fDecoder->decodeYUV8Planes(&stream, sizes, planes, rowBytes, colorSpace);
    }
    
};

SkImageGenerator* SkImageGenerator::NewFromData(SkData* data) {
    if (NULL == data) {
        return NULL;
    }

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
