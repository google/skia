/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */

#ifndef GrPathRenderer_DEFINED
#define GrPathRenderer_DEFINED

#include "GrDrawTarget.h"
#include "SkTemplates.h"

class SkPath;
struct GrPoint;

/**
 *  Base class for drawing paths into a GrDrawTarget.
 *  Paths may be drawn multiple times as when tiling for supersampling. The 
 *  calls on GrPathRenderer to draw a path will look like this:
 *  
 *  pr->setPath(target, path, fill, translate); // sets the path to draw
 *      pr->drawPath(...);  // draw the path
 *      pr->drawPath(...);
 *      ...
 *  pr->clearPath();  // finished with the path
 */
class GR_API GrPathRenderer : public GrRefCnt {
public:
    GrPathRenderer(void);
    /**
     * Returns true if this path renderer is able to render the path.
     * Returning false allows the caller to fallback to another path renderer.
     *
     * @param path      The path to draw
     * @param fill      The fill rule to use
     *
     * @return  true if the path can be drawn by this object, false otherwise.
     */
    virtual bool canDrawPath(const SkPath& path, GrPathFill fill) const = 0;

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
     * @return true if the path renderer can perform anti-aliasing (aside from
     * having FSAA enabled for a render target). Target is provided to
     * communicate the draw state (blend mode, stage settings, etc).
     */
    virtual bool supportsAA(GrDrawTarget* target,
                            const SkPath& path,
                            GrPathFill fill) { return false; }

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
     * @param translate             optional additional translation to apply to
     *                              the path. NULL means (0,0).
     */
    void setPath(GrDrawTarget* target,
                 const SkPath* path,
                 GrPathFill fill,
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
     * This is called to install a custom path renderer in every GrContext at
     * create time. The default implementation in GrCreatePathRenderer_none.cpp
     * returns NULL. Link against another implementation to install your own.
     */
    static GrPathRenderer* CreatePathRenderer();

    /**
     * Multiply curve tolerance by the given value, increasing or decreasing
     * the maximum error permitted in tesselating curves with short straight
     * line segments.
     */
    void scaleCurveTolerance(GrScalar multiplier) {
        GrAssert(multiplier > 0);
        fCurveTolerance = SkScalarMul(fCurveTolerance, multiplier);
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
                       const GrPoint* translate) {
            GrAssert(NULL != pr);
            pr->setPath(target, path, fill, translate);
            fPathRenderer = pr;
        }
        void set(GrPathRenderer* pr,
                 GrDrawTarget* target,
                 const SkPath* path,
                 GrPathFill fill,
                 const GrPoint* translate) {
            if (NULL != fPathRenderer) {
                fPathRenderer->clearPath();
            }
            GrAssert(NULL != pr);
            pr->setPath(target, path, fill, translate);
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

    GrScalar fCurveTolerance;
    const SkPath*               fPath;
    GrDrawTarget*               fTarget;
    GrPathFill                  fFill;
    GrPoint                     fTranslate;

private:

    typedef GrRefCnt INHERITED;
};

/**
 *  Subclass that renders the path using the stencil buffer to resolve fill
 *  rules (e.g. winding, even-odd)
 */
class GR_API GrDefaultPathRenderer : public GrPathRenderer {
public:
    GrDefaultPathRenderer(bool separateStencilSupport,
                          bool stencilWrapOpsSupport);

    virtual bool canDrawPath(const SkPath& path,
                             GrPathFill fill) const { return true; }

    virtual bool requiresStencilPass(const GrDrawTarget* target,
                                     const SkPath& path,
                                     GrPathFill fill) const;

    virtual void drawPath(GrDrawTarget::StageBitfield stages);
    virtual void drawPathToStencil();

protected:
    virtual void pathWillClear();

private:

    void onDrawPath(GrDrawTarget::StageBitfield stages, bool stencilOnly);

    bool createGeom(GrScalar srcSpaceTol,
                    GrDrawTarget::StageBitfield stages);

    bool    fSeparateStencil;
    bool    fStencilWrapOps;

    int                         fSubpathCount;
    SkAutoSTMalloc<8, uint16_t> fSubpathVertCount;
    int                         fIndexCnt;
    int                         fVertexCnt;
    GrScalar                    fPreviousSrcTol;
    GrDrawTarget::StageBitfield fPreviousStages;
    GrPrimitiveType             fPrimitiveType;
    bool                        fUseIndexedDraw;

    typedef GrPathRenderer INHERITED;
};

#endif

