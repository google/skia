/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/gpu/GrContextOptions.h"

namespace skiagm {

class BatchedConvexPathsGM : public GM {
private:
    SkString getName() const override { return SkString("batchedconvexpaths"); }
    SkISize getISize() override { return SkISize::Make(512, 512); }

    void modifyGrContextOptions(GrContextOptions* ctxOptions) override {
        // Ensure our paths don't go through the atlas path renderer.
        ctxOptions->fGpuPathRenderers &= ~GpuPathRenderers::kAtlas;
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        canvas->clear(SK_ColorBLACK);
        for (uint32_t i = 0; i < 10; ++i) {
            SkAutoCanvasRestore acr(canvas, true);

            int numPoints = (i + 3) * 3;
            SkPath path;
            path.moveTo(1, 0);
            for (float j = 1; j < numPoints; j += 3) {
                constexpr float k2PI = SK_ScalarPI * 2;
                path.cubicTo(cosf(j/numPoints * k2PI), sinf(j/numPoints * k2PI),
                             cosf((j+1)/numPoints * k2PI), sinf((j+1)/numPoints * k2PI),
                             j+2 == numPoints ? 1 : cosf((j+2)/numPoints * k2PI),
                             j+2 == numPoints ? 0 : sinf((j+2)/numPoints * k2PI));
            }
            float scale = 256 - i*24;
            canvas->translate(scale + (256 - scale) * .33f, scale + (256 - scale) * .33f);
            canvas->scale(scale, scale);

            SkPaint paint;
            paint.setColor(((i + 123458383u) * 285018463u) | 0xff808080);
            paint.setAlphaf(0.3f);
            paint.setAntiAlias(true);

            canvas->drawPath(path, paint);
        }
        return DrawResult::kOk;
    }
};

DEF_GM( return new BatchedConvexPathsGM; )

}  // namespace skiagm
