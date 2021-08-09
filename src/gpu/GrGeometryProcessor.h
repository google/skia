/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGeometryProcessor_DEFINED
#define GrGeometryProcessor_DEFINED

#include "src/gpu/GrColor.h"
#include "src/gpu/GrNonAtomicRef.h"
#include "src/gpu/GrProcessor.h"
#include "src/gpu/GrShaderVar.h"
#include "src/gpu/GrSwizzle.h"

class GrGLSLUniformHandler;

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
 *
 * TODO: This class does not really need to be ref counted. Instances should be allocated using
 * GrOpFlushState's arena and destroyed when the arena is torn down.
 */
class GrGeometryProcessor : public GrProcessor, public GrNonAtomicRef<GrGeometryProcessor> {
public:
    /**
     * Any GrGeometryProcessor is capable of creating a subclass of ProgramImpl. The ProgramImpl
     * emits the shader code that implements the GrGeometryProcessor, is attached to the generated
     * backend API pipeline/program and used to extract uniform data from GrGeometryProcessor
     * instances.
     */
    class ProgramImpl;

    class TextureSampler;

    /** Describes a vertex or instance attribute. */
    class Attribute {
    public:
        constexpr Attribute() = default;
        constexpr Attribute(const char* name,
                            GrVertexAttribType cpuType,
                            GrSLType gpuType)
                : fName(name), fCPUType(cpuType), fGPUType(gpuType) {
            SkASSERT(name && gpuType != kVoid_GrSLType);
        }
        constexpr Attribute(const Attribute&) = default;

        Attribute& operator=(const Attribute&) = default;

        constexpr bool isInitialized() const { return fGPUType != kVoid_GrSLType; }

        constexpr const char* name() const { return fName; }
        constexpr GrVertexAttribType cpuType() const { return fCPUType; }
        constexpr GrSLType           gpuType() const { return fGPUType; }

        inline constexpr size_t size() const;
        constexpr size_t sizeAlign4() const { return SkAlign4(this->size()); }

        GrShaderVar asShaderVar() const {
            return {fName, fGPUType, GrShaderVar::TypeModifier::In};
        }

    private:
        const char* fName = nullptr;
        GrVertexAttribType fCPUType = kFloat_GrVertexAttribType;
        GrSLType fGPUType = kVoid_GrSLType;
    };

    class Iter {
    public:
        Iter() : fCurr(nullptr), fRemaining(0) {}
        Iter(const Iter& iter) : fCurr(iter.fCurr), fRemaining(iter.fRemaining) {}
        Iter& operator= (const Iter& iter) {
            fCurr = iter.fCurr;
            fRemaining = iter.fRemaining;
            return *this;
        }
        Iter(const Attribute* attrs, int count) : fCurr(attrs), fRemaining(count) {
            this->skipUninitialized();
        }

        bool operator!=(const Iter& that) const { return fCurr != that.fCurr; }
        const Attribute& operator*() const { return *fCurr; }
        void operator++() {
            if (fRemaining) {
                fRemaining--;
                fCurr++;
                this->skipUninitialized();
            }
        }

    private:
        void skipUninitialized() {
            if (!fRemaining) {
                fCurr = nullptr;
            } else {
                while (!fCurr->isInitialized()) {
                    ++fCurr;
                }
            }
        }

        const Attribute* fCurr;
        int fRemaining;
    };

    class AttributeSet {
    public:
        Iter begin() const { return Iter(fAttributes, fCount); }
        Iter end() const { return Iter(); }

        int count() const { return fCount; }
        size_t stride() const { return fStride; }

    private:
        friend class GrGeometryProcessor;

        void init(const Attribute* attrs, int count) {
            fAttributes = attrs;
            fRawCount = count;
            fCount = 0;
            fStride = 0;
            for (int i = 0; i < count; ++i) {
                if (attrs[i].isInitialized()) {
                    fCount++;
                    fStride += attrs[i].sizeAlign4();
                }
            }
        }

        const Attribute* fAttributes = nullptr;
        int              fRawCount = 0;
        int              fCount = 0;
        size_t           fStride = 0;
    };

    GrGeometryProcessor(ClassID);

    int numTextureSamplers() const { return fTextureSamplerCnt; }
    const TextureSampler& textureSampler(int index) const;
    int numVertexAttributes() const { return fVertexAttributes.fCount; }
    const AttributeSet& vertexAttributes() const { return fVertexAttributes; }
    int numInstanceAttributes() const { return fInstanceAttributes.fCount; }
    const AttributeSet& instanceAttributes() const { return fInstanceAttributes; }

    bool hasVertexAttributes() const { return SkToBool(fVertexAttributes.fCount); }
    bool hasInstanceAttributes() const { return SkToBool(fInstanceAttributes.fCount); }

    /**
     * A common practice is to populate the the vertex/instance's memory using an implicit array of
     * structs. In this case, it is best to assert that:
     *     stride == sizeof(struct)
     */
    size_t vertexStride() const { return fVertexAttributes.fStride; }
    size_t instanceStride() const { return fInstanceAttributes.fStride; }

    bool willUseTessellationShaders() const {
        return fShaders & (kTessControl_GrShaderFlag | kTessEvaluation_GrShaderFlag);
    }

