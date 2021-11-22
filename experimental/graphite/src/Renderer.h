/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Renderer_DEFINED
#define skgpu_Renderer_DEFINED

#include "experimental/graphite/src/DrawTypes.h"

#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"

#include <array>

namespace skgpu {

class DrawWriter;
class ResourceProvider;
class Shape;
class Uniform;

class RenderStep {
public:
    virtual ~RenderStep() {}

    virtual const char* name()            const = 0;
    virtual bool        requiresStencil() const = 0;
    virtual bool        requiresMSAA()    const = 0;
    virtual bool        performsShading() const = 0;

    // The DrawWriter is configured with the vertex and instance strides of the RenderStep, and its
    // primitive type. The recorded draws will be executed with a graphics pipeline compatible with
    // this RenderStep.
    virtual void writeVertices(DrawWriter*, const Shape&) const = 0;

    // While RenderStep does not define the full program that's run for a draw, it defines the
    // entire vertex layout of the pipeline.
    virtual size_t        vertexStride()   const = 0;
    virtual size_t        instanceStride() const = 0;
    virtual PrimitiveType primitiveType()  const = 0;

    // TODO: Actual API to do things
    // 1. Provide stencil settings
    // 2. Provide shader key or MSL(?) for the vertex stage
    // 4. Write uniform data given a Shape/Transform/Stroke info
    // 6. Some Renderers benefit from being able to share vertices between RenderSteps. Must find a
    //    way to support that. It may mean that RenderSteps get state per draw.
    //    - Does Renderer make RenderStepFactories that create steps for each DrawList::Draw?
    //    - Does DrawList->DrawPass conversion build a separate array of blind data that the
    //      stateless Renderstep can refer to for {draw,step} pairs?
    //    - Does each DrawList::Draw have extra space (e.g. 8 bytes) that steps can cache data in?
protected:
    RenderStep() {}

private:
    // Cannot copy or move
    RenderStep(const RenderStep&) = delete;
    RenderStep(RenderStep&&)      = delete;
};

/**
 * The actual technique for rasterizing a high-level draw recorded in a DrawList is handled by a
 * specific Renderer. Each technique has an associated singleton Renderer that decomposes the
 * technique into a series of RenderSteps that must be executed in the specified order for the draw.
 * However, the RenderStep executions for multiple draws can be re-arranged so batches of each
 * step can be performed in a larger GPU operation. This re-arranging relies on accurate
 * determination of the DisjointStencilIndex for each draw so that stencil steps are not corrupted
 * by another draw before its cover step is executed. It also relies on the CompressedPaintersOrder
 * for each draw to ensure steps are not re-arranged in a way that violates the original draw order.
 *
 * Renderer itself is non-virtual since it simply has to point to a list of RenderSteps. RenderSteps
 * on the other hand are virtual implement the technique specific functionality. It is entirely
 * possible for certain types of steps, e.g. a bounding box cover, to be re-used across different
 * Renderers even if the preceeding steps were different.
 */
class Renderer {
public:
    // Graphite defines a limited set of renderers in order to increase likelihood of batching
    // across draw calls, and reduce the number of shader permutations required. These Renderers
    // are stateless singletons and remain alive for the entire program. Each Renderer corresponds
    // to a specific recording function on DrawList.
    static const Renderer& StencilAndFillPath();
    // TODO: Not on the immediate sprint target, but show what needs to be added for DrawList's API
    // static const Renderer& FillConvexPath();
    // static const Renderer& StrokePath();
    // TODO: Will add more of these as primitive rendering etc. is fleshed out

    // The maximum number of render steps that any Renderer is allowed to have.
    static constexpr int kMaxRenderSteps = 4;

    SkSpan<const RenderStep* const> steps() const {
        return {&fSteps.front(), static_cast<size_t>(fStepCount) };
    }

    const char* name()            const { return fName.c_str();    }
    int         numRenderSteps()  const { return fStepCount;       }
    bool        requiresStencil() const { return fRequiresStencil; }
    bool        requiresMSAA()    const { return fRequiresMSAA;    }

private:
    // max render steps is 4, so just spell the options out for now...
    Renderer(const char* name, const RenderStep* s1)
            : Renderer(name, std::array<const RenderStep*, 1>{s1}) {}

    Renderer(const char* name, const RenderStep* s1, const RenderStep* s2)
            : Renderer(name, std::array<const RenderStep*, 2>{s1, s2}) {}

    Renderer(const char* name, const RenderStep* s1, const RenderStep* s2, const RenderStep* s3)
            : Renderer(name, std::array<const RenderStep*, 3>{s1, s2, s3}) {}

    Renderer(const char* name, const RenderStep* s1, const RenderStep* s2,
             const RenderStep* s3, const RenderStep* s4)
            : Renderer(name, std::array<const RenderStep*, 4>{s1, s2, s3, s4}) {}

    template<size_t N>
    Renderer(const char* name, std::array<const RenderStep*, N> steps)
            : fName(name)
            , fStepCount(SkTo<int>(N))
            , fRequiresStencil(false)
            , fRequiresMSAA(false) {
        static_assert(N <= kMaxRenderSteps);
        SkDEBUGCODE(bool performsShading = false;)
        for (int i = 0 ; i < fStepCount; ++i) {
            fSteps[i] = steps[i];
            fRequiresStencil |= fSteps[i]->requiresStencil();
            fRequiresMSAA |= fSteps[i]->requiresMSAA();
            SkDEBUGCODE(performsShading |= fSteps[i]->performsShading());
        }
        SkASSERT(performsShading); // at least one step needs to actually shade
    }

    // Cannot move or copy
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&)      = delete;

    std::array<const RenderStep*, kMaxRenderSteps> fSteps;

    SkString fName;
    int      fStepCount;
    bool     fRequiresStencil;
    bool     fRequiresMSAA;
};

} // skgpu namespace

#endif // skgpu_Renderer_DEFINED
