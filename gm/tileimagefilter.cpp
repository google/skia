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
#include "include/core/SkColorFilter.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/effects/SkImageFilters.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#include <stddef.h>
#include <utility>

#define WIDTH 400
#define HEIGHT 200
#define MARGIN 12

namespace skiagm {

class TileImageFilterGM : public GM {
public:
    TileImageFilterGM() {
        this->setBGColor(0xFF000000);
    }

protected:
    SkString getName() const override { return SkString("tileimagefilter"); }

    SkISize getISize() override { return SkISize::Make(WIDTH, HEIGHT); }

    void onOnceBeforeDraw() override {
        fBitmap = ToolUtils::CreateStringImage(50, 50, 0xD000D000, 10, 45, 50, "e");

        fCheckerboard = ToolUtils::create_checkerboard_image(80, 80, 0xFFA0A0A0, 0xFF404040, 8);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);
        SkPaint red;
        red.setColor(SK_ColorRED);
        red.setStyle(SkPaint::kStroke_Style);
        SkPaint blue;
        blue.setColor(SK_ColorBLUE);
        blue.setStyle(SkPaint::kStroke_Style);

        int x = 0, y = 0;

        {
            for (size_t i = 0; i < 4; i++) {
                sk_sp<SkImage> image = (i & 0x01) ? fCheckerboard : fBitmap;
                SkRect srcRect = SkRect::MakeXYWH(SkIntToScalar(image->width()/4),
                                                  SkIntToScalar(image->height()/4),
                                                  SkIntToScalar(image->width()/(i+1)),
                                                  SkIntToScalar(image->height()/(i+1)));
                SkRect dstRect = SkRect::MakeXYWH(SkIntToScalar(i * 8),
                                                  SkIntToScalar(i * 4),
                                                  SkIntToScalar(image->width() - i * 12),
                                                  SkIntToScalar(image->height()) - i * 12);
                sk_sp<SkImageFilter> tileInput(SkImageFilters::Image(image, SkFilterMode::kLinear));
                sk_sp<SkImageFilter> filter(SkImageFilters::Tile(srcRect, dstRect,
                                                                 std::move(tileInput)));
                canvas->save();
                canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
                SkPaint paint;
                paint.setImageFilter(std::move(filter));
                canvas->drawImage(fBitmap.get(), 0, 0, SkSamplingOptions(), &paint);
                canvas->drawRect(srcRect, red);
                canvas->drawRect(dstRect, blue);
                canvas->restore();
                x += image->width() + MARGIN;
                if (x + image->width() > WIDTH) {
                    x = 0;
                    y += image->height() + MARGIN;
                }
            }
        }

        {
            float matrix[20] = { 1, 0, 0, 0, 0,
                                 0, 1, 0, 0, 0,
                                 0, 0, 1, 0, 0,
                                 0, 0, 0, 1, 0 };

            SkRect srcRect = SkRect::MakeWH(SkIntToScalar(fBitmap->width()),
                                            SkIntToScalar(fBitmap->height()));
            SkRect dstRect = SkRect::MakeWH(SkIntToScalar(fBitmap->width() * 2),
                                            SkIntToScalar(fBitmap->height() * 2));
            sk_sp<SkImageFilter> tile(SkImageFilters::Tile(srcRect, dstRect, nullptr));
            sk_sp<SkColorFilter> cf(SkColorFilters::Matrix(matrix));

            SkPaint paint;
            paint.setImageFilter(SkImageFilters::ColorFilter(std::move(cf), std::move(tile)));
            canvas->save();
            canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
            canvas->clipRect(dstRect);
            canvas->saveLayer(&dstRect, &paint);
            canvas->drawImage(fBitmap.get(), 0, 0);
            canvas->restore();
            canvas->drawRect(srcRect, red);
            canvas->drawRect(dstRect, blue);
            canvas->restore();
        }

        // test that the crop rect properly applies to the tile image filter
        {
            canvas->translate(0, SkIntToScalar(100));

            SkRect srcRect = SkRect::MakeXYWH(0, 0, 50, 50);
            SkRect dstRect = SkRect::MakeXYWH(0, 0, 100, 100);
            SkIRect cropRect = SkIRect::MakeXYWH(5, 5, 40, 40);
            sk_sp<SkColorFilter> greenCF = SkColorFilters::Blend(SK_ColorGREEN, SkBlendMode::kSrc);
            sk_sp<SkImageFilter> green(SkImageFilters::ColorFilter(std::move(greenCF),
                                                                      nullptr,
                                                                      &cropRect));
            SkPaint paint;
            paint.setColor(SK_ColorRED);
            paint.setImageFilter(SkImageFilters::Tile(srcRect, dstRect, std::move(green)));
            canvas->drawRect(dstRect, paint);
        }
    }
private:
    sk_sp<SkImage> fBitmap, fCheckerboard;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new TileImageFilterGM;)
}  // namespace skiagm
