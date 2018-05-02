/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGPath_DEFINED
#define SkSGPath_DEFINED

#include "SkSGGeometryNode.h"

#include "SkPath.h"

class SkCanvas;
class SkPaint;

namespace sksg {

/**
 * Concrete Geometry node, wrapping an SkPath.
 */
class Path : public GeometryNode {
public:
    static sk_sp<Path> Make()                { return sk_sp<Path>(new Path(SkPath())); }
    static sk_sp<Path> Make(const SkPath& r) { return sk_sp<Path>(new Path(r)); }

    SG_ATTRIBUTE(Path, SkPath, fPath)

protected:
    void onClip(SkCanvas*, bool antiAlias) const override;
    void onDraw(SkCanvas*, const SkPaint&) const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;
    SkPath onAsPath() const override;

private:
    explicit Path(const SkPath&);

    SkPath fPath;

    using INHERITED = GeometryNode;
};

} // namespace sksg

#endif // SkSGPath_DEFINED
