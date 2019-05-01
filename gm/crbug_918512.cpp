// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/effects/SkLumaColorFilter.h"

// PDF backend should produce correct results.
DEF_SIMPLE_GM(crbug_918512, canvas, 256, 256) {
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
