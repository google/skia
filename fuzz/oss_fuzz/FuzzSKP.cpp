/*
 * Copyright 2020 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkPicture.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"

constexpr static SkISize kCanvasSize= {128, 160};

void FuzzSKP(sk_sp<SkData> bytes) {
    sk_sp<SkPicture> pic = SkPicture::MakeFromData(bytes->data(), bytes->size());
    if (!pic) {
        SkDebugf("[terminated] Couldn't decode as a picture.\n");
        return;
    }
    sk_sp<SkSurface> surface = SkSurface::MakeRasterN32Premul(kCanvasSize.width(),
                                                              kCanvasSize.height());
    surface->getCanvas()->drawPicture(pic);
    pic->approximateBytesUsed();
    pic->approximateOpCount();
    return;
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    auto bytes = SkData::MakeWithoutCopy(data, size);
    FuzzSKP(bytes);
    return 0;
}
#endif
