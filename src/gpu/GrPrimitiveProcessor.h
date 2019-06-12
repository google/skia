/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPrimitiveProcessor_DEFINED
#define GrPrimitiveProcessor_DEFINED

#include "include/private/GrColor.h"
#include "include/private/GrProxyRef.h"
#include "src/gpu/GrNonAtomicRef.h"
#include "src/gpu/GrProcessor.h"
#include "src/gpu/GrShaderVar.h"

class GrCoordTransform;

/*
 * The GrPrimitiveProcessor represents some kind of geometric primitive.  This includes the shape
 * of the primitive and the inherent color of the primitive.  The GrPrimitiveProcessor is
 * responsible for providing a color and coverage input into the Ganesh rendering pipeline.  Through
 * optimization, Ganesh may decide a different color, no color, and / or no coverage are required
 * from the GrPrimitiveProcessor, so the GrPrimitiveProcessor must be able to support this
 * functionality.
 *
 * There are two feedback loops between the GrFragmentProcessors, the GrXferProcessor, and the
 * GrPrimitiveProcessor. These loops run on the CPU and to determine known properties of the final
 * color and coverage inputs to the GrXferProcessor in order to perform optimizations that preserve
 * correctness. The GrDrawOp seeds these loops with initial color and coverage, in its
 * getProcessorAnalysisInputs implementation. These seed values are processed by the
 * subsequent
 * stages of the rendering pipeline and the output is then fed back into the GrDrawOp in
 * the applyPipelineOptimizations call, where the op can use the information to inform decisions
 * about GrPrimitiveProcessor creation.
 */

class GrGLSLPrimitiveProcessor;

/**
 * GrPrimitiveProcessor defines an interface which all subclasses must implement.  All
 * GrPrimitiveProcessors must proivide seed color and coverage for the Ganesh color / coverage
 * pipelines, and they must provide some notion of equality
 *
 * TODO: This class does not really need to be ref counted. Instances should be allocated using
 * GrOpFlushState's arena and destroyed when the arena is torn down.
 */
class GrPrimitiveProcessor : public GrProcessor, public GrNonAtomicRef<GrPrimitiveProcessor> {
public:
    class TextureSampler;

    /** Describes a vertex or instance attribute. */
    class Attribute {
    public:
        constexpr Attribute() = default;
        constexpr Attribute(const char* name,
                            GrVertexAttribType cpuType,
                            GrSLType gpuType)
            : fName(name), fCPUType(cpuType), fGPUType(gpuType) {}
        constexpr Attribute(const Attribute&) = default;

        Attribute& operator=(const Attribute&) = default;

        constexpr bool isInitialized() const { return SkToBool(fName); }

        constexpr const char* name() const { return fName; }
        constexpr GrVertexAttribType cpuType() const { return fCPUType; }
        constexpr GrSLType           gpuType() const { return fGPUType; }

        inline constexpr size_t size() const;
        constexpr size_t sizeAlign4() const { return SkAlign4(this->size()); }

        GrShaderVar asShaderVar() const {
            return {fName, fGPUType, GrShaderVar::kIn_TypeModifier};
        }

    private:
        const char* fName = nullptr;
        GrVertexAttribType fCPUType = kFloat_GrVertexAttribType;
        GrSLType fGPUType = kFloat_GrSLType;
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

    private:
        friend class GrPrimitiveProcessor;

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

    GrPrimitiveProcessor(ClassID);

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

    // Only the GrGeometryProcessor subclass actually has a geo shader or vertex attributes, but
    // we put these calls on the base class to prevent having to cast
    virtual bool willUseGeoShader() const = 0;

    /**
     * Computes a transformKey from an array of coord transforms. Will only look at the first
     * <numCoords> transforms in the array.
     *
     * TODO: A better name for this function  would be "compute" instead of "get".
     */
    uint32_t getTransformKey(const SkTArray<const GrCoordTransform*, true>& coords,
                             int numCoords) const;

    /**
     * Sets a unique key on the GrProcessorKeyBuilder that is directly associated with this geometry
     * processor's GL backend implementation.
     *
     * TODO: A better name for this function  would be "compute" instead of "get".
     */
    virtual void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const = 0;


