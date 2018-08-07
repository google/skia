/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGRenderContext_DEFINED
#define SkSGRenderContext_DEFINED

#include "SkCanvas.h"
#include "SkTLazy.h"

#include <cstddef>

namespace sksg {

class RenderContext final {
public:
    explicit RenderContext(SkCanvas* canvas);
    ~RenderContext();

    RenderContext makeLocal() const;

    SkCanvas* canvas() const { return fCanvas; }

    void applyOpacity(SkScalar o);

    bool modulateLayer(const SkRect& bounds);

    SkTCopyOnFirstWrite<SkPaint> modulatePaint(const SkPaint&) const;

private:
    SkCanvas* fCanvas;
    int       fRestoreCount;

    // Paint modulators.
    SkScalar fOpacity = 1;
};

} // namespace sksg

#endif // SkSGRenderContext_DEFINED