    bool willUseGeoShader() const {
        return fShaders & kGeometry_GrShaderFlag;
    }

    /**
     * Computes a key for the transforms owned by an FP based on the shader code that will be
     * emitted by the primitive processor to implement them.
     */
    static uint32_t ComputeCoordTransformsKey(const GrFragmentProcessor& fp);

    static constexpr int kCoordTransformKeyBits = 4;

    /**
     * Adds a key on the GrProcessorKeyBuilder that reflects any variety in the code that the
     * geometry processor subclass can emit.
     */
    virtual void addToKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const = 0;

    void getAttributeKey(GrProcessorKeyBuilder* b) const {
        // Ensure that our CPU and GPU type fields fit together in a 32-bit value, and we never
        // collide with the "uninitialized" value.
        static_assert(kGrVertexAttribTypeCount < (1 << 8), "");
        static_assert(kGrSLTypeCount           < (1 << 8), "");

        auto add_attributes = [=](const Attribute* attrs, int attrCount) {
            for (int i = 0; i < attrCount; ++i) {
                const Attribute& attr = attrs[i];
                b->appendComment(attr.isInitialized() ? attr.name() : "unusedAttr");
                b->addBits(8, attr.isInitialized() ? attr.cpuType() : 0xff, "attrType");
                b->addBits(8, attr.isInitialized() ? attr.gpuType() : 0xff, "attrGpuType");
            }
        };
        b->add32(fVertexAttributes.fRawCount, "numVertexAttributes");
        add_attributes(fVertexAttributes.fAttributes, fVertexAttributes.fRawCount);
        b->add32(fInstanceAttributes.fRawCount, "numInstanceAttributes");
        add_attributes(fInstanceAttributes.fAttributes, fInstanceAttributes.fRawCount);
    }

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
                 kHalf4_GrSLType };
    }

    void setVertexAttributes(const Attribute* attrs, int attrCount) {
        fVertexAttributes.init(attrs, attrCount);
    }
    void setInstanceAttributes(const Attribute* attrs, int attrCount) {
        SkASSERT(attrCount >= 0);
        fInstanceAttributes.init(attrs, attrCount);
    }
    void setWillUseTessellationShaders() {
        fShaders |= kTessControl_GrShaderFlag | kTessEvaluation_GrShaderFlag;
    }
    void setWillUseGeoShader() { fShaders |= kGeometry_GrShaderFlag; }
    void setTextureSamplerCnt(int cnt) {
        SkASSERT(cnt >= 0);
        fTextureSamplerCnt = cnt;
    }

    /**
     * Helper for implementing onTextureSampler(). E.g.:
     * return IthTexureSampler(i, fMyFirstSampler, fMySecondSampler, fMyThirdSampler);
     */
    template <typename... Args>
    static const TextureSampler& IthTextureSampler(int i, const TextureSampler& samp0,
                                                   const Args&... samps) {
        return (0 == i) ? samp0 : IthTextureSampler(i - 1, samps...);
    }
    inline static const TextureSampler& IthTextureSampler(int i);

private:
    virtual const TextureSampler& onTextureSampler(int) const { return IthTextureSampler(0); }

    GrShaderFlags fShaders = kVertex_GrShaderFlag | kFragment_GrShaderFlag;

    AttributeSet fVertexAttributes;
    AttributeSet fInstanceAttributes;

    int fTextureSamplerCnt = 0;
    using INHERITED = GrProcessor;
};

//////////////////////////////////////////////////////////////////////////////

/**
 * Used to capture the properties of the GrTextureProxies required/expected by a primitiveProcessor
 * along with an associated GrSamplerState. The actual proxies used are stored in either the
 * fixed or dynamic state arrays. TextureSamplers don't perform any coord manipulation to account
 * for texture origin.
 */
class GrGeometryProcessor::TextureSampler {
public:
    TextureSampler() = default;

    TextureSampler(GrSamplerState, const GrBackendFormat&, const GrSwizzle&);

    TextureSampler(const TextureSampler&) = delete;
    TextureSampler& operator=(const TextureSampler&) = delete;

    void reset(GrSamplerState, const GrBackendFormat&, const GrSwizzle&);

    const GrBackendFormat& backendFormat() const { return fBackendFormat; }
    GrTextureType textureType() const { return fBackendFormat.textureType(); }

    GrSamplerState samplerState() const { return fSamplerState; }
    const GrSwizzle& swizzle() const { return fSwizzle; }

    bool isInitialized() const { return fIsInitialized; }

private:
    GrSamplerState  fSamplerState;
    GrBackendFormat fBackendFormat;
    GrSwizzle       fSwizzle;
    bool            fIsInitialized = false;
};

const GrGeometryProcessor::TextureSampler& GrGeometryProcessor::IthTextureSampler(int i) {
    SK_ABORT("Illegal texture sampler index");
    static const TextureSampler kBogus;
    return kBogus;
}

//////////////////////////////////////////////////////////////////////////////

/**
 * Returns the size of the attrib type in bytes.
 * This was moved from include/private/GrTypesPriv.h in service of Skia dependents that build
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
        case kUint_GrVertexAttribType:
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
