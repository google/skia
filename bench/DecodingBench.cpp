/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DecodingBench.h"
#include "SkBitmap.h"
#include "SkData.h"
#include "SkImageDecoder.h"
#include "SkMallocPixelRef.h"
#include "SkOSFile.h"
#include "SkStream.h"

/*
 *
 * This benchmark is designed to test the performance of image decoding.
 * It is invoked from the nanobench.cpp file.
 *
 */
DecodingBench::DecodingBench(SkString path, SkColorType colorType)
    : fColorType(colorType)
    , fData(SkData::NewFromFileName(path.c_str()))
{
    // Parse filename and the color type to give the benchmark a useful name
    SkString baseName = SkOSPath::Basename(path.c_str());
    const char* colorName;
    switch(colorType) {
        case kN32_SkColorType:
            colorName = "N32";
            break;
        case kRGB_565_SkColorType:
            colorName = "565";
            break;
        case kAlpha_8_SkColorType:
            colorName = "Alpha8";
            break;
        default:
            colorName = "Unknown";
    }
    fName.printf("Decode_%s_%s", baseName.c_str(), colorName);
    
#ifdef SK_DEBUG
    // Ensure that we can create a decoder.
    SkAutoTDelete<SkStreamRewindable> stream(new SkMemoryStream(fData));
    SkAutoTDelete<SkImageDecoder> decoder(SkImageDecoder::Factory(stream));
    SkASSERT(decoder != NULL);
#endif
}

const char* DecodingBench::onGetName() {
    return fName.c_str();
}

bool DecodingBench::isSuitableFor(Backend backend) {
    return kNonRendering_Backend == backend;
}

void DecodingBench::onPreDraw() {
    // Allocate the pixels now, to remove it from the loop.
    SkAutoTDelete<SkStreamRewindable> stream(new SkMemoryStream(fData));
    SkAutoTDelete<SkImageDecoder> decoder(SkImageDecoder::Factory(stream));
    SkBitmap bm;
#ifdef SK_DEBUG
    SkImageDecoder::Result result =
#endif
    decoder->decode(stream, &bm, fColorType, SkImageDecoder::kDecodeBounds_Mode);
    SkASSERT(SkImageDecoder::kFailure != result);

    const size_t rowBytes = bm.info().minRowBytes();
    fPixelStorage.reset(bm.info().getSafeSize(rowBytes));
}

// Allocator which just uses an existing block of memory.
class TargetAllocator : public SkBitmap::Allocator {
public:
    explicit TargetAllocator(void* storage)
        : fPixelStorage(storage) {}

    bool allocPixelRef(SkBitmap* bm, SkColorTable* ct) override {
        // We depend on the fact that this will only ever be used to
        // decode to a bitmap with the same settings used to create
        // fPixelStorage.
        bm->setPixelRef(SkMallocPixelRef::NewDirect(bm->info(),
                fPixelStorage, bm->rowBytes(), ct))->unref();
        return true;
    }

private:
    void* fPixelStorage; // Unowned. DecodingBench owns this.
};

void DecodingBench::onDraw(const int n, SkCanvas* canvas) {
    SkBitmap bitmap;
    // Declare the allocator before the decoder, so it will outlive the
    // decoder, which will unref it.
    TargetAllocator allocator(fPixelStorage.get());
    SkAutoTDelete<SkImageDecoder> decoder;
    SkAutoTDelete<SkStreamRewindable> stream;
    for (int i = 0; i < n; i++) {
        // create a new stream and a new decoder to mimic the behavior of
        // CodecBench.
        stream.reset(new SkMemoryStream(fData));
        decoder.reset(SkImageDecoder::Factory(stream));
        decoder->setAllocator(&allocator);
        decoder->decode(stream, &bitmap, fColorType,
                        SkImageDecoder::kDecodePixels_Mode);
    }
}
