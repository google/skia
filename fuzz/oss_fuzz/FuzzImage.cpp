/*
 * Copyright 2018 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage.h"
#include "SkPaint.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkSurface.h"

void FuzzImage(sk_sp<SkData> bytes) {
    auto img = SkImage::MakeFromEncoded(bytes);
    if (nullptr == img.get()) {
        return;
    }

    auto s = SkSurface::MakeRasterN32Premul(128, 128);
    if (!s) {
        // May return nullptr in memory-constrained fuzzing environments
        return;
    }

    SkPaint p;
    s->getCanvas()->drawImage(img, 0, 0, &p);

}

#if defined(IS_FUZZING_WITH_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    auto bytes = SkData::MakeWithoutCopy(data, size);
    FuzzImage(bytes);
    return 0;
}
#endif
