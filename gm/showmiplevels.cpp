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
#include "src/core/SkColorPriv.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkMipmapBuilder.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include <math.h>

/**
 * This GM explicitly makes mip map levels solid colors so we can see which are being used.
 *
 * Rows 0 and 1 don't use mip maps, so the rocket ship gets smaller (and tiled) from
 * left to right. The only difference is the filter mode - Row 0 (nearest) looks blockier
 * than Row 1 (linear).
 *
 * Rows 2 and 3 use the nearest mip map mode, which means we expect
 * Everything other than the first two columns (which are closest to mip level 0 - original image)
 * will be solid colors (Red, Green, Blue) to show this is happening. The sublte difference in
 * filtering can still be observed in Column 1.
 *
 * Rows 4 and 5 use a linear mipmap mode, which means the we'll see a blend of red and spaceships
 * for columns 1 and 2, and non-pure colors for the remaining columns as the breakpoints don't
 * land directly on the mip level changes.
 *
 * If the image is being rendered and all rows show the spaceships from left to right, the
 * hand-crafted mips are being lost somewhere in rendering and that is *wrong*.
 */
class ShowMipLevels : public skiagm::GM {
    sk_sp<SkImage> fImg;

    SkString getName() const override { return SkString("showmiplevels_explicit"); }

    SkISize getISize() override { return {1130, 970}; }

    void onOnceBeforeDraw() override {
        fImg = ToolUtils::GetResourceAsImage("images/ship.png");
        fImg = fImg->makeRasterImage(nullptr); // makeWithMips only works on raster for now

        const SkColor colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };

        SkMipmapBuilder builder(fImg->imageInfo());
        // The number of levels is derived from the size of the image, so we intentionally
        // pick an image that's big enough to support 3+ levels.
        SkASSERT(builder.countLevels() >= (int)std::size(colors));
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
};
DEF_GM(return new ShowMipLevels;)
