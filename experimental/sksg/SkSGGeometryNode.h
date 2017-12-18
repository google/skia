/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGGeometryNode_DEFINED
#define SkSGGeometryNode_DEFINED

#include "SkSGNode.h"

#include "SkRect.h"

class SkCanvas;
class SkPaint;

namespace sksg {

class GeometryNode : public Node {
public:
    void draw(SkCanvas*, const SkPaint&) const;

protected:
    GeometryNode();

    virtual void onDraw(SkCanvas*, const SkPaint&) const = 0;

    virtual SkRect onComputeBounds() const = 0;

    void onRevalidate(InvalidationController*) override;

private:
    SkRect fBounds;

    typedef Node INHERITED;
};

} // namespace sksg

#endif // SkSGGeometryNode_DEFINED
