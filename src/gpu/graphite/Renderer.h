/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Renderer_DEFINED
#define skgpu_graphite_Renderer_DEFINED

#include "src/core/SkEnumBitMask.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/ResourceTypes.h"

#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "src/core/SkUniform.h"

#include <array>
#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

enum class SkPathFillType;
class SkPipelineDataGatherer;

namespace skgpu { enum class MaskFormat; }

namespace skgpu::graphite {
class DrawWriter;
class DrawParams;
class ResourceProvider;

struct Varying {
    const char* fName;
    SkSLType fType;
    // TODO: add modifier (e.g., flat and noperspective) support
};

class RenderStep {
public:
    virtual ~RenderStep() = default;

    // The DrawWriter is configured with the vertex and instance strides of the RenderStep, and its
    // primitive type. The recorded draws will be executed with a graphics pipeline compatible with
    // this RenderStep.
    virtual void writeVertices(DrawWriter*, const DrawParams&) const = 0;

    // Write out the uniform values (aligned for the layout). These values will be de-duplicated
    // across all draws using the RenderStep before uploading to the GPU, but it can be assumed the
    // uniforms will be bound before the draws recorded in 'writeVertices' are executed.
    // TODO: We definitely want this to return CPU memory since it's better for the caller to handle
    // the de-duplication and GPU upload/binding (DrawPass tracks all this). However, a RenderStep's
    // uniforms aren't going to change, and the Layout won't change during a process, so it would be
    // nice if we could remember the offsets for the layout/gpu and reuse them across draws.
    // Similarly, it would be nice if this could write into reusable storage and then DrawPass or
    // UniformCache handles making an sk_sp if we need to assign a new unique ID to the uniform data
    virtual void writeUniforms(const DrawParams&, SkPipelineDataGatherer*) const = 0;

    // Returns a name formatted as "Subclass[variant]", where "Subclass" matches the C++ class name
    // and variant is a unique term describing instance's specific configuration.
    const char* name() const { return fName.c_str(); }

    // TODO: This is only temporary. Eventually the RenderStep will define its logic in SkSL and
    // be able to have code operate in both the vertex and fragment shaders. Ideally the RenderStep
    // will provide two functions that fit some ABI for integrating with the common and paint SkSL,
    // although we could go as far as allowing RenderStep to handle composing the final SkSL if
    // given the paint combination's SkSL.

    // Returns the body of a vertex function, which must define a float4 devPosition variable.
    // It has access to the variables declared by vertexAttributes(), instanceAttributes(),
    // and uniforms(). The 'devPosition' variable's z must store the PaintDepth normalized to a
    // float from [0, 1], for each processed draw although the RenderStep can choose to upload it
    // as attributes or uniforms.
    //
    // NOTE: The above contract is mainly so that the entire SkSL program can be created by just str
    // concatenating struct definitions generated from the RenderStep and paint Combination
    // and then including the function bodies returned here.
    virtual const char* vertexSkSL() const = 0;

    bool          requiresMSAA()    const { return fFlags & Flags::kRequiresMSAA;    }
    bool          performsShading() const { return fFlags & Flags::kPerformsShading; }

    PrimitiveType primitiveType()   const { return fPrimitiveType;  }
    size_t        vertexStride()    const { return fVertexStride;   }
    size_t        instanceStride()  const { return fInstanceStride; }

    const DepthStencilSettings& depthStencilSettings() const { return fDepthStencilSettings; }

    SkEnumBitMask<DepthStencilFlags> depthStencilFlags() const {
        return (fDepthStencilSettings.fStencilTestEnabled
                        ? DepthStencilFlags::kStencil : DepthStencilFlags::kNone) |
               (fDepthStencilSettings.fDepthTestEnabled || fDepthStencilSettings.fDepthWriteEnabled
                        ? DepthStencilFlags::kDepth : DepthStencilFlags::kNone);
    }

    size_t numUniforms()            const { return fUniforms.size();      }
    size_t numVertexAttributes()    const { return fVertexAttrs.size();   }
    size_t numInstanceAttributes()  const { return fInstanceAttrs.size(); }
    size_t numVaryings()            const { return fVaryings.size(); }

    // The uniforms of a RenderStep are bound to the kRenderStep slot, the rest of the pipeline
    // may still use uniforms bound to other slots.
    SkSpan<const SkUniform> uniforms()           const { return SkSpan(fUniforms);      }
    SkSpan<const Attribute> vertexAttributes()   const { return SkSpan(fVertexAttrs);   }
    SkSpan<const Attribute> instanceAttributes() const { return SkSpan(fInstanceAttrs); }
    SkSpan<const Varying>   varyings()           const { return SkSpan(fVaryings);      }

