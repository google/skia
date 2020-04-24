/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGGeometryTransform_DEFINED
#define SkSGGeometryTransform_DEFINED

#include "modules/sksg/include/SkSGGeometryNode.h"

#include "include/core/SkPath.h"

class SkMatrix;

namespace sksg {

class Transform;

/**
 * Concrete Effect node, binding a Matrix to a GeometryNode.
 */
class GeometryTransform final : public GeometryNode {
public:
    static sk_sp<GeometryTransform> Make(sk_sp<GeometryNode> child, sk_sp<Transform> transform) {
        return child && transform
            ? sk_sp<GeometryTransform>(new GeometryTransform(std::move(child),
                                                             std::move(transform)))
            : nullptr;
    }

    ~GeometryTransform() override;

    const sk_sp<Transform>& getTransform() const { return fTransform; }

protected:
    void onClip(SkCanvas*, bool antiAlias) const override;
    void onDraw(SkCanvas*, const SkPaint&) const override;
    bool onContains(const SkPoint&)        const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;
    SkPath onAsPath() const override;

private:
    GeometryTransform(sk_sp<GeometryNode>, sk_sp<Transform>);

    const sk_sp<GeometryNode> fChild;
    const sk_sp<Transform>    fTransform;
    SkPath                    fTransformedPath;

    using INHERITED = GeometryNode;
};

}

#endif // SkSGGeometryTransform_DEFINED
