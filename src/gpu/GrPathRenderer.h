/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathRenderer_DEFINED
#define GrPathRenderer_DEFINED

#include "GrDrawTarget.h"
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
    enum StencilSupport {
        kNoSupport_StencilSupport,
        kStencilOnly_StencilSupport,
        kNoRestriction_StencilSupport,
    };

    /**
     * This function is to get the stencil support for a particular path. The path's fill must
     * not be an inverse type.
     *
     * @param path      the path that will be drawn
     * @param stroke    the stroke information (width, join, cap).
     */
    StencilSupport getStencilSupport(const SkPath& path, const GrStrokeInfo& stroke) const {
        SkASSERT(!path.isInverseFillType());
        return this->onGetStencilSupport(path, stroke);
    }

    /** Args to canDrawPath()
     *
     * fShaderCaps       The shader caps
     * fPipelineBuilder  The pipelineBuilder
     * fViewMatrix       The viewMatrix
     * fPath             The path to draw
     * fStroke           The stroke information (width, join, cap)
     * fAntiAlias        True if anti-aliasing is required.
     */
    struct CanDrawPathArgs {
        const GrShaderCaps*         fShaderCaps;
        const SkMatrix*             fViewMatrix;
        const SkPath*               fPath;
        const GrStrokeInfo*         fStroke;
        bool                        fAntiAlias;

        // These next two are only used by GrStencilAndCoverPathRenderer
        bool                        fIsStencilDisabled;
        bool                        fIsStencilBufferMSAA;

        void validate() const {
            SkASSERT(fShaderCaps);
            SkASSERT(fViewMatrix);
            SkASSERT(fPath);
            SkASSERT(fStroke);
            SkASSERT(!fPath->isEmpty());
        }
    };

    /**
     * Returns true if this path renderer is able to render the path. Returning false allows the
     * caller to fallback to another path renderer This function is called when searching for a path
     * renderer capable of rendering a path.
     *
     * @return  true if the path can be drawn by this object, false otherwise.
     */
    bool canDrawPath(const CanDrawPathArgs& args) const {
        SkDEBUGCODE(args.validate();)
        return this->onCanDrawPath(args);
    }

    /**
     * Args to drawPath()
     *
     * fTarget                The target that the path will be rendered to
     * fResourceProvider      The resource provider for creating gpu resources to render the path
     * fPipelineBuilder       The pipelineBuilder
     * fColor                 Color to render with
     * fViewMatrix            The viewMatrix
     * fPath                  the path to draw.
     * fStroke                the stroke information (width, join, cap)
     * fAntiAlias             true if anti-aliasing is required.
     */
    struct DrawPathArgs {
        GrDrawTarget*               fTarget;
        GrResourceProvider*         fResourceProvider;
        GrPipelineBuilder*          fPipelineBuilder;
        GrColor                     fColor;
        const SkMatrix*             fViewMatrix;
        const SkPath*               fPath;
        const GrStrokeInfo*         fStroke;
        bool                        fAntiAlias;

        void validate() const {
            SkASSERT(fTarget);
            SkASSERT(fResourceProvider);
            SkASSERT(fPipelineBuilder);
            SkASSERT(fViewMatrix);
            SkASSERT(fPath);
            SkASSERT(fStroke);
            SkASSERT(!fPath->isEmpty());
        }
    };

    /**
     * Draws the path into the draw target. If getStencilSupport() would return kNoRestriction then
     * the subclass must respect the stencil settings of the GrPipelineBuilder.
     */
    bool drawPath(const DrawPathArgs& args) {
        SkDEBUGCODE(args.validate();)
#ifdef SK_DEBUG
        CanDrawPathArgs canArgs;
        canArgs.fShaderCaps = args.fTarget->caps()->shaderCaps();
        canArgs.fViewMatrix = args.fViewMatrix;
        canArgs.fPath = args.fPath;
        canArgs.fStroke = args.fStroke;
        canArgs.fAntiAlias = args.fAntiAlias;

        canArgs.fIsStencilDisabled = args.fPipelineBuilder->getStencil().isDisabled();
        canArgs.fIsStencilBufferMSAA =
                          args.fPipelineBuilder->getRenderTarget()->isStencilBufferMultisampled();
        SkASSERT(this->canDrawPath(canArgs));
        SkASSERT(args.fPipelineBuilder->getStencil().isDisabled() ||
                 kNoRestriction_StencilSupport == this->getStencilSupport(*args.fPath,
                                                                          *args.fStroke));
#endif
        return this->onDrawPath(args);
    }

    /* Args to stencilPath().
     *
     * fTarget                The target that the path will be rendered to.
     * fResourceProvider      The resource provider for creating gpu resources to render the path
     * fPipelineBuilder       The pipeline builder.
     * fViewMatrix            Matrix applied to the path.
     * fPath                  The path to draw.
     * fStroke                The stroke information (width, join, cap)
     */
    struct StencilPathArgs {
        GrDrawTarget*       fTarget;
        GrResourceProvider* fResourceProvider;
        GrPipelineBuilder*  fPipelineBuilder;
        const SkMatrix*     fViewMatrix;
        const SkPath*       fPath;
        const GrStrokeInfo* fStroke;

        void validate() const {
            SkASSERT(fTarget);
            SkASSERT(fResourceProvider);
            SkASSERT(fPipelineBuilder);
            SkASSERT(fViewMatrix);
            SkASSERT(fPath);
            SkASSERT(fStroke);
            SkASSERT(!fPath->isEmpty());
        }
    };

    /**
     * Draws the path to the stencil buffer. Assume the writable stencil bits are already
     * initialized to zero. The pixels inside the path will have non-zero stencil values afterwards.
     *
     */
    void stencilPath(const StencilPathArgs& args) {
        SkDEBUGCODE(args.validate();)
        SkASSERT(kNoSupport_StencilSupport != this->getStencilSupport(*args.fPath, *args.fStroke));

        this->onStencilPath(args);
    }

    // Helper for determining if we can treat a thin stroke as a hairline w/ coverage.
    // If we can, we draw lots faster (raster device does this same test).
    static bool IsStrokeHairlineOrEquivalent(const GrStrokeInfo& stroke, const SkMatrix& matrix,
                                             SkScalar* outCoverage) {
        if (stroke.isDashed()) {
            return false;
        }
        if (stroke.isHairlineStyle()) {
            if (outCoverage) {
                *outCoverage = SK_Scalar1;
            }
            return true;
        }
        return stroke.getStyle() == SkStrokeRec::kStroke_Style &&
            SkDrawTreatAAStrokeAsHairline(stroke.getWidth(), matrix, outCoverage);
    }

