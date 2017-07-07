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

/**
 * This is the geometry processor for the simple convex primitive shapes (triangles and closed curve
 * segments) from which ccpr paths are composed. The output is a single-channel alpha value,
 * positive for clockwise primitives and negative for counter-clockwise, that indicates coverage.
 *
 * The caller is responsible to render all modes for all applicable primitives into a cleared,
 * floating point, alpha-only render target using SkBlendMode::kPlus. Once all of a path's
 * primitives have been drawn, the render target contains a composite coverage count that can then
 * be used to draw the path (see GrCCPRPathProcessor).
 *
 * Caller provides the primitives' (x,y) points in an fp32x2 (RG) texel buffer, and an instance
 * buffer with a single int32x4 attrib for each primitive (defined below). There are no vertex
 * attribs.
 *
 * Draw calls are instanced, with one vertex per bezier point (3 for triangles). They use the
 * corresponding GrPrimitiveType as defined below.
 */
class GrCCPRCoverageProcessor : public GrGeometryProcessor {
public:
    // Use top-left to avoid a uniform access in the fragment shader.
    static constexpr GrSurfaceOrigin kAtlasOrigin = kTopLeft_GrSurfaceOrigin;

    static constexpr GrPrimitiveType kTrianglesGrPrimitiveType = GrPrimitiveType::kTriangles;
    static constexpr GrPrimitiveType kQuadraticsGrPrimitiveType = GrPrimitiveType::kTriangles;
    static constexpr GrPrimitiveType kCubicsGrPrimitiveType = GrPrimitiveType::kLinesAdjacency;

    struct PrimitiveInstance {
        union {
            struct {
                int32_t fPt0Idx;
                int32_t fPt1Idx;
                int32_t fPt2Idx;
            } fTriangleData;

            struct {
                int32_t fControlPtIdx;
                int32_t fEndPtsIdx; // The endpoints (P0 and P2) are adjacent in the texel buffer.
            } fQuadraticData;

            struct {
                int32_t fControlPtsKLMRootsIdx; // The control points (P1 and P2) are adjacent in
                                                // the texel buffer, followed immediately by the
                                                // homogenous KLM roots ({tl,sl}, {tm,sm}).
                int32_t fEndPtsIdx; // The endpoints (P0 and P3) are adjacent in the texel buffer.
            } fCubicData;
        };

        int32_t fPackedAtlasOffset; // (offsetY << 16) | (offsetX & 0xffff)
    };

    GR_STATIC_ASSERT(4 * 4 == sizeof(PrimitiveInstance));

    enum class Mode {
        // Triangles.
        kTriangleHulls,
        kTriangleEdges,
        kCombinedTriangleHullsAndEdges,
        kTriangleCorners,

        // Quadratics.
        kQuadraticHulls,
        kQuadraticFlatEdges,

        // Cubics.
        kSerpentineInsets,
        kSerpentineBorders,
        kLoopInsets,
        kLoopBorders
    };
    static const char* GetProcessorName(Mode);

    GrCCPRCoverageProcessor(Mode, GrBuffer* pointsBuffer);

    const char* instanceAttrib() const { return fInstanceAttrib.fName; }
    const char* name() const override { return GetProcessorName(fMode); }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override;

#ifdef SK_DEBUG
    static constexpr float kDebugBloat = 50;

    // Increases the 1/2 pixel AA bloat by a factor of kDebugBloat and outputs color instead of
    // coverage (coverage=+1 -> green, coverage=0 -> black, coverage=-1 -> red).
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

/**
 * This class represents the actual SKSL implementation for the various primitives and modes of
 * GrCCPRCoverageProcessor.
 */
class GrCCPRCoverageProcessor::PrimitiveProcessor : public GrGLSLGeometryProcessor {
public:
    // We achieve conservative raster by defining pixel-size boxes around vertices, and then drawing
    // convex hulls around those boxes. We slightly undershoot a radius of 0.5 so vertices that fall
    // on integer boundaries don't accidentally bleed into neighbor pixels.
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
