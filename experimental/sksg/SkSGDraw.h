/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGDraw_DEFINED
#define SkSGDraw_DEFINED

#include "SkSGRenderNode.h"

namespace sksg {

class GeometryNode;
class PaintNode;

/**
 * Concrete rendering node.
 *
 * Wraps and draws a [geometry, paint] tuple.
 *
 * Think Skia SkCanvas::drawFoo(foo, paint) calls.
 */
class Draw : public RenderNode {
public:
    static sk_sp<Draw> Make(sk_sp<GeometryNode> geo, sk_sp<PaintNode> paint) {
        return (geo && paint) ? sk_sp<Draw>(new Draw(std::move(geo), std::move(paint))) : nullptr;
    }

protected:
    Draw(sk_sp<GeometryNode>, sk_sp<PaintNode> paint);
    ~Draw() override;

    void onRender(SkCanvas*) const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;

private:
    sk_sp<GeometryNode> fGeometry;
    sk_sp<PaintNode>    fPaint;

    typedef RenderNode INHERITED;
};

} // namespace sksg

#endif // SkSGDraw_DEFINED
