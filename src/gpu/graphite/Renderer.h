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
#include "include/gpu/graphite/GraphiteTypes.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkVx.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/Uniform.h"

#include <array>
#include <initializer_list>
#include <optional>
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

// If this list is modified in any way, please increment the
// RenderStep::kRenderStepIDVersion value. The enum values generated from this
// list are serialized and the kRenderStepIDVersion value is the signal to
// abandon older serialized data.
#define SKGPU_RENDERSTEP_TYPES(M1, M2)              \
        M1(Invalid)                                 \
        M1(CircularArc)                             \
        M1(AnalyticRRect)                           \
        M1(AnalyticBlur)                            \
        M1(PerEdgeAAQuad)                           \
        M2(CoverBounds,      NonAAFill)             \
        M2(CoverBounds,      RegularCover)          \
        M2(CoverBounds,      InverseCover)          \
        M1(CoverageMask)                            \
        M2(BitmapText,       Mask)                  \
        M2(BitmapText,       LCD)                   \
        M2(BitmapText,       Color)                 \
        M2(MiddleOutFan,     EvenOdd)               \
        M2(MiddleOutFan,     Winding)               \
        M1(SDFTextLCD)                              \
        M1(SDFText)                                 \
        M2(TessellateCurves, EvenOdd)               \
        M2(TessellateCurves, Winding)               \
        M1(TessellateStrokes)                       \
        M2(TessellateWedges, Convex)                \
        M2(TessellateWedges, EvenOdd)               \
        M2(TessellateWedges, Winding)               \
        M2(Vertices,         Tris)                  \
        M2(Vertices,         TrisColor)             \
        M2(Vertices,         TrisTexCoords)         \
        M2(Vertices,         TrisColorTexCoords)    \
        M2(Vertices,         Tristrips)             \
        M2(Vertices,         TristripsColor)        \
        M2(Vertices,         TristripsTexCoords)    \
        M2(Vertices,         TristripsColorTexCoords)

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
/**
 * Each RenderStep may have "Static" data and/or "Append" data. Each type of data has associated
 * attributes, strides, and layouts (see fStaticAttrs, fAppendAttrs, etc.), and resides on a
 * specific binding on the GPU. This minimizes the number of bindings during a draw pass.
 * - Static data is information that is fixed in count, does not change between calls of the same
 *   RenderStep, and known after recieving device capabilities. Consequently, it is uploaded ONCE by
 *   the StaticBufferManager prior to any drawPasses, and is initialized during the constructor of a
 *   RenderStep. Currently, static data can be either Indices or Vertices.
 * - Append data might not be fixed in count and its' usage is not known prior to the draw pass.
 *   Instead, it is uploaded as needed during drawPass through the overload of the writeVertices()
 *   function. Currently, either Vertices or Instances can be appended, and this can be queried by
 *   getRenderStateFlags().
 */
class RenderStep {
public:
    virtual ~RenderStep() = default;

    // Returns an empty result if no state change is necessary, otherwise returns the scissor rect
    // that should be active for all draws recorded by a subsequent call to writeVertices().
    std::optional<SkIRect> getScissor(const DrawParams&,
                                      SkIRect currentScissor,
                                      SkIRect deviceBounds) const;

