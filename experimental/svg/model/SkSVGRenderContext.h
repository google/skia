/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGRenderContext_DEFINED
#define SkSVGRenderContext_DEFINED

#include "SkPaint.h"
#include "SkRect.h"
#include "SkSize.h"
#include "SkTLazy.h"
#include "SkTypes.h"

class SkCanvas;
class SkSVGLength;

class SkSVGLengthContext {
public:
    SkSVGLengthContext(const SkSize& viewport) : fViewport(viewport) {}

    enum class LengthType {
        kHorizontal,
        kVertical,
        kOther,
    };

    const SkSize& viewPort() const { return fViewport; }
    void setViewPort(const SkSize& viewport) { fViewport = viewport; }

    SkScalar resolve(const SkSVGLength&, LengthType) const;
    SkRect   resolveRect(const SkSVGLength& x, const SkSVGLength& y,
                         const SkSVGLength& w, const SkSVGLength& h) const;

private:
    SkSize fViewport;
};

class SkSVGPresentationContext {
public:
    SkSVGPresentationContext();
    SkSVGPresentationContext(const SkSVGPresentationContext&);
    SkSVGPresentationContext& operator=(const SkSVGPresentationContext&);

    const SkPaint* fillPaint() const { return fFill.getMaybeNull(); }
    const SkPaint* strokePaint() const { return fStroke.getMaybeNull(); }

    void setFillColor(SkColor);
    void setStrokeColor(SkColor);

private:
    void initFrom(const SkSVGPresentationContext&);

    SkPaint& ensureFill();
    SkPaint& ensureStroke();

    // TODO: convert to regular SkPaints and track explicit attribute values instead.
    SkTLazy<SkPaint> fFill;
    SkTLazy<SkPaint> fStroke;
};

class SkSVGRenderContext {
public:
    SkSVGRenderContext(SkCanvas*, const SkSVGLengthContext&, const SkSVGPresentationContext&);
    SkSVGRenderContext(const SkSVGRenderContext&);
    ~SkSVGRenderContext();

    const SkSVGLengthContext& lengthContext() const { return *fLengthContext; }
    SkSVGLengthContext* writableLengthContext() { return fLengthContext.writable(); }

    const SkSVGPresentationContext& presentationContext() const { return *fPresentationContext; }
    SkSVGPresentationContext* writablePresentationContext() {
        return fPresentationContext.writable();
    }

    SkCanvas* canvas() const { return fCanvas; }

private:
    // Stack-only
    void* operator new(size_t)                               = delete;
    void* operator new(size_t, void*)                        = delete;
    SkSVGRenderContext& operator=(const SkSVGRenderContext&) = delete;

    SkTCopyOnFirstWrite<SkSVGLengthContext>       fLengthContext;
    SkTCopyOnFirstWrite<SkSVGPresentationContext> fPresentationContext;
    SkCanvas*                                     fCanvas;
    // The save count on 'fCanvas' at construction time.
    // A restoreToCount() will be issued on destruction.
    int                                           fCanvasSaveCount;
};

#endif // SkSVGRenderContext_DEFINED
