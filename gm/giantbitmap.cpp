/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFilterQuality.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"

namespace {
/*
 *  Want to ensure that our bitmap sampler (in bitmap shader) keeps plenty of
 *  precision when scaling very large images (where the dx might get very small.
 */

static constexpr int W = 257;
static constexpr int H = 161;

class GiantBitmapGM : public skiagm::GM {
public:
    GiantBitmapGM(SkTileMode mode, bool doFilter, bool doRotate, const char* name)
        : fName(name), fMode(mode), fDoFilter(doFilter), fDoRotate(doRotate) {}

private:
    const char* fName = nullptr;
    SkBitmap fBM;
    SkTileMode fMode;
    bool fDoFilter;
    bool fDoRotate;

    SkString onShortName() override { return SkString(fName); }

    SkISize onISize() override { return {640, 480}; }

    void onOnceBeforeDraw() override {
        fBM.allocN32Pixels(W, H);
        fBM.eraseColor(SK_ColorWHITE);

        const SkColor colors[] = {
            SK_ColorBLUE, SK_ColorRED, SK_ColorBLACK, SK_ColorGREEN
        };

        SkCanvas canvas(fBM);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStrokeWidth(SkIntToScalar(20));
        for (int x = -W; x < W; x += 60) {
            paint.setColor(colors[x/60 & 0x3]);

            SkScalar xx = SkIntToScalar(x);
            canvas.drawLine(xx, 0, xx, SkIntToScalar(H),
                            paint);
        }
    }
    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;

        SkMatrix m;
        if (fDoRotate) {
            m.setSkew(SK_Scalar1, 0, 0, 0);
        } else {
            SkScalar scale = 11*SK_Scalar1/12;
            m.setScale(scale, scale);
        }
        paint.setShader(fBM.makeShader(fMode, fMode, &m));
        paint.setFilterQuality(fDoFilter ? kLow_SkFilterQuality : kNone_SkFilterQuality);

        canvas->translate(SkIntToScalar(50), SkIntToScalar(50));
        canvas->drawPaint(paint);
    }
};
}  // namespace

#define M(MODE, FILTER, ROTATE, NAME) \
    DEF_GM( return new GiantBitmapGM(MODE, FILTER, ROTATE, NAME); )
M(SkTileMode::kClamp,  false, false, "giantbitmap_clamp_point_scale"   )
M(SkTileMode::kRepeat, false, false, "giantbitmap_repeat_point_scale"  )
M(SkTileMode::kMirror, false, false, "giantbitmap_mirror_point_scale"  )
M(SkTileMode::kClamp,  true,  false, "giantbitmap_clamp_bilerp_scale"  )
M(SkTileMode::kRepeat, true,  false, "giantbitmap_repeat_bilerp_scale" )
M(SkTileMode::kMirror, true,  false, "giantbitmap_mirror_bilerp_scale" )

M(SkTileMode::kClamp,  false, true,  "giantbitmap_clamp_point_rotate"  )
M(SkTileMode::kRepeat, false, true,  "giantbitmap_repeat_point_rotate" )
M(SkTileMode::kMirror, false, true,  "giantbitmap_mirror_point_rotate" )
M(SkTileMode::kClamp,  true,  true,  "giantbitmap_clamp_bilerp_rotate" )
M(SkTileMode::kRepeat, true,  true,  "giantbitmap_repeat_bilerp_rotate")
M(SkTileMode::kMirror, true,  true,  "giantbitmap_mirror_bilerp_rotate")
#undef M
