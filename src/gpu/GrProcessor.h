/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProcessor_DEFINED
#define GrProcessor_DEFINED

#include "../private/SkAtomics.h"
#include "GrBuffer.h"
#include "GrColor.h"
#include "GrGpuResourceRef.h"
#include "GrProcessorUnitTest.h"
#include "GrProgramElement.h"
#include "GrSamplerState.h"
#include "GrShaderVar.h"
#include "GrSurfaceProxyPriv.h"
#include "GrSurfaceProxyRef.h"
#include "GrTextureProxy.h"
#include "SkMath.h"
#include "SkString.h"

class GrContext;
class GrCoordTransform;
class GrInvariantOutput;
class GrResourceProvider;

/**
 * Used by processors to build their keys. It incorporates each per-processor key into a larger
 * shader key.
 */
class GrProcessorKeyBuilder {
public:
    GrProcessorKeyBuilder(SkTArray<unsigned char, true>* data) : fData(data), fCount(0) {
        SkASSERT(0 == fData->count() % sizeof(uint32_t));
    }

    void add32(uint32_t v) {
        ++fCount;
        fData->push_back_n(4, reinterpret_cast<uint8_t*>(&v));
    }

    /** Inserts count uint32_ts into the key. The returned pointer is only valid until the next
        add*() call. */
    uint32_t* SK_WARN_UNUSED_RESULT add32n(int count) {
        SkASSERT(count > 0);
        fCount += count;
        return reinterpret_cast<uint32_t*>(fData->push_back_n(4 * count));
    }

    size_t size() const { return sizeof(uint32_t) * fCount; }

private:
    SkTArray<uint8_t, true>* fData; // unowned ptr to the larger key.
    int fCount;                     // number of uint32_ts added to fData by the processor.
};

/** Provides custom shader code to the Ganesh shading pipeline. GrProcessor objects *must* be
    immutable: after being constructed, their fields may not change.

    Dynamically allocated GrProcessors are managed by a per-thread memory pool. The ref count of an
    processor must reach 0 before the thread terminates and the pool is destroyed.
 */
