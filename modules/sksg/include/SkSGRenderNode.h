/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGRenderNode_DEFINED
#define SkSGRenderNode_DEFINED

#include "SkSGNode.h"

class SkCanvas;

namespace sksg {

/**
 * Base class for nodes which can render to a canvas.
 */
class RenderNode : public Node {
public:
    // Render the node and its descendants to the canvas.
    void render(SkCanvas*) const;

protected:
    RenderNode();

    virtual void onRender(SkCanvas*) const = 0;

private:
    typedef Node INHERITED;
};

} // namespace sksg

#endif // SkSGRenderNode_DEFINED
