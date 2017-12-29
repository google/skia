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

/**
 * Base class for nodes which provide 'geometry' (as opposed to paint)
 * for drawing.
 *
 * Think SkRect, SkPath, etc.
 */
class GeometryNode : public Node {
public:
    void draw(SkCanvas*, const SkPaint&) const;

    // SkPath asPath() const;  // unused for now

protected:
    GeometryNode();

    virtual void onDraw(SkCanvas*, const SkPaint&) const = 0;

    virtual SkRect onComputeBounds() const = 0;

    // virtual SkPath onAsPath() const = 0; // unused for now

    void onRevalidate(InvalidationController*, const SkMatrix&) override;

private:
    friend class Draw; // wants to know the cached bounds.

    SkRect fBounds;

    typedef Node INHERITED;
};

} // namespace sksg

#endif // SkSGGeometryNode_DEFINED
