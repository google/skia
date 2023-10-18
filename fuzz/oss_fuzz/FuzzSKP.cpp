/*
 * Copyright 2020 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkPicture.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"

constexpr static SkISize kCanvasSize= {128, 160};

void FuzzSKP(const uint8_t *data, size_t size) {
    sk_sp<SkPicture> pic = SkPicture::MakeFromData(data, size);
    if (!pic) {
        return;
    }
    sk_sp<SkSurface> surface = SkSurfaces::Raster(
            SkImageInfo::MakeN32Premul(kCanvasSize.width(), kCanvasSize.height()));
    surface->getCanvas()->drawPicture(pic);
    pic->approximateBytesUsed();
    pic->approximateOpCount();
    return;
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    FuzzSKP(data, size);
    return 0;
}
#endif
