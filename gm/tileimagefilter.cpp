/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "sk_tool_utils.h"
#include "SkColorFilterImageFilter.h"
#include "SkColorMatrixFilter.h"
#include "SkImage.h"
#include "SkImageSource.h"
#include "SkTileImageFilter.h"
#include "gm.h"

#define WIDTH 400
#define HEIGHT 100
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
        fBitmap.reset(SkImage::NewFromBitmap(
            sk_tool_utils::create_string_bitmap(50, 50, 0xD000D000, 10, 45, 50, "e")));

        fCheckerboard.reset(SkImage::NewFromBitmap(
            sk_tool_utils::create_checkerboard_bitmap(80, 80,
                                                      sk_tool_utils::color_to_565(0xFFA0A0A0),
                                                      sk_tool_utils::color_to_565(0xFF404040),
                                                      8)));
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
            const SkImage* image = (i & 0x01) ? fCheckerboard : fBitmap;
            SkRect srcRect = SkRect::MakeXYWH(SkIntToScalar(image->width()/4),
                                              SkIntToScalar(image->height()/4),
                                              SkIntToScalar(image->width()/(i+1)),
                                              SkIntToScalar(image->height()/(i+1)));
            SkRect dstRect = SkRect::MakeXYWH(SkIntToScalar(i * 8),
                                              SkIntToScalar(i * 4),
                                              SkIntToScalar(image->width() - i * 12),
                                              SkIntToScalar(image->height()) - i * 12);
            SkAutoTUnref<SkImageFilter> tileInput(SkImageSource::Create(image));
            SkAutoTUnref<SkImageFilter> filter(
                SkTileImageFilter::Create(srcRect, dstRect, tileInput));
            canvas->save();
            canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
            SkPaint paint;
            paint.setImageFilter(filter);
            canvas->drawImage(fBitmap, 0, 0, &paint);
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
        SkAutoTUnref<SkImageFilter> tile(SkTileImageFilter::Create(srcRect, dstRect, nullptr));
        SkAutoTUnref<SkColorFilter> cf(SkColorMatrixFilter::Create(matrix));

        SkAutoTUnref<SkImageFilter> cfif(SkColorFilterImageFilter::Create(cf, tile.get()));
        SkPaint paint;
        paint.setImageFilter(cfif);
        canvas->save();
        canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
        canvas->clipRect(dstRect);
        canvas->saveLayer(&dstRect, &paint);
        canvas->drawImage(fBitmap, 0, 0);
        canvas->restore();
        canvas->drawRect(srcRect, red);
        canvas->drawRect(dstRect, blue);
        canvas->restore();
    }
private:
    SkAutoTUnref<SkImage> fBitmap, fCheckerboard;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new TileImageFilterGM;)
}
