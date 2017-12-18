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

    // TODO: this duplicates RenderNode bounds functionality... move to common base?
    const SkRect& getBounds();

protected:
    GeometryNode();

    virtual void onDraw(SkCanvas*, const SkPaint&) const = 0;

    virtual SkRect onComputeBounds() const = 0;

private:
     SkRect fBounds;
};

} // namespace sksg

#endif // SkSGGeometryNode_DEFINED
