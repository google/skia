/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGeometryProcessor_DEFINED
#define GrGeometryProcessor_DEFINED

#include "include/gpu/GrBackendSurface.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrColor.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrProcessor.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/GrShaderVar.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/ganesh/glsl/GrGLSLVarying.h"

#include <unordered_map>

class GrGLSLFPFragmentBuilder;
class GrGLSLVaryingHandler;
class GrGLSLUniformHandler;
class GrGLSLVertexBuilder;

/**
 * The GrGeometryProcessor represents some kind of geometric primitive.  This includes the shape
 * of the primitive and the inherent color of the primitive.  The GrGeometryProcessor is
 * responsible for providing a color and coverage input into the Ganesh rendering pipeline. Through
 * optimization, Ganesh may decide a different color, no color, and / or no coverage are required
 * from the GrGeometryProcessor, so the GrGeometryProcessor must be able to support this
 * functionality.
 *
 * There are two feedback loops between the GrFragmentProcessors, the GrXferProcessor, and the
 * GrGeometryProcessor. These loops run on the CPU and to determine known properties of the final
 * color and coverage inputs to the GrXferProcessor in order to perform optimizations that preserve
 * correctness. The GrDrawOp seeds these loops with initial color and coverage, in its
 * getProcessorAnalysisInputs implementation. These seed values are processed by the
 * subsequent stages of the rendering pipeline and the output is then fed back into the GrDrawOp
 * in the applyPipelineOptimizations call, where the op can use the information to inform
 * decisions about GrGeometryProcessor creation.
 *
 * Note that all derived classes should hide their constructors and provide a Make factory
 * function that takes an arena (except for Tesselation-specific classes). This is because
 * geometry processors can be created in either the record-time or flush-time arenas which
 * define their lifetimes (i.e., a DDLs life time in the first case and a single flush in
 * the second case).
 */
class GrGeometryProcessor : public GrProcessor {
public:
    /**
     * Every GrGeometryProcessor must be capable of creating a subclass of ProgramImpl. The
     * ProgramImpl emits the shader code that implements the GrGeometryProcessor, is attached to the
     * generated backend API pipeline/program and used to extract uniform data from
     * GrGeometryProcessor instances.
     */
    class ProgramImpl;

    class TextureSampler;

    /** Describes a vertex or instance attribute. */
    class Attribute {
    public:
        static constexpr size_t AlignOffset(size_t offset) { return SkAlign4(offset); }

        constexpr Attribute() = default;
        /**
         * Makes an attribute whose offset will be implicitly determined by the types and ordering
         * of an array attributes.
         */
        constexpr Attribute(const char* name,
                            GrVertexAttribType cpuType,
                            SkSLType gpuType)
                : fName(name), fCPUType(cpuType), fGPUType(gpuType) {
            SkASSERT(name && gpuType != SkSLType::kVoid);
        }
        /**
         * Makes an attribute with an explicit offset.
         */
        constexpr Attribute(const char*        name,
                            GrVertexAttribType cpuType,
                            SkSLType           gpuType,
                            size_t             offset)
                : fName(name), fCPUType(cpuType), fGPUType(gpuType), fOffset(SkToU32(offset)) {
            SkASSERT(AlignOffset(offset) == offset);
            SkASSERT(name && gpuType != SkSLType::kVoid);
        }
        constexpr Attribute(const Attribute&) = default;

        Attribute& operator=(const Attribute&) = default;

        constexpr bool isInitialized() const { return fGPUType != SkSLType::kVoid; }

        constexpr const char*           name() const { return fName; }
        constexpr GrVertexAttribType cpuType() const { return fCPUType; }
        constexpr SkSLType           gpuType() const { return fGPUType; }
        /**
         * Returns the offset if attributes were specified with explicit offsets. Otherwise,
         * offsets (and total vertex stride) are implicitly determined from attribute order and
         * types.
         */
        std::optional<size_t> offset() const {
            if (fOffset != kImplicitOffset) {
                SkASSERT(AlignOffset(fOffset) == fOffset);
                return {fOffset};
            }
            return std::nullopt;
        }

        inline constexpr size_t size() const;

        GrShaderVar asShaderVar() const {
            return {fName, fGPUType, GrShaderVar::TypeModifier::In};
        }

    private:
        static constexpr uint32_t kImplicitOffset = 1;  // 1 is not valid because it isn't aligned.

        const char*        fName    = nullptr;
        GrVertexAttribType fCPUType = kFloat_GrVertexAttribType;
        SkSLType           fGPUType = SkSLType::kVoid;
        uint32_t           fOffset  = kImplicitOffset;
    };

