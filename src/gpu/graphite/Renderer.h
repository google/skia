/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Renderer_DEFINED
#define skgpu_graphite_Renderer_DEFINED

#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkVx.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/Uniform.h"

#include <array>
#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

enum class SkPathFillType;

namespace skgpu { enum class MaskFormat; }

namespace skgpu::graphite {

class DrawWriter;
class DrawParams;
class PipelineDataGatherer;
class Rect;
class ResourceProvider;
class TextureDataBlock;
class Transform;

struct ResourceBindingRequirements;

enum class Coverage { kNone, kSingleChannel, kLCD };

struct Varying {
    const char* fName;
    SkSLType fType;
    // TODO: add modifier (e.g., flat and noperspective) support
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
 *
 * All Renderers are accessed through the SharedContext's RendererProvider.
 */
class RenderStep {
public:
    virtual ~RenderStep() = default;

    // The DrawWriter is configured with the vertex and instance strides of the RenderStep, and its
    // primitive type. The recorded draws will be executed with a graphics pipeline compatible with
    // this RenderStep.
    virtual void writeVertices(DrawWriter*, const DrawParams&, skvx::ushort2 ssboIndices) const = 0;

    // Write out the uniform values (aligned for the layout), textures, and samplers. The uniform
    // values will be de-duplicated across all draws using the RenderStep before uploading to the
    // GPU, but it can be assumed the uniforms will be bound before the draws recorded in
    // 'writeVertices' are executed.
    virtual void writeUniformsAndTextures(const DrawParams&, PipelineDataGatherer*) const = 0;

    // Returns the body of a vertex function, which must define a float4 devPosition variable and
    // must write to an already-defined float2 stepLocalCoords variable. This will be automatically
    // set to a varying for the fragment shader if the paint requires local coords. This SkSL has
    // access to the variables declared by vertexAttributes(), instanceAttributes(), and uniforms().
    // The 'devPosition' variable's z must store the PaintDepth normalized to a float from [0, 1],
    // for each processed draw although the RenderStep can choose to upload it in any manner.
    //
    // NOTE: The above contract is mainly so that the entire SkSL program can be created by just str
    // concatenating struct definitions generated from the RenderStep and paint Combination
    // and then including the function bodies returned here.
    virtual std::string vertexSkSL() const = 0;

    // Emits code to set up textures and samplers. Should only be defined if hasTextures is true.
    virtual std::string texturesAndSamplersSkSL(const ResourceBindingRequirements&,
                                                int* nextBindingIndex) const {
        return R"()";
    }

    // Emits code to set up coverage value. Should only be defined if overridesCoverage is true.
    // When implemented the returned SkSL fragment should write its coverage into a
    // 'half4 outputCoverage' variable (defined in the calling code) with the actual
    // coverage splatted out into all four channels.
    virtual const char* fragmentCoverageSkSL() const { return R"()"; }

    // Emits code to set up a primitive color value. Should only be defined if emitsPrimitiveColor
    // is true. When implemented, the returned SkSL fragment should write its color into a
    // 'half4 primitiveColor' variable (defined in the calling code).
    virtual const char* fragmentColorSkSL() const { return R"()"; }

    uint32_t uniqueID() const { return fUniqueID; }

    // Returns a name formatted as "Subclass[variant]", where "Subclass" matches the C++ class name
    // and variant is a unique term describing instance's specific configuration.
    const char* name() const { return fName.c_str(); }

    bool requiresMSAA()        const { return SkToBool(fFlags & Flags::kRequiresMSAA);        }
    bool performsShading()     const { return SkToBool(fFlags & Flags::kPerformsShading);     }
    bool hasTextures()         const { return SkToBool(fFlags & Flags::kHasTextures);         }
    bool emitsPrimitiveColor() const { return SkToBool(fFlags & Flags::kEmitsPrimitiveColor); }
    bool outsetBoundsForAA()   const { return SkToBool(fFlags & Flags::kOutsetBoundsForAA);   }

    Coverage coverage() const { return RenderStep::GetCoverage(fFlags); }

    PrimitiveType primitiveType()  const { return fPrimitiveType;  }
    size_t        vertexStride()   const { return fVertexStride;   }
    size_t        instanceStride() const { return fInstanceStride; }

