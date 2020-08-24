/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

static constexpr int kImgW = 100;
static constexpr int kImgH =  80;

/**
  This function was used to create the images used by these test. It saves them as PNGs (so they
  are lossless). Then the following bash script was used to create the oriented JPGs with
  imagemagick:
#!/bin/bash

function originname()  {
  case $1 in
    1)
      echo -n "top-left"
      ;;
    2)
      echo -n "top-right"
      ;;
    3)
      echo -n "bottom-right"
      ;;
    4)
      echo -n "bottom-left"
      ;;
    5)
      echo -n "left-top"
      ;;
    6)
      echo -n "right-top"
      ;;
    7)
      echo -n "right-bottom"
      ;;
    8)
      echo -n "left-bottom"
      ;;
    *)
      echo -n "unknown"
      ;;
  esac
}

for s in 444 422 420 440 411 410; do
  for i in {1..8}; do
    magick convert $i.png -sampling-factor ${s:0:1}:${s:1:1}:${s:2:1} \
    -orient $(originname $i) $i\_$s.jpg;
  done
done
 */
static void make_images() {
    auto surf = SkSurface::MakeRaster(SkImageInfo::Make({kImgW, kImgH},
                                                        kRGBA_8888_SkColorType,
                                                        kPremul_SkAlphaType));
    for (int i = 1; i <= 8; ++i) {
        auto* canvas = surf->getCanvas();
        canvas->clear(SK_ColorBLACK);
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        SkScalar midX = kImgW / 2.f;
        SkScalar midY = kImgH / 2.f;
        SkScalar w = midX - 1;
        SkScalar h = midY - 1;
        canvas->drawRect(SkRect::MakeXYWH(1, 1, w, h), paint);
        paint.setColor(SK_ColorBLUE);
        canvas->drawRect(SkRect::MakeXYWH(midX, 1, w, h), paint);
        paint.setColor(SK_ColorGREEN);
        canvas->drawRect(SkRect::MakeXYWH(1, midY, w, h), paint);
        paint.setColor(SK_ColorYELLOW);
        canvas->drawRect(SkRect::MakeXYWH(midX, midY, w, h), paint);
        SkFont font(ToolUtils::create_portable_typeface(), kImgH / 4.f);

        SkPaint blurPaint;
        blurPaint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, .75f));
        blurPaint.setColor(SK_ColorBLACK);
        paint.setColor(SK_ColorWHITE);

        auto drawLabel = [&](const char* string, SkScalar x, SkScalar y) {
            canvas->save();
            canvas->translate(1, 1);
            canvas->drawString(string, x, y, font, blurPaint);
            canvas->restore();
            canvas->drawString(string, x, y, font, paint);
        };

        auto measure = [&font](const char* text) {
            SkRect bounds;
            font.measureText(text, strlen(text), SkTextEncoding::kUTF8, &bounds);
            return bounds;
        };

        static constexpr SkScalar kPad = 3.f;
        SkRect bounds;

        bounds = measure("top");
        drawLabel("top", midX - bounds.centerX(), -bounds.top() + kPad);

        bounds = measure("bottom");
        drawLabel("bottom", midX - bounds.centerX(), kImgH - kPad - bounds.bottom());

        // It looks weird if "left" and "right" and the number at the center aren't vertically
        // aligned.
        SkScalar baseY = midY - measure("leftright").centerY();
        bounds = measure("left");
        drawLabel("left", kPad - bounds.left(), baseY);

        bounds = measure("right");
        drawLabel("right", midX - kPad - bounds.right(), baseY);

        SkString num = SkStringPrintf("%d", i);
        bounds = measure(num.c_str());
        drawLabel(num.c_str(), midX - bounds.centerX(), baseY);
        num.append(".png");
        SkPixmap pm;
        surf->makeImageSnapshot()->peekPixels(&pm);
        ToolUtils::EncodeImageToFile(num.c_str(), pm, SkEncodedImageFormat::kPNG, 100);
    }
}

// This gm draws 8 images that are mostly the same when respecting the
// EXIF orientation tag. Each one has four quadrants (red, blue, green,
// yellow), and labels on the left, top, right and bottom. The only
// visual difference is a number in the middle corresponding to the
// EXIF tag for that image's jpg file.
static void draw(SkCanvas* canvas, const char* suffix) {
    // Avoid unused function warning.
    if (0) {
        make_images();
    }
    canvas->save();
    for (char i = '1'; i <= '8'; i++) {
        SkString path = SkStringPrintf("images/orientation/%c%s.jpg", i, suffix);
        auto image = GetResourceAsImage(path.c_str());
        if (!image) {
            continue;
        }
        canvas->drawImage(image, 0, 0);
        if ('4' == i) {
            canvas->restore();
            canvas->translate(0, image->height());
        } else {
            canvas->translate(image->width(), 0);
        }
    }
}

#define MAKE_GM(subsample) DEF_SIMPLE_GM(orientation_##subsample, canvas, 4*kImgW, 2*kImgH) { \
        draw(canvas, "_" #subsample);                                                         \
}

MAKE_GM(410)
MAKE_GM(411)
MAKE_GM(420)
MAKE_GM(422)
MAKE_GM(440)
MAKE_GM(444)
