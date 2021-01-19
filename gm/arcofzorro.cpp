/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/utils/SkRandom.h"

#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrResourceProvider.h"

#if 0
static void yeah(const GrCaps* caps) {
    sk_sp<GrTextureProxy> proxy = GrProxyProvider::MakeFullyLazyProxy(
            [this](GrResourceProvider* rp,
                   const GrSurfaceProxy::LazySurfaceDesc& desc)
                    -> GrSurfaceProxy::LazyCallbackResult {
                static constexpr SkISize kDimensions = {1234, 567};
                sk_sp<GrTexture> texture = rp->createTexture(
                        kDimensions, desc.fFormat, desc.fRenderable, desc.fSampleCnt,
                        desc.fMipmapped, desc.fBudgeted, desc.fProtected);
                return texture;
            },
            format, GrRenderable::kNo, 1, GrProtected::kNo, *caps, GrSurfaceProxy::UseAllocator::kYes);
}
#endif

namespace skiagm {

// This GM draws a lot of arcs in a 'Z' shape. It particularly exercises
// the 'drawArc' code near a singularly of its processing (i.e., near the
// edge of one of its underlying quads).
class ArcOfZorroGM : public GM {
public:
    ArcOfZorroGM()
            : fTopLeftContentRect(SkIRect::MakeXYWH(kXOff, kYOff, kContentSize, kContentSize)) {
        this->setBGColor(0xFFCCCCCC);
    }

protected:

    SkString onShortName() override {
        return SkString("arcofzorro");
    }

    SkISize onISize() override {
        return SkISize::Make((2*kContentSize+kPad2) * kSkTileModeCount + kPad2,
                             (2*kContentSize+kPad2) * kSkTileModeCount + kPad2);
    }

    void onOnceBeforeDraw() override {
        SkImageInfo info = SkImageInfo::Make(kXOff + kContentSize + kXPad,
                                             kYOff + kContentSize + kYPad,
                                             kRGBA_8888_SkColorType, kPremul_SkAlphaType);

        SkBitmap bm;
        bm.allocPixels(info);

        bm.eraseColor(SK_ColorBLACK);

        bm.eraseArea(fTopLeftContentRect, SK_ColorWHITE);

        int halfM1 = kContentSize/2 - 1;

        bm.eraseArea(SkIRect::MakeXYWH(kXOff + halfM1,         kYOff + 2,      2,              halfM1), SK_ColorGRAY);
        bm.eraseArea(SkIRect::MakeXYWH(kXOff + 2,              kYOff + halfM1, kContentSize-4, 2), SK_ColorGRAY);
        bm.eraseArea(SkIRect::MakeXYWH(kXOff + 2,              kYOff + halfM1, 2,              2), SK_ColorRED);
        bm.eraseArea(SkIRect::MakeXYWH(kXOff + kContentSize-4, kYOff + halfM1, 2,              2), SK_ColorGREEN);

        bm.eraseArea(SkIRect::MakeXYWH(kXOff, kYOff, kContentSize, 1), SK_ColorBLUE);
        bm.eraseArea(SkIRect::MakeXYWH(kXOff, kYOff, 1, kContentSize), SK_ColorBLUE);
        bm.eraseArea(SkIRect::MakeXYWH(kXOff+kContentSize-1, kYOff, 1, kContentSize), SK_ColorBLUE);
        bm.eraseArea(SkIRect::MakeXYWH(kXOff, kYOff+kContentSize-1, kContentSize, 1), SK_ColorBLUE);

        fTopLeft = bm;
    }

    void onDraw(SkCanvas* canvas) override {
//        canvas->drawBitmapRect(fTopLeft, fTopLeftContentRect, SkRect::MakeWH(kContentSize, kContentSize), nullptr);

#if 1
        SkSamplingOptions sampling(SkFilterMode::kNearest, SkMipmapMode::kNone);
        SkPaint p;

        int y = kPad2;
        for (auto yMode : { SkTileMode::kClamp, SkTileMode::kRepeat, SkTileMode::kMirror, SkTileMode::kDecal }) {
            int x = kPad2;
            for (auto xMode : { SkTileMode::kClamp, SkTileMode::kRepeat, SkTileMode::kMirror, SkTileMode::kDecal }) {
                SkRect clipRect = SkRect::MakeXYWH(x, y, 2*kContentSize, 2*kContentSize);
                SkRect drawRect = SkRect::MakeXYWH(x+kContentSize/2, y+kContentSize/2, kContentSize, kContentSize);

                SkMatrix matrix = SkMatrix::Translate(drawRect.x(), drawRect.y());

                p.setShader(fTopLeft.makeShader(xMode, yMode, sampling, &matrix));


                canvas->drawRect(clipRect, p);
                x += 2*kContentSize+kPad2;
            }

            y += 2*kContentSize+kPad2;
        }
#endif

    }

private:
    static constexpr int kXOff = 0; //8;
    static constexpr int kXPad = 0; //24;
    static constexpr int kYOff = 0; //16;
    static constexpr int kYPad = 0; //80;
    static constexpr int kPad2 = 4; //80;


    static constexpr int kContentSize = 32;

    SkIRect  fTopLeftContentRect;
    SkBitmap fTopLeft;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}  // namespace skiagm
