/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GPU backend.

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/private/GrSharedEnums.h"
#include "include/private/GrTypesPriv.h"
#include "src/core/SkTLList.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/effects/generated/GrAARectEffect.h"
#include "tools/gpu/TestOps.h"

#include <memory>
#include <utility>

class GrAppliedClip;

namespace skiagm {

/**
 * This GM directly exercises a GrProcessor that clips against rects.
 */
class AARectEffect : public GpuGM {
public:
    AARectEffect() { this->setBGColor(0xFFFFFFFF); }

protected:
    SkString onShortName() override { return SkString("aa_rect_effect"); }

    SkISize onISize() override { return SkISize::Make(210, 250); }

    void onOnceBeforeDraw() override {}

    void onDraw(GrRecordingContext* context,
                GrSurfaceDrawContext* surfaceDrawContext,
                SkCanvas* canvas) override {
        SkScalar y = 12.f;
        static constexpr SkScalar kDX = 12.f;
        static constexpr SkScalar kOutset = 5.f;

        static constexpr SkRect kRects[] = {
                // integer edges
                SkRect::MakeLTRB(5.f, 1.f, 30.f, 25.f),
                // half-integer edges
                SkRect::MakeLTRB(5.5f, 0.5f, 29.5f, 24.5f),
                // vertically/horizontally thin rects that cover pixel centers
                SkRect::MakeLTRB(5.25f, 0.5f, 5.75f, 24.5f),
                SkRect::MakeLTRB(5.5f,  0.5f, 29.5f, 0.75f),
                // vertically/horizontally thin rects that don't cover pixel centers
                SkRect::MakeLTRB(5.55f, 0.5f, 5.75f, 24.5f),
                SkRect::MakeLTRB(5.5f, .05f, 29.5f, .25f),
                // small in x and y
                SkRect::MakeLTRB(5.05f, .55f, 5.45f, .85f),
        };

        for (auto r : kRects) {
            SkScalar x = kDX;

            for (int et = 0; et < kGrClipEdgeTypeCnt; ++et) {
                SkRect rect = r.makeOffset(x, y);
                GrClipEdgeType edgeType = static_cast<GrClipEdgeType>(et);
                auto fp = GrAARectEffect::Make(/*inputFP=*/nullptr, edgeType, rect);
                SkASSERT(fp);

                GrPaint grPaint;
                grPaint.setColor4f({ 0, 0, 0, 1.f });
                grPaint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));
                grPaint.setCoverageFragmentProcessor(std::move(fp));
                auto drawRect = rect.makeOutset(kOutset, kOutset);
                auto op = sk_gpu_test::test_ops::MakeRect(context, std::move(grPaint), drawRect);
                surfaceDrawContext->addDrawOp(std::move(op));

                x += SkScalarCeilToScalar(rect.width() + kDX);
            }

            // Draw rect without and with AA using normal API for reference
            canvas->save();
            canvas->translate(x, y);
            SkPaint paint;
            canvas->drawRect(r, paint);
            x += SkScalarCeilToScalar(r.width() + kDX);
            paint.setAntiAlias(true);
            canvas->drawRect(r, paint);
            canvas->restore();

            y += SkScalarCeilToScalar(r.height() + 20.f);
        }
    }

private:
    using INHERITED = GM;
};

DEF_GM(return new AARectEffect;)
}  // namespace skiagm
