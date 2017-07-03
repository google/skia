/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPRCoverageProcessor_DEFINED
#define GrCCPRCoverageProcessor_DEFINED

#include "GrGeometryProcessor.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLVarying.h"

class GrGLSLFragmentBuilder;

class GrCCPRCoverageProcessor : public GrGeometryProcessor {
public:
    // Use top-left to avoid a uniform access in the fragment shader.
    static constexpr GrSurfaceOrigin kAtlasOrigin = kTopLeft_GrSurfaceOrigin;

    enum class Mode {
        kAnchorHulls,
        kAnchorEdges,
        kCombinedFanHullsAndEdges,
        kAnchorCorners,
        kQuadraticHulls,
        kQuadraticInnerEdges,
        kSerpInset,
        kSerpBorder,
        kLoopInset,
        kLoopBorder
    };
    static const char* GetProcessorName(Mode);

    GrCCPRCoverageProcessor(Mode, GrBuffer* pointsBuffer);

    const char* instanceAttrib() const { return fInstanceAttrib.fName; }
    const char* name() const override { return GetProcessorName(fMode); }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override;

#ifdef SK_DEBUG
    static constexpr float kDebugBloat = 50;
    void enableDebugVisualizations() { fDebugVisualizations = true; }
    bool debugVisualizations() const { return fDebugVisualizations; }

    static void Validate(GrRenderTarget* atlasTexture);
#endif

    class PrimitiveProcessor;

private:
    const Mode         fMode;
    const Attribute&   fInstanceAttrib;
    BufferAccess       fPointsBufferAccess;
    SkDEBUGCODE(bool   fDebugVisualizations = false;)

    typedef GrGeometryProcessor INHERITED;
};

class GrCCPRCoverageProcessor::PrimitiveProcessor : public GrGLSLGeometryProcessor {
public:
    // We achieve conservative raster by defining pixel-size boxes around vertices, and then drawing
    // convex hulls around those boxes. We slightly undershoot a radius of 0.5 so vertices that fall
    // on integer boundaries don't unexpectedly bleed into neighbor pixels.
    static constexpr float kAABloatRadius = 0.491111f;

    enum class CoverageType {
        kOne,
        kInterpolated,
        kShader
    };

    PrimitiveProcessor(CoverageType coverageType)
        : fCoverageType(coverageType)
        , fGeomWind("wind", kFloat_GrSLType, GrShaderVar::kNonArray, kLow_GrSLPrecision)
        , fFragWind(kFloat_GrSLType)
        , fFragCoverageTimesWind(kFloat_GrSLType) {}

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor&,
                 FPCoordTransformIter&& transformIter) final {
        this->setTransformDataHelper(SkMatrix::I(), pdman, &transformIter);
    }

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) final;

protected:
    virtual void resetVaryings(GrGLSLVaryingHandler*) {}
    virtual void onEmitVertexShader(const GrCCPRCoverageProcessor&, GrGLSLVertexBuilder*,
                                    const char* rtAdjustName, const TexelBufferHandle& pointsBuffer,
                                    GrGPArgs*) const = 0;
    virtual void emitWind(GrGLSLGeometryBuilder*, const char* windName,
                          const char* rtAdjustName) const;
    virtual void onEmitGeometryShader(GrGLSLGeometryBuilder*, const char* emitVertexFn,
                                      const char* wind, const char* rtAdjustName) const = 0;
    virtual void emitPerVertexGeometryCode(SkString* fnBody, const char* position,
                                           const char* coverage, const char* wind) const {}
    virtual void emitShaderCoverage(GrGLSLFragmentBuilder*, const char* outputCoverage) const {
        SkFAIL("Shader coverage not implemented when using CoverageType::kShader.");
    }

    int emitHullGeometry(GrGLSLGeometryBuilder*, int numPts, const char* wedgeIdx,
                         const char* emitVertexFn, const char* hullPts,
                         const char* insetPts = nullptr) const;
    void emitEdgeDistanceEquation(GrGLSLGeometryBuilder*, const char* leftPt, const char* rightPt,
                                  const char* distanceEquationName) const;
    int emitEdgeGeometry(GrGLSLGeometryBuilder*, const char* emitVertexFn,
                         const char* leftPt, const char* rightPt,
                         const char* distanceEquation = nullptr) const;
    int definePseudoSampleLocations(GrGLSLFragmentBuilder*, const char* samplesName) const;

private:
    void emitVertexShader(const GrCCPRCoverageProcessor&, GrGLSLVertexBuilder*,
                          const char* rtAdjustName, const TexelBufferHandle& pointsBuffer,
                          GrGPArgs* gpArgs) const;
    void emitGeometryShader(const GrCCPRCoverageProcessor&, GrGLSLGeometryBuilder*,
                            const char* rtAdjustName) const;
    void emitCoverage(const GrCCPRCoverageProcessor&, GrGLSLFragmentBuilder*,
                      const char* outputColor, const char* outputCoverage) const;

    const CoverageType   fCoverageType;
    GrShaderVar          fGeomWind;
    GrGLSLGeoToFrag      fFragWind;
    GrGLSLGeoToFrag      fFragCoverageTimesWind;

    typedef GrGLSLGeometryProcessor INHERITED;
};

#endif
