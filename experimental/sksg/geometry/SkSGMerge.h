/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGMerge_DEFINED
#define SkSGMerge_DEFINED

#include "SkSGGeometryNode.h"

#include "SkPath.h"

#include <vector>

class SkCanvas;
class SkPaint;

namespace sksg {

/**
 * Concrete Geometry node, combining other geometries based on Mode.
 */
class Merge final : public GeometryNode {
public:
    enum class Mode {
        // Append path mode.
        kMerge,

        // SkPathOp ops.
        kUnion,
        kIntersect,
        kDifference,
        kReverseDifference,
        kXOR,
    };

    static sk_sp<Merge> Make(std::vector<sk_sp<GeometryNode>>&& geos, Mode mode) {
        return sk_sp<Merge>(new Merge(std::move(geos), mode));
    }

    ~Merge() override;

protected:
    void onDraw(SkCanvas*, const SkPaint&) const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;
    SkPath onAsPath() const override;

private:
    Merge(std::vector<sk_sp<GeometryNode>>&& geos, Mode);

    std::vector<sk_sp<GeometryNode>> fGeos;
    SkPath                           fMerged;
    Mode                             fMode;
};

} // namespace sksg

#endif // SkSGMerge_DEFINED
