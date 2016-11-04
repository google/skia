/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawContextPriv_DEFINED
#define GrDrawContextPriv_DEFINED

#include "GrDrawContext.h"
#include "GrDrawTarget.h"
#include "GrPathRendering.h"

class GrFixedClip;
class GrPath;
struct GrUserStencilSettings;

/** Class that adds methods to GrDrawContext that are only intended for use internal to Skia.
    This class is purely a privileged window into GrDrawContext. It should never have additional
    data members or virtual methods. */
class GrDrawContextPriv {
public:
    gr_instanced::InstancedRendering* accessInstancedRendering() const {
        return fDrawContext->getDrawTarget()->instancedRendering();
    }

    void clear(const GrFixedClip&, const GrColor, bool canIgnoreClip);

    void clearStencilClip(const GrFixedClip&, bool insideStencilMask);

    void stencilRect(const GrClip& clip,
                     const GrUserStencilSettings* ss,
                     bool useHWAA,
                     const SkMatrix& viewMatrix,
                     const SkRect& rect);

    void stencilPath(const GrClip&,
                     bool useHWAA,
                     const SkMatrix& viewMatrix,
                     const GrPath*);

    bool drawAndStencilRect(const GrClip&,
                            const GrUserStencilSettings*,
                            SkRegion::Op op,
                            bool invert,
                            bool doAA,
                            const SkMatrix& viewMatrix,
                            const SkRect&);

    bool drawAndStencilPath(const GrClip&,
                            const GrUserStencilSettings*,
                            SkRegion::Op op,
                            bool invert,
                            bool doAA,
                            const SkMatrix& viewMatrix,
                            const SkPath&);

    SkBudgeted isBudgeted() const;

    void testingOnly_drawBatch(const GrPaint&,
                               GrDrawBatch* batch,
                               const GrUserStencilSettings* = nullptr,
                               bool snapToCenters = false);

private:
    explicit GrDrawContextPriv(GrDrawContext* drawContext) : fDrawContext(drawContext) {}
    GrDrawContextPriv(const GrRenderTargetPriv&) {} // unimpl
    GrDrawContextPriv& operator=(const GrRenderTargetPriv&); // unimpl

    // No taking addresses of this type.
    const GrDrawContextPriv* operator&() const;
    GrDrawContextPriv* operator&();

    GrDrawContext* fDrawContext;

    friend class GrDrawContext; // to construct/copy this type.
};

inline GrDrawContextPriv GrDrawContext::drawContextPriv() { return GrDrawContextPriv(this); }

inline const GrDrawContextPriv GrDrawContext::drawContextPriv () const {
    return GrDrawContextPriv(const_cast<GrDrawContext*>(this));
}

#endif
