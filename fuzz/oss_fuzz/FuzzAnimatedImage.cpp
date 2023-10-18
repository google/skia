/*
 * Copyright 2018 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/android/SkAnimatedImage.h"
#include "include/codec/SkAndroidCodec.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"

bool FuzzAnimatedImage(const uint8_t *data, size_t size) {
    auto codec = SkAndroidCodec::MakeFromStream(SkMemoryStream::MakeDirect(data, size));
    if (nullptr == codec) {
        return false;
    }
    auto aImg = SkAnimatedImage::Make(std::move(codec));
    if (nullptr == aImg) {
        return false;
    }

    auto s = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(128, 128));
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

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 10240) {
        return 0;
    }
    FuzzAnimatedImage(data, size);
    return 0;
}
#endif
