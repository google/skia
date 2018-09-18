/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkArithmeticImageFilter.h"
#include "SkImage.h"
#include "SkImageSource.h"
#include "SkOffsetImageFilter.h"
#include "SkXfermodeImageFilter.h"

#define WIDTH 600
#define HEIGHT 700
#define MARGIN 12

namespace skiagm {

class XfermodeImageFilterGM : public GM {
public:
    XfermodeImageFilterGM(){
        this->setBGColor(0xFF000000);
    }

protected:
    SkString onShortName() override {
        return SkString("xfermodeimagefilter");
    }

    SkISize onISize() override {
        return SkISize::Make(WIDTH, HEIGHT);
    }

    void onOnceBeforeDraw() override {
        fBitmap = sk_tool_utils::create_string_bitmap(80, 80, 0xD000D000, 15, 65, 96, "e");

        fCheckerboard = SkImage::MakeFromBitmap(
            sk_tool_utils::create_checkerboard_bitmap(80, 80,
                                                      sk_tool_utils::color_to_565(0xFFA0A0A0),
                                                      sk_tool_utils::color_to_565(0xFF404040),
                                                      8));
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);
        SkPaint paint;

        const SkBlendMode gModes[] = {
            SkBlendMode::kClear,
            SkBlendMode::kSrc,
            SkBlendMode::kDst,
            SkBlendMode::kSrcOver,
            SkBlendMode::kDstOver,
            SkBlendMode::kSrcIn,
            SkBlendMode::kDstIn,
            SkBlendMode::kSrcOut,
            SkBlendMode::kDstOut,
            SkBlendMode::kSrcATop,
            SkBlendMode::kDstATop,
            SkBlendMode::kXor,

            SkBlendMode::kPlus,
            SkBlendMode::kModulate,
            SkBlendMode::kScreen,
            SkBlendMode::kOverlay,
            SkBlendMode::kDarken,
            SkBlendMode::kLighten,
            SkBlendMode::kColorDodge,
            SkBlendMode::kColorBurn,
            SkBlendMode::kHardLight,
            SkBlendMode::kSoftLight,
            SkBlendMode::kDifference,
            SkBlendMode::kExclusion,
            SkBlendMode::kMultiply,
            SkBlendMode::kHue,
            SkBlendMode::kSaturation,
            SkBlendMode::kColor,
            SkBlendMode::kLuminosity,
        };

