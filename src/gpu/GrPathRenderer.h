
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

#include "SkTArray.h"

class SkPath;

struct GrPoint;

/**
 *  Base class for drawing paths into a GrDrawTarget.
 *  Paths may be drawn multiple times as when tiling for supersampling. The 
 *  calls on GrPathRenderer to draw a path will look like this:
 *  
 *  pr->setPath(target, path, fill, aa, translate); // sets the path to draw
 *      pr->drawPath(...);  // draw the path
 *      pr->drawPath(...);
 *      ...
 *  pr->clearPath();  // finished with the path
 */
class GR_API GrPathRenderer : public GrRefCnt {
public:

    /**
     * This is called to install custom path renderers in every GrContext at
     * create time. The default implementation in GrCreatePathRenderer_none.cpp
     * does not add any additional renderers. Link against another
     * implementation to install your own. The first added is the most preferred
     * path renderer, second is second most preferred, etc.
     *
     * @param context   the context that will use the path renderer
     * @param flags     flags indicating how path renderers will be used
     * @param prChain   the chain to add path renderers to.
     */
    static void AddPathRenderers(GrContext* context,
                                 GrPathRendererChain::UsageFlags flags,
                                 GrPathRendererChain* prChain);


    GrPathRenderer();

    /**
     * For complex clips Gr uses the stencil buffer. The path renderer must be
     * able to render paths into the stencil buffer. However, the path renderer
     * itself may require the stencil buffer to resolve the path fill rule.
     * This function queries whether the path render needs its own stencil
     * pass. If this returns false then drawPath() should not modify the
     * the target's stencil settings but use those already set on target. The
     * target is passed as a param in case the answer depends upon draw state.
     * The view matrix and render target set on the draw target may change 
     * before setPath/drawPath is called and so shouldn't be considered.
     *
     * @param target target that the path will be rendered to
     * @param path   the path that will be drawn
     * @param fill   the fill rule that will be used, will never be an inverse
     *               rule.
     *
     * @return false if this path renderer can generate interior-only fragments
     *         without changing the stencil settings on the target. If it
     *         returns true the drawPathToStencil will be used when rendering
     *         clips.
     */
    virtual bool requiresStencilPass(const SkPath& path,
                                     GrPathFill fill,
                                     const GrDrawTarget* target) const {
        return false;
    }

    virtual bool canDrawPath(const SkPath& path,
                             GrPathFill fill,
                             const GrDrawTarget* target,
                             bool antiAlias) const = 0;
    /**
     * Draws the path into the draw target. If requiresStencilBuffer returned
     * false then the target may be setup for stencil rendering (since the 
     * path renderer didn't claim that it needs to use the stencil internally).
     *
     * @param stages                bitfield that indicates which stages are
     *                              in use. All enabled stages expect positions
     *                              as texture coordinates. The path renderer
     *                              use the remaining stages for its path
     *                              filling algorithm.
     */
    virtual bool drawPath(const SkPath& path,
                          GrPathFill fill,
                          const GrVec* translate,
                          GrDrawTarget* target,
                          GrDrawState::StageMask stageMask,
                          bool antiAlias) {
        GrAssert(this->canDrawPath(path, fill, target, antiAlias));
        return this->onDrawPath(path, fill, translate,
                                target, stageMask, antiAlias);
    }

    /**
     * Draws the path to the stencil buffer. Assume the writable stencil bits
     * are already initialized to zero. Fill will always be either
     * kWinding_PathFill or kEvenOdd_PathFill.
     *
     * Only called if requiresStencilPass returns true for the same combo of
     * target, path, and fill. Never called with an inverse fill.
     *
     * The default implementation assumes the path filling algorithm doesn't
     * require a separate stencil pass and so crashes.
     *
     */
    virtual void drawPathToStencil(const SkPath& path,
                                   GrPathFill fill,
                                   GrDrawTarget* target) {
        GrCrash("Unexpected call to drawPathToStencil.");
    }

protected:
    virtual bool onDrawPath(const SkPath& path,
                            GrPathFill fill,
                            const GrVec* translate,
                            GrDrawTarget* target,
                            GrDrawState::StageMask stageMask,
                            bool antiAlias) = 0;

private:

    typedef GrRefCnt INHERITED;
};

#endif