    /**
     * A set of attributes that can iterated. The iterator handles hides two pieces of complexity:
     * 1) It skips uninitialized attributes.
     * 2) It always returns an attribute with a known offset.
     */
    class AttributeSet {
        class Iter {
        public:
            Iter() = default;
            Iter(const Iter& iter) = default;
            Iter& operator=(const Iter& iter) = default;

            Iter(const Attribute* attrs, int count) : fCurr(attrs), fRemaining(count) {
                this->skipUninitialized();
            }

            bool operator!=(const Iter& that) const { return fCurr != that.fCurr; }
            Attribute operator*() const;
            void operator++();

        private:
            void skipUninitialized();

            const Attribute* fCurr           = nullptr;
            int              fRemaining      = 0;
            size_t           fImplicitOffset = 0;
        };

    public:
        Iter begin() const;
        Iter end() const;

        int count() const { return fCount; }
        size_t stride() const { return fStride; }

        // Init with implicit offsets and stride. No attributes can have a predetermined stride.
        void initImplicit(const Attribute* attrs, int count);
        // Init with explicit offsets and stride. All attributes must be initialized and have
        // an explicit offset aligned to 4 bytes and with no attribute crossing stride boundaries.
        void initExplicit(const Attribute* attrs, int count, size_t stride);

        void addToKey(skgpu::KeyBuilder* b) const;

    private:
        const Attribute* fAttributes = nullptr;
        int              fRawCount = 0;
        int              fCount = 0;
        size_t           fStride = 0;
    };

    GrGeometryProcessor(ClassID);

    int numTextureSamplers() const { return fTextureSamplerCnt; }
    const TextureSampler& textureSampler(int index) const;
    int numVertexAttributes() const { return fVertexAttributes.count(); }
    const AttributeSet& vertexAttributes() const { return fVertexAttributes; }
    int numInstanceAttributes() const { return fInstanceAttributes.count(); }
    const AttributeSet& instanceAttributes() const { return fInstanceAttributes; }

    bool hasVertexAttributes() const { return SkToBool(fVertexAttributes.count()); }
    bool hasInstanceAttributes() const { return SkToBool(fInstanceAttributes.count()); }

    /**
     * A common practice is to populate the the vertex/instance's memory using an implicit array of
     * structs. In this case, it is best to assert that:
     *     stride == sizeof(struct)
     */
    size_t vertexStride() const { return fVertexAttributes.stride(); }
    size_t instanceStride() const { return fInstanceAttributes.stride(); }

    /**
     * Computes a key for the transforms owned by an FP based on the shader code that will be
     * emitted by the primitive processor to implement them.
     */
    static uint32_t ComputeCoordTransformsKey(const GrFragmentProcessor& fp);

    inline static constexpr int kCoordTransformKeyBits = 4;

    /**
     * Adds a key on the skgpu::KeyBuilder that reflects any variety in the code that the
     * geometry processor subclass can emit.
     */
    virtual void addToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const = 0;

    void getAttributeKey(skgpu::KeyBuilder* b) const;

    /**
     * Returns a new instance of the appropriate implementation class for the given
     * GrGeometryProcessor.
     */
    virtual std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const = 0;

protected:
    // GPs that need to use either float or ubyte colors can just call this to get a correctly
    // configured Attribute struct
    static Attribute MakeColorAttribute(const char* name, bool wideColor) {
        return { name,
                 wideColor ? kFloat4_GrVertexAttribType : kUByte4_norm_GrVertexAttribType,
                 SkSLType::kHalf4 };
    }
    void setVertexAttributes(const Attribute* attrs, int attrCount, size_t stride) {
        fVertexAttributes.initExplicit(attrs, attrCount, stride);
    }
    void setInstanceAttributes(const Attribute* attrs, int attrCount, size_t stride) {
        SkASSERT(attrCount >= 0);
        fInstanceAttributes.initExplicit(attrs, attrCount, stride);
    }

    void setVertexAttributesWithImplicitOffsets(const Attribute* attrs, int attrCount) {
        fVertexAttributes.initImplicit(attrs, attrCount);
    }
    void setInstanceAttributesWithImplicitOffsets(const Attribute* attrs, int attrCount) {
        SkASSERT(attrCount >= 0);
        fInstanceAttributes.initImplicit(attrs, attrCount);
    }
    void setTextureSamplerCnt(int cnt) {
        SkASSERT(cnt >= 0);
        fTextureSamplerCnt = cnt;
    }

private:
    virtual const TextureSampler& onTextureSampler(int) const { SK_ABORT("no texture samplers"); }

    AttributeSet fVertexAttributes;
    AttributeSet fInstanceAttributes;

