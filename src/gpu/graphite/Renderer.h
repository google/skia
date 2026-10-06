/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Renderer_DEFINED
#define skgpu_graphite_Renderer_DEFINED

#include "include/core/SkPathTypes.h"
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/private/SkEnumBitMask.h"
#include "src/core/SkVx.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/DescriptorData.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/Uniform.h"

#include <array>
#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace skgpu { enum class MaskFormat; }

namespace skgpu::graphite {

class DrawParams;
class DrawWriter;
class PipelineDataGatherer;
class Rect;
class ResourceProvider;
class StorageContext;
class TextureDataBlock;
class Transform;
class UniformOffsetCalculator;

struct ResourceBindingRequirements;
struct RootNodesInfo;

enum class Coverage { kNone, kSingleChannel, kLCD };

// If this list is modified in any way, please increment the
// RenderStep::kRenderStepIDVersion value. The enum values generated from this
// list are serialized and the kRenderStepIDVersion value is the signal to
// abandon older serialized data.
#define SKGPU_RENDERSTEP_TYPES(M1, M2)          \
        M1(Invalid)                             \
        M1(CircularArc)                         \
        M1(AnalyticRRect)                       \
        M1(AnalyticBlur)                        \
        M1(AnalyticRRectBlur)                   \
        M1(PerEdgeAAQuad)                       \
        M2(CoverBounds,      NonAAFill)         \
        M2(CoverBounds,      RegularCover)      \
        M2(CoverBounds,      InverseCover)      \
        M1(CoverageMask)                        \
        M2(BitmapText,       Mask)              \
        M2(BitmapText,       LCD)               \
        M2(BitmapText,       Color)             \
        M2(MiddleOutFan,     EvenOdd)           \
        M2(MiddleOutFan,     Winding)           \
        M1(SDFTextLCD)                          \
        M1(SDFText)                             \
        M2(TessellateCurves, EvenOdd)           \
        M2(TessellateCurves, Winding)           \
        M2(TessellateStrokes,Fill)              \
        M2(TessellateStrokes,InverseFill)       \
        M2(TessellateWedges, Convex)            \
        M2(TessellateWedges, EvenOdd)           \
        M2(TessellateWedges, Winding)           \
        M2(Vertices,         Pos)               \
        M2(Vertices,         PosColor)          \
        M2(Vertices,         PosTexCoords)      \
        M2(Vertices,         PosColorTexCoords) \
        M1(Mesh)                                \
        M1(EndCap)                              \
        M1(WideTile)

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
    virtual void writeVertices(DrawWriter*,
                               StorageContext*,
                               const DrawParams&,
                               uint32_t ssboIndex) const = 0;

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
    virtual std::string vertexSkSL(const RootNodesInfo&) const = 0;

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
    virtual std::string fragmentColorSkSL(const RootNodesInfo&) const { return ""; }

    // Returns a pointer to the name of the local coordinates variable to use for
    // shader sampling if non-null.
    virtual const char* fragmentColorSkSLLocalCoordsVariable() const { return nullptr; }

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
    bool vsUsesStorage()       const { return SkToBool(fFlags & Flags::kVsUsesStorage);       }
    bool fsUsesStorage()       const { return SkToBool(fFlags & Flags::kFsUsesStorage);       }

