/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkTileImageFilter.h"
#include "SkBitmapSource.h"

#define WIDTH 400
#define HEIGHT 100
#define MARGIN 12

namespace skiagm {

class TileImageFilterGM : public GM {
public:
    TileImageFilterGM() : fInitialized(false) {
        this->setBGColor(0xFF000000);
    }

protected:
    virtual SkString onShortName() {
        return SkString("tileimagefilter");
    }

    void make_bitmap() {
        fBitmap.setConfig(SkBitmap::kARGB_8888_Config, 80, 80);
        fBitmap.allocPixels();
        SkBitmapDevice device(fBitmap);
        SkCanvas canvas(&device);
        canvas.clear(0x00000000);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(0xD000D000);
        paint.setTextSize(SkIntToScalar(96));
        const char* str = "e";
        canvas.drawText(str, strlen(str), SkIntToScalar(15), SkIntToScalar(65), paint);
    }

    void make_checkerboard() {
        fCheckerboard.setConfig(SkBitmap::kARGB_8888_Config, 80, 80);
        fCheckerboard.allocPixels();
        SkBitmapDevice device(fCheckerboard);
        SkCanvas canvas(&device);
        canvas.clear(0x00000000);
        SkPaint darkPaint;
        darkPaint.setColor(0xFF404040);
        SkPaint lightPaint;
        lightPaint.setColor(0xFFA0A0A0);
        for (int y = 0; y < 80; y += 16) {
          for (int x = 0; x < 80; x += 16) {
            canvas.save();
            canvas.translate(SkIntToScalar(x), SkIntToScalar(y));
            canvas.drawRect(SkRect::MakeXYWH(0, 0, 8, 8), darkPaint);
            canvas.drawRect(SkRect::MakeXYWH(8, 0, 8, 8), lightPaint);
            canvas.drawRect(SkRect::MakeXYWH(0, 8, 8, 8), lightPaint);
            canvas.drawRect(SkRect::MakeXYWH(8, 8, 8, 8), darkPaint);
            canvas.restore();
          }
        }
    }

    virtual SkISize onISize() {
        return make_isize(WIDTH, HEIGHT);
    }

    void drawClippedBitmap(SkCanvas* canvas, const SkBitmap& bitmap, const SkPaint& paint,
                           SkScalar x, SkScalar y) {
        canvas->save();
        canvas->translate(x, y);
        canvas->clipRect(SkRect::MakeXYWH(0, 0,
            SkIntToScalar(bitmap.width()), SkIntToScalar(bitmap.height())));
        canvas->drawBitmap(bitmap, 0, 0, &paint);
        canvas->restore();
    }

    virtual void onDraw(SkCanvas* canvas) {
        if (!fInitialized) {
            make_bitmap();
            make_checkerboard();
            fInitialized = true;
        }
        canvas->clear(0x00000000);
        SkPaint paint;

        int x = 0, y = 0;
        for (size_t i = 0; i < 4; i++) {
            SkBitmap* bitmap = (i & 0x01) ? &fCheckerboard : &fBitmap;
            SkRect srcRect = SkRect::MakeXYWH(SkIntToScalar(bitmap->width()/4),
                                              SkIntToScalar(bitmap->height()/4),
                                              SkIntToScalar(bitmap->width()/(i+1)),
                                              SkIntToScalar(bitmap->height()/(i+1)));
            SkRect dstRect = SkRect::MakeXYWH(SkIntToScalar(i * 8),
                                              SkIntToScalar(i * 4),
                                              SkIntToScalar(bitmap->width() - i * 4),
                                              SkIntToScalar(bitmap->height()) - i * 8);
            SkAutoTUnref<SkImageFilter> tileInput(SkNEW_ARGS(SkBitmapSource, (*bitmap)));
            SkAutoTUnref<SkImageFilter> filter(SkNEW_ARGS(
                SkTileImageFilter, (srcRect, dstRect, tileInput)));
            paint.setImageFilter(filter);
            drawClippedBitmap(canvas, *bitmap, paint, SkIntToScalar(x), SkIntToScalar(y));
            x += bitmap->width() + MARGIN;
            if (x + bitmap->width() > WIDTH) {
                x = 0;
                y += bitmap->height() + MARGIN;
            }
        }
    }
private:
    typedef GM INHERITED;
    SkBitmap fBitmap, fCheckerboard;
    bool fInitialized;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new TileImageFilterGM; }
static GMRegistry reg(MyFactory);

}