class GrProcessor {
public:
    enum ClassID {
        kBigKeyProcessor_ClassID,
        kBlockInputFragmentProcessor_ClassID,
        kCircleGeometryProcessor_ClassID,
        kCircularRRectEffect_ClassID,
        kColorMatrixEffect_ClassID,
        kColorTableEffect_ClassID,
        kComposeOneFragmentProcessor_ClassID,
        kComposeTwoFragmentProcessor_ClassID,
        kCoverageSetOpXP_ClassID,
        kCustomXP_ClassID,
        kDashingCircleEffect_ClassID,
        kDashingLineEffect_ClassID,
        kDefaultGeoProc_ClassID,
        kDIEllipseGeometryProcessor_ClassID,
        kDisableColorXP_ClassID,
        kTwoPointConicalEffect_ClassID,
        kEllipseGeometryProcessor_ClassID,
        kEllipticalRRectEffect_ClassID,
        kGP_ClassID,
        kGrAARectEffect_ClassID,
        kGrAlphaThresholdFragmentProcessor_ClassID,
        kGrArithmeticFP_ClassID,
        kGrBicubicEffect_ClassID,
        kGrBitmapTextGeoProc_ClassID,
        kGrBlurredEdgeFragmentProcessor_ClassID,
        kGrCCClipProcessor_ClassID,
        kGrCCCoverageProcessor_ClassID,
        kGrCCPathProcessor_ClassID,
        kGrCircleBlurFragmentProcessor_ClassID,
        kGrCircleEffect_ClassID,
        kGrColorSpaceXformEffect_ClassID,
        kGrConfigConversionEffect_ClassID,
        kGrConicEffect_ClassID,
        kGrConstColorProcessor_ClassID,
        kGrConvexPolyEffect_ClassID,
        kGrCubicEffect_ClassID,
        kGrDeviceSpaceTextureDecalFragmentProcessor_ClassID,
        kGrDiffuseLightingEffect_ClassID,
        kGrDisplacementMapEffect_ClassID,
        kGrDistanceFieldA8TextGeoProc_ClassID,
        kGrDistanceFieldLCDTextGeoProc_ClassID,
        kGrDistanceFieldPathGeoProc_ClassID,
        kGrDitherEffect_ClassID,
        kGrEllipseEffect_ClassID,
        kGrGaussianConvolutionFragmentProcessor_ClassID,
        kGrImprovedPerlinNoiseEffect_ClassID,
        kGrLightingEffect_ClassID,
        kGrLinearGradient_ClassID,
        kGrLumaColorFilterEffect_ClassID,
        kGrMagnifierEffect_ClassID,
        kGrMatrixConvolutionEffect_ClassID,
        kGrMeshTestProcessor_ClassID,
        kGrMorphologyEffect_ClassID,
        kGrNonlinearColorSpaceXformEffect_ClassID,
        kGrOverdrawFragmentProcessor_ClassID,
        kGrPathProcessor_ClassID,
        kGrPerlinNoise2Effect_ClassID,
        kGrPipelineDynamicStateTestProcessor_ClassID,
        kGrPremulInputFragmentProcessor_ClassID,
        kGrQuadEffect_ClassID,
        kGrRadialGradient_ClassID,
        kGrRectBlurEffect_ClassID,
        kGrRRectBlurEffect_ClassID,
        kGrRRectShadowGeoProc_ClassID,
        kGrSimpleTextureEffect_ClassID,
        kGrSpecularLightingEffect_ClassID,
        kGrSRGBEffect_ClassID,
        kGrSweepGradient_ClassID,
        kGrTextureDomainEffect_ClassID,
        kGrUnpremulInputFragmentProcessor_ClassID,
        kGrYUVtoRGBEffect_ClassID,
        kHighContrastFilterEffect_ClassID,
        kInstanceProcessor_ClassID,
        kLumaColorFilterEffect_ClassID,
        kMSAAQuadProcessor_ClassID,
        kPDLCDXferProcessor_ClassID,
        kPorterDuffXferProcessor_ClassID,
        kPremulFragmentProcessor_ClassID,
        kQuadEdgeEffect_ClassID,
        kReplaceInputFragmentProcessor_ClassID,
        kRRectsGaussianEdgeFP_ClassID,
        kSeriesFragmentProcessor_ClassID,
        kShaderPDXferProcessor_ClassID,
        kSwizzleFragmentProcessor_ClassID,
        kTestFP_ClassID,
        kTextureGeometryProcessor_ClassID,
    };

    virtual ~GrProcessor() = default;

    /** Human-meaningful string to identify this prcoessor; may be embedded in generated shader
        code. */
    virtual const char* name() const = 0;

    /** Human-readable dump of all information */
    virtual SkString dumpInfo() const {
        SkString str;
        str.appendf("Missing data");
        return str;
    }

    /**
     * Platform specific built-in features that a processor can request for the fragment shader.
     */
    enum RequiredFeatures {
        kNone_RequiredFeatures             = 0,
        kSampleLocations_RequiredFeature   = 1 << 0
    };

    GR_DECL_BITFIELD_OPS_FRIENDS(RequiredFeatures);

    RequiredFeatures requiredFeatures() const { return fRequiredFeatures; }

    void* operator new(size_t size);
    void operator delete(void* target);

    void* operator new(size_t size, void* placement) {
        return ::operator new(size, placement);
    }
    void operator delete(void* target, void* placement) {
        ::operator delete(target, placement);
    }

    /** Helper for down-casting to a GrProcessor subclass */
    template <typename T> const T& cast() const { return *static_cast<const T*>(this); }

    ClassID classID() const { return fClassID; }

protected:
    GrProcessor(ClassID classID)
    : fClassID(classID)
    , fRequiredFeatures(kNone_RequiredFeatures) {}

