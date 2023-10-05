/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "src/base/SkMathPriv.h"
#include "src/base/SkRandom.h"
#include "tools/GpuToolUtils.h"
#include "tools/ToolUtils.h"

static sk_sp<SkImage> makebm(int w, int h) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    auto surface(SkSurfaces::Raster(info));
    SkCanvas* canvas = surface->getCanvas();

    const SkScalar wScalar = SkIntToScalar(w);
    const SkScalar hScalar = SkIntToScalar(h);

    const SkPoint     pt = { wScalar / 2, hScalar / 2 };

    const SkScalar    radius = 4 * std::max(wScalar, hScalar);

    constexpr SkColor     colors[] = { SK_ColorRED, SK_ColorYELLOW,
                                          SK_ColorGREEN, SK_ColorMAGENTA,
                                          SK_ColorBLUE, SK_ColorCYAN,
                                          SK_ColorRED};

    constexpr SkScalar    pos[] = {0,
                                      SK_Scalar1 / 6,
                                      2 * SK_Scalar1 / 6,
                                      3 * SK_Scalar1 / 6,
                                      4 * SK_Scalar1 / 6,
                                      5 * SK_Scalar1 / 6,
                                      SK_Scalar1};

    SkASSERT(std::size(colors) == std::size(pos));
    SkPaint     paint;
    SkRect rect = SkRect::MakeWH(wScalar, hScalar);
    SkMatrix mat = SkMatrix::I();
    for (int i = 0; i < 4; ++i) {
        paint.setShader(SkGradientShader::MakeRadial(
                        pt, radius,
                        colors, pos,
                        std::size(colors),
                        SkTileMode::kRepeat,
                        0, &mat));
        canvas->drawRect(rect, paint);
        rect.inset(wScalar / 8, hScalar / 8);
        mat.postScale(SK_Scalar1 / 4, SK_Scalar1 / 4);
    }
    return surface->makeImageSnapshot();
}

constexpr int gSize = 1024;
constexpr int gSurfaceSize = 2048;

// This GM calls drawImageRect several times using the same texture. This is intended to exercise
// combining GrDrawOps during these calls.
class DrawMiniBitmapRectGM : public skiagm::GM {
public:
    DrawMiniBitmapRectGM(bool antiAlias) : fAA(antiAlias) {
        fName.set("drawminibitmaprect");
        if (fAA) {
            fName.appendf("_aa");
        }
    }

protected:
    SkString getName() const override { return fName; }

    SkISize getISize() override { return SkISize::Make(gSize, gSize); }

    void onDraw(SkCanvas* canvas) override {
        if (nullptr == fImage) {
            fImage = ToolUtils::MakeTextureImage(canvas, makebm(gSurfaceSize, gSurfaceSize));
        }

        const SkRect dstRect = { 0, 0, SkIntToScalar(64), SkIntToScalar(64)};
        const int kMaxSrcRectSize = 1 << (SkNextLog2(gSurfaceSize) + 2);

        constexpr int kPadX = 30;
        constexpr int kPadY = 40;

        int rowCount = 0;
        canvas->translate(SkIntToScalar(kPadX), SkIntToScalar(kPadY));
        canvas->save();
        SkRandom random;

        SkPaint paint;
        paint.setAntiAlias(fAA);
        for (int w = 1; w <= kMaxSrcRectSize; w *= 3) {
            for (int h = 1; h <= kMaxSrcRectSize; h *= 3) {

                const SkIRect srcRect =
                        SkIRect::MakeXYWH((gSurfaceSize - w) / 2, (gSurfaceSize - h) / 2, w, h);
                canvas->save();
                switch (random.nextU() % 3) {
                    case 0:
                        canvas->rotate(random.nextF() * 10.f);
                        break;
                    case 1:
                        canvas->rotate(-random.nextF() * 10.f);
                        break;
                    case 2:
                        // rect stays rect
                        break;
                }
                canvas->drawImageRect(fImage.get(), SkRect::Make(srcRect), dstRect,
                                      SkSamplingOptions(), &paint,
                                      SkCanvas::kFast_SrcRectConstraint);
                canvas->restore();

                canvas->translate(dstRect.width() + SK_Scalar1 * kPadX, 0);
                ++rowCount;
                if ((dstRect.width() + 2 * kPadX) * rowCount > gSize) {
                    canvas->restore();
                    canvas->translate(0, dstRect.height() + SK_Scalar1 * kPadY);
                    canvas->save();
                    rowCount = 0;
                }
            }
        }
        canvas->restore();
    }

private:
    bool            fAA;
    sk_sp<SkImage>  fImage;
    SkString        fName;

    using INHERITED = skiagm::GM;
};

DEF_GM( return new DrawMiniBitmapRectGM(true); )
DEF_GM( return new DrawMiniBitmapRectGM(false); )
