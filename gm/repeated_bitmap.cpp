/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkImage.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

static skiagm::DrawResult draw_rotated_image(SkCanvas* canvas, const SkImage* image,
                                             SkString* errorMsg) {
    ToolUtils::draw_checkerboard(canvas, SkColorSetRGB(156, 154, 156), SK_ColorWHITE, 12);
    if (!image) {
        *errorMsg = "No image. Did you forget to set the resourcePath?";
        return skiagm::DrawResult::kFail;
    }
    SkRect rect = SkRect::MakeLTRB(-68.0f, -68.0f, 68.0f, 68.0f);
    SkPaint paint;
    paint.setColor(SkColorSetRGB(49, 48, 49));
    SkScalar scale = SkTMin(128.0f / image->width(),
                            128.0f / image->height());
    SkScalar point[2] = {-0.5f * image->width(), -0.5f * image->height()};
    for (int j = 0; j < 4; ++j) {
        for (int i = 0; i < 4; ++i) {
            SkAutoCanvasRestore autoCanvasRestore(canvas, true);
            canvas->translate(96.0f + 192.0f * i, 96.0f + 192.0f * j);
            canvas->rotate(18.0f * (i + 4 * j));
            canvas->drawRect(rect, paint);
            canvas->scale(scale, scale);
            canvas->drawImage(image, point[0], point[1]);
        }
    }
    return skiagm::DrawResult::kOk;
}

DEF_SIMPLE_GM_CAN_FAIL(repeated_bitmap, canvas, errorMsg, 576, 576) {
    return draw_rotated_image(canvas, GetResourceAsImage("images/randPixels.png").get(), errorMsg);
}

DEF_SIMPLE_GM_CAN_FAIL(repeated_bitmap_jpg, canvas, errorMsg, 576, 576) {
    return draw_rotated_image(canvas, GetResourceAsImage("images/color_wheel.jpg").get(), errorMsg);
}
