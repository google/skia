/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkData.h"
#include "SkImage.h"
#include "gm.h"
#include "sk_tool_utils.h"

static void draw_image(SkCanvas* canvas, const char* resource, int x, int y) {
    sk_sp<SkImage> image(GetResourceAsImage(resource));
    if (image) {
        canvas->drawImage(image, SkIntToScalar(x), SkIntToScalar(y));
    } else {
        SkDebugf("\nCould not decode file '%s'. Did you forget"
                 " to set the resourcePath?\n", resource);
    }
}

/*
  This GM tests whether the image decoders properly decode each color
  channel.  Four copies of the same image should appear in the GM, and
  the letter R should be red, B should be blue, G green, C cyan, M
  magenta, Y yellow, and K black. In all but the JPEG version of the
  image, the letters should be on a white disc on a transparent
  background (rendered as a checkerboard).  The JPEG image has a grey
  background and compression artifacts.
 */
DEF_SIMPLE_GM(colorwheel, canvas, 256, 256) {
    sk_tool_utils::draw_checkerboard(canvas);
    draw_image(canvas, "color_wheel.png", 0, 0);  // top left
    draw_image(canvas, "color_wheel.gif", 128, 0);  // top right
    draw_image(canvas, "color_wheel.webp", 0, 128);  // bottom left
    draw_image(canvas, "color_wheel.jpg", 128, 128);  // bottom right
}

DEF_SIMPLE_GM(colorwheelnative, canvas, 128, 28) {
    SkPaint paint;
    sk_tool_utils::set_portable_typeface(&paint, "sans-serif",
                                         SkFontStyle::FromOldStyle(SkTypeface::kBold));
    paint.setTextSize(18.0f);

    canvas->clear(sk_tool_utils::color_to_565(SK_ColorLTGRAY));
    paint.setColor(SK_ColorRED);
    canvas->drawText("R", 1, 8.0f, 20.0f, paint);
    paint.setColor(SK_ColorGREEN);
    canvas->drawText("G", 1, 24.0f, 20.0f, paint);
    paint.setColor(SK_ColorBLUE);
    canvas->drawText("B", 1, 40.0f, 20.0f, paint);
    paint.setColor(SK_ColorCYAN);
    canvas->drawText("C", 1, 56.0f, 20.0f, paint);
    paint.setColor(SK_ColorMAGENTA);
    canvas->drawText("M", 1, 72.0f, 20.0f, paint);
    paint.setColor(SK_ColorYELLOW);
    canvas->drawText("Y", 1, 88.0f, 20.0f, paint);
    paint.setColor(SK_ColorBLACK);
    canvas->drawText("K", 1, 104.0f, 20.0f, paint);
}
