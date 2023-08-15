/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/gpu/GrContextOptions.h"
#include "include/private/base/SkTArray.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"

using namespace skia_private;

/** This tests the GPU backend's caching of path coverage masks */
class PathMaskCache : public skiagm::GM {
public:
    PathMaskCache() {}

protected:
    SkString getName() const override { return SkString("path_mask_cache"); }

    SkISize getISize() override { return SkISize::Make(650, 950); }

    void onDraw(SkCanvas* canvas) override {
        static constexpr SkScalar kPad = 5.f;

        SkPaint paint;
        paint.setAntiAlias(true);
        auto drawPathSet = [canvas] (const SkPath& path, const SkMatrix& m) {
            SkPaint paint;
            paint.setAntiAlias(true);
            SkRect bounds = path.getBounds();
            m.mapRect(&bounds);
            bounds.roundOut();
            canvas->save();
                canvas->translate(-bounds.fLeft, -bounds.fTop);

                canvas->save();
                    canvas->concat(m);
                    canvas->drawPath(path, paint);
                canvas->restore();

                // translate by integer
                canvas->translate(bounds.width() + kPad, 0.f);
                canvas->save();
                    canvas->concat(m);
                    canvas->drawPath(path, paint);
                canvas->restore();

                // translate by non-integer
                canvas->translate(bounds.width() + kPad + 0.15f, 0.f);
                canvas->save();
                    canvas->concat(m);
                    canvas->drawPath(path, paint);
                canvas->restore();

                // translate again so total translate fraction is almost identical to previous.
                canvas->translate(bounds.width() + kPad + 0.002f, 0.f);
                canvas->save();
                    canvas->concat(m);
                    canvas->drawPath(path, paint);
                canvas->restore();
            canvas->restore();
            return bounds.fBottom + kPad;
        };


        TArray<SkPath> paths;
        paths.push_back();
        paths.back().moveTo(0.f, 0.f);
        paths.back().lineTo(98.f, 100.f);
        paths.back().lineTo(100.f, 100.f);
        paths.back().conicTo(150.f, 50.f, 100.f, 0.f, 0.6f);
        paths.back().conicTo(148.f, 50.f, 100.f, 100.f, 0.6f);
        paths.back().conicTo(50.f, 30.f, 0.f, 100.f, 0.9f);

        paths.push_back();
        paths.back().addCircle(30.f, 30.f, 30.f);
        paths.back().addRect(SkRect::MakeXYWH(45.f, 45.f, 50.f, 60.f));
        paths.back().setFillType(SkPathFillType::kEvenOdd);

        canvas->translate(kPad, kPad);

        for (const SkPath& path : paths) {
            SkScalar ty = drawPathSet(path, SkMatrix::I());
            canvas->translate(0, ty);

            // Non-uniform scale.
            SkMatrix s;
            s.setScale(0.5f, 2.f);
            ty = drawPathSet(path, s);
            canvas->translate(0.f, ty);

            // Rotation
            SkMatrix r;
            r.setRotate(60.f, path.getBounds().centerX(), path.getBounds().centerY());
            ty = drawPathSet(path, r);
            canvas->translate(0.f, ty);
        }
    }

    void modifyGrContextOptions(GrContextOptions* options) override {
        options->fGpuPathRenderers = GpuPathRenderers::kNone;
        options->fAllowPathMaskCaching = true;
    }

private:
    using INHERITED = GM;
};

DEF_GM( return new PathMaskCache(); )
