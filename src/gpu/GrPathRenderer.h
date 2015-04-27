
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
#include "GrStrokeInfo.h"

#include "SkDrawProcs.h"
#include "SkTArray.h"

class SkPath;

struct GrPoint;

/**
 *  Base class for drawing paths into a GrDrawTarget.
 *
 *  Derived classes can use stages GrPaint::kTotalStages through GrPipelineBuilder::kNumStages-1.
 *  The stages before GrPaint::kTotalStages are reserved for setting up the draw (i.e., textures and
 *  filter masks).
 */
class SK_API GrPathRenderer : public SkRefCnt {
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
     * use a GrProcessor coverage stage that sets coverage to zero to eliminate pixels that are
     * covered by bounding geometry but outside the path. These exterior pixels would still be
     * rendered into the stencil.
     *
     * A GrPathRenderer can provide three levels of support for stenciling paths:
     * 1) kNoRestriction: This is the most general. The caller sets up the GrPipelineBuilder on the target
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
    StencilSupport getStencilSupport(const GrDrawTarget* target,
                                     const GrPipelineBuilder* pipelineBuilder,
                                     const SkPath& path,
                                     const GrStrokeInfo& stroke) const {
        SkASSERT(!path.isInverseFillType());
        return this->onGetStencilSupport(target, pipelineBuilder, path, stroke);
    }

    /**
     * Returns true if this path renderer is able to render the path. Returning false allows the
     * caller to fallback to another path renderer This function is called when searching for a path
     * renderer capable of rendering a path.
     *
     * @param target           The target that the path will be rendered to
     * @param pipelineBuilder  The pipelineBuilder
     * @param viewMatrix       The viewMatrix
     * @param path             The path to draw
     * @param stroke           The stroke information (width, join, cap)
     * @param antiAlias        True if anti-aliasing is required.
     *
     * @return  true if the path can be drawn by this object, false otherwise.
     */
    virtual bool canDrawPath(const GrDrawTarget* target,
                             const GrPipelineBuilder* pipelineBuilder,
                             const SkMatrix& viewMatrix,
                             const SkPath& path,
                             const GrStrokeInfo& rec,
                             bool antiAlias) const = 0;
    /**
     * Draws the path into the draw target. If getStencilSupport() would return kNoRestriction then
     * the subclass must respect the stencil settings of the target's draw state.
     *
     * @param target                The target that the path will be rendered to
     * @param pipelineBuilder       The pipelineBuilder
     * @param viewMatrix            The viewMatrix
     * @param path                  the path to draw.
     * @param stroke                the stroke information (width, join, cap)
     * @param antiAlias             true if anti-aliasing is required.
     */
    bool drawPath(GrDrawTarget* target,
                  GrPipelineBuilder* ds,
                  GrColor color,
                  const SkMatrix& viewMatrix,
                  const SkPath& path,
                  const GrStrokeInfo& stroke,
                  bool antiAlias) {
        SkASSERT(!path.isEmpty());
        SkASSERT(this->canDrawPath(target, ds, viewMatrix, path, stroke, antiAlias));
        SkASSERT(ds->getStencil().isDisabled() ||
                 kNoRestriction_StencilSupport == this->getStencilSupport(target, ds, path,
                                                                          stroke));
        return this->onDrawPath(target, ds, color, viewMatrix, path, stroke, antiAlias);
    }

    /**
     * Draws the path to the stencil buffer. Assume the writable stencil bits are already
     * initialized to zero. The pixels inside the path will have non-zero stencil values afterwards.
     *
     * @param path                  the path to draw.
     * @param stroke                the stroke information (width, join, cap)
     * @param target                target that the path will be rendered to
     */
    void stencilPath(GrDrawTarget* target,
                     GrPipelineBuilder* ds,
                     const SkMatrix& viewMatrix,
                     const SkPath& path,
                     const GrStrokeInfo& stroke) {
        SkASSERT(!path.isEmpty());
        SkASSERT(kNoSupport_StencilSupport != this->getStencilSupport(target, ds, path, stroke));
        this->onStencilPath(target, ds, viewMatrix, path, stroke);
    }

    // Helper for determining if we can treat a thin stroke as a hairline w/ coverage.
    // If we can, we draw lots faster (raster device does this same test).
    static bool IsStrokeHairlineOrEquivalent(const GrStrokeInfo& stroke, const SkMatrix& matrix,
                                             SkScalar* outCoverage) {
        if (stroke.isDashed()) {
            return false;
        }
        if (stroke.getStrokeRec().isHairlineStyle()) {
            if (outCoverage) {
                *outCoverage = SK_Scalar1;
            }
            return true;
        }
        return stroke.getStrokeRec().getStyle() == SkStrokeRec::kStroke_Style &&
            SkDrawTreatAAStrokeAsHairline(stroke.getStrokeRec().getWidth(), matrix, outCoverage);
    }

protected:
    /**
     * Subclass overrides if it has any limitations of stenciling support.
     */
    virtual StencilSupport onGetStencilSupport(const GrDrawTarget*,
                                               const GrPipelineBuilder*,
                                               const SkPath&,
                                               const GrStrokeInfo&) const {
        return kNoRestriction_StencilSupport;
    }

    /**
     * Subclass implementation of drawPath()
     */
    virtual bool onDrawPath(GrDrawTarget*,
                            GrPipelineBuilder*,
                            GrColor,
                            const SkMatrix& viewMatrix,
                            const SkPath&,
                            const GrStrokeInfo&,
                            bool antiAlias) = 0;

    /**
     * Subclass implementation of stencilPath(). Subclass must override iff it ever returns
     * kStencilOnly in onGetStencilSupport().
     */
    virtual void onStencilPath(GrDrawTarget* target,
                               GrPipelineBuilder* pipelineBuilder,
                               const SkMatrix& viewMatrix,
                               const SkPath& path,
                               const GrStrokeInfo& stroke) {
        GR_STATIC_CONST_SAME_STENCIL(kIncrementStencil,
                                     kReplace_StencilOp,
                                     kReplace_StencilOp,
                                     kAlways_StencilFunc,
                                     0xffff,
                                     0xffff,
                                     0xffff);
        pipelineBuilder->setStencil(kIncrementStencil);
        pipelineBuilder->setDisableColorXPFactory();
        this->drawPath(target, pipelineBuilder, GrColor_WHITE, viewMatrix, path, stroke, false);
    }

    // Helper for getting the device bounds of a path. Inverse filled paths will have bounds set
    // by devSize. Non-inverse path bounds will not necessarily be clipped to devSize.
    static void GetPathDevBounds(const SkPath& path,
                                 int devW,
                                 int devH,
                                 const SkMatrix& matrix,
                                 SkRect* bounds);

    // Helper version that gets the dev width and height from a GrSurface.
    static void GetPathDevBounds(const SkPath& path,
                                 const GrSurface* device,
                                 const SkMatrix& matrix,
                                 SkRect* bounds) {
        GetPathDevBounds(path, device->width(), device->height(), matrix, bounds);
    }

private:

    typedef SkRefCnt INHERITED;
};

#endif
