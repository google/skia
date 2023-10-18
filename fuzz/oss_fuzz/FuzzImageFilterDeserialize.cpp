/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "src/core/SkFontMgrPriv.h"
#include "tools/fonts/TestFontMgr.h"

void FuzzImageFilterDeserialize(const uint8_t *data, size_t size) {
    const int BitmapSize = 24;
    SkBitmap bitmap;
    bitmap.allocN32Pixels(BitmapSize, BitmapSize);
    SkCanvas canvas(bitmap);
    canvas.clear(0x00000000);

    auto flattenable = SkImageFilter::Deserialize(data, size);

    if (flattenable != nullptr) {
        // Let's see if using the filters can cause any trouble...
        SkPaint paint;
        paint.setImageFilter(flattenable);
        canvas.save();
        canvas.clipIRect(bitmap.bounds());

        // This call shouldn't crash or cause ASAN to flag any memory issues
        // If nothing bad happens within this call, everything is fine
        canvas.drawImage(bitmap.asImage(), 0, 0, SkSamplingOptions(), &paint);

        canvas.restore();
    }
}

// TODO(kjlubick): remove IS_FUZZING... after https://crrev.com/c/2410304 lands
#if defined(SK_BUILD_FOR_LIBFUZZER) || defined(IS_FUZZING_WITH_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 10024) {
        return 0;
    }
    gSkFontMgr_DefaultFactory = &ToolUtils::MakePortableFontMgr;
    FuzzImageFilterDeserialize(data, size);
    return 0;
}
#endif
