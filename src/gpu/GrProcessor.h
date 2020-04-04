/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProcessor_DEFINED
#define GrProcessor_DEFINED

#include "include/core/SkMath.h"
#include "include/core/SkString.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrGpuBuffer.h"
#include "src/gpu/GrProcessorUnitTest.h"
#include "src/gpu/GrSamplerState.h"
#include "src/gpu/GrShaderVar.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTextureProxy.h"

class GrContext;
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
        kButtCapStrokedCircleGeometryProcessor_ClassID,
        kCircleGeometryProcessor_ClassID,
        kCircularRRectEffect_ClassID,
        kClockwiseTestProcessor_ClassID,
        kColorTableEffect_ClassID,
        kComposeOneFragmentProcessor_ClassID,
        kComposeTwoFragmentProcessor_ClassID,
        kCoverageSetOpXP_ClassID,
        kCubicStrokeProcessor_ClassID,
        kCustomXP_ClassID,
        kDashingCircleEffect_ClassID,
        kDashingLineEffect_ClassID,
        kDefaultGeoProc_ClassID,
        kDIEllipseGeometryProcessor_ClassID,
        kDisableColorXP_ClassID,
        kEllipseGeometryProcessor_ClassID,
        kEllipticalRRectEffect_ClassID,
        kGP_ClassID,
        kVertexColorSpaceBenchGP_ClassID,
        kGrAARectEffect_ClassID,
        kGrAlphaThresholdFragmentProcessor_ClassID,
        kGrBicubicEffect_ClassID,
        kGrBitmapTextGeoProc_ClassID,
        kGrBlurredEdgeFragmentProcessor_ClassID,
        kGrCCClipProcessor_ClassID,
        kGrCCPathProcessor_ClassID,
        kGrCircleBlurFragmentProcessor_ClassID,
        kGrCircleEffect_ClassID,
        kGrClampedGradientEffect_ClassID,
        kGrColorMatrixFragmentProcessor_ClassID,
        kGrColorSpaceXformEffect_ClassID,
        kGrComposeLerpEffect_ClassID,
        kGrComposeLerpRedEffect_ClassID,
        kGrConfigConversionEffect_ClassID,
        kGrConicEffect_ClassID,
        kGrConstColorProcessor_ClassID,
        kGrConvexPolyEffect_ClassID,
        kGrDeviceSpaceTextureDecalFragmentProcessor_ClassID,
        kGrDiffuseLightingEffect_ClassID,
        kGrDisplacementMapEffect_ClassID,
        kGrDistanceFieldA8TextGeoProc_ClassID,
        kGrDistanceFieldLCDTextGeoProc_ClassID,
        kGrDistanceFieldPathGeoProc_ClassID,
        kGrDomainEffect_ClassID,
        kGrDualIntervalGradientColorizer_ClassID,
        kGrEllipseEffect_ClassID,
        kGrFillRRectOp_Processor_ClassID,
        kGrGaussianConvolutionFragmentProcessor_ClassID,
        kGrGSCoverageProcessor_ClassID,
        kGrHSLToRGBFilterEffect_ClassID,
        kGrImprovedPerlinNoiseEffect_ClassID,
        kGrLinearGradientLayout_ClassID,
        kGrLumaColorFilterEffect_ClassID,
        kGrMagnifierEffect_ClassID,
        kGrMatrixConvolutionEffect_ClassID,
        kGrMeshTestProcessor_ClassID,
        kGrMorphologyEffect_ClassID,
        kGrMixerEffect_ClassID,
        kGrOverrideInputFragmentProcessor_ClassID,
        kGrPathProcessor_ClassID,
        kGrPerlinNoise2Effect_ClassID,
        kGrPipelineDynamicStateTestProcessor_ClassID,
        kGrPremulInputFragmentProcessor_ClassID,
        kGrQuadEffect_ClassID,
        kGrRadialGradientLayout_ClassID,
        kGrRectBlurEffect_ClassID,
        kGrRGBToHSLFilterEffect_ClassID,
        kGrRRectBlurEffect_ClassID,
        kGrRRectShadowGeoProc_ClassID,
        kGrSimpleTextureEffect_ClassID,
        kGrSingleIntervalGradientColorizer_ClassID,
        kGrSkSLFP_ClassID,
        kGrSpecularLightingEffect_ClassID,
        kGrSRGBEffect_ClassID,
        kGrSampleMaskProcessor_ClassID,
        kGrSaturateProcessor_ClassID,
        kGrSweepGradientLayout_ClassID,
        kGrTextureGradientColorizer_ClassID,
        kGrTiledGradientEffect_ClassID,
        kGrTwoPointConicalGradientLayout_ClassID,
        kGrUnrolledBinaryGradientColorizer_ClassID,
        kGrVSCoverageProcessor_ClassID,
        kGrYUVtoRGBEffect_ClassID,
        kHighContrastFilterEffect_ClassID,
        kLatticeGP_ClassID,
        kPDLCDXferProcessor_ClassID,
        kPorterDuffXferProcessor_ClassID,
        kPremulFragmentProcessor_ClassID,
        kQuadEdgeEffect_ClassID,
        kQuadPerEdgeAAGeometryProcessor_ClassID,
        kSampleLocationsTestProcessor_ClassID,
        kSeriesFragmentProcessor_ClassID,
        kShaderPDXferProcessor_ClassID,
        kStencilResolveProcessor_ClassID,
        kFwidthSquircleTestProcessor_ClassID,
        kSwizzleFragmentProcessor_ClassID,
        kTestFP_ClassID,
        kTestRectOp_ClassID,
        kFlatNormalsFP_ClassID,
        kMappedNormalsFP_ClassID,
        kLightingFP_ClassID,
        kLinearStrokeProcessor_ClassID,
    };

    virtual ~GrProcessor() = default;

    /** Human-meaningful string to identify this prcoessor; may be embedded in generated shader
        code. */
    virtual const char* name() const = 0;

    /** Human-readable dump of all information */
#ifdef SK_DEBUG
    virtual SkString dumpInfo() const {
        SkString str;
        str.appendf("Missing data");
        return str;
    }
#else
    SkString dumpInfo() const { return SkString("<Processor information unavailable>"); }
#endif

    /**
     * Custom shader features provided by the framework. These require special handling when
     * preparing shaders, so a processor must call setWillUseCustomFeature() from its constructor if
     * it intends to use one.
     */
    enum class CustomFeatures {
        kNone = 0,
        kSampleLocations = 1 << 0,
    };

    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(CustomFeatures);

    CustomFeatures requestedFeatures() const { return fRequestedFeatures; }

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
    GrProcessor(ClassID classID) : fClassID(classID) {}
    GrProcessor(const GrProcessor&) = delete;
    GrProcessor& operator=(const GrProcessor&) = delete;

    void setWillUseCustomFeature(CustomFeatures feature) { fRequestedFeatures |= feature; }
    void resetCustomFeatures() { fRequestedFeatures = CustomFeatures::kNone; }

    const ClassID fClassID;
    CustomFeatures fRequestedFeatures = CustomFeatures::kNone;
};

GR_MAKE_BITFIELD_CLASS_OPS(GrProcessor::CustomFeatures)

#endif
