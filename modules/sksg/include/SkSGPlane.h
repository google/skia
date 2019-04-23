/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGPlane_DEFINED
#define SkSGPlane_DEFINED

#include "modules/sksg/include/SkSGGeometryNode.h"

class SkCanvas;
class SkPaint;

namespace sksg {

/**
 * Concrete Geometry node, representing the whole canvas.
 */
class Plane final : public GeometryNode {
public:
    static sk_sp<Plane> Make() { return sk_sp<Plane>(new Plane()); }

protected:
    void onClip(SkCanvas*, bool antiAlias) const override;
    void onDraw(SkCanvas*, const SkPaint&) const override;
    bool onContains(const SkPoint&)        const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;
    SkPath onAsPath() const override;

private:
    Plane();

    using INHERITED = GeometryNode;
};

} // namespace sksg

#endif // SkSGPlane_DEFINED
