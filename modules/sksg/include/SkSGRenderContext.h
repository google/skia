/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGRenderContext_DEFINED
#define SkSGRenderContext_DEFINED

#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkTLazy.h"

#include <cstddef>

class SkColorFilter;

namespace sksg {

class RenderContext final {
public:
    explicit RenderContext(SkCanvas* canvas);
    ~RenderContext();

    RenderContext makeLocal() const;

    SkCanvas* canvas() const { return fCanvas; }

    void applyOpacity(SkScalar o);
    void applyColorFilter(sk_sp<SkColorFilter>);

    bool modulateLayer(const SkRect& bounds);

    SkTCopyOnFirstWrite<SkPaint> modulatePaint(const SkPaint&) const;

private:
    SkCanvas* fCanvas;
    int       fRestoreCount;

    // Paint modulators.
    sk_sp<SkColorFilter> fColorFilter;
    SkScalar             fOpacity = 1;
};

} // namespace sksg

#endif // SkSGRenderContext_DEFINED
