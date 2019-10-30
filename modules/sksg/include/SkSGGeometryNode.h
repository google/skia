/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGGeometryNode_DEFINED
#define SkSGGeometryNode_DEFINED

#include "modules/sksg/include/SkSGNode.h"

class SkCanvas;
class SkPaint;
class SkPath;

namespace sksg {

/**
 * Base class for nodes which provide 'geometry' (as opposed to paint)
 * for drawing.
 *
 * Think SkRect, SkPath, etc.
 */
class GeometryNode : public Node {
public:
    void clip(SkCanvas*, bool antiAlias) const;
    void draw(SkCanvas*, const SkPaint&) const;

    bool contains(const SkPoint&) const;

    SkPath asPath() const;

protected:
    GeometryNode();

    virtual void onClip(SkCanvas*, bool antiAlias) const = 0;

    virtual void onDraw(SkCanvas*, const SkPaint&) const = 0;

    virtual bool onContains(const SkPoint&) const = 0;

    virtual SkPath onAsPath() const = 0;

private:
    friend class Draw; // wants to know the cached bounds.

    typedef Node INHERITED;
};

} // namespace sksg

#endif // SkSGGeometryNode_DEFINED