protected:
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
    /**
     * Subclass overrides if it has any limitations of stenciling support.
     */
    virtual StencilSupport onGetStencilSupport(const SkPath&, const GrStrokeInfo&) const {
        return kNoRestriction_StencilSupport;
    }

    /**
     * Subclass implementation of drawPath()
     */
    virtual bool onDrawPath(const DrawPathArgs& args) = 0;

    /**
     * Subclass implementation of canDrawPath()
     */
    virtual bool onCanDrawPath(const CanDrawPathArgs& args) const = 0;

    /**
     * Subclass implementation of stencilPath(). Subclass must override iff it ever returns
     * kStencilOnly in onGetStencilSupport().
     */
    virtual void onStencilPath(const StencilPathArgs& args) {
        GR_STATIC_CONST_SAME_STENCIL(kIncrementStencil,
                                     kReplace_StencilOp,
                                     kReplace_StencilOp,
                                     kAlways_StencilFunc,
                                     0xffff,
                                     0xffff,
                                     0xffff);
        args.fPipelineBuilder->setStencil(kIncrementStencil);
        args.fPipelineBuilder->setDisableColorXPFactory();
        DrawPathArgs drawArgs;
        drawArgs.fTarget = args.fTarget;
        drawArgs.fResourceProvider = args.fResourceProvider;
        drawArgs.fPipelineBuilder = args.fPipelineBuilder;
        drawArgs.fColor = 0xFFFFFFFF;
        drawArgs.fViewMatrix = args.fViewMatrix;
        drawArgs.fPath = args.fPath;
        drawArgs.fStroke = args.fStroke;
        drawArgs.fAntiAlias = false;
        this->drawPath(drawArgs);
    }


    typedef SkRefCnt INHERITED;
};

#endif