    void getAttributeKey(GrProcessorKeyBuilder* b) const {
        // Ensure that our CPU and GPU type fields fit together in a 32-bit value, and we never
        // collide with the "uninitialized" value.
        static_assert(kGrVertexAttribTypeCount < (1 << 8), "");
        static_assert(kGrSLTypeCount           < (1 << 8), "");

        auto add_attributes = [=](const Attribute* attrs, int attrCount) {
            for (int i = 0; i < attrCount; ++i) {
                b->add32(attrs[i].isInitialized() ? (attrs[i].cpuType() << 16) | attrs[i].gpuType()
                                                  : ~0);
            }
        };
        add_attributes(fVertexAttributes.fAttributes, fVertexAttributes.fRawCount);
        add_attributes(fInstanceAttributes.fAttributes, fInstanceAttributes.fRawCount);
    }

    /** Returns a new instance of the appropriate *GL* implementation class
        for the given GrProcessor; caller is responsible for deleting
        the object. */
    virtual GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const = 0;

    virtual bool isPathRendering() const { return false; }

protected:
    void setVertexAttributes(const Attribute* attrs, int attrCount) {
        fVertexAttributes.init(attrs, attrCount);
    }
    void setInstanceAttributes(const Attribute* attrs, int attrCount) {
        SkASSERT(attrCount >= 0);
        fInstanceAttributes.init(attrs, attrCount);
    }
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

    AttributeSet fVertexAttributes;
    AttributeSet fInstanceAttributes;

    int fTextureSamplerCnt = 0;
    typedef GrProcessor INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

/**
 * Used to represent a texture that is required by a GrPrimitiveProcessor. It holds a GrTextureProxy
 * along with an associated GrSamplerState. TextureSamplers don't perform any coord manipulation to
 * account for texture origin.
 */
class GrPrimitiveProcessor::TextureSampler {
public:
    TextureSampler() = default;

    TextureSampler(GrTextureType, GrPixelConfig, const GrSamplerState&, uint32_t extraSamplerKey);

    explicit TextureSampler(GrTextureType, GrPixelConfig,
                            GrSamplerState::Filter = GrSamplerState::Filter::kNearest,
                            GrSamplerState::WrapMode wrapXAndY = GrSamplerState::WrapMode::kClamp);

    TextureSampler(const TextureSampler&) = delete;
    TextureSampler& operator=(const TextureSampler&) = delete;

    void reset(GrTextureType, GrPixelConfig, const GrSamplerState&, uint32_t extraSamplerKey = 0);
    void reset(GrTextureType, GrPixelConfig,
               GrSamplerState::Filter,
               GrSamplerState::WrapMode wrapXAndY);

    GrTextureType textureType() const { return fTextureType; }
    GrPixelConfig config() const { return fConfig; }

    const GrSamplerState& samplerState() const { return fSamplerState; }

    uint32_t extraSamplerKey() const { return fExtraSamplerKey; }

    bool isInitialized() const { return fConfig != kUnknown_GrPixelConfig; }

private:
    GrSamplerState fSamplerState;
    GrTextureType fTextureType = GrTextureType::k2D;
    GrPixelConfig fConfig = kUnknown_GrPixelConfig;
    uint32_t fExtraSamplerKey = 0;
};

const GrPrimitiveProcessor::TextureSampler& GrPrimitiveProcessor::IthTextureSampler(int i) {
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
        case kHalf3_GrVertexAttribType:
            return 3 * sizeof(uint16_t);
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
        case kByte3_GrVertexAttribType:
            return 3 * sizeof(char);
        case kByte4_GrVertexAttribType:
            return 4 * sizeof(char);
        case kUByte_GrVertexAttribType:
            return 1 * sizeof(char);
        case kUByte2_GrVertexAttribType:
            return 2 * sizeof(char);
        case kUByte3_GrVertexAttribType:
            return 3 * sizeof(char);
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
        // Experimental (for Y416)
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

constexpr size_t GrPrimitiveProcessor::Attribute::size() const {
    return GrVertexAttribTypeSize(fCPUType);
}

#endif
