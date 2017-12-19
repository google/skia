/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGPaintNode_DEFINED
#define SkSGPaintNode_DEFINED

#include "SkSGNode.h"

#include "SkPaint.h"

namespace sksg {

/**
 * Base class for nodes which provide a 'paint' (as opposed to geometry) for
 * drawing (e.g. colors, gradients, patterns).
 *
 * Roughly equivalent to Skia's SkPaint.
 */
class PaintNode : public Node {
public:

    const SkPaint& makePaint();

protected:
    PaintNode();

    virtual SkPaint onMakePaint() const = 0;

    void onRevalidate(InvalidationController*, const SkMatrix&) override;

private:
    SkPaint fPaint;

    typedef Node INHERITED;
};

} // namespace sksg

#endif // SkSGGeometryNode_DEFINED
