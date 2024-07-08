/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PathRenderer_DEFINED
#define PathRenderer_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "src/gpu/ganesh/GrPaint.h"

#include <string.h>

class GrCaps;
class GrClip;
class GrHardClip;
class GrRecordingContext;
class GrRenderTargetProxy;
class GrStyledShape;
class SkMatrix;
class SkPath;
class SkSurfaceProps;
enum class GrAA : bool;
enum class GrAAType : unsigned int;
struct GrUserStencilSettings;
struct SkIRect;
struct SkISize;
struct SkRect;

namespace skgpu::ganesh {

class SurfaceDrawContext;

/**
 *  Base class for drawing paths into a OpsTask.
 */
class PathRenderer : public SkRefCnt {
public:
    PathRenderer() = default;

    virtual const char* name() const = 0;

    /**
     * A caller may wish to use a path renderer to draw a path into the stencil buffer. However,
     * the path renderer itself may require use of the stencil buffer. Also a path renderer may
     * use a GrProcessor coverage stage that sets coverage to zero to eliminate pixels that are
     * covered by bounding geometry but outside the path. These exterior pixels would still be
     * rendered into the stencil.
     *
     * A PathRenderer can provide three levels of support for stenciling paths:
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
    StencilSupport getStencilSupport(const GrStyledShape& shape) const;

    enum class CanDrawPath {
        kNo,
        kAsBackup, // i.e. This renderer is better than SW fallback if no others can draw the path.
        kYes
    };

    struct CanDrawPathArgs {
        SkDEBUGCODE(CanDrawPathArgs() { memset(this, 0, sizeof(*this)); }) // For validation.

        const GrCaps*               fCaps;
        const GrRenderTargetProxy*  fProxy;
        const SkIRect*              fClipConservativeBounds;
        const SkMatrix*             fViewMatrix;
        const GrStyledShape*        fShape;
        const GrPaint*              fPaint;
        const SkSurfaceProps*       fSurfaceProps;
        GrAAType                    fAAType;

        // This is only used by TessellationPathRenderer
        bool                        fHasUserStencilSettings;

#ifdef SK_DEBUG
        void validate() const {
            SkASSERT(fCaps);
            SkASSERT(fProxy);
            SkASSERT(fClipConservativeBounds);
            SkASSERT(fViewMatrix);
            SkASSERT(fShape);
            SkASSERT(fSurfaceProps);
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
        GrRecordingContext*          fContext;
        GrPaint&&                    fPaint;
        const GrUserStencilSettings* fUserStencilSettings;
        SurfaceDrawContext*          fSurfaceDrawContext;
        const GrClip*                fClip;
        const SkIRect*               fClipConservativeBounds;
        const SkMatrix*              fViewMatrix;
        const GrStyledShape*         fShape;
        GrAAType                     fAAType;
        bool                         fGammaCorrect;
#ifdef SK_DEBUG
        void validate() const {
            SkASSERT(fContext);
            SkASSERT(fUserStencilSettings);
            SkASSERT(fSurfaceDrawContext);
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
    bool drawPath(const DrawPathArgs& args);
    /**
     * Args to stencilPath(). fAAType cannot be kCoverage.
     */
    struct StencilPathArgs {
        SkDEBUGCODE(StencilPathArgs() { memset(this, 0, sizeof(*this)); }) // For validation.

        GrRecordingContext*  fContext;
        SurfaceDrawContext*  fSurfaceDrawContext;
        const GrHardClip*    fClip;
        const SkIRect*       fClipConservativeBounds;
        const SkMatrix*      fViewMatrix;
        const GrStyledShape* fShape;
        GrAA                 fDoStencilMSAA;

        SkDEBUGCODE(void validate() const;)
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

protected:
    // Helper for getting the device bounds of a path. Inverse filled paths will have bounds set
    // by devSize. Non-inverse path bounds will not necessarily be clipped to devSize.
    static void GetPathDevBounds(const SkPath& path,
                                 SkISize devSize,
                                 const SkMatrix& matrix,
                                 SkRect* bounds);

private:
    /**
     * Subclass overrides if it has any limitations of stenciling support.
     */
    virtual StencilSupport onGetStencilSupport(const GrStyledShape&) const {
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
    virtual void onStencilPath(const StencilPathArgs&);

    using INHERITED = SkRefCnt;
};

}  // namespace skgpu::ganesh

#endif // PathRenderer_DEFINED
