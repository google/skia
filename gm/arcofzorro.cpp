/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkRandom.h"
#include "SkSurface.h"
#include "GrContext.h"

namespace skiagm {

// This GM draws a lot of arcs in a 'Z' shape. It particularly exercises
// the 'drawArc' code near a singularly of its processing (i.e., near the
// edge of one of its underlying quads).
class ArcOfZorroGM : public GM {
public:
    ArcOfZorroGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:
    bool runAsBench() const override { return true; }

    SkString onShortName() override {
        return SkString("arcofzorro");
    }

    SkISize onISize() override {
        return SkISize::Make(1024, 1024);
    }

    void onDraw(SkCanvas* canvas) override {

        GrContext* context = canvas->getGrContext();

        SkImageInfo surfaceII = SkImageInfo::Make(1024, 1024, kBGRA_8888_SkColorType, kPremul_SkAlphaType);
        sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, surfaceII, 0, nullptr);

        SkImageInfo bmII = SkImageInfo::Make(1024, 1024, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType);

        for (int i = 0; i < 32; ++i) {
            SkBitmap src;
            src.allocPixels(bmII);
            src.eraseColor(SK_ColorGREEN);

            surface->writePixels(src, 0, 0);
        }

        context->flush();
    }

private:

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}