    int fTextureSamplerCnt = 0;
    using INHERITED = GrProcessor;
};

//////////////////////////////////////////////////////////////////////////////

class GrGeometryProcessor::ProgramImpl {
public:
    using UniformHandle = GrGLSLProgramDataManager::UniformHandle;
    using SamplerHandle = GrGLSLUniformHandler::SamplerHandle;
    /**
     * Struct of optional varying that replaces the input coords and bool indicating whether the FP
     * should take a coord param as an argument. The latter may be false if the coords are simply
     * unused or if the GP has lifted their computation to a varying emitted by the VS.
     */
    struct FPCoords {GrShaderVar coordsVarying; bool hasCoordsParam;};
    using FPCoordsMap = std::unordered_map<const GrFragmentProcessor*, FPCoords>;

    virtual ~ProgramImpl() = default;

    struct EmitArgs {
        EmitArgs(GrGLSLVertexBuilder* vertBuilder,
                 GrGLSLFPFragmentBuilder* fragBuilder,
                 GrGLSLVaryingHandler* varyingHandler,
                 GrGLSLUniformHandler* uniformHandler,
                 const GrShaderCaps* caps,
                 const GrGeometryProcessor& geomProc,
                 const char* outputColor,
                 const char* outputCoverage,
                 const SamplerHandle* texSamplers)
                : fVertBuilder(vertBuilder)
                , fFragBuilder(fragBuilder)
                , fVaryingHandler(varyingHandler)
                , fUniformHandler(uniformHandler)
                , fShaderCaps(caps)
                , fGeomProc(geomProc)
                , fOutputColor(outputColor)
                , fOutputCoverage(outputCoverage)
                , fTexSamplers(texSamplers) {}
        GrGLSLVertexBuilder* fVertBuilder;
        GrGLSLFPFragmentBuilder* fFragBuilder;
        GrGLSLVaryingHandler* fVaryingHandler;
        GrGLSLUniformHandler* fUniformHandler;
        const GrShaderCaps* fShaderCaps;
        const GrGeometryProcessor& fGeomProc;
        const char* fOutputColor;
        const char* fOutputCoverage;
        const SamplerHandle* fTexSamplers;
    };

    /**
     * Emits the code from this geometry processor into the shaders. For any FP in the pipeline that
     * has its input coords implemented by the GP as a varying, the varying will be accessible in
     * the returned map and should be used when the FP code is emitted. The FS variable containing
     * the GP's output local coords is also returned.
     **/
    std::tuple<FPCoordsMap, GrShaderVar> emitCode(EmitArgs&, const GrPipeline& pipeline);

    /**
     * Called after all effect emitCode() functions, to give the processor a chance to write out
     * additional transformation code now that all uniforms have been emitted.
     * It generates the final code for assigning transformed coordinates to the varyings recorded
     * in the call to collectTransforms(). This must happen after FP code emission so that it has
     * access to any uniforms the FPs registered for uniform sample matrix invocations.
     */
    void emitTransformCode(GrGLSLVertexBuilder* vb, GrGLSLUniformHandler* uniformHandler);

    /**
     * A ProgramImpl instance can be reused with any GrGeometryProcessor that produces the same key.
     * This function reads data from a GrGeometryProcessor and updates any uniform variables
     * required by the shaders created in emitCode(). The GrGeometryProcessor parameter is
     * guaranteed to be of the same type and to have an identical processor key as the
     * GrGeometryProcessor that created this ProgramImpl.
     */
    virtual void setData(const GrGLSLProgramDataManager&,
                         const GrShaderCaps&,
                         const GrGeometryProcessor&) = 0;

    // GPs that use writeOutputPosition and/or writeLocalCoord must incorporate the matrix type
    // into their key, and should use this function or one of the other related helpers.
    static uint32_t ComputeMatrixKey(const GrShaderCaps& caps, const SkMatrix& mat) {
        if (!caps.fReducedShaderMode) {
            if (mat.isIdentity()) {
                return 0b00;
            }
            if (mat.isScaleTranslate()) {
                return 0b01;
            }
        }
        if (!mat.hasPerspective()) {
            return 0b10;
        }
        return 0b11;
    }

    static uint32_t ComputeMatrixKeys(const GrShaderCaps& shaderCaps,
                                      const SkMatrix& viewMatrix,
                                      const SkMatrix& localMatrix) {
        return (ComputeMatrixKey(shaderCaps, viewMatrix) << kMatrixKeyBits) |
               ComputeMatrixKey(shaderCaps, localMatrix);
    }

