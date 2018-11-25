/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "sk_tool_utils.h"

namespace skiagm {

static const char* color_type_name(SkColorType colorType) {
    switch (colorType) {
        case kUnknown_SkColorType:      return "unknown";
        case kAlpha_8_SkColorType:      return "A8";
        case kRGB_565_SkColorType:      return "565";
        case kARGB_4444_SkColorType:    return "4444";
        case kRGBA_8888_SkColorType:    return "8888";
        case kRGB_888x_SkColorType:     return "888x";
        case kBGRA_8888_SkColorType:    return "8888";
        case kRGBA_1010102_SkColorType: return "1010102";
        case kRGB_101010x_SkColorType:  return "101010x";
        case kGray_8_SkColorType:       return "G8";
        case kRGBA_F16_SkColorType:     return "F16";
    }
    return "";
}

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

class BitmapCopyGM : public GM {
public:
    SkBitmap    fDst[NUM_CONFIGS];

    BitmapCopyGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFDDDDDD));
    }

protected:
    virtual SkString onShortName() {
        return SkString("bitmapcopy");
    }

    virtual SkISize onISize() {
        return SkISize::Make(540, 330);
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkPaint paint;
        SkScalar horizMargin = 10;
        SkScalar vertMargin = 10;

        SkBitmap src;
        src.allocN32Pixels(40, 40, kOpaque_SkAlphaType);
        SkCanvas canvasTmp(src);

        draw_checks(&canvasTmp, 40, 40);

        for (unsigned i = 0; i < NUM_CONFIGS; ++i) {
            sk_tool_utils::copy_to(&fDst[i], gColorTypes[i], src);
        }

        canvas->clear(sk_tool_utils::color_to_565(0xFFDDDDDD));
        paint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&paint);

        SkScalar width = SkIntToScalar(40);
        SkScalar height = SkIntToScalar(40);
        if (paint.getFontSpacing() > height) {
            height = paint.getFontSpacing();
        }
        for (unsigned i = 0; i < NUM_CONFIGS; i++) {
            const char* name = color_type_name(src.colorType());
            SkScalar textWidth = paint.measureText(name, strlen(name));
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
            const char* name = color_type_name(fDst[i].colorType());
            SkScalar textWidth = paint.measureText(name, strlen(name));
            SkScalar x = (width - textWidth) / SkScalar(2);
            SkScalar y = paint.getFontSpacing() / SkScalar(2);
            canvas->drawString(name, x, y, paint);

            // Draw destination bitmap
            canvas->translate(0, vertOffset);
            x = (width - 40) / SkScalar(2);
            canvas->drawBitmap(fDst[i], x, 0, &paint);
            canvas->restore();

            canvas->translate(horizOffset, 0);
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new BitmapCopyGM; }
static GMRegistry reg(MyFactory);
}
