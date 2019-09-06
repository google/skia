/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "tools/ToolUtils.h"

#include <string.h>

namespace {

constexpr SkColorType gColorTypes[] = {
    kRGB_565_SkColorType,
    kARGB_4444_SkColorType,
    kN32_SkColorType,
};

#define NUM_CONFIGS SK_ARRAY_COUNT(gColorTypes)

static void draw_checks(SkCanvas* canvas, int width, int height) {
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    canvas->drawRect(SkRect::MakeIWH(width/2, height/2), paint);
    paint.setColor(SK_ColorGREEN);
    canvas->drawRect({ SkIntToScalar(width/2), 0, SkIntToScalar(width), SkIntToScalar(height/2) },
                     paint);
    paint.setColor(SK_ColorBLUE);
    canvas->drawRect({ 0, SkIntToScalar(height/2), SkIntToScalar(width/2), SkIntToScalar(height) },
                     paint);
    paint.setColor(SK_ColorYELLOW);
    canvas->drawRect({ SkIntToScalar(width/2), SkIntToScalar(height/2), SkIntToScalar(width),
                     SkIntToScalar(height) }, paint);
}

class BitmapCopyGM : public skiagm::GM {
    SkBitmap    fDst[NUM_CONFIGS];

    void onOnceBeforeDraw() override { this->setBGColor(0xFFDDDDDD); }

    SkString onShortName() override { return SkString("bitmapcopy"); }

    SkISize onISize() override { return {540, 330}; }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        SkScalar horizMargin = 10;
        SkScalar vertMargin = 10;

        SkBitmap src;
        src.allocN32Pixels(40, 40, kOpaque_SkAlphaType);
        SkCanvas canvasTmp(src);

        draw_checks(&canvasTmp, 40, 40);

        for (unsigned i = 0; i < NUM_CONFIGS; ++i) {
            ToolUtils::copy_to(&fDst[i], gColorTypes[i], src);
        }

        canvas->clear(0xFFDDDDDD);
        paint.setAntiAlias(true);

        SkFont font(ToolUtils::create_portable_typeface());

        SkScalar width = SkIntToScalar(40);
        SkScalar height = SkIntToScalar(40);
        if (font.getSpacing() > height) {
            height = font.getSpacing();
        }
        for (unsigned i = 0; i < NUM_CONFIGS; i++) {
            const char* name = ToolUtils::colortype_name(src.colorType());
            SkScalar textWidth = font.measureText(name, strlen(name), SkTextEncoding::kUTF8);
            if (textWidth > width) {
                width = textWidth;
            }
        }
        SkScalar horizOffset = width + horizMargin;
        SkScalar vertOffset = height + vertMargin;
        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));

        for (unsigned i = 0; i < NUM_CONFIGS; i++) {
            canvas->save();
            // Draw destination config name
            const char* name = ToolUtils::colortype_name(fDst[i].colorType());
            SkScalar textWidth = font.measureText(name, strlen(name), SkTextEncoding::kUTF8);
            SkScalar x = (width - textWidth) / SkScalar(2);
            SkScalar y = font.getSpacing() / SkScalar(2);
            canvas->drawSimpleText(name, strlen(name), SkTextEncoding::kUTF8, x, y, font, paint);

            // Draw destination bitmap
            canvas->translate(0, vertOffset);
            x = (width - 40) / SkScalar(2);
            canvas->drawBitmap(fDst[i], x, 0, &paint);
            canvas->restore();

            canvas->translate(horizOffset, 0);
        }
    }
};
}  // namespace

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new BitmapCopyGM; )