        int x = 0, y = 0;
        sk_sp<SkImageFilter> background(SkImageSource::Make(fCheckerboard));
        for (size_t i = 0; i < SK_ARRAY_COUNT(gModes); i++) {
            paint.setImageFilter(SkXfermodeImageFilter::Make(gModes[i], background));
            DrawClippedBitmap(canvas, fBitmap, paint, x, y);
            x += fBitmap.width() + MARGIN;
            if (x + fBitmap.width() > WIDTH) {
                x = 0;
                y += fBitmap.height() + MARGIN;
            }
        }
        // Test arithmetic mode as image filter
        paint.setImageFilter(SkArithmeticImageFilter::Make(0, 1, 1, 0, true, background,
                                                           nullptr, nullptr));
        DrawClippedBitmap(canvas, fBitmap, paint, x, y);
        x += fBitmap.width() + MARGIN;
        if (x + fBitmap.width() > WIDTH) {
            x = 0;
            y += fBitmap.height() + MARGIN;
        }
        // Test nullptr mode
        paint.setImageFilter(SkXfermodeImageFilter::Make(SkBlendMode::kSrcOver, background));
        DrawClippedBitmap(canvas, fBitmap, paint, x, y);
        x += fBitmap.width() + MARGIN;
        if (x + fBitmap.width() > WIDTH) {
            x = 0;
            y += fBitmap.height() + MARGIN;
        }
        SkRect clipRect = SkRect::MakeWH(SkIntToScalar(fBitmap.width() + 4),
                                         SkIntToScalar(fBitmap.height() + 4));
        // Test offsets on SrcMode (uses fixed-function blend)
        sk_sp<SkImage> bitmapImage(SkImage::MakeFromBitmap(fBitmap));
        sk_sp<SkImageFilter> foreground(SkImageSource::Make(std::move(bitmapImage)));
        sk_sp<SkImageFilter> offsetForeground(SkOffsetImageFilter::Make(SkIntToScalar(4),
                                                                        SkIntToScalar(-4),
                                                                        foreground));
        sk_sp<SkImageFilter> offsetBackground(SkOffsetImageFilter::Make(SkIntToScalar(4),
                                                                        SkIntToScalar(4),
                                                                        background));
        paint.setImageFilter(SkXfermodeImageFilter::Make(SkBlendMode::kSrcOver,
                                                         offsetBackground,
                                                         offsetForeground,
                                                         nullptr));
        DrawClippedPaint(canvas, clipRect, paint, x, y);
        x += fBitmap.width() + MARGIN;
        if (x + fBitmap.width() > WIDTH) {
            x = 0;
            y += fBitmap.height() + MARGIN;
        }
        // Test offsets on Darken (uses shader blend)
        paint.setImageFilter(SkXfermodeImageFilter::Make(SkBlendMode::kDarken,
                                                         offsetBackground,
                                                         offsetForeground,
                                                         nullptr));
        DrawClippedPaint(canvas, clipRect, paint, x, y);
        x += fBitmap.width() + MARGIN;
        if (x + fBitmap.width() > WIDTH) {
            x = 0;
            y += fBitmap.height() + MARGIN;
        }
        // Test cropping
        constexpr size_t nbSamples = 3;
        const SkBlendMode sampledModes[nbSamples] = {
            SkBlendMode::kOverlay, SkBlendMode::kSrcOver, SkBlendMode::kPlus
        };
        int offsets[nbSamples][4] = {{ 10,  10, -16, -16},
                                     { 10,  10,  10,  10},
                                     {-10, -10,  -6,  -6}};
        for (size_t i = 0; i < nbSamples; ++i) {
            SkIRect cropRect = SkIRect::MakeXYWH(offsets[i][0],
                                                 offsets[i][1],
                                                 fBitmap.width()  + offsets[i][2],
                                                 fBitmap.height() + offsets[i][3]);
            SkImageFilter::CropRect rect(SkRect::Make(cropRect));
            paint.setImageFilter(SkXfermodeImageFilter::Make(sampledModes[i],
                                                             offsetBackground,
                                                             offsetForeground,
                                                             &rect));
            DrawClippedPaint(canvas, clipRect, paint, x, y);
            x += fBitmap.width() + MARGIN;
            if (x + fBitmap.width() > WIDTH) {
                x = 0;
                y += fBitmap.height() + MARGIN;
            }
        }
        // Test small bg, large fg with Screen (uses shader blend)
        SkBlendMode mode = SkBlendMode::kScreen;
        SkImageFilter::CropRect cropRect(SkRect::MakeXYWH(10, 10, 60, 60));
        sk_sp<SkImageFilter> cropped(SkOffsetImageFilter::Make(0, 0, foreground, &cropRect));
        paint.setImageFilter(SkXfermodeImageFilter::Make(mode, cropped, background, nullptr));
        DrawClippedPaint(canvas, clipRect, paint, x, y);
        x += fBitmap.width() + MARGIN;
        if (x + fBitmap.width() > WIDTH) {
            x = 0;
            y += fBitmap.height() + MARGIN;
        }
        // Test small fg, large bg with Screen (uses shader blend)
        paint.setImageFilter(SkXfermodeImageFilter::Make(mode, background, cropped, nullptr));
        DrawClippedPaint(canvas, clipRect, paint, x, y);
        x += fBitmap.width() + MARGIN;
        if (x + fBitmap.width() > WIDTH) {
            x = 0;
            y += fBitmap.height() + MARGIN;
        }
        // Test small fg, large bg with SrcIn with a crop that forces it to full size.
        // This tests that SkXfermodeImageFilter correctly applies the compositing mode to
        // the region outside the foreground.
        mode = SkBlendMode::kSrcIn;
        SkImageFilter::CropRect cropRectFull(SkRect::MakeXYWH(0, 0, 80, 80));
        paint.setImageFilter(SkXfermodeImageFilter::Make(mode, background,
                                                         cropped, &cropRectFull));
        DrawClippedPaint(canvas, clipRect, paint, x, y);
        x += fBitmap.width() + MARGIN;
        if (x + fBitmap.width() > WIDTH) {
            x = 0;
            y += fBitmap.height() + MARGIN;
        }
    }

private:
    static void DrawClippedBitmap(SkCanvas* canvas, const SkBitmap& bitmap, const SkPaint& paint,
                                  int x, int y) {
        canvas->save();
        canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
        canvas->clipRect(SkRect::MakeIWH(bitmap.width(), bitmap.height()));
        canvas->drawBitmap(bitmap, 0, 0, &paint);
        canvas->restore();
    }

    static void DrawClippedPaint(SkCanvas* canvas, const SkRect& rect, const SkPaint& paint,
                                 int x, int y) {
        canvas->save();
        canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
        canvas->clipRect(rect);
        canvas->drawPaint(paint);
        canvas->restore();
    }

    SkBitmap        fBitmap;
    sk_sp<SkImage>  fCheckerboard;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new XfermodeImageFilterGM; );

}
