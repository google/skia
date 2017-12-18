/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGRenderNode_DEFINED
#define SkSGRenderNode_DEFINED

#include "SkSGNode.h"

#include "SkRect.h"

class SkCanvas;

namespace sksg {

class RenderNode : public Node {
public:
    void render(SkCanvas*) const;

    const SkRect& getBounds();

protected:
    RenderNode();

    virtual void onRender(SkCanvas*) const = 0;

    virtual SkRect onComputeBounds() const = 0;

private:
    SkRect fBounds;
};

} // namespace sksg

#endif // SkSGRenderNode_DEFINED
