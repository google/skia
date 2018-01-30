/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkanGeometryNode_DEFINED
#define SkanGeometryNode_DEFINED

#include "SkanNode.h"

class SkCanvas;
class SkPaint;
class SkPath;

namespace skan {

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

    SkPath asPath() const;

protected:
    GeometryNode();

    virtual void onClip(SkCanvas*, bool antiAlias) const = 0;

    virtual void onDraw(SkCanvas*, const SkPaint&) const = 0;

    virtual SkPath onAsPath() const = 0;

private:
    friend class Draw; // wants to know the cached bounds.

    typedef Node INHERITED;
};

} // namespace skan

#endif // SkanGeometryNode_DEFINED
