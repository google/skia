/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#include <utility>

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
    SkString getName() const override { return SkString("xfermodeimagefilter"); }

    SkISize getISize() override { return SkISize::Make(WIDTH, HEIGHT); }

    void onOnceBeforeDraw() override {
        fBitmap = ToolUtils::CreateStringBitmap(80, 80, 0xD000D000, 15, 65, 96, "e");

        fCheckerboard = ToolUtils::create_checkerboard_image(80, 80, 0xFFA0A0A0, 0xFF404040, 8);
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
        sk_sp<SkImageFilter> background(SkImageFilters::Image(fCheckerboard,
                                                              SkFilterMode::kNearest));
        for (size_t i = 0; i < std::size(gModes); i++) {
            paint.setImageFilter(SkImageFilters::Blend(gModes[i], background));
            DrawClippedBitmap(canvas, fBitmap, paint, x, y);
            x += fBitmap.width() + MARGIN;
            if (x + fBitmap.width() > WIDTH) {
                x = 0;
                y += fBitmap.height() + MARGIN;
            }
        }
        // Test arithmetic mode as image filter
        paint.setImageFilter(SkImageFilters::Arithmetic(0, 1, 1, 0, true, background, nullptr));
        DrawClippedBitmap(canvas, fBitmap, paint, x, y);
        x += fBitmap.width() + MARGIN;
        if (x + fBitmap.width() > WIDTH) {
            x = 0;
            y += fBitmap.height() + MARGIN;
        }
        // Test nullptr mode
        paint.setImageFilter(SkImageFilters::Blend(SkBlendMode::kSrcOver, background));
        DrawClippedBitmap(canvas, fBitmap, paint, x, y);
        x += fBitmap.width() + MARGIN;
        if (x + fBitmap.width() > WIDTH) {
            x = 0;
            y += fBitmap.height() + MARGIN;
        }
        SkRect clipRect = SkRect::MakeWH(SkIntToScalar(fBitmap.width() + 4),
                                         SkIntToScalar(fBitmap.height() + 4));
        // Test offsets on SrcMode (uses fixed-function blend)
        sk_sp<SkImage> bitmapImage(fBitmap.asImage());
        sk_sp<SkImageFilter> foreground(SkImageFilters::Image(std::move(bitmapImage),
                                                              SkFilterMode::kNearest));
        sk_sp<SkImageFilter> offsetForeground(SkImageFilters::Offset(4, -4, foreground));
        sk_sp<SkImageFilter> offsetBackground(SkImageFilters::Offset(4, 4, background));
        paint.setImageFilter(SkImageFilters::Blend(
                SkBlendMode::kSrcOver, offsetBackground, offsetForeground));
        DrawClippedPaint(canvas, clipRect, paint, x, y);
        x += fBitmap.width() + MARGIN;
        if (x + fBitmap.width() > WIDTH) {
            x = 0;
            y += fBitmap.height() + MARGIN;
        }
        // Test offsets on Darken (uses shader blend)
        paint.setImageFilter(SkImageFilters::Blend(
                SkBlendMode::kDarken, offsetBackground, offsetForeground));
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
            paint.setImageFilter(SkImageFilters::Blend(sampledModes[i], offsetBackground,
                                                       offsetForeground, &cropRect));
            DrawClippedPaint(canvas, clipRect, paint, x, y);
            x += fBitmap.width() + MARGIN;
            if (x + fBitmap.width() > WIDTH) {
                x = 0;
                y += fBitmap.height() + MARGIN;
            }
        }
        // Test small bg, large fg with Screen (uses shader blend)
        SkIRect cropRect = SkIRect::MakeXYWH(10, 10, 60, 60);
        sk_sp<SkImageFilter> cropped(SkImageFilters::Offset(0, 0, foreground, &cropRect));
        paint.setImageFilter(SkImageFilters::Blend(SkBlendMode::kScreen, cropped, background));
        DrawClippedPaint(canvas, clipRect, paint, x, y);
        x += fBitmap.width() + MARGIN;
        if (x + fBitmap.width() > WIDTH) {
            x = 0;
            y += fBitmap.height() + MARGIN;
        }
        // Test small fg, large bg with Screen (uses shader blend)
        paint.setImageFilter(SkImageFilters::Blend(SkBlendMode::kScreen, background, cropped));
        DrawClippedPaint(canvas, clipRect, paint, x, y);
        x += fBitmap.width() + MARGIN;
        if (x + fBitmap.width() > WIDTH) {
            x = 0;
            y += fBitmap.height() + MARGIN;
        }
        // Test small fg, large bg with SrcIn with a crop that forces it to full size.
        // This tests that SkXfermodeImageFilter correctly applies the compositing mode to
        // the region outside the foreground.
        SkIRect cropRectFull = SkIRect::MakeXYWH(0, 0, 80, 80);
        paint.setImageFilter(SkImageFilters::Blend(SkBlendMode::kSrcIn, background, cropped,
                                                   &cropRectFull));
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
        canvas->clipIRect(bitmap.bounds());
        canvas->drawImage(bitmap.asImage(), 0, 0, SkSamplingOptions(), &paint);
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

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new XfermodeImageFilterGM; );

}  // namespace skiagm
