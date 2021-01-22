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

bool FuzzImageDecode(sk_sp<SkData> bytes) {
    auto img = SkImage::MakeFromEncoded(bytes);
    if (nullptr == img.get()) {
        return false;
    }

    auto s = SkSurface::MakeRasterN32Premul(128, 128);
    if (!s) {
        // May return nullptr in memory-constrained fuzzing environments
        return false;
    }

    s->getCanvas()->drawImage(img, 0, 0);
    return true;
}

// TODO(kjlubick): remove IS_FUZZING... after https://crrev.com/c/2410304 lands
#if defined(SK_BUILD_FOR_LIBFUZZER) || defined(IS_FUZZING_WITH_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 10240) {
        return 0;
    }
    auto bytes = SkData::MakeWithoutCopy(data, size);
    FuzzImageDecode(bytes);
    return 0;
}
#endif
