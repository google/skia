/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGPath_DEFINED
#define SkSGPath_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "modules/sksg/include/SkSGGeometryNode.h"
#include "modules/sksg/include/SkSGNode.h"

class SkCanvas;
class SkMatrix;
class SkPaint;
enum class SkPathFillType;
struct SkPoint;

namespace sksg {
class InvalidationController;

/**
 * Concrete Geometry node, wrapping an SkPath.
 */
class Path : public GeometryNode {
public:
    static sk_sp<Path> Make()                { return sk_sp<Path>(new Path(SkPath())); }
    static sk_sp<Path> Make(const SkPath& r) { return sk_sp<Path>(new Path(r)); }

    SG_ATTRIBUTE(Path, SkPath, fPath)

    // Temporarily inlined for SkPathFillType staging
    // SG_MAPPED_ATTRIBUTE(FillType, SkPathFillType, fPath)

    SkPathFillType getFillType() const {
        return fPath.getFillType();
    }

    void setFillType(SkPathFillType fillType) {
        if (fillType != fPath.getFillType()) {
            fPath.setFillType(fillType);
            this->invalidate();
        }
    }

protected:
    void onClip(SkCanvas*, bool antiAlias) const override;
    void onDraw(SkCanvas*, const SkPaint&) const override;
    bool onContains(const SkPoint&)        const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;
    SkPath onAsPath() const override;

private:
    explicit Path(const SkPath&);

    SkPath fPath;

    using INHERITED = GeometryNode;
};

} // namespace sksg

#endif // SkSGPath_DEFINED