    size_t numUniforms()           const { return fUniforms.size();      }
    size_t numVertexAttributes()   const { return fVertexAttrs.size();   }
    size_t numInstanceAttributes() const { return fInstanceAttrs.size(); }

    // Name of an attribute containing both render step and shading SSBO indices, if used.
    static const char* ssboIndicesAttribute() { return "ssboIndices"; }

    // Name of a varying to pass SSBO indices to fragment shader. Both render step and shading
    // indices are passed, because render step uniforms are sometimes used for coverage.
    static const char* ssboIndicesVarying() { return "ssboIndicesVar"; }

    // The uniforms of a RenderStep are bound to the kRenderStep slot, the rest of the pipeline
    // may still use uniforms bound to other slots.
    SkSpan<const Uniform> uniforms()             const { return SkSpan(fUniforms);      }
    SkSpan<const Attribute> vertexAttributes()   const { return SkSpan(fVertexAttrs);   }
    SkSpan<const Attribute> instanceAttributes() const { return SkSpan(fInstanceAttrs); }
    SkSpan<const Varying>   varyings()           const { return SkSpan(fVaryings);      }

    const DepthStencilSettings& depthStencilSettings() const { return fDepthStencilSettings; }

    SkEnumBitMask<DepthStencilFlags> depthStencilFlags() const {
        return (fDepthStencilSettings.fStencilTestEnabled
                        ? DepthStencilFlags::kStencil : DepthStencilFlags::kNone) |
               (fDepthStencilSettings.fDepthTestEnabled || fDepthStencilSettings.fDepthWriteEnabled
                        ? DepthStencilFlags::kDepth : DepthStencilFlags::kNone);
    }

    // TODO: Actual API to do things
    // 6. Some Renderers benefit from being able to share vertices between RenderSteps. Must find a
    //    way to support that. It may mean that RenderSteps get state per draw.
    //    - Does Renderer make RenderStepFactories that create steps for each DrawList::Draw?
    //    - Does DrawList->DrawPass conversion build a separate array of blind data that the
    //      stateless Renderstep can refer to for {draw,step} pairs?
    //    - Does each DrawList::Draw have extra space (e.g. 8 bytes) that steps can cache data in?
protected:
    enum class Flags : unsigned {
        kNone                  = 0b0000000,
        kRequiresMSAA          = 0b0000001,
        kPerformsShading       = 0b0000010,
        kHasTextures           = 0b0000100,
        kEmitsCoverage         = 0b0001000,
        kLCDCoverage           = 0b0010000,
        kEmitsPrimitiveColor   = 0b0100000,
        kOutsetBoundsForAA     = 0b1000000,
    };
    SK_DECL_BITMASK_OPS_FRIENDS(Flags);

    // While RenderStep does not define the full program that's run for a draw, it defines the
    // entire vertex layout of the pipeline. This is not allowed to change, so can be provided to
    // the RenderStep constructor by subclasses.
    RenderStep(std::string_view className,
               std::string_view variantName,
               SkEnumBitMask<Flags> flags,
               std::initializer_list<Uniform> uniforms,
               PrimitiveType primitiveType,
               DepthStencilSettings depthStencilSettings,
               SkSpan<const Attribute> vertexAttrs,
               SkSpan<const Attribute> instanceAttrs,
               SkSpan<const Varying> varyings = {});

private:
    friend class Renderer; // for Flags

    // Cannot copy or move
    RenderStep(const RenderStep&) = delete;
    RenderStep(RenderStep&&)      = delete;

    static Coverage GetCoverage(SkEnumBitMask<Flags>);

    uint32_t fUniqueID;
    SkEnumBitMask<Flags> fFlags;
    PrimitiveType        fPrimitiveType;

    DepthStencilSettings fDepthStencilSettings;

    // TODO: When we always use C++17 for builds, we should be able to just let subclasses declare
    // constexpr arrays and point to those, but we need explicit storage for C++14.
    // Alternatively, if we imposed a max attr count, similar to Renderer's num render steps, we
    // could just have this be std::array and keep all attributes inline with the RenderStep memory.
    // On the other hand, the attributes are only needed when creating a new pipeline so it's not
    // that performance sensitive.
    std::vector<Uniform>   fUniforms;
    std::vector<Attribute> fVertexAttrs;
    std::vector<Attribute> fInstanceAttrs;
    std::vector<Varying>   fVaryings;

