/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGRenderContext_DEFINED
#define SkSVGRenderContext_DEFINED

#include "SkSize.h"
#include "SkPaint.h"
#include "SkTLazy.h"

class SkPaint;
class SkSVGLength;

class SkSVGLengthContext {
public:
    SkSVGLengthContext(const SkSize& viewport) : fViewport(viewport) {}

    enum class LengthType {
        kHorizontal,
        kVertical,
        kOther,
    };

    void setViewPort(const SkSize& viewport) { fViewport = viewport; }

    SkScalar resolve(const SkSVGLength&, LengthType) const;

private:
    SkSize fViewport;
};

class SkSVGRenderContext {
public:
    explicit SkSVGRenderContext(const SkSize& initialViewport);
    SkSVGRenderContext(const SkSVGRenderContext&) = default;
    SkSVGRenderContext& operator=(const SkSVGRenderContext&);

    const SkSVGLengthContext& lengthContext() const { return fLengthContext; }

    const SkPaint* fillPaint() const { return fFill.getMaybeNull(); }
    const SkPaint* strokePaint() const { return fStroke.getMaybeNull(); }

    void setFillColor(SkColor);
    void setStrokeColor(SkColor);

private:
    SkPaint& ensureFill();
    SkPaint& ensureStroke();

    SkSVGLengthContext fLengthContext;
    SkTLazy<SkPaint>   fFill;
    SkTLazy<SkPaint>   fStroke;
};

#endif // SkSVGRenderContext_DEFINED
