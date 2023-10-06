/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "tools/DecodeUtils.h"
#include "tools/GpuToolUtils.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

static void draw_image(SkCanvas* canvas, const char* resource, int x, int y) {
    sk_sp<SkImage> image(ToolUtils::GetResourceAsImage(resource));
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
    ToolUtils::draw_checkerboard(canvas);
    draw_image(canvas, "images/color_wheel.png", 0, 0);  // top left
    draw_image(canvas, "images/color_wheel.gif", 128, 0);  // top right
    draw_image(canvas, "images/color_wheel.webp", 0, 128);  // bottom left
    draw_image(canvas, "images/color_wheel.jpg", 128, 128);  // bottom right
}

DEF_SIMPLE_GM(colorwheelnative, canvas, 128, 28) {
    SkFont font(ToolUtils::create_portable_typeface("sans-serif", SkFontStyle::Bold()), 18);
    font.setEdging(SkFont::Edging::kAlias);

    canvas->clear(SK_ColorLTGRAY);
    canvas->drawString("R", 8.0f,   20.0f, font, SkPaint(SkColors::kRed));
    canvas->drawString("G", 24.0f,  20.0f, font, SkPaint(SkColors::kGreen));
    canvas->drawString("B", 40.0f,  20.0f, font, SkPaint(SkColors::kBlue));
    canvas->drawString("C", 56.0f,  20.0f, font, SkPaint(SkColors::kCyan));
    canvas->drawString("M", 72.0f,  20.0f, font, SkPaint(SkColors::kMagenta));
    canvas->drawString("Y", 88.0f,  20.0f, font, SkPaint(SkColors::kYellow));
    canvas->drawString("K", 104.0f, 20.0f, font, SkPaint(SkColors::kBlack));
}

/**
 * This GM tests decoding images with non-default (overridden) alpha types.
 */
DEF_SIMPLE_GM(colorwheel_alphatypes, canvas, 256, 128) {
    canvas->clear(SK_ColorWHITE);

    sk_sp<SkData> imgData = GetResourceAsData("images/color_wheel.png");

    auto pmImg = ToolUtils::MakeTextureImage(
            canvas, SkImages::DeferredFromEncodedData(imgData, kPremul_SkAlphaType));
    auto upmImg = ToolUtils::MakeTextureImage(
            canvas, SkImages::DeferredFromEncodedData(imgData, kUnpremul_SkAlphaType));

    SkSamplingOptions linear{SkFilterMode::kLinear};

    // We draw a tiny (8x8) section of the image that falls right on the edge of transparency,
    // and blow it up so we can really see the impact of filtering in premul or unpremul.
    SkRect srcRect = SkRect::MakeXYWH(12, 102, 8, 8);
    SkRect dstRect = SkRect::MakeLTRB(0, 0, 128, 128);

    // First, we draw the normal (premul-then-filter) image, which looks good. The yellow image
    // transitions to the white background like you'd expect.
    canvas->drawImageRect(pmImg, srcRect, dstRect,
                          linear, nullptr, SkCanvas::kFast_SrcRectConstraint);
    // Next, we draw the unpremul (filter-then-premul) image, which looks bad. Filtering in unpremul
    // causes the implicit black of the transparent pixels to be bleed into the filtered edge,
    // creating a dark "fringe" at the boundary between the yellow image and white background.
    canvas->drawImageRect(upmImg, srcRect, dstRect.makeOffset(128, 0),
                          linear, nullptr, SkCanvas::kFast_SrcRectConstraint);
}
