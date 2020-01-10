// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(crbug_918512, 256, 256, false, 0) {
// https://crbug.com/918512
// Verify that PDF draws correctly.
void draw(SkCanvas* canvas) {
    canvas->drawColor(SK_ColorYELLOW);
    {
        SkAutoCanvasRestore autoCanvasRestore1(canvas, false);
        canvas->saveLayer(nullptr, nullptr);
        canvas->drawColor(SK_ColorCYAN);
        {
            SkAutoCanvasRestore autoCanvasRestore2(canvas, false);
            SkPaint lumaFilter;
            lumaFilter.setBlendMode(SkBlendMode::kDstIn);
            lumaFilter.setColorFilter(SkLumaColorFilter::Make());
            canvas->saveLayer(nullptr, &lumaFilter);

            canvas->drawColor(SK_ColorTRANSPARENT);
            SkPaint paint;
            paint.setColor(SK_ColorGRAY);
            canvas->drawRect(SkRect{0, 0, 128, 256}, paint);
        }
    }
}
}  // END FIDDLE
