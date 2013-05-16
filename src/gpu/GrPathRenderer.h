
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrPathRenderer_DEFINED
#define GrPathRenderer_DEFINED

#include "GrDrawTarget.h"
#include "GrPathRendererChain.h"
#include "GrStencil.h"

#include "SkStrokeRec.h"
#include "SkTArray.h"

class SkPath;

struct GrPoint;

/**
 *  Base class for drawing paths into a GrDrawTarget.
 *
 *  Derived classes can use stages GrPaint::kTotalStages through GrDrawState::kNumStages-1. The
 *  stages before GrPaint::kTotalStages are reserved for setting up the draw (i.e., textures and
 *  filter masks).
 */
class GR_API GrPathRenderer : public GrRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrPathRenderer)

    /**
     * This is called to install custom path renderers in every GrContext at create time. The
     * default implementation in GrCreatePathRenderer_none.cpp does not add any additional
     * renderers. Link against another implementation to install your own. The first added is the
     * most preferred path renderer, second is second most preferred, etc.
     *
     * @param context   the context that will use the path renderer
     * @param prChain   the chain to add path renderers to.
     */
    static void AddPathRenderers(GrContext* context, GrPathRendererChain* prChain);


    GrPathRenderer();

    /**
     * A caller may wish to use a path renderer to draw a path into the stencil buffer. However,
     * the path renderer itself may require use of the stencil buffer. Also a path renderer may
     * use a GrEffect coverage stage that sets coverage to zero to eliminate pixels that are covered
     * by bounding geometry but outside the path. These exterior pixels would still be rendered into
     * the stencil.
     *
     * A GrPathRenderer can provide three levels of support for stenciling paths:
     * 1) kNoRestriction: This is the most general. The caller sets up the GrDrawState on the target
     *                    and calls drawPath(). The path is rendered exactly as the draw state
     *                    indicates including support for simultaneous color and stenciling with
     *                    arbitrary stenciling rules. Pixels partially covered by AA paths are
     *                    affected by the stencil settings.
     * 2) kStencilOnly: The path renderer cannot apply arbitrary stencil rules nor shade and stencil
     *                  simultaneously. The path renderer does support the stencilPath() function
     *                  which performs no color writes and writes a non-zero stencil value to pixels
     *                  covered by the path.
     * 3) kNoSupport: This path renderer cannot be used to stencil the path.
     */
    typedef GrPathRendererChain::StencilSupport StencilSupport;
    static const StencilSupport kNoSupport_StencilSupport =
        GrPathRendererChain::kNoSupport_StencilSupport;
    static const StencilSupport kStencilOnly_StencilSupport =
        GrPathRendererChain::kStencilOnly_StencilSupport;
    static const StencilSupport kNoRestriction_StencilSupport =
        GrPathRendererChain::kNoRestriction_StencilSupport;

    /**
     * This function is to get the stencil support for a particular path. The path's fill must
     * not be an inverse type.
     *
     * @param target    target that the path will be rendered to
     * @param path      the path that will be drawn
     * @param stroke    the stroke information (width, join, cap).
     */
    StencilSupport getStencilSupport(const SkPath& path,
                                     const SkStrokeRec& stroke,
                                     const GrDrawTarget* target) const {
        GrAssert(!path.isInverseFillType());
        return this->onGetStencilSupport(path, stroke, target);
    }

    /**
     * Returns true if this path renderer is able to render the path. Returning false allows the
     * caller to fallback to another path renderer This function is called when searching for a path
     * renderer capable of rendering a path.
     *
     * @param path       The path to draw
     * @param stroke     The stroke information (width, join, cap)
     * @param target     The target that the path will be rendered to
     * @param antiAlias  True if anti-aliasing is required.
     *
     * @return  true if the path can be drawn by this object, false otherwise.
     */
    virtual bool canDrawPath(const SkPath& path,
                             const SkStrokeRec& rec,
                             const GrDrawTarget* target,
                             bool antiAlias) const = 0;
    /**
     * Draws the path into the draw target. If getStencilSupport() would return kNoRestriction then
     * the subclass must respect the stencil settings of the target's draw state.
     *
     * @param path                  the path to draw.
     * @param stroke                the stroke information (width, join, cap)
     * @param target                target that the path will be rendered to
     * @param antiAlias             true if anti-aliasing is required.
     */
    bool drawPath(const SkPath& path,
                  const SkStrokeRec& stroke,
                  GrDrawTarget* target,
                  bool antiAlias) {
        GrAssert(this->canDrawPath(path, stroke, target, antiAlias));
        GrAssert(target->drawState()->getStencil().isDisabled() ||
                 kNoRestriction_StencilSupport == this->getStencilSupport(path, stroke, target));
        return this->onDrawPath(path, stroke, target, antiAlias);
    }

    /**
     * Draws the path to the stencil buffer. Assume the writable stencil bits are already
     * initialized to zero. The pixels inside the path will have non-zero stencil values afterwards.
     *
     * @param path                  the path to draw.
     * @param stroke                the stroke information (width, join, cap)
     * @param target                target that the path will be rendered to
     */
    void stencilPath(const SkPath& path, const SkStrokeRec& stroke, GrDrawTarget* target) {
        GrAssert(kNoSupport_StencilSupport != this->getStencilSupport(path, stroke, target));
        this->onStencilPath(path, stroke, target);
    }

protected:
    /**
     * Subclass overrides if it has any limitations of stenciling support.
     */
    virtual StencilSupport onGetStencilSupport(const SkPath&,
                                               const SkStrokeRec&,
                                               const GrDrawTarget*) const {
        return kNoRestriction_StencilSupport;
    }

    /**
     * Subclass implementation of drawPath()
     */
    virtual bool onDrawPath(const SkPath& path,
                            const SkStrokeRec& stroke,
                            GrDrawTarget* target,
                            bool antiAlias) = 0;

    /**
     * Subclass implementation of stencilPath(). Subclass must override iff it ever returns
     * kStencilOnly in onGetStencilSupport().
     */
    virtual void onStencilPath(const SkPath& path,  const SkStrokeRec& stroke, GrDrawTarget* target) {
        GrDrawTarget::AutoStateRestore asr(target, GrDrawTarget::kPreserve_ASRInit);
        GrDrawState* drawState = target->drawState();
        GR_STATIC_CONST_SAME_STENCIL(kIncrementStencil,
                                     kReplace_StencilOp,
                                     kReplace_StencilOp,
                                     kAlways_StencilFunc,
                                     0xffff,
                                     0xffff,
                                     0xffff);
        drawState->setStencil(kIncrementStencil);
        drawState->enableState(GrDrawState::kNoColorWrites_StateBit);
        this->drawPath(path, stroke, target, false);
    }

private:

    typedef GrRefCnt INHERITED;
};

#endif
