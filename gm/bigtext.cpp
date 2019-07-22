/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "tools/ToolUtils.h"

/**
 *  Skia may draw from outlines when the size is very large, so we exercise that
 *  here.
*/
static constexpr int kWidth  = 640;
static constexpr int kHeight = 480;

DEF_SIMPLE_GM(bigtext, canvas, kWidth, kHeight)  {
        SkPaint paint;
        paint.setAntiAlias(true);
        SkFont font(ToolUtils::create_portable_typeface(), 1500);

        SkRect r;
        (void)font.measureText("/", 1, SkTextEncoding::kUTF8, &r);
        SkPoint pos = {
            0.5f * kWidth  - r.centerX(),
            0.5f * kHeight - r.centerY()
        };

        paint.setColor(SK_ColorRED);
        canvas->drawSimpleText("/", 1, SkTextEncoding::kUTF8, pos.fX, pos.fY, font, paint);

        paint.setColor(SK_ColorBLUE);
        canvas->drawSimpleText("\\", 1, SkTextEncoding::kUTF8, pos.fX, pos.fY, font, paint);
}
