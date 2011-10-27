
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


    GrPathRenderer(void);
    /**
     * Returns true if this path renderer is able to render the path.
     * Returning false allows the caller to fallback to another path renderer.
     * When searching for a path renderer capable of rendering a path this
     * function is called.
     *
     * @param targetCaps The caps of the draw target that will be used to draw
     *                   the path.
     * @param path       The path to draw
     * @param fill       The fill rule to use
     * @param antiAlias  True if anti-aliasing is required.
     *
     * @return  true if the path can be drawn by this object, false otherwise.
     */
    virtual bool canDrawPath(const GrDrawTarget::Caps& targetCaps,
                             const SkPath& path,
                             GrPathFill fill,
                             bool antiAlias) const = 0;

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
    virtual bool requiresStencilPass(const GrDrawTarget* target,
                                     const SkPath& path,
                                     GrPathFill fill) const { return false; }

    /**
     * Sets the path to render and target to render into. All calls to drawPath
     * and drawPathToStencil must occur between setPath and clearPath. The
     * path cannot be modified externally between setPath and clearPath. The
     * path may be drawn several times (e.g. tiled supersampler). The target's
     * state may change between setPath and drawPath* calls. However, if the
     * path renderer specified vertices/indices during setPath or drawPath*
     * they will still be set at subsequent drawPath* calls until the next
     * clearPath. The target's draw state may change between drawPath* calls
     * so if the subclass does any caching of tesselation, etc. then it must
     * validate that target parameters that guided the decisions still hold.
     *
     * @param target                the target to draw into.
     * @param path                  the path to draw.
     * @param fill                  the fill rule to apply.
     * @param antiAlias             perform antiAliasing when drawing the path.
     * @param translate             optional additional translation to apply to
     *                              the path. NULL means (0,0).
     */
    void setPath(GrDrawTarget* target,
                 const SkPath* path,
                 GrPathFill fill,
                 bool antiAlias,
                 const GrPoint* translate);

    /**
     * Notifies path renderer that path set in setPath is no longer in use.
     */
    void clearPath();

    /**
     * Draws the path into the draw target. If requiresStencilBuffer returned
     * false then the target may be setup for stencil rendering (since the 
     * path renderer didn't claim that it needs to use the stencil internally).
     *
     * Only called between setPath / clearPath.
     *
     * @param stages                bitfield that indicates which stages are
     *                              in use. All enabled stages expect positions
     *                              as texture coordinates. The path renderer
     *                              use the remaining stages for its path
     *                              filling algorithm.
     */
    virtual void drawPath(GrDrawTarget::StageBitfield stages) = 0;

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
     * Only called between setPath / clearPath.
     */
    virtual void drawPathToStencil() {
        GrCrash("Unexpected call to drawPathToStencil.");
    }

    /**
     * Helper that sets a path and automatically remove it in destructor.
     */
    class AutoClearPath {
    public:
        AutoClearPath() {
            fPathRenderer = NULL;
        }
        AutoClearPath(GrPathRenderer* pr,
                      GrDrawTarget* target,
                      const SkPath* path,
                      GrPathFill fill,
                      bool antiAlias,
                      const GrPoint* translate) {
            GrAssert(NULL != pr);
            pr->setPath(target, path, fill, antiAlias, translate);
            fPathRenderer = pr;
        }
        void set(GrPathRenderer* pr,
                 GrDrawTarget* target,
                 const SkPath* path,
                 GrPathFill fill,
                 bool antiAlias,
                 const GrPoint* translate) {
            if (NULL != fPathRenderer) {
                fPathRenderer->clearPath();
            }
            GrAssert(NULL != pr);
            pr->setPath(target, path, fill, antiAlias, translate);
            fPathRenderer = pr;
        }
        ~AutoClearPath() {
            if (NULL != fPathRenderer) {
                fPathRenderer->clearPath();
            }
        }
    private:
        GrPathRenderer* fPathRenderer;
    };

protected:

    // subclass can override these to be notified just after a path is set
    // and just before the path is cleared.
    virtual void pathWasSet() {}
    virtual void pathWillClear() {}

    const SkPath*               fPath;
    GrDrawTarget*               fTarget;
    GrPathFill                  fFill;
    GrPoint                     fTranslate;
    bool                        fAntiAlias;

private:

    typedef GrRefCnt INHERITED;
};

#endif