    SkEnumBitMask<PipelineStageFlags> storageBufferStages() const { return fStorageBufferStages; }
    SkEnumBitMask<RenderStateFlags>   getRenderStateFlags() const {
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

    PrimitiveType  primitiveType()    const { return fPrimitiveType;    }
    size_t         staticDataStride() const { return fStaticDataStride; }
    virtual size_t appendDataStride(const DrawParams& params) const { return fAppendDataStride; }

    size_t storageUniformStride() const { return fStorageUniformStride;    }
    size_t storageUniformAlignment() const { return fStorageUniformAlignment; }

    size_t numUniforms()          const { return fUniforms.size();        }
    int    uniformAlignment()     const { return fUniformAlignment;       }
    size_t numStaticAttributes()  const { return fStaticAttrs.size();     }
    size_t numAppendAttributes()  const { return fAppendAttrs.size();     }
    size_t numStorageUniforms()   const { return fStorageUniforms.size(); }

    // Name of an attribute containing both the render step and shading SSBO index, if used.
    static const char* ssboIndexAttribute() { return "ssboIndex"; }

    // Name of a varying to pass the SSBO index to fragment shader
    static const char* ssboIndexVarying() { return "ssboIndexVar"; }

    // The uniforms of a RenderStep are bound to the kRenderStep slot, the rest of the pipeline
    // may still use uniforms bound to other slots.
    SkSpan<const Uniform>   uniforms()          const { return SkSpan(fUniforms);        }
    SkSpan<const Attribute> staticAttributes()  const { return SkSpan(fStaticAttrs);     }
    SkSpan<const Attribute> appendAttributes()  const { return SkSpan(fAppendAttrs);     }
    SkSpan<const Uniform>   storageUniforms()   const { return SkSpan(fStorageUniforms); }
    SkSpan<const Varying>   varyings()          const { return SkSpan(fVaryings);        }

    // RenderSteps have control over how they modify the stencil buffer, with the requirement that
    // a Renderer must ensure such that their last step leaves the stencil set to 0.
    //
    // RenderSteps cannot control depth settings as the depth attachment is used more fluidly to
    // ensure rendering correctness. Instead, the render step's flags are the source of truth for
    // its depth behavior. These depth settings must be consistent with those flags under the
    // assumption that they are used by a renderpass that has depth and is combined with a paint
    // that is opaque. ShaderInfo inspects the flags to relax the settings and the renderpass
    // when possible.
    const DepthStencilSettings& depthStencilSettings() const { return fDepthStencilSettings; }

    bool usesStencil() const { return fDepthStencilSettings.stencilEnabled(); }
    bool usesDepth() const { return fDepthStencilSettings.depthEnabled(); }

    // This is true if the RenderStep forces a renderpass to have a depth attachment, which is
    // stricter than the depthWrite and depthTest provided in `depthStencilSettings()`, as those
    // represent what should be set *if* depth ends up being used.
    bool requiresDepth() const {
        // NOTE: kUseNonAAInnerFill in a RenderStep will likely trigger the use of depth for the
        // renderpass, but that is a result of it producing a second draw that is ordered front
        // to back, and not strictly-speaking a requirement of the RenderStep's internal behavior.
        // In the event that the inner fill isn't recorded, we don't want to force depth.
        return !SkToBool(fFlags & (Flags::kAllowsSelfIntersection | Flags::kNoSelfIntersections));
    }

    SkEnumBitMask<DepthStencilFlags> depthStencilFlags() const {
        return (this->usesStencil() ? DepthStencilFlags::kStencil : DepthStencilFlags::kNone) |
               (this->usesDepth()   ? DepthStencilFlags::kDepth   : DepthStencilFlags::kNone);
    }

    static const int kRenderStepIDVersion = 2;

#define ENUM1(BaseName) k##BaseName,
#define ENUM2(BaseName, VariantName) k##BaseName##_##VariantName,
    enum class RenderStepID : uint32_t {
        SKGPU_RENDERSTEP_TYPES(ENUM1, ENUM2)

        kLast = kWideTile,
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
    kNone                   = 0,

    // Attribute/uniform/layout properties
    kFixed                  = 1 << 0,  // Uses explicit DrawWriter::draw functions
    kAppendVertices         = 1 << 1,  // Appends vertices
    kAppendInstances        = 1 << 2,  // Appends instances with static vertex count
    kAppendDynamicInstances = 1 << 3,  // Appends instances with a flexible vertex count
    kVsUsesStorage          = 1 << 4,  // Does the vertex shader require storage buffer access?
    kFsUsesStorage          = 1 << 5,  // Does the fragment shader require storage buffer access?

    // Shading behavior
    kPerformsShading        = 1 << 6,  // This step is responsible for shading/color output
    kHasTextures            = 1 << 7,  // Adds textures via overridden texturesAndSamplersSkSL()
    kEmitsCoverage          = 1 << 8,  // Adds analytic coverage via fragmentCoverageSkSL()
    kLCDCoverage            = 1 << 9,  // The added analytic coverage is LCD, not single channel
    kEmitsPrimitiveColor    = 1 << 10, // Injects primitive color via fragmentColorSkSL()

    // Rasterization/geometry properties
    kRequiresMSAA           = 1 << 11, // MSAA is required for anti-aliasing
    kAllowsSelfIntersection = 1 << 12, // Rendered triangles can self-intersect, but that's desired.
    kNoSelfIntersections    = 1 << 13, // Rendered triangles will never self-intersect (if neither
                                       // this nor kAllowsSelfIntersection is set, the depth test is
                                       // used to avoid self intersections).
    kOutsetBoundsForAA      = 1 << 14, // Drawn geometry will be outset beyond shape's bounds for AA
    kUseNonAAInnerFill      = 1 << 15, // Opt into Device recording extra inner fill draws
    kIgnoreInverseFill      = 1 << 16, // Rasterization treats all shapes as noninverted for scissor
    kInverseFillsScissor    = 1 << 17, // Rasterization of inverse fills scissor geometrically
};
SK_DECL_BITMASK_OPS_FRIENDS(Flags)

    // While RenderStep does not define the full program that's run for a draw, it defines the
    // entire vertex layout of the pipeline. This is not allowed to change, so can be provided to
    // the RenderStep constructor by subclasses.
    RenderStep(Layout layout,
               RenderStepID renderStepID,
               SkEnumBitMask<Flags> flags,
               std::initializer_list<Uniform> uniforms,
               PrimitiveType primitiveType,
               DepthStencilSettings depthStencilSettings,
               SkSpan<const Attribute> staticAttrs,
               SkSpan<const Attribute> appendAttrs,
               SkSpan<const Uniform> storageUniforms = {},
               SkSpan<const Varying> varyings = {});

private:
    friend class Renderer; // for Flags

    // Cannot copy or move
    RenderStep(const RenderStep&) = delete;
    RenderStep(RenderStep&&)      = delete;

    static Coverage GetCoverage(SkEnumBitMask<Flags>);

    RenderStepID                      fRenderStepID;
    SkEnumBitMask<Flags>              fFlags;
    SkEnumBitMask<PipelineStageFlags> fStorageBufferStages;
    PrimitiveType                     fPrimitiveType;
    DepthStencilSettings              fDepthStencilSettings;

    // TODO: When we always use C++17 for builds, we should be able to just let subclasses declare
    // constexpr arrays and point to those, but we need explicit storage for C++14.
    // Alternatively, if we imposed a max attr count, similar to Renderer's num render steps, we
    // could just have this be std::array and keep all attributes inline with the RenderStep memory.
    // On the other hand, the attributes are only needed when creating a new pipeline so it's not
    // that performance sensitive.
    std::vector<Uniform>   fUniforms;
    std::vector<Attribute> fStaticAttrs;
    std::vector<Attribute> fAppendAttrs;
    std::vector<Uniform>   fStorageUniforms;
    std::vector<Varying>   fVaryings;

    int    fUniformAlignment;        // derived from the renderstep uniforms
    size_t fStaticDataStride;        // derived from vertex attribute set
    size_t fAppendDataStride;        // derived from instance attribute set
    size_t fStorageUniformStride;    // derived from storage uniform set
    size_t fStorageUniformAlignment; // derived from storage uniform set
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
    bool usesStencil() const {
        return SkToBool(fDepthStencilFlags & DepthStencilFlags::kStencil);
    }
    // TODO(michaelludwig): Once ShaderInfo controls basic depth settings,
    // fDepthStencilFlags::kDepth will match requiresDepth() and this can be simplified.
    bool requiresDepth() const {
        for (int i = 0; i < fStepCount; ++i) {
            if (fSteps[i]->requiresDepth()) {
                return true;
            }
        }
        return false;
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
        // multi-step renderer (to keep reasoning simple).
        SkASSERT(!this->useNonAAInnerFill() || fStepCount == 1);
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
