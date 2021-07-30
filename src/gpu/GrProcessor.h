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
#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/GrSamplerState.h"
#include "src/gpu/GrShaderVar.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTextureProxy.h"

class GrResourceProvider;

/** Provides custom shader code to the Ganesh shading pipeline. GrProcessor objects *must* be
    immutable: after being constructed, their fields may not change.

    Dynamically allocated GrProcessors are managed by a per-thread memory pool. The ref count of an
    processor must reach 0 before the thread terminates and the pool is destroyed.
 */
class GrProcessor {
public:
    enum ClassID {
        kNull_ClassID,  // Reserved ID for missing (null) processors

        kBigKeyProcessor_ClassID,
        kBlendFragmentProcessor_ClassID,
        kBlockInputFragmentProcessor_ClassID,
        kButtCapStrokedCircleGeometryProcessor_ClassID,
        kCircleGeometryProcessor_ClassID,
        kCircularRRectEffect_ClassID,
        kClockwiseTestProcessor_ClassID,
        kColorTableEffect_ClassID,
        kCoverageSetOpXP_ClassID,
        kCustomXP_ClassID,
        kDashingCircleEffect_ClassID,
        kDashingLineEffect_ClassID,
        kDefaultGeoProc_ClassID,
        kDeviceSpace_ClassID,
        kDIEllipseGeometryProcessor_ClassID,
        kDisableColorXP_ClassID,
        kDrawAtlasPathShader_ClassID,
        kEllipseGeometryProcessor_ClassID,
        kEllipticalRRectEffect_ClassID,
        kFwidthSquircleTestProcessor_ClassID,
        kGP_ClassID,
        kGrBicubicEffect_ClassID,
        kGrBitmapTextGeoProc_ClassID,
        kGrColorSpaceXformEffect_ClassID,
        kGrConicEffect_ClassID,
        kGrConvexPolyEffect_ClassID,
        kGrDiffuseLightingEffect_ClassID,
        kGrDisplacementMapEffect_ClassID,
        kGrDistanceFieldA8TextGeoProc_ClassID,
        kGrDistanceFieldLCDTextGeoProc_ClassID,
        kGrDistanceFieldPathGeoProc_ClassID,
        kGrDSLFPTest_DoStatement_ClassID,
        kGrDSLFPTest_ForStatement_ClassID,
        kGrDSLFPTest_IfStatement_ClassID,
        kGrDSLFPTest_SwitchStatement_ClassID,
        kGrDSLFPTest_Swizzle_ClassID,
        kGrDSLFPTest_Ternary_ClassID,
        kGrDSLFPTest_WhileStatement_ClassID,
        kGrFillRRectOp_Processor_ClassID,
        kGrGaussianConvolutionFragmentProcessor_ClassID,
        kGrMatrixConvolutionEffect_ClassID,
        kGrMatrixEffect_ClassID,
        kGrMeshTestProcessor_ClassID,
        kGrMorphologyEffect_ClassID,
        kGrPerlinNoise2Effect_ClassID,
        kGrPipelineDynamicStateTestProcessor_ClassID,
        kGrQuadEffect_ClassID,
        kGrRRectShadowGeoProc_ClassID,
        kGrSkSLFP_ClassID,
        kGrSpecularLightingEffect_ClassID,
        kGrTextureEffect_ClassID,
        kGrUnrolledBinaryGradientColorizer_ClassID,
        kGrYUVtoRGBEffect_ClassID,
        kHighPrecisionFragmentProcessor_ClassID,
        kLatticeGP_ClassID,
        kPDLCDXferProcessor_ClassID,
        kPorterDuffXferProcessor_ClassID,
        kPremulFragmentProcessor_ClassID,
        kQuadEdgeEffect_ClassID,
        kQuadPerEdgeAAGeometryProcessor_ClassID,
        kSeriesFragmentProcessor_ClassID,
        kShaderPDXferProcessor_ClassID,
        kSurfaceColorProcessor_ClassID,
        kSwizzleFragmentProcessor_ClassID,
        kTessellate_BoundingBoxShader_ClassID,
        kTessellate_GrModulateAtlasCoverageEffect_ClassID,
        kTessellate_GrStrokeTessellationShader_ClassID,
        kTessellate_HardwareCurveShader_ClassID,
        kTessellate_HardwareWedgeShader_ClassID,
        kTessellate_HullShader_ClassID,
        kTessellate_MiddleOutShader_ClassID,
        kTessellate_SimpleTriangleShader_ClassID,
        kTessellationTestTriShader_ClassID,
        kTestFP_ClassID,
        kTestRectOp_ClassID,
        kVertexColorSpaceBenchGP_ClassID,
        kVerticesGP_ClassID,
    };

    virtual ~GrProcessor() = default;

    /** Human-meaningful string to identify this processor; may be embedded in generated shader
        code and must be a legal SkSL identifier prefix. */
    virtual const char* name() const = 0;

    /** Human-readable dump of all information */
#if GR_TEST_UTILS
    virtual SkString onDumpInfo() const { return SkString(); }

    SkString dumpInfo() const {
        SkString info(name());
        info.append(this->onDumpInfo());
        return info;
    }
#endif

    /**
     * Custom shader features provided by the framework. These require special handling when
     * preparing shaders, so a processor must call setWillUseCustomFeature() from its constructor if
     * it intends to use one.
     */
    enum class CustomFeatures {
        kNone = 0,
    };

    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(CustomFeatures);

    CustomFeatures requestedFeatures() const { return fRequestedFeatures; }

    void* operator new(size_t size);
    void* operator new(size_t object_size, size_t footer_size);
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