    // The DrawWriter is configured with the vertex and instance strides of the RenderStep, and its
    // primitive type. The recorded draws will be executed with a graphics pipeline compatible with
    // this RenderStep.
    virtual void writeVertices(DrawWriter*, const DrawParams&, skvx::uint2 ssboIndices) const = 0;

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
        return "";
    }

    // Emits code to set up coverage value. Should only be defined if overridesCoverage is true.
    // When implemented the returned SkSL fragment should write its coverage into a
    // 'half4 outputCoverage' variable (defined in the calling code) with the actual
    // coverage splatted out into all four channels.
    virtual const char* fragmentCoverageSkSL() const { return ""; }

    // Emits code to set up a primitive color value. Should only be defined if emitsPrimitiveColor
    // is true. When implemented, the returned SkSL fragment should write its color into a
    // 'half4 primitiveColor' variable (defined in the calling code).
    virtual const char* fragmentColorSkSL() const { return ""; }

    // Indicates whether this RenderStep's uniforms are referenced in its fragment shader code.
    // If not, its uniforms can be omitted from the fragment shader entirely.
    // By default, we assume that RenderSteps use their uniforms for emitting coverage or primitive
    // colors.
    virtual bool usesUniformsInFragmentSkSL() const {
        return this->coverage() != Coverage::kNone || this->emitsPrimitiveColor();
    }

    // Returns a name formatted as "Subclass[variant]", where "Subclass" matches the C++ class name
    // and variant is a unique term describing instance's specific configuration.
    const char* name() const { return RenderStepName(fRenderStepID); }

    bool requiresMSAA()        const { return SkToBool(fFlags & Flags::kRequiresMSAA);        }
    bool performsShading()     const { return SkToBool(fFlags & Flags::kPerformsShading);     }
    bool hasTextures()         const { return SkToBool(fFlags & Flags::kHasTextures);         }
    bool emitsPrimitiveColor() const { return SkToBool(fFlags & Flags::kEmitsPrimitiveColor); }
    bool outsetBoundsForAA()   const { return SkToBool(fFlags & Flags::kOutsetBoundsForAA);   }
    bool useNonAAInnerFill()   const { return SkToBool(fFlags & Flags::kUseNonAAInnerFill);   }
    bool appendsVertices()     const { return SkToBool(fFlags & Flags::kAppendVertices);      }
    SkEnumBitMask<RenderStateFlags> getRenderStateFlags() const {
        SkEnumBitMask<RenderStateFlags> rs = RenderStateFlags::kNone;
        if (fFlags & Flags::kFixed)             { rs |= RenderStateFlags::kFixed;           }
        if (fFlags & Flags::kAppendVertices)    { rs |= RenderStateFlags::kAppendVertices;  }
        if (fFlags & Flags::kAppendInstances)   { rs |= RenderStateFlags::kAppendInstances; }
        if (fFlags & Flags::kAppendDynamicInstances) {
             rs |= RenderStateFlags::kAppendDynamicInstances;
        }
        return rs;
    }

    Coverage coverage() const { return RenderStep::GetCoverage(fFlags); }

    PrimitiveType primitiveType()    const { return fPrimitiveType;    }
    size_t        staticDataStride() const { return fStaticDataStride; }
    size_t        appendDataStride() const { return fAppendDataStride; }

    size_t numUniforms()         const { return fUniforms.size();    }
    size_t numStaticAttributes() const { return fStaticAttrs.size(); }
    size_t numAppendAttributes() const { return fAppendAttrs.size(); }

    // Name of an attribute containing both render step and shading SSBO indices, if used.
    static const char* ssboIndicesAttribute() { return "ssboIndices"; }

    // Name of a varying to pass SSBO indices to fragment shader. Both render step and shading
    // indices are passed, because render step uniforms are sometimes used for coverage.
    static const char* ssboIndicesVarying() { return "ssboIndicesVar"; }

    // The uniforms of a RenderStep are bound to the kRenderStep slot, the rest of the pipeline
    // may still use uniforms bound to other slots.
    SkSpan<const Uniform>   uniforms()         const { return SkSpan(fUniforms);      }
    SkSpan<const Attribute> staticAttributes() const { return SkSpan(fStaticAttrs);   }
    SkSpan<const Attribute> appendAttributes() const { return SkSpan(fAppendAttrs);   }
    SkSpan<const Varying>   varyings()         const { return SkSpan(fVaryings);      }

    const DepthStencilSettings& depthStencilSettings() const { return fDepthStencilSettings; }

    SkEnumBitMask<DepthStencilFlags> depthStencilFlags() const {
        return (fDepthStencilSettings.fStencilTestEnabled
                        ? DepthStencilFlags::kStencil : DepthStencilFlags::kNone) |
               (fDepthStencilSettings.fDepthTestEnabled || fDepthStencilSettings.fDepthWriteEnabled
                        ? DepthStencilFlags::kDepth : DepthStencilFlags::kNone);
    }

    static const int kRenderStepIDVersion = 1;

#define ENUM1(BaseName) k##BaseName,
#define ENUM2(BaseName, VariantName) k##BaseName##_##VariantName,
    enum class RenderStepID : uint32_t {
        SKGPU_RENDERSTEP_TYPES(ENUM1, ENUM2)

        kLast = kVertices_TristripsColorTexCoords,
    };
