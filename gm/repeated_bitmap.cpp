/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkImage.h"
#include "gm.h"
#include "sk_tool_utils.h"

static void draw_rotated_image(SkCanvas* canvas, const SkImage* image) {
    sk_tool_utils::draw_checkerboard(canvas, SkColorSetRGB(156, 154, 156),
                                     SK_ColorWHITE, 12);
    if (!image) {
        return;
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
}

DEF_SIMPLE_GM(repeated_bitmap, canvas, 576, 576) {
    SkAutoTUnref<SkImage> image(GetResourceAsImage("randPixels.png"));
    draw_rotated_image(canvas, image);
}

DEF_SIMPLE_GM(repeated_bitmap_jpg, canvas, 576, 576) {
    SkAutoTUnref<SkImage> image(GetResourceAsImage("color_wheel.jpg"));
    draw_rotated_image(canvas, image);
}
