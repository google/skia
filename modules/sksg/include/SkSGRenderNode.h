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

class RenderContext;

/**
 * Base class for nodes which can render to a canvas.
 */
class RenderNode : public Node {
public:
    // Render the node and its descendants to the canvas.
    void render(const RenderContext&) const;

protected:
    RenderNode();

    virtual void onRender(const RenderContext&) const = 0;

private:
    typedef Node INHERITED;
};

} // namespace sksg

#endif // SkSGRenderNode_DEFINED