    static uint32_t AddMatrixKeys(const GrShaderCaps& shaderCaps,
                                  uint32_t flags,
                                  const SkMatrix& viewMatrix,
                                  const SkMatrix& localMatrix) {
        // Shifting to make room for the matrix keys shouldn't lose bits
        SkASSERT(((flags << (2 * kMatrixKeyBits)) >> (2 * kMatrixKeyBits)) == flags);
        return (flags << (2 * kMatrixKeyBits)) |
               ComputeMatrixKeys(shaderCaps, viewMatrix, localMatrix);
    }
    inline static constexpr int kMatrixKeyBits = 2;

protected:
    void setupUniformColor(GrGLSLFPFragmentBuilder* fragBuilder,
                           GrGLSLUniformHandler* uniformHandler,
                           const char* outputName,
                           UniformHandle* colorUniform);

    // A helper for setting the matrix on a uniform handle initialized through
    // writeOutputPosition or writeLocalCoord. Automatically handles elided uniforms,
    // scale+translate matrices, and state tracking (if provided state pointer is non-null).
    static void SetTransform(const GrGLSLProgramDataManager&,
                             const GrShaderCaps&,
                             const UniformHandle& uniform,
                             const SkMatrix& matrix,
                             SkMatrix* state = nullptr);

    struct GrGPArgs {
        // Used to specify the output variable used by the GP to store its device position. It can
        // either be a float2 or a float3 (in order to handle perspective). The subclass sets this
        // in its onEmitCode().
        GrShaderVar fPositionVar;
        // Used to specify the variable storing the draw's local coordinates. It can be either a
        // float2, float3, or void. It can only be void when no FP needs local coordinates. This
        // variable can be an attribute or local variable, but should not itself be a varying.
        // ProgramImpl automatically determines if this must be passed to a FS.
        GrShaderVar fLocalCoordVar;
        // The GP can specify the local coord var either in the VS or FS. When either is possible
        // the VS is preferable. It may allow derived coordinates to be interpolated from the VS
        // instead of computed in the FS per pixel.
        GrShaderType fLocalCoordShader = kVertex_GrShaderType;
    };

    // Helpers for adding code to write the transformed vertex position. The first simple version
    // just writes a variable named by 'posName' into the position output variable with the
    // assumption that the position is 2D. The second version transforms the input position by a
    // view matrix and the output variable is 2D or 3D depending on whether the view matrix is
    // perspective. Both versions declare the output position variable and will set
    // GrGPArgs::fPositionVar.
    static void WriteOutputPosition(GrGLSLVertexBuilder*, GrGPArgs*, const char* posName);
    static void WriteOutputPosition(GrGLSLVertexBuilder*,
                                    GrGLSLUniformHandler*,
                                    const GrShaderCaps&,
                                    GrGPArgs*,
                                    const char* posName,
                                    const SkMatrix& viewMatrix,
                                    UniformHandle* viewMatrixUniform);

    // Helper to transform an existing variable by a given local matrix (e.g. the inverse view
    // matrix). It will declare the transformed local coord variable and will set
    // GrGPArgs::fLocalCoordVar.
    static void WriteLocalCoord(GrGLSLVertexBuilder*,
                                GrGLSLUniformHandler*,
                                const GrShaderCaps&,
                                GrGPArgs*,
                                GrShaderVar localVar,
                                const SkMatrix& localMatrix,
                                UniformHandle* localMatrixUniform);

private:
    virtual void onEmitCode(EmitArgs&, GrGPArgs*) = 0;

    // Iterates over the FPs beginning with the passed iter to register additional varyings and
    // uniforms to support VS-promoted local coord evaluation for the FPs.
    //
    // This must happen before FP code emission so that the FPs can find the appropriate varying
    // handles they use in place of explicit coord sampling; it is automatically called after
    // onEmitCode() returns using the value stored in GpArgs::fLocalCoordVar and
    // GpArgs::fPositionVar.
    FPCoordsMap collectTransforms(GrGLSLVertexBuilder* vb,
                                  GrGLSLVaryingHandler* varyingHandler,
                                  GrGLSLUniformHandler* uniformHandler,
                                  GrShaderType localCoordsShader,
                                  const GrShaderVar& localCoordsVar,
                                  const GrShaderVar& positionVar,
                                  const GrPipeline& pipeline);
    struct TransformInfo {
        // The varying that conveys the coordinates to one or more FPs in the FS.
        GrGLSLVarying varying;
        // The coordinate to be transformed. varying is computed from this.
        GrShaderVar   inputCoords;
        // Used to sort so that ancestor FP varyings are initialized before descendant FP varyings.
        int           traversalOrder;
    };
    // Populated by collectTransforms() for use in emitTransformCode(). When we lift the computation
    // of a FP's input coord to a varying we propagate that varying up the FP tree to the highest
    // node that shares the same coordinates. This allows multiple FPs in a subtree to share a
    // varying.
    std::unordered_map<const GrFragmentProcessor*, TransformInfo> fTransformVaryingsMap;
};

