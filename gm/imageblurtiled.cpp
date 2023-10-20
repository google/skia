/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#define WIDTH 640
#define HEIGHT 480

namespace skiagm {

class ImageBlurTiledGM : public GM {
public:
    ImageBlurTiledGM(SkScalar sigmaX, SkScalar sigmaY)
        : fSigmaX(sigmaX), fSigmaY(sigmaY) {
    }

protected:
    SkString getName() const override { return SkString("imageblurtiled"); }

    SkISize getISize() override { return SkISize::Make(WIDTH, HEIGHT); }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setImageFilter(SkImageFilters::Blur(fSigmaX, fSigmaY, nullptr));
        const SkScalar tileSize = SkIntToScalar(128);
        SkRect bounds = canvas->getLocalClipBounds();
        for (SkScalar y = bounds.top(); y < bounds.bottom(); y += tileSize) {
            for (SkScalar x = bounds.left(); x < bounds.right(); x += tileSize) {
                canvas->save();
                canvas->clipRect(SkRect::MakeXYWH(x, y, tileSize, tileSize));
                canvas->saveLayer(nullptr, &paint);
                const char* str[] = {
                    "The quick",
                    "brown fox",
                    "jumped over",
                    "the lazy dog.",
                };
                SkFont font(ToolUtils::DefaultPortableTypeface(), 100);
                int posY = 0;
                for (unsigned i = 0; i < std::size(str); i++) {
                    posY += 100;
                    canvas->drawString(str[i], 0, SkIntToScalar(posY), font, SkPaint());
                }
                canvas->restore();
                canvas->restore();
            }
        }
    }

private:
    SkScalar fSigmaX;
    SkScalar fSigmaY;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new  ImageBlurTiledGM(3.0f, 3.0f);)

}  // namespace skiagm