    size_t fVertexStride;   // derived from vertex attribute set
    size_t fInstanceStride; // derived from instance attribute set

    std::string fName;
};
SK_MAKE_BITMASK_OPS(RenderStep::Flags)

class Renderer {
    using StepFlags = RenderStep::Flags;
public:
    // The maximum number of render steps that any Renderer is allowed to have.
    static constexpr int kMaxRenderSteps = 4;

    const RenderStep& step(int i) const {
        SkASSERT(i >= 0 && i < fStepCount);
        return *fSteps[i];
    }
    SkSpan<const RenderStep* const> steps() const {
        SkASSERT(fStepCount > 0); // steps() should only be called on valid Renderers.
        return {fSteps.data(), static_cast<size_t>(fStepCount) };
    }

    const char*   name()           const { return fName.c_str(); }
    DrawTypeFlags drawTypes()      const { return fDrawTypes; }
    int           numRenderSteps() const { return fStepCount;    }

    bool requiresMSAA() const {
        return SkToBool(fStepFlags & StepFlags::kRequiresMSAA);
    }
    bool emitsPrimitiveColor() const {
        return SkToBool(fStepFlags & StepFlags::kEmitsPrimitiveColor);
    }
    bool outsetBoundsForAA() const {
        return SkToBool(fStepFlags & StepFlags::kOutsetBoundsForAA);
    }

    SkEnumBitMask<DepthStencilFlags> depthStencilFlags() const { return fDepthStencilFlags; }

    Coverage coverage() const { return RenderStep::GetCoverage(fStepFlags); }

private:
    friend class RendererProvider; // for ctors

    // Max render steps is 4, so just spell the options out for now...
    Renderer(std::string_view name, DrawTypeFlags drawTypes, const RenderStep* s1)
            : Renderer(name, drawTypes, std::array<const RenderStep*, 1>{s1}) {}

    Renderer(std::string_view name, DrawTypeFlags drawTypes,
             const RenderStep* s1, const RenderStep* s2)
            : Renderer(name, drawTypes, std::array<const RenderStep*, 2>{s1, s2}) {}

    Renderer(std::string_view name, DrawTypeFlags drawTypes,
             const RenderStep* s1, const RenderStep* s2, const RenderStep* s3)
            : Renderer(name, drawTypes, std::array<const RenderStep*, 3>{s1, s2, s3}) {}

    Renderer(std::string_view name, DrawTypeFlags drawTypes,
             const RenderStep* s1, const RenderStep* s2, const RenderStep* s3, const RenderStep* s4)
            : Renderer(name, drawTypes, std::array<const RenderStep*, 4>{s1, s2, s3, s4}) {}

    template<size_t N>
    Renderer(std::string_view name, DrawTypeFlags drawTypes, std::array<const RenderStep*, N> steps)
            : fName(name)
            , fDrawTypes(drawTypes)
            , fStepCount(SkTo<int>(N)) {
        static_assert(N <= kMaxRenderSteps);
        for (int i = 0 ; i < fStepCount; ++i) {
            fSteps[i] = steps[i];
            fStepFlags |= fSteps[i]->fFlags;
            fDepthStencilFlags |= fSteps[i]->depthStencilFlags();
        }
        // At least one step needs to actually shade.
        SkASSERT(fStepFlags & RenderStep::Flags::kPerformsShading);
    }

    // For RendererProvider to manage initialization; it will never expose a Renderer that is only
    // default-initialized and not replaced because it's algorithm is disabled by caps/options.
    Renderer() : fSteps(), fName(""), fStepCount(0) {}
    Renderer& operator=(Renderer&&) = default;

    std::array<const RenderStep*, kMaxRenderSteps> fSteps;
    std::string fName;
    DrawTypeFlags fDrawTypes = DrawTypeFlags::kAll;
    int fStepCount;

    SkEnumBitMask<StepFlags> fStepFlags = StepFlags::kNone;
    SkEnumBitMask<DepthStencilFlags> fDepthStencilFlags = DepthStencilFlags::kNone;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Renderer_DEFINED