#undef ENUM1
#undef ENUM2
    static const int kNumRenderSteps = static_cast<int>(RenderStepID::kLast) + 1;

    RenderStepID renderStepID() const { return fRenderStepID; }

    static const char* RenderStepName(RenderStepID);
    static bool IsValidRenderStepID(uint32_t);

    // TODO: Actual API to do things
    // 6. Some Renderers benefit from being able to share vertices between RenderSteps. Must find a
    //    way to support that. It may mean that RenderSteps get state per draw.
    //    - Does Renderer make RenderStepFactories that create steps for each DrawList::Draw?
    //    - Does DrawList->DrawPass conversion build a separate array of blind data that the
    //      stateless Renderstep can refer to for {draw,step} pairs?
    //    - Does each DrawList::Draw have extra space (e.g. 8 bytes) that steps can cache data in?
protected:
enum class Flags : unsigned {
    kNone                   = 0x0000,
    kFixed                  = 0x0001, // Uses explicit DrawWriter::draw functions
    kAppendVertices         = 0x0002, // Appends vertices
    kAppendInstances        = 0x0004, // Appends instances with static vertex count
    kAppendDynamicInstances = 0x0008, // Appends instances with a flexible vertex count
    kRequiresMSAA           = 0x0010, // MSAA is required for anti-aliasing
    kPerformsShading        = 0x0020, // This step is responsible for shading/color output
    kHasTextures            = 0x0040, // Adds textures via overridden texturesAndSamplersSkSL()
    kEmitsCoverage          = 0x0080, // Adds analytic coverage via fragmentCoverageSkSL()
    kLCDCoverage            = 0x0100, // The added analytic coverage is LCD, not single channel
    kEmitsPrimitiveColor    = 0x0200, // Injects primitive color via fragmentColorSkSL()
    kOutsetBoundsForAA      = 0x0400, // Drawn geometry will be outset beyond shape's bounds for AA
    kUseNonAAInnerFill      = 0x0800, // Opt into Device recording extra inner fill draws
    kIgnoreInverseFill      = 0x1000, // Rasterization treats all shapes as non-inverted for scissor
    kInverseFillsScissor    = 0x2000, // Rasterization of inverse fills scissor geometrically
};
SK_DECL_BITMASK_OPS_FRIENDS(Flags)

    // While RenderStep does not define the full program that's run for a draw, it defines the
    // entire vertex layout of the pipeline. This is not allowed to change, so can be provided to
    // the RenderStep constructor by subclasses.
    RenderStep(RenderStepID renderStepID,
               SkEnumBitMask<Flags> flags,
               std::initializer_list<Uniform> uniforms,
               PrimitiveType primitiveType,
               DepthStencilSettings depthStencilSettings,
               SkSpan<const Attribute> staticAttrs,
               SkSpan<const Attribute> appendAttrs,
               SkSpan<const Varying> varyings = {});

private:
    friend class Renderer; // for Flags

    // Cannot copy or move
    RenderStep(const RenderStep&) = delete;
    RenderStep(RenderStep&&)      = delete;

    static Coverage GetCoverage(SkEnumBitMask<Flags>);

    RenderStepID fRenderStepID;
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
    std::vector<Attribute> fStaticAttrs;
    std::vector<Attribute> fAppendAttrs;
    std::vector<Varying>   fVaryings;

    size_t fStaticDataStride; // derived from vertex attribute set
    size_t fAppendDataStride; // derived from instance attribute set
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
    bool useNonAAInnerFill() const {
        return SkToBool(fStepFlags & StepFlags::kUseNonAAInnerFill);
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
        // A render step using non-AA inner fills with a second draw should not also be part of a
        // multi-step renderer (to keep reasoning simple) and must use the GREATER depth test.
        SkASSERT(!this->useNonAAInnerFill() ||
                 (fStepCount == 1 && fSteps[0]->depthStencilSettings().fDepthTestEnabled &&
                  fSteps[0]->depthStencilSettings().fDepthCompareOp == CompareOp::kGreater));
    }

    // For RendererProvider to manage initialization; it will never expose a Renderer that is only
    // default-initialized and not replaced because it's algorithm is disabled by caps/options.
    Renderer() : fSteps(), fName(""), fStepCount(0) {}
    Renderer& operator=(Renderer&&) = default;

    std::array<const RenderStep*, kMaxRenderSteps> fSteps;
    std::string fName;
    DrawTypeFlags fDrawTypes = DrawTypeFlags::kNone;
    int fStepCount;

    SkEnumBitMask<StepFlags> fStepFlags = StepFlags::kNone;
    SkEnumBitMask<DepthStencilFlags> fDepthStencilFlags = DepthStencilFlags::kNone;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Renderer_DEFINED
