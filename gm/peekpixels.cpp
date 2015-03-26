/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkSurface.h"
#include "SkPicture.h"

static void draw_content(SkCanvas* canvas) {
    SkImageInfo info = canvas->imageInfo();
    SkPaint paint;
    paint.setAntiAlias(true);
    canvas->drawCircle(SkScalarHalf(info.width()), SkScalarHalf(info.height()),
                       SkScalarHalf(info.width()), paint);
}

class PeekPixelsGM : public skiagm::GM {
public:
    PeekPixelsGM() {}

protected:
    SkString onShortName() override {
        return SkString("peekpixels");
    }

    SkISize onISize() override {
        return SkISize::Make(360, 120);
    }

    void onDraw(SkCanvas* canvas) override {
        SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);
        SkAutoTUnref<SkSurface> surface(canvas->newSurface(info));
        if (surface.get()) {
            SkCanvas* surfCanvas = surface->getCanvas();

            draw_content(surfCanvas);
            SkBitmap bitmap;

            // test peekPixels
            {
                SkImageInfo info;
                size_t rowBytes;
                const void* addr = surfCanvas->peekPixels(&info, &rowBytes);
                if (addr && bitmap.installPixels(info, const_cast<void*>(addr), rowBytes)) {
                    canvas->drawBitmap(bitmap, 0, 0, NULL);
                }
            }

            // test ROCanvasPixels
            canvas->translate(120, 0);
            SkAutoROCanvasPixels ropixels(surfCanvas);
            if (ropixels.asROBitmap(&bitmap)) {
                canvas->drawBitmap(bitmap, 0, 0, NULL);
            }

            // test Surface
            canvas->translate(120, 0);
            surface->draw(canvas, 0, 0, NULL);
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM( return SkNEW(PeekPixelsGM); )