///////////////////////////////////////////////////////////////////////////

/**
 * Used to capture the properties of the GrTextureProxies required/expected by a primitiveProcessor
 * along with an associated GrSamplerState. The actual proxies used are stored in either the
 * fixed or dynamic state arrays. TextureSamplers don't perform any coord manipulation to account
 * for texture origin.
 */
class GrGeometryProcessor::TextureSampler {
public:
    TextureSampler() = default;

    TextureSampler(GrSamplerState, const GrBackendFormat&, const skgpu::Swizzle&);

    TextureSampler(const TextureSampler&) = delete;
    TextureSampler& operator=(const TextureSampler&) = delete;

    TextureSampler(TextureSampler&&) = default;
    TextureSampler& operator=(TextureSampler&&) = default;

    void reset(GrSamplerState, const GrBackendFormat&, const skgpu::Swizzle&);

    const GrBackendFormat& backendFormat() const { return fBackendFormat; }
    GrTextureType textureType() const { return fBackendFormat.textureType(); }

    GrSamplerState samplerState() const { return fSamplerState; }
    const skgpu::Swizzle& swizzle() const { return fSwizzle; }

    bool isInitialized() const { return fIsInitialized; }

private:
    GrSamplerState  fSamplerState;
    GrBackendFormat fBackendFormat;
    skgpu::Swizzle  fSwizzle;
    bool            fIsInitialized = false;
};

//////////////////////////////////////////////////////////////////////////////

/**
 * Returns the size of the attrib type in bytes.
 * This was moved from include/private/gpu/ganesh/GrTypesPriv.h in service of Skia dependents that build
 * with C++11.
 */
static constexpr inline size_t GrVertexAttribTypeSize(GrVertexAttribType type) {
    switch (type) {
        case kFloat_GrVertexAttribType:
            return sizeof(float);
        case kFloat2_GrVertexAttribType:
            return 2 * sizeof(float);
        case kFloat3_GrVertexAttribType:
            return 3 * sizeof(float);
        case kFloat4_GrVertexAttribType:
            return 4 * sizeof(float);
        case kHalf_GrVertexAttribType:
            return sizeof(uint16_t);
        case kHalf2_GrVertexAttribType:
            return 2 * sizeof(uint16_t);
        case kHalf4_GrVertexAttribType:
            return 4 * sizeof(uint16_t);
        case kInt2_GrVertexAttribType:
            return 2 * sizeof(int32_t);
        case kInt3_GrVertexAttribType:
            return 3 * sizeof(int32_t);
        case kInt4_GrVertexAttribType:
            return 4 * sizeof(int32_t);
        case kByte_GrVertexAttribType:
            return 1 * sizeof(char);
        case kByte2_GrVertexAttribType:
            return 2 * sizeof(char);
        case kByte4_GrVertexAttribType:
            return 4 * sizeof(char);
        case kUByte_GrVertexAttribType:
            return 1 * sizeof(char);
        case kUByte2_GrVertexAttribType:
            return 2 * sizeof(char);
        case kUByte4_GrVertexAttribType:
            return 4 * sizeof(char);
        case kUByte_norm_GrVertexAttribType:
            return 1 * sizeof(char);
        case kUByte4_norm_GrVertexAttribType:
            return 4 * sizeof(char);
        case kShort2_GrVertexAttribType:
            return 2 * sizeof(int16_t);
        case kShort4_GrVertexAttribType:
            return 4 * sizeof(int16_t);
        case kUShort2_GrVertexAttribType: // fall through
        case kUShort2_norm_GrVertexAttribType:
            return 2 * sizeof(uint16_t);
        case kInt_GrVertexAttribType:
            return sizeof(int32_t);
        case kUInt_GrVertexAttribType:
            return sizeof(uint32_t);
        case kUShort_norm_GrVertexAttribType:
            return sizeof(uint16_t);
        case kUShort4_norm_GrVertexAttribType:
            return 4 * sizeof(uint16_t);
    }
    // GCC fails because SK_ABORT evaluates to non constexpr. clang and cl.exe think this is
    // unreachable and don't complain.
#if defined(__clang__) || !defined(__GNUC__)
    SK_ABORT("Unsupported type conversion");
#endif
    return 0;
}

constexpr size_t GrGeometryProcessor::Attribute::size() const {
    return GrVertexAttribTypeSize(fCPUType);
}

#endif
