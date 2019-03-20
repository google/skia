/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilterImageFilter.h"
#include "SkColorMatrixFilter.h"
#include "SkImage.h"
#include "SkImageSource.h"
#include "SkTileImageFilter.h"
#include "ToolUtils.h"
#include "gm.h"

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
    SkString onShortName() override {
        return SkString("tileimagefilter");
    }

    SkISize onISize() override{
        return SkISize::Make(WIDTH, HEIGHT);
    }

    void onOnceBeforeDraw() override {
        fBitmap = SkImage::MakeFromBitmap(
                ToolUtils::create_string_bitmap(50, 50, 0xD000D000, 10, 45, 50, "e"));

        fCheckerboard = SkImage::MakeFromBitmap(
                ToolUtils::create_checkerboard_bitmap(80, 80, 0xFFA0A0A0, 0xFF404040, 8));
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
            sk_sp<SkImageFilter> tileInput(SkImageSource::Make(image));
            sk_sp<SkImageFilter> filter(SkTileImageFilter::Make(srcRect,
                                                                dstRect,
                                                                std::move(tileInput)));
            canvas->save();
            canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
            SkPaint paint;
            paint.setImageFilter(std::move(filter));
            canvas->drawImage(fBitmap.get(), 0, 0, &paint);
            canvas->drawRect(srcRect, red);
            canvas->drawRect(dstRect, blue);
            canvas->restore();
            x += image->width() + MARGIN;
            if (x + image->width() > WIDTH) {
                x = 0;
                y += image->height() + MARGIN;
            }
        }

        SkScalar matrix[20] = { SK_Scalar1, 0, 0, 0, 0,
                                0, SK_Scalar1, 0, 0, 0,
                                0, 0, SK_Scalar1, 0, 0,
                                0, 0, 0, SK_Scalar1, 0 };

        SkRect srcRect = SkRect::MakeWH(SkIntToScalar(fBitmap->width()),
                                        SkIntToScalar(fBitmap->height()));
        SkRect dstRect = SkRect::MakeWH(SkIntToScalar(fBitmap->width() * 2),
                                        SkIntToScalar(fBitmap->height() * 2));
        sk_sp<SkImageFilter> tile(SkTileImageFilter::Make(srcRect, dstRect, nullptr));
        sk_sp<SkColorFilter> cf(SkColorFilter::MakeMatrixFilterRowMajor255(matrix));

        SkPaint paint;
        paint.setImageFilter(SkColorFilterImageFilter::Make(std::move(cf), std::move(tile)));
        canvas->save();
        canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
        canvas->clipRect(dstRect);
        canvas->saveLayer(&dstRect, &paint);
        canvas->drawImage(fBitmap.get(), 0, 0);
        canvas->restore();
        canvas->drawRect(srcRect, red);
        canvas->drawRect(dstRect, blue);
        canvas->restore();

        canvas->translate(0, SkIntToScalar(100));

        srcRect = SkRect::MakeXYWH(0, 0, 50, 50);
        dstRect = SkRect::MakeXYWH(0, 0, 100, 100);
        SkImageFilter::CropRect cropRect(SkRect::MakeXYWH(5, 5, 40, 40));
        sk_sp<SkColorFilter> greenCF = SkColorFilter::MakeModeFilter(SK_ColorGREEN,
                                                                     SkBlendMode::kSrc);
        sk_sp<SkImageFilter> green(SkColorFilterImageFilter::Make(std::move(greenCF),
                                                                  nullptr,
                                                                  &cropRect));
        tile = SkTileImageFilter::Make(srcRect, dstRect, std::move(green));
        paint.setColor(SK_ColorRED);
        paint.setImageFilter(std::move(tile));
        canvas->drawRect(dstRect, paint);
    }
private:
    sk_sp<SkImage> fBitmap, fCheckerboard;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new TileImageFilterGM;)
}
