/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGStroke_DEFINED
#define SkSGStroke_DEFINED

#include "SkSGPaintNode.h"

#include "SkScalar.h"

namespace sksg {

/**
 * Concrete Paint node wrapper, applying a stroke effect to an existing paint.
 */
class Stroke final : public PaintNode {
public:
    static sk_sp<Stroke> Make(sk_sp<PaintNode> paint) {
        return paint ? sk_sp<Stroke>(new Stroke(std::move(paint))) : nullptr;
    }

    SG_ATTRIBUTE(StrokeWidth, SkScalar     , fStrokeWidth)
    SG_ATTRIBUTE(StrokeMiter, SkScalar     , fStrokeMiter)
    SG_ATTRIBUTE(StrokeJoin , SkPaint::Join, fStrokeJoin )
    SG_ATTRIBUTE(StrokeCap  , SkPaint::Cap , fStrokeCap  )

protected:
    void onRevalidate(InvalidationController*, const SkMatrix&) override;

    SkPaint onMakePaint() const override;

private:
    explicit Stroke(sk_sp<PaintNode>);
    ~Stroke() override;

    sk_sp<PaintNode> fPaint;
    SkScalar         fStrokeWidth = 1,
                     fStrokeMiter = 4;
    SkPaint::Join    fStrokeJoin  = SkPaint::kMiter_Join;
    SkPaint::Cap     fStrokeCap   = SkPaint::kButt_Cap;

    typedef PaintNode INHERITED;
};

} // namespace sksg

#endif // SkSGStroke_DEFINED
