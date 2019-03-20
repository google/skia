/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkFontMgrPriv.h"
#include "SkImageFilter.h"
#include "SkPaint.h"
#include "TestFontMgr.h"

void FuzzImageFilterDeserialize(sk_sp<SkData> bytes) {
    const int BitmapSize = 24;
    SkBitmap bitmap;
    bitmap.allocN32Pixels(BitmapSize, BitmapSize);
    SkCanvas canvas(bitmap);
    canvas.clear(0x00000000);

    auto flattenable = SkImageFilter::Deserialize(bytes->data(), bytes->size());

    if (flattenable != nullptr) {
        // Let's see if using the filters can cause any trouble...
        SkPaint paint;
        paint.setImageFilter(flattenable);
        canvas.save();
        canvas.clipRect(SkRect::MakeXYWH(
            0, 0, SkIntToScalar(BitmapSize), SkIntToScalar(BitmapSize)));

        // This call shouldn't crash or cause ASAN to flag any memory issues
        // If nothing bad happens within this call, everything is fine
        canvas.drawBitmap(bitmap, 0, 0, &paint);

        canvas.restore();
    }
}

#if defined(IS_FUZZING_WITH_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    gSkFontMgr_DefaultFactory = &sk_tool_utils::MakePortableFontMgr;
    auto bytes = SkData::MakeWithoutCopy(data, size);
    FuzzImageFilterDeserialize(bytes);
    return 0;
}
#endif
