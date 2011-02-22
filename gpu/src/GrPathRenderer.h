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
     *                              the path NULL means (0,0).
     */
    virtual void drawPath(GrDrawTarget* target,
                          GrDrawTarget::StageBitfield stages,
                          GrPathIter* path,
                          GrPathFill fill,
                          const GrPoint* translate) = 0;
};

class GrDefaultPathRenderer : public GrPathRenderer {
public:
    GrDefaultPathRenderer(bool singlePassWindingStencil);

    virtual void drawPath(GrDrawTarget* target,
                          GrDrawTarget::StageBitfield stages,
                          GrPathIter* path,
                          GrPathFill fill,
                          const GrPoint* translate);
private:
    bool    fSinglePassWindingStencil;
};

#endif