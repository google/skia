/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathRenderer_DEFINED
#define GrPathRenderer_DEFINED

#include "GrCaps.h"
#include "GrRenderTargetContext.h"
#include "GrPaint.h"
#include "GrShape.h"
#include "GrUserStencilSettings.h"

#include "SkDrawProcs.h"
#include "SkTArray.h"

class SkPath;
class GrFixedClip;
class GrHardClip;
struct GrPoint;

/**
 *  Base class for drawing paths into a GrOpList.
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
     * 1) kNoRestriction: This is the most general. The caller passes a GrPaint and calls drawPath().
     *                    The path is rendered exactly as the draw state indicates including support
     *                    for simultaneous color and stenciling with arbitrary stenciling rules.
     *                    Pixels partially covered by AA paths are affected by the stencil settings.
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
     * not be an inverse type. The path will always be filled and not stroked.
     *
     * @param shape   the shape that will be drawn. Must be simple fill styled and non-inverse
     *                filled.
     */
    StencilSupport getStencilSupport(const GrShape& shape) const {
        SkDEBUGCODE(SkPath path;)
        SkDEBUGCODE(shape.asPath(&path);)
        SkASSERT(shape.style().isSimpleFill());
        SkASSERT(!path.isInverseFillType());
        return this->onGetStencilSupport(shape);
    }

    enum class CanDrawPath {
        kNo,
        kAsBackup, // i.e. This renderer is better than SW fallback if no others can draw the path.
        kYes
    };

    struct CanDrawPathArgs {
        SkDEBUGCODE(CanDrawPathArgs() { memset(this, 0, sizeof(*this)); }) // For validation.

        const GrCaps*               fCaps;
        const SkIRect*              fClipConservativeBounds;
        const SkMatrix*             fViewMatrix;
        const GrShape*              fShape;
        GrAAType                    fAAType;

        // These next two are only used by GrStencilAndCoverPathRenderer
        bool                        fHasUserStencilSettings;

#ifdef SK_DEBUG
        void validate() const {
            SkASSERT(fCaps);
            SkASSERT(fClipConservativeBounds);
            SkASSERT(fViewMatrix);
            SkASSERT(fShape);
        }
#endif
    };

    /**
     * Returns how well this path renderer is able to render the given path. Returning kNo or
     * kAsBackup allows the caller to keep searching for a better path renderer. This function is
     * called when searching for the best path renderer to draw a path.
     */
    CanDrawPath canDrawPath(const CanDrawPathArgs& args) const {
        SkDEBUGCODE(args.validate();)
        return this->onCanDrawPath(args);
    }

    struct DrawPathArgs {
        GrContext*                   fContext;
        GrPaint&&                    fPaint;
        const GrUserStencilSettings* fUserStencilSettings;
        GrRenderTargetContext*       fRenderTargetContext;
        const GrClip*                fClip;
        const SkIRect*               fClipConservativeBounds;
        const SkMatrix*              fViewMatrix;
        const GrShape*               fShape;
        GrAAType                     fAAType;
        bool                         fGammaCorrect;
#ifdef SK_DEBUG
        void validate() const {
            SkASSERT(fContext);
            SkASSERT(fUserStencilSettings);
            SkASSERT(fRenderTargetContext);
            SkASSERT(fClip);
            SkASSERT(fClipConservativeBounds);
            SkASSERT(fViewMatrix);
            SkASSERT(fShape);
        }
#endif
    };

    /**
     * Draws the path into the draw target. If getStencilSupport() would return kNoRestriction then
     * the subclass must respect the stencil settings.
     */
    bool drawPath(const DrawPathArgs& args) {
        SkDEBUGCODE(args.validate();)
#ifdef SK_DEBUG
        CanDrawPathArgs canArgs;
        canArgs.fCaps = args.fContext->caps();
        canArgs.fClipConservativeBounds = args.fClipConservativeBounds;
        canArgs.fViewMatrix = args.fViewMatrix;
        canArgs.fShape = args.fShape;
        canArgs.fAAType = args.fAAType;
        canArgs.validate();

        canArgs.fHasUserStencilSettings = !args.fUserStencilSettings->isUnused();
        SkASSERT(!(canArgs.fAAType == GrAAType::kMSAA &&
                   GrFSAAType::kUnifiedMSAA != args.fRenderTargetContext->fsaaType()));
        SkASSERT(!(canArgs.fAAType == GrAAType::kMixedSamples &&
                   GrFSAAType::kMixedSamples != args.fRenderTargetContext->fsaaType()));
        SkASSERT(CanDrawPath::kNo != this->canDrawPath(canArgs));
        if (!args.fUserStencilSettings->isUnused()) {
            SkPath path;
            args.fShape->asPath(&path);
            SkASSERT(args.fShape->style().isSimpleFill());
            SkASSERT(kNoRestriction_StencilSupport == this->getStencilSupport(*args.fShape));
        }
#endif
        return this->onDrawPath(args);
    }

    /**
     * Args to stencilPath(). fAAType cannot be kCoverage.
     */
    struct StencilPathArgs {
        SkDEBUGCODE(StencilPathArgs() { memset(this, 0, sizeof(*this)); }) // For validation.

        GrContext*             fContext;
        GrRenderTargetContext* fRenderTargetContext;
        const GrHardClip*      fClip;
        const SkIRect*         fClipConservativeBounds;
        const SkMatrix*        fViewMatrix;
        GrAAType               fAAType;
        const GrShape*         fShape;

#ifdef SK_DEBUG
        void validate() const {
            SkASSERT(fContext);
            SkASSERT(fRenderTargetContext);
            SkASSERT(fClipConservativeBounds);
            SkASSERT(fViewMatrix);
            SkASSERT(fShape);
            SkASSERT(fShape->style().isSimpleFill());
            SkASSERT(GrAAType::kCoverage != fAAType);
            SkPath path;
            fShape->asPath(&path);
            SkASSERT(!path.isInverseFillType());
        }
#endif
    };

    /**
     * Draws the path to the stencil buffer. Assume the writable stencil bits are already
     * initialized to zero. The pixels inside the path will have non-zero stencil values afterwards.
     */
    void stencilPath(const StencilPathArgs& args) {
        SkDEBUGCODE(args.validate();)
        SkASSERT(kNoSupport_StencilSupport != this->getStencilSupport(*args.fShape));
        this->onStencilPath(args);
    }

    // Helper for determining if we can treat a thin stroke as a hairline w/ coverage.
    // If we can, we draw lots faster (raster device does this same test).
    static bool IsStrokeHairlineOrEquivalent(const GrStyle& style, const SkMatrix& matrix,
                                             SkScalar* outCoverage) {
        if (style.pathEffect()) {
            return false;
        }
        const SkStrokeRec& stroke = style.strokeRec();
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

private:
    /**
     * Subclass overrides if it has any limitations of stenciling support.
     */
    virtual StencilSupport onGetStencilSupport(const GrShape&) const {
        return kNoRestriction_StencilSupport;
    }

    /**
     * Subclass implementation of drawPath()
     */
    virtual bool onDrawPath(const DrawPathArgs& args) = 0;

    /**
     * Subclass implementation of canDrawPath()
     */
    virtual CanDrawPath onCanDrawPath(const CanDrawPathArgs& args) const = 0;

    /**
     * Subclass implementation of stencilPath(). Subclass must override iff it ever returns
     * kStencilOnly in onGetStencilSupport().
     */
    virtual void onStencilPath(const StencilPathArgs& args) {
        static constexpr GrUserStencilSettings kIncrementStencil(
            GrUserStencilSettings::StaticInit<
                 0xffff,
                 GrUserStencilTest::kAlways,
                 0xffff,
                 GrUserStencilOp::kReplace,
                 GrUserStencilOp::kReplace,
                 0xffff>()
        );

        GrPaint paint;
        DrawPathArgs drawArgs{args.fContext,
                              std::move(paint),
                              &kIncrementStencil,
                              args.fRenderTargetContext,
                              nullptr,  // clip
                              args.fClipConservativeBounds,
                              args.fViewMatrix,
                              args.fShape,
                              args.fAAType,
                              false};
        this->drawPath(drawArgs);
    }

    typedef SkRefCnt INHERITED;
};

#endif
