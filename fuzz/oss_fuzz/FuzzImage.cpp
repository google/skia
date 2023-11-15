/*
 * Copyright 2018 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"

bool FuzzImageDecode(const uint8_t *data, size_t size) {
    auto img = SkImages::DeferredFromEncodedData(SkData::MakeWithoutCopy(data, size));
    if (nullptr == img.get()) {
        return false;
    }

    auto s = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(128, 128));
    if (!s) {
        // May return nullptr in memory-constrained fuzzing environments
        return false;
    }

    s->getCanvas()->drawImage(img, 0, 0);
    return true;
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 10240) {
        return 0;
    }
    FuzzImageDecode(data, size);
    return 0;
}
#endif