    /**
     * If the prcoessor will generate code that uses platform specific built-in features, then it
     * must call these methods from its constructor. Otherwise, requests to use these features will
     * be denied.
     */
    void setWillUseSampleLocations() { fRequiredFeatures |= kSampleLocations_RequiredFeature; }

    void combineRequiredFeatures(const GrProcessor& other) {
        fRequiredFeatures |= other.fRequiredFeatures;
    }

private:
    GrProcessor(const GrProcessor&) = delete;
    GrProcessor& operator=(const GrProcessor&) = delete;

    ClassID          fClassID;
    RequiredFeatures fRequiredFeatures;
};

GR_MAKE_BITFIELD_OPS(GrProcessor::RequiredFeatures);

/** A GrProcessor with the ability to access textures, buffers, and image storages. */
class GrResourceIOProcessor : public GrProcessor {
public:
    class TextureSampler;
    class BufferAccess;

    int numTextureSamplers() const { return fTextureSamplers.count(); }

    /** Returns the access pattern for the texture at index. index must be valid according to
        numTextureSamplers(). */
    const TextureSampler& textureSampler(int index) const { return *fTextureSamplers[index]; }

    int numBuffers() const { return fBufferAccesses.count(); }

    /** Returns the access pattern for the buffer at index. index must be valid according to
        numBuffers(). */
    const BufferAccess& bufferAccess(int index) const { return *fBufferAccesses[index]; }

    bool instantiate(GrResourceProvider* resourceProvider) const;

protected:
    GrResourceIOProcessor(ClassID classID)
    : INHERITED(classID) {}

    /**
     * Subclasses call these from their constructor to register sampler sources. The processor
     * subclass manages the lifetime of the objects (these functions only store pointers). The
     * TextureSampler and/or BufferAccess instances are typically member fields of the GrProcessor
     * subclass. These must only be called from the constructor because GrProcessors are immutable.
     */
    void addTextureSampler(const TextureSampler*);
    void addBufferAccess(const BufferAccess*);

    bool hasSameSamplersAndAccesses(const GrResourceIOProcessor&) const;

    // These methods can be used by derived classes that also derive from GrProgramElement.
    void addPendingIOs() const;
    void removeRefs() const;
    void pendingIOComplete() const;

private:
    SkSTArray<4, const TextureSampler*, true> fTextureSamplers;
    SkSTArray<1, const BufferAccess*, true> fBufferAccesses;

    typedef GrProcessor INHERITED;
};

/**
 * Used to represent a texture that is required by a GrResourceIOProcessor. It holds a GrTexture
 * along with an associated GrSamplerState. TextureSamplers don't perform any coord manipulation to
 * account for texture origin.
 */
class GrResourceIOProcessor::TextureSampler {
public:
    /**
     * Must be initialized before adding to a GrProcessor's texture access list.
     */
    TextureSampler();
    /**
     * This copy constructor is used by GrFragmentProcessor::clone() implementations. The copy
     * always takes a new ref on the texture proxy as the new fragment processor will not yet be
     * in pending execution state.
     */
    explicit TextureSampler(const TextureSampler& that)
            : fProxyRef(sk_ref_sp(that.fProxyRef.get()), that.fProxyRef.ioType())
            , fSamplerState(that.fSamplerState)
            , fVisibility(that.fVisibility) {}

    TextureSampler(sk_sp<GrTextureProxy>, const GrSamplerState&);

    explicit TextureSampler(sk_sp<GrTextureProxy>,
                            GrSamplerState::Filter = GrSamplerState::Filter::kNearest,
                            GrSamplerState::WrapMode wrapXAndY = GrSamplerState::WrapMode::kClamp,
                            GrShaderFlags visibility = kFragment_GrShaderFlag);

    TextureSampler& operator=(const TextureSampler&) = delete;

