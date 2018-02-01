/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGRect_DEFINED
#define SkSGRect_DEFINED

#include "SkSGGeometryNode.h"

#include "SkRect.h"
#include "SkRRect.h"

class SkCanvas;
class SkPaint;

namespace sksg {

/**
 * Concrete Geometry node, wrapping an SkRect.
 */
class Rect final : public GeometryNode {
public:
    static sk_sp<Rect> Make()                { return sk_sp<Rect>(new Rect(SkRect::MakeEmpty())); }
    static sk_sp<Rect> Make(const SkRect& r) { return sk_sp<Rect>(new Rect(r)); }

    SG_ATTRIBUTE(L, SkScalar, fRect.fLeft  )
    SG_ATTRIBUTE(T, SkScalar, fRect.fTop   )
    SG_ATTRIBUTE(R, SkScalar, fRect.fRight )
    SG_ATTRIBUTE(B, SkScalar, fRect.fBottom)

protected:
    void onClip(SkCanvas*, bool antiAlias) const override;
    void onDraw(SkCanvas*, const SkPaint&) const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;
    SkPath onAsPath() const override;

private:
    explicit Rect(const SkRect&);

    SkRect fRect;

    using INHERITED = GeometryNode;
};

/**
 * Concrete Geometry node, wrapping an SkRRect.
 */
class RRect final : public GeometryNode {
public:
    static sk_sp<RRect> Make()                  { return sk_sp<RRect>(new RRect(SkRRect())); }
    static sk_sp<RRect> Make(const SkRRect& rr) { return sk_sp<RRect>(new RRect(rr)); }

    SG_ATTRIBUTE(RRect, SkRRect, fRRect)

protected:
    void onClip(SkCanvas*, bool antiAlias) const override;
    void onDraw(SkCanvas*, const SkPaint&) const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;
    SkPath onAsPath() const override;

private:
    explicit RRect(const SkRRect&);

    SkRRect fRRect;

    using INHERITED = GeometryNode;
};

} // namespace sksg

#endif // SkSGRect_DEFINED
