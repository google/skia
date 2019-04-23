/*
 * Copyright 2018 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/android/SkAnimatedImage.h"
#include "include/codec/SkAndroidCodec.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkSurface.h"

bool FuzzAnimatedImage(sk_sp<SkData> bytes) {
    auto codec = SkAndroidCodec::MakeFromData(bytes);
    if (nullptr == codec) {
        return false;
    }
    auto aImg = SkAnimatedImage::Make(std::move(codec));
    if (nullptr == aImg) {
        return false;
    }

    auto s = SkSurface::MakeRasterN32Premul(128, 128);
    if (!s) {
        // May return nullptr in memory-constrained fuzzing environments
        return false;
    }

    int escape = 0;
    while (!aImg->isFinished() && escape < 100) {
        aImg->draw(s->getCanvas());
        escape++;
        aImg->decodeNextFrame();
    }
    return true;
}

#if defined(IS_FUZZING_WITH_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    auto bytes = SkData::MakeWithoutCopy(data, size);
    FuzzAnimatedImage(bytes);
    return 0;
}
#endif