    void reset(sk_sp<GrTextureProxy>, const GrSamplerState&,
               GrShaderFlags visibility = kFragment_GrShaderFlag);
    void reset(sk_sp<GrTextureProxy>,
               GrSamplerState::Filter = GrSamplerState::Filter::kNearest,
               GrSamplerState::WrapMode wrapXAndY = GrSamplerState::WrapMode::kClamp,
               GrShaderFlags visibility = kFragment_GrShaderFlag);

    bool operator==(const TextureSampler& that) const {
        return this->proxy()->underlyingUniqueID() == that.proxy()->underlyingUniqueID() &&
               fSamplerState == that.fSamplerState && fVisibility == that.fVisibility;
    }

    bool operator!=(const TextureSampler& other) const { return !(*this == other); }

    // 'instantiate' should only ever be called at flush time.
    bool instantiate(GrResourceProvider* resourceProvider) const {
        return SkToBool(fProxyRef.get()->instantiate(resourceProvider));
    }

    // 'peekTexture' should only ever be called after a successful 'instantiate' call
    GrTexture* peekTexture() const {
        SkASSERT(fProxyRef.get()->priv().peekTexture());
        return fProxyRef.get()->priv().peekTexture();
    }

    GrTextureProxy* proxy() const { return fProxyRef.get()->asTextureProxy(); }
    GrShaderFlags visibility() const { return fVisibility; }
    const GrSamplerState& samplerState() const { return fSamplerState; }

    bool isInitialized() const { return SkToBool(fProxyRef.get()); }
    /**
     * For internal use by GrProcessor.
     */
    const GrSurfaceProxyRef* programProxy() const { return &fProxyRef; }

private:
    GrSurfaceProxyRef fProxyRef;
    GrSamplerState fSamplerState;
    GrShaderFlags fVisibility;
};

/**
 * Used to represent a texel buffer that will be read in a GrResourceIOProcessor. It holds a
 * GrBuffer along with an associated offset and texel config.
 */
class GrResourceIOProcessor::BufferAccess {
public:
    BufferAccess() = default;
    BufferAccess(GrPixelConfig texelConfig, GrBuffer* buffer,
                 GrShaderFlags visibility = kFragment_GrShaderFlag) {
        this->reset(texelConfig, buffer, visibility);
    }
    /**
     * This copy constructor is used by GrFragmentProcessor::clone() implementations. The copy
     * always takes a new ref on the buffer proxy as the new fragment processor will not yet be
     * in pending execution state.
     */
    explicit BufferAccess(const BufferAccess& that) {
        this->reset(that.fTexelConfig, that.fBuffer.get(), that.fVisibility);
    }

    BufferAccess& operator=(const BufferAccess&) = delete;

    /**
     * Must be initialized before adding to a GrProcessor's buffer access list.
     */
    void reset(GrPixelConfig texelConfig, GrBuffer* buffer,
               GrShaderFlags visibility = kFragment_GrShaderFlag) {
        fTexelConfig = texelConfig;
        fBuffer.set(SkRef(buffer), kRead_GrIOType);
        fVisibility = visibility;
    }

    bool operator==(const BufferAccess& that) const {
        return fTexelConfig == that.fTexelConfig &&
               this->buffer() == that.buffer() &&
               fVisibility == that.fVisibility;
    }

    bool operator!=(const BufferAccess& that) const { return !(*this == that); }

    GrPixelConfig texelConfig() const { return fTexelConfig; }
    GrBuffer* buffer() const { return fBuffer.get(); }
    GrShaderFlags visibility() const { return fVisibility; }

    /**
     * For internal use by GrProcessor.
     */
    const GrGpuResourceRef* programBuffer() const { return &fBuffer;}

private:
    GrPixelConfig fTexelConfig;
    GrTGpuResourceRef<GrBuffer> fBuffer;
    GrShaderFlags fVisibility;

    typedef SkNoncopyable INHERITED;
};

#endif
