/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBlurImageFilter.h"
#include "SkRandom.h"

#define WIDTH 640
#define HEIGHT 480

namespace skiagm {

class ImageBlurTiledGM : public GM {
public:
    ImageBlurTiledGM(SkScalar sigmaX, SkScalar sigmaY)
        : fSigmaX(sigmaX), fSigmaY(sigmaY) {
    }

protected:
    virtual SkString onShortName() {
        return SkString("imageblurtiled");
    }

    virtual SkISize onISize() {
        return SkISize::Make(WIDTH, HEIGHT);
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkPaint paint;
        paint.setImageFilter(SkBlurImageFilter::Create(fSigmaX, fSigmaY))->unref();
        const SkScalar tile_size = SkIntToScalar(128);
        SkRect bounds;
        if (!canvas->getClipBounds(&bounds)) {
            bounds.setEmpty();
        }
        for (SkScalar y = bounds.top(); y < bounds.bottom(); y += tile_size) {
            for (SkScalar x = bounds.left(); x < bounds.right(); x += tile_size) {
                canvas->save();
                canvas->clipRect(SkRect::MakeXYWH(x, y, tile_size, tile_size));
                canvas->saveLayer(NULL, &paint);
                const char* str[] = {
                    "The quick",
                    "brown fox",
                    "jumped over",
                    "the lazy dog.",
                };
                SkPaint textPaint;
                textPaint.setAntiAlias(true);
                sk_tool_utils::set_portable_typeface(&textPaint);
                textPaint.setTextSize(SkIntToScalar(100));
                int posY = 0;
                for (unsigned i = 0; i < SK_ARRAY_COUNT(str); i++) {
                    posY += 100;
                    canvas->drawText(str[i], strlen(str[i]), SkIntToScalar(0),
                                     SkIntToScalar(posY), textPaint);
                }
                canvas->restore();
                canvas->restore();
            }
        }
    }

private:
    SkScalar fSigmaX;
    SkScalar fSigmaY;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory1(void*) { return new ImageBlurTiledGM(3.0f, 3.0f); }
static GMRegistry reg1(MyFactory1);

}
