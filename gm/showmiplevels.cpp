/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkMipmapBuilder.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include <math.h>

class ShowMipLevels3 : public skiagm::GM {
    sk_sp<SkImage> fImg;

    SkString getName() const override { return SkString("showmiplevels_explicit"); }

    SkISize getISize() override { return {1130, 970}; }

    void onOnceBeforeDraw() override {
        fImg = ToolUtils::GetResourceAsImage("images/ship.png");
        fImg = fImg->makeRasterImage(); // makeWithMips only works on raster for now

        const SkColor colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };

        SkMipmapBuilder builder(fImg->imageInfo());
        for (int i = 0; i < builder.countLevels(); ++i) {
            auto surf = SkSurfaces::WrapPixels(builder.level(i));
            surf->getCanvas()->drawColor(colors[i % std::size(colors)]);
        }
        fImg = builder.attachTo(fImg);
    }

    DrawResult onDraw(SkCanvas* canvas, SkString*) override {
        canvas->drawColor(0xFFDDDDDD);

        canvas->translate(10, 10);
        for (auto mm : {SkMipmapMode::kNone, SkMipmapMode::kNearest, SkMipmapMode::kLinear}) {
            for (auto fm : {SkFilterMode::kNearest, SkFilterMode::kLinear}) {
                canvas->translate(0, draw_downscaling(canvas, {fm, mm}));
            }
        }
        return DrawResult::kOk;
    }

private:
    SkScalar draw_downscaling(SkCanvas* canvas, SkSamplingOptions sampling) {
        SkAutoCanvasRestore acr(canvas, true);

        SkPaint paint;
        SkRect r = {0, 0, 150, 150};
        for (float scale = 1; scale >= 0.1f; scale *= 0.7f) {
            SkMatrix matrix = SkMatrix::Scale(scale, scale);
            paint.setShader(fImg->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat,
                                             sampling, &matrix));
            canvas->drawRect(r, paint);
            canvas->translate(r.width() + 10, 0);
        }
        return r.height() + 10;
    }

    using INHERITED = skiagm::GM;
};
DEF_GM( return new ShowMipLevels3; )
