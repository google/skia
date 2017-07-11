/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "CodecBench.h"
#include "CodecBenchPriv.h"
#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkCommandLineFlags.h"
#include "SkOSFile.h"

// Actually zeroing the memory would throw off timing, so we just lie.
DEFINE_bool(zero_init, false, "Pretend our destination is zero-intialized, simulating Android?");

CodecBench::CodecBench(SkString baseName, SkData* encoded, SkColorType colorType,
        SkAlphaType alphaType)
    : fColorType(colorType)
    , fAlphaType(alphaType)
    , fData(SkRef(encoded))
{
    // Parse filename and the color type to give the benchmark a useful name
    fName.printf("Codec_%s_%s%s", baseName.c_str(), color_type_to_str(colorType),
            alpha_type_to_str(alphaType));
#ifdef SK_DEBUG
    // Ensure that we can create an SkCodec from this data.
    std::unique_ptr<SkCodec> codec(SkCodec::NewFromData(fData));
    SkASSERT(codec);
#endif
}

const char* CodecBench::onGetName() {
    return fName.c_str();
}

bool CodecBench::isSuitableFor(Backend backend) {
    return kNonRendering_Backend == backend;
}

void CodecBench::onDelayedSetup() {
    std::unique_ptr<SkCodec> codec(SkCodec::NewFromData(fData));

    fInfo = codec->getInfo().makeColorType(fColorType)
                            .makeAlphaType(fAlphaType)
                            .makeColorSpace(nullptr);

    fPixelStorage.reset(fInfo.getSafeSize(fInfo.minRowBytes()));
}

void CodecBench::onDraw(int n, SkCanvas* canvas) {
    std::unique_ptr<SkCodec> codec;
    SkCodec::Options options;
    if (FLAGS_zero_init) {
        options.fZeroInitialized = SkCodec::kYes_ZeroInitialized;
    }
    for (int i = 0; i < n; i++) {
        codec.reset(SkCodec::NewFromData(fData));
#ifdef SK_DEBUG
        const SkCodec::Result result =
#endif
        codec->getPixels(fInfo, fPixelStorage.get(), fInfo.minRowBytes(),
                         &options);
        SkASSERT(result == SkCodec::kSuccess
                 || result == SkCodec::kIncompleteInput);
    }
}
