/*
 * Copyright 2014 Google Inc.
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
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/private/GrTypesPriv.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/effects/GrConvexPolyEffect.h"
#include "src/gpu/effects/GrPorterDuffXferProcessor.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"
#include "tools/gpu/TestOps.h"

#include <memory>
#include <utility>

class GrAppliedClip;

namespace skiagm {

/**
 * This GM directly exercises a GrProcessor that draws convex polygons.
 */
class ConvexPolyEffect : public GpuGM {
public:
    ConvexPolyEffect() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        return SkString("convex_poly_effect");
    }

    SkISize onISize() override { return SkISize::Make(720, 550); }

    void onOnceBeforeDraw() override {
        SkPath tri;
        tri.moveTo(5.f, 5.f);
        tri.lineTo(100.f, 20.f);
        tri.lineTo(15.f, 100.f);

        fPaths.push_back(tri);
        fPaths.emplace_back();
        fPaths.back().reverseAddPath(tri);

        tri.close();
        fPaths.push_back(tri);

        SkPath ngon;
        constexpr SkScalar kRadius = 50.f;
        const SkPoint center = { kRadius, kRadius };
        for (int i = 0; i < GrConvexPolyEffect::kMaxEdges; ++i) {
            SkScalar angle = 2 * SK_ScalarPI * i / GrConvexPolyEffect::kMaxEdges;
            SkPoint point = { SkScalarCos(angle), SkScalarSin(angle) };
            point.scale(kRadius);
            point = center + point;
            if (0 == i) {
                ngon.moveTo(point);
            } else {
                ngon.lineTo(point);
            }
        }

        fPaths.push_back(ngon);
        SkMatrix scaleM;
        scaleM.setScale(1.1f, 0.4f);
        ngon.transform(scaleM);
        fPaths.push_back(ngon);

        SkPath linePath;
        linePath.moveTo(5.f, 5.f);
        linePath.lineTo(6.f, 6.f);
        fPaths.push_back(linePath);
    }

    DrawResult onDraw(GrRecordingContext* rContext, SkCanvas* canvas, SkString* errorMsg) override {
        auto sdc = SkCanvasPriv::TopDeviceSurfaceDrawContext(canvas);
        if (!sdc) {
            *errorMsg = kErrorMsg_DrawSkippedGpuOnly;
            return DrawResult::kSkip;
        }

        SkScalar y = 0;
        static constexpr SkScalar kDX = 12.f;
        static constexpr SkScalar kOutset = 5.f;

        for (const SkPath& path : fPaths) {
            SkScalar x = 0;

            for (int et = 0; et < kGrClipEdgeTypeCnt; ++et) {
                const SkMatrix m = SkMatrix::Translate(x, y);
                SkPath p;
                path.transform(m, &p);

                GrClipEdgeType edgeType = (GrClipEdgeType) et;
                auto [success, fp] = GrConvexPolyEffect::Make(/*inputFP=*/nullptr, edgeType, p);
                if (!success) {
                    continue;
                }

                GrPaint grPaint;
                grPaint.setColor4f({ 0, 0, 0, 1.f });
                grPaint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));
                grPaint.setCoverageFragmentProcessor(std::move(fp));
                auto rect = p.getBounds().makeOutset(kOutset, kOutset);
                auto op = sk_gpu_test::test_ops::MakeRect(rContext, std::move(grPaint), rect);
                sdc->addDrawOp(std::move(op));

                x += SkScalarCeilToScalar(path.getBounds().width() + kDX);
            }

            // Draw AA and non AA paths using normal API for reference.
            canvas->save();
            canvas->translate(x, y);
            SkPaint paint;
            canvas->drawPath(path, paint);
            canvas->translate(path.getBounds().width() + 10.f, 0);
            paint.setAntiAlias(true);
            canvas->drawPath(path, paint);
            canvas->restore();

            y += SkScalarCeilToScalar(path.getBounds().height() + 20.f);
        }

        return DrawResult::kOk;
    }

private:
    std::vector<SkPath> fPaths;

    using INHERITED = GM;
};

DEF_GM(return new ConvexPolyEffect;)

}  // namespace skiagm