    // TODO: Actual API to do things
    // 6. Some Renderers benefit from being able to share vertices between RenderSteps. Must find a
    //    way to support that. It may mean that RenderSteps get state per draw.
    //    - Does Renderer make RenderStepFactories that create steps for each DrawList::Draw?
    //    - Does DrawList->DrawPass conversion build a separate array of blind data that the
    //      stateless Renderstep can refer to for {draw,step} pairs?
    //    - Does each DrawList::Draw have extra space (e.g. 8 bytes) that steps can cache data in?
    // 7. Painter's order for the draw sent as uniform or attribute
protected:
    enum class Flags : unsigned {
        kNone            = 0b000,
        kRequiresMSAA    = 0b001,
        kPerformsShading = 0b010,
    };
    SK_DECL_BITMASK_OPS_FRIENDS(Flags);

    // While RenderStep does not define the full program that's run for a draw, it defines the
    // entire vertex layout of the pipeline. This is not allowed to change, so can be provided to
    // the RenderStep constructor by subclasses.
    RenderStep(std::string_view className,
               std::string_view variantName,
               SkEnumBitMask<Flags> flags,
               std::initializer_list<SkUniform> uniforms,
               PrimitiveType primitiveType,
               DepthStencilSettings depthStencilSettings,
               std::initializer_list<Attribute> vertexAttrs,
               std::initializer_list<Attribute> instanceAttrs,
               std::initializer_list<Varying> varyings = {})
            : fFlags(flags)
            , fPrimitiveType(primitiveType)
            , fDepthStencilSettings(depthStencilSettings)
            , fUniforms(uniforms)
            , fVertexAttrs(vertexAttrs)
            , fInstanceAttrs(instanceAttrs)
            , fVaryings(varyings)
            , fVertexStride(0)
            , fInstanceStride(0)
            , fName(className) {
        for (auto v : this->vertexAttributes()) {
            fVertexStride += v.sizeAlign4();
        }
        for (auto i : this->instanceAttributes()) {
            fInstanceStride += i.sizeAlign4();
        }
        if (variantName.size() > 0) {
            fName += "[";
            fName += variantName;
            fName += "]";
        }
    }

private:
    // Cannot copy or move
    RenderStep(const RenderStep&) = delete;
    RenderStep(RenderStep&&)      = delete;

    SkEnumBitMask<Flags> fFlags;
    PrimitiveType        fPrimitiveType;

    DepthStencilSettings fDepthStencilSettings;

    // TODO: When we always use C++17 for builds, we should be able to just let subclasses declare
    // constexpr arrays and point to those, but we need explicit storage for C++14.
    // Alternatively, if we imposed a max attr count, similar to Renderer's num render steps, we
    // could just have this be std::array and keep all attributes inline with the RenderStep memory.
    // On the other hand, the attributes are only needed when creating a new pipeline so it's not
    // that performance sensitive.
    std::vector<SkUniform> fUniforms;
    std::vector<Attribute> fVertexAttrs;
    std::vector<Attribute> fInstanceAttrs;
    std::vector<Varying>   fVaryings;

    size_t fVertexStride;   // derived from vertex attribute set
    size_t fInstanceStride; // derived from instance attribute set

    std::string fName;
};
SK_MAKE_BITMASK_OPS(RenderStep::Flags);

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
    // to a specific recording function on DrawList and fill type.
    static const Renderer& StencilTessellatedCurvesAndTris(SkPathFillType);

    static const Renderer& StencilTessellatedWedges(SkPathFillType);

    static const Renderer& ConvexTessellatedWedges();

    static const Renderer& TessellatedStrokes();

    static const Renderer& TextDirect();

    static const Renderer& TextSDF(bool useLCDText);

    // TODO: Add renderers for primitives (rect, rrect, etc.), special draws (atlas, vertices, text)
    // and support inverse filled strokes.

    // The maximum number of render steps that any Renderer is allowed to have.
    static constexpr int kMaxRenderSteps = 4;

    SkSpan<const RenderStep* const> steps() const {
        return {&fSteps.front(), static_cast<size_t>(fStepCount) };
    }

    const char* name()            const { return fName.c_str();    }
    int         numRenderSteps()  const { return fStepCount;       }
    bool        requiresMSAA()    const { return fRequiresMSAA;    }

    SkEnumBitMask<DepthStencilFlags> depthStencilFlags() const { return fDepthStencilFlags; }

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
            , fStepCount(SkTo<int>(N)) {
        static_assert(N <= kMaxRenderSteps);
        SkDEBUGCODE(bool performsShading = false;)
        for (int i = 0 ; i < fStepCount; ++i) {
            fSteps[i] = steps[i];
            fRequiresMSAA |= fSteps[i]->requiresMSAA();
            fDepthStencilFlags |= fSteps[i]->depthStencilFlags();
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
    bool     fRequiresMSAA = false;

    SkEnumBitMask<DepthStencilFlags> fDepthStencilFlags = DepthStencilFlags::kNone;
};

} // skgpu namespace::graphite

#endif // skgpu_graphite_Renderer_DEFINED
