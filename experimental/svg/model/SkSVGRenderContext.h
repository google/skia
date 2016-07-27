/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGRenderContext_DEFINED
#define SkSVGRenderContext_DEFINED

#include "SkPaint.h"
#include "SkTLazy.h"

class SkPaint;

class SkSVGRenderContext {
public:
    SkSVGRenderContext();
    SkSVGRenderContext(const SkSVGRenderContext&) = default;
    SkSVGRenderContext& operator=(const SkSVGRenderContext&);

    const SkPaint* fillPaint() const { return fFill.getMaybeNull(); }
    const SkPaint* strokePaint() const { return fStroke.getMaybeNull(); }

    void setFillColor(SkColor);
    void setStrokeColor(SkColor);

private:
    SkPaint& ensureFill();
    SkPaint& ensureStroke();

    SkTLazy<SkPaint> fFill;
    SkTLazy<SkPaint> fStroke;
};

#endif // SkSVGRenderContext_DEFINED
