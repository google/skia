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

class GrPathIter;
struct GrPoint;

/**
 * A path renderer.
 */
class GrPathRenderer {
public:
    /**
     * Draws a path into the draw target. The target will already have its draw
     * state configured for the draw.
     * @param target                the target to draw into.
     * @param stages                indicates which stages the are already
     *                              in use. All enabled stages expect positions
     *                              as texture coordinates. The path renderer
     *                              use the remaining stages for its path
     *                              filling algorithm.
     * @param path                  the path to draw.
     * @param fill                  the fill rule to apply.
     * @param translate             optional additional translation to apply to
     *                              the path. NULL means (0,0).
     */
    virtual void drawPath(GrDrawTarget* target,
                          GrDrawTarget::StageBitfield stages,
                          GrPathIter* path,
                          GrPathFill fill,
                          const GrPoint* translate) = 0;

    void drawPath(GrDrawTarget* target,
                  GrDrawTarget::StageBitfield stages,
                  const GrPath& path,
                  GrPathFill fill,
                  const GrPoint* translate) {
            GrPath::Iter iter(path);
            this->drawPath(target, stages, &iter, fill, translate);
    }

    /**
     * For complex clips Gr uses the stencil buffer. The path renderer must be
     * able to render paths into the stencil buffer. However, the path renderer
     * itself may require the stencil buffer to resolve the path fill rule. This
     * function queries whether the path render requires its own stencil
     * pass. If this returns false then drawPath() should not modify the
     * the target's stencil settings.
     *
     * @return false if this path renderer can generate interior-only fragments
     *         without changing the stencil settings on the target. If it
     *         returns true the drawPathToStencil will be used when rendering
     *         clips.
     */
    virtual bool requiresStencilPass(GrPathIter*) const { return false; }
    
    bool requiresStencilPass(const GrPath& path) const { 
        GrPath::Iter iter(path);
        return requiresStencilPass(&iter);
    }

    /**
     * Draws a path to the stencil buffer. Assume the writable bits are zero
     * prior and write a nonzero value in interior samples. The default
     * implementation assumes the path filling algorithm doesn't require a
     * separate stencil pass and so just calls drawPath.
     *
     * Fill will never be an inverse fill rule.
     *
     * @param target                the target to draw into.
     * @param path                  the path to draw.
     * @param fill                  the fill rule to apply.
     * @param translate             optional additional translation to apply to
     *                              the path. NULL means (0,0).
     */
    virtual void drawPathToStencil(GrDrawTarget* target,
                                   GrPathIter* path,
                                   GrPathFill fill,
                                   const GrPoint* translate) {
        GrAssert(kInverseEvenOdd_PathFill != fill);
        GrAssert(kInverseWinding_PathFill != fill);

        this->drawPath(target, 0, path, fill, translate);
    }

    void drawPathToStencil(GrDrawTarget* target,
                           const GrPath& path,
                           GrPathFill fill,
                           const GrPoint* translate) {
        GrPath::Iter iter(path);
        this->drawPathToStencil(target, &iter, fill, translate);
    }
};

class GrDefaultPathRenderer : public GrPathRenderer {
public:
    GrDefaultPathRenderer(bool separateStencilSupport,
                          bool stencilWrapOpsSupport);

    virtual void drawPath(GrDrawTarget* target,
                          GrDrawTarget::StageBitfield stages,
                          GrPathIter* path,
                          GrPathFill fill,
                          const GrPoint* translate);
    virtual bool requiresStencilPass(GrPath&) const { return true; }
    virtual void drawPathToStencil(GrDrawTarget* target,
                                   GrPathIter* path,
                                   GrPathFill fill,
                                   const GrPoint* translate);
private:

    void drawPathHelper(GrDrawTarget* target,
                        GrDrawTarget::StageBitfield stages,
                        GrPathIter* path,
                        GrPathFill fill,
                        const GrPoint* translate,
                        bool stencilOnly);

    bool    fSeparateStencil;
    bool    fStencilWrapOps;
};

#endif