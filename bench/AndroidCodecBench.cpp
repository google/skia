/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "AndroidCodecBench.h"
#include "CodecBenchPriv.h"
#include "SkBitmap.h"
#include "SkAndroidCodec.h"
#include "SkCommandLineFlags.h"
#include "SkOSFile.h"

AndroidCodecBench::AndroidCodecBench(SkString baseName, SkData* encoded, int sampleSize)
    : fData(SkRef(encoded))
    , fSampleSize(sampleSize)
{
    // Parse filename and the color type to give the benchmark a useful name
    fName.printf("AndroidCodec_%s_SampleSize%d", baseName.c_str(), sampleSize);
}

const char* AndroidCodecBench::onGetName() {
    return fName.c_str();
}

bool AndroidCodecBench::isSuitableFor(Backend backend) {
    return kNonRendering_Backend == backend;
}

void AndroidCodecBench::onDelayedSetup() {
    std::unique_ptr<SkAndroidCodec> codec(SkAndroidCodec::NewFromData(fData));
    SkISize scaledSize = codec->getSampledDimensions(fSampleSize);

    fInfo = codec->getInfo().makeWH(scaledSize.width(), scaledSize.height())
            .makeColorType(kN32_SkColorType);
    if (kUnpremul_SkAlphaType == fInfo.alphaType()) {
        fInfo = fInfo.makeAlphaType(kPremul_SkAlphaType);
    }

    fPixelStorage.reset(fInfo.getSafeSize(fInfo.minRowBytes()));
}

void AndroidCodecBench::onDraw(int n, SkCanvas* canvas) {
    std::unique_ptr<SkAndroidCodec> codec;
    SkAndroidCodec::AndroidOptions options;
    options.fSampleSize = fSampleSize;
    for (int i = 0; i < n; i++) {
        codec.reset(SkAndroidCodec::NewFromData(fData));
#ifdef SK_DEBUG
        const SkCodec::Result result =
#endif
        codec->getAndroidPixels(fInfo, fPixelStorage.get(), fInfo.minRowBytes(), &options);
        SkASSERT(result == SkCodec::kSuccess || result == SkCodec::kIncompleteInput);
    }
}
