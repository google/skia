/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGRenderContext_DEFINED
#define SkSVGRenderContext_DEFINED

#include "experimental/svg/model/SkSVGAttribute.h"
#include "experimental/svg/model/SkSVGIDMapper.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "src/core/SkTLazy.h"

class SkCanvas;
class SkSVGLength;

class SkSVGLengthContext {
public:
    SkSVGLengthContext(const SkSize& viewport, SkScalar dpi = 90)
        : fViewport(viewport), fDPI(dpi) {}

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
    SkSize   fViewport;
    SkScalar fDPI;
};

struct SkSVGPresentationContext {
    SkSVGPresentationContext();
    SkSVGPresentationContext(const SkSVGPresentationContext&)            = default;
    SkSVGPresentationContext& operator=(const SkSVGPresentationContext&) = default;

    // Inherited presentation attributes, computed for the current node.
    SkSVGPresentationAttributes fInherited;

    // Cached paints, reflecting the current presentation attributes.
    SkPaint fFillPaint;
    SkPaint fStrokePaint;
};

class SkSVGRenderContext {
public:
    SkSVGRenderContext(SkCanvas*, const SkSVGIDMapper&, const SkSVGLengthContext&,
                       const SkSVGPresentationContext&);
    SkSVGRenderContext(const SkSVGRenderContext&);
    SkSVGRenderContext(const SkSVGRenderContext&, SkCanvas*);
    ~SkSVGRenderContext();

    const SkSVGLengthContext& lengthContext() const { return *fLengthContext; }
    SkSVGLengthContext* writableLengthContext() { return fLengthContext.writable(); }

    const SkSVGPresentationContext& presentationContext() const { return *fPresentationContext; }

    SkCanvas* canvas() const { return fCanvas; }
    void saveOnce();

    enum ApplyFlags {
        kLeaf = 1 << 0, // the target node doesn't have descendants
    };
    void applyPresentationAttributes(const SkSVGPresentationAttributes&, uint32_t flags);

    const SkSVGNode* findNodeById(const SkString&) const;

    const SkPaint* fillPaint() const;
    const SkPaint* strokePaint() const;

    // The local computed clip path (not inherited).
    const SkPath* clipPath() const { return fClipPath.getMaybeNull(); }

private:
    // Stack-only
    void* operator new(size_t)                               = delete;
    void* operator new(size_t, void*)                        = delete;
    SkSVGRenderContext& operator=(const SkSVGRenderContext&) = delete;

    void applyOpacity(SkScalar opacity, uint32_t flags);
    void applyClip(const SkSVGClip&);

    const SkSVGIDMapper&                          fIDMapper;
    SkTCopyOnFirstWrite<SkSVGLengthContext>       fLengthContext;
    SkTCopyOnFirstWrite<SkSVGPresentationContext> fPresentationContext;
    SkCanvas*                                     fCanvas;
    // The save count on 'fCanvas' at construction time.
    // A restoreToCount() will be issued on destruction.
    int                                           fCanvasSaveCount;

    // clipPath, if present for the current context (not inherited).
    SkTLazy<SkPath>                               fClipPath;
};

#endif // SkSVGRenderContext_DEFINED
