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
    SkString dumpInfo() const override {
        return SkStringPrintf("%s\n%s", this->name(), this->INHERITED::dumpInfo().c_str());
    }

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
protected:
    // Slightly undershoot a bloat radius of 0.5 so vertices that fall on integer boundaries don't
    // accidentally bleed into neighbor pixels.
    static constexpr float kAABloatRadius = 0.491111f;

    // Specifies how the fragment shader should calculate sk_FragColor.a.
    enum class CoverageType {
        kOne, // Output +1 all around, modulated by wind.
        kInterpolated, // Interpolate the coverage values that the geometry shader associates with
                       // each point, modulated by wind.
        kShader // Call emitShaderCoverage and let the subclass decide, then a modulate by wind.
    };

    PrimitiveProcessor(CoverageType coverageType)
            : fCoverageType(coverageType)
            , fGeomWind("wind", kFloat_GrSLType, GrShaderVar::kNonArray, kLow_GrSLPrecision)
            , fFragWind(kFloat_GrSLType)
            , fFragCoverageTimesWind(kFloat_GrSLType) {}

    // Called before generating shader code. Subclass should add its custom varyings to the handler
    // and update its corresponding internal member variables.
    virtual void resetVaryings(GrGLSLVaryingHandler*) {}

    // Here the subclass fetches its vertex from the texel buffer, translates by atlasOffset, and
    // sets "fPositionVar" in the GrGPArgs.
    virtual void onEmitVertexShader(const GrCCPRCoverageProcessor&, GrGLSLVertexBuilder*,
                                    const TexelBufferHandle& pointsBuffer, const char* atlasOffset,
                                    const char* rtAdjust, GrGPArgs*) const = 0;

    // Here the subclass determines the winding direction of its primitive. It must write a value of
    // either -1, 0, or +1 to "outputWind" (e.g. "sign(area)"). Fractional values are not valid.
    virtual void emitWind(GrGLSLGeometryBuilder*, const char* rtAdjust,
                          const char* outputWind) const = 0;

    // This is where the subclass generates the actual geometry to be rasterized by hardware:
    //
    //   emitVertexFn(point1, coverage);
    //   emitVertexFn(point2, coverage);
    //   ...
    //   EndPrimitive();
    //
    // Generally a subclass will want to use emitHullGeometry and/or emitEdgeGeometry rather than
    // calling emitVertexFn directly.
    //
    // Subclass must also call GrGLSLGeometryBuilder::configure.
    virtual void onEmitGeometryShader(GrGLSLGeometryBuilder*, const char* emitVertexFn,
                                      const char* wind, const char* rtAdjust) const = 0;

    // This is a hook to inject code in the geometry shader's "emitVertex" function. Subclass
    // should use this to write values to its custom varyings.
    // NOTE: even flat varyings should be rewritten at each vertex.
    virtual void emitPerVertexGeometryCode(SkString* fnBody, const char* position,
                                           const char* coverage, const char* wind) const {}

    // Called when the subclass has selected CoverageType::kShader. Primitives should produce
    // coverage values between +0..1. Base class modulates the sign for wind.
    // TODO: subclasses might have good spots to stuff the winding information without burning a
    // whole new varying slot. Consider requiring them to generate the correct coverage sign.
    virtual void emitShaderCoverage(GrGLSLFragmentBuilder*, const char* outputCoverage) const {
        SkFAIL("Shader coverage not implemented when using CoverageType::kShader.");
    }

    // Emits one wedge of the conservative raster hull of a convex polygon. The complete hull has
    // one wedge for each side of the polygon (i.e. call this N times, generally from different
    // geometry shader invocations). Coverage is +1 all around.
    //
    // Logically, the conservative raster hull is equivalent to the convex hull of pixel-size boxes
    // centered on the vertices.
    //
    // If an optional inset polygon is provided, then this emits a border from the inset to the
    // hull, rather than the entire hull.
    //
    // Geometry shader must be configured to output triangle strips.
    //
    // Returns the maximum number of vertices that will be emitted.
    int emitHullGeometry(GrGLSLGeometryBuilder*, const char* emitVertexFn, const char* polygonPts,
                         int numSides, const char* wedgeIdx, const char* insetPts = nullptr) const;

    // Emits the conservative raster of an edge (i.e. convex hull of two pixel-size boxes centered
    // on the endpoints). Coverage is -1 on the outside border of the edge geometry and 0 on the
    // inside. This effectively converts a jagged conservative raster edge into a smooth antialiased
    // edge when using CoverageType::kInterpolated.
    //
    // If the subclass has already called emitEdgeDistanceEquation, then provide the distance
    // equation. Otherwise this function will call emitEdgeDistanceEquation implicitly.
    //
    // Geometry shader must be configured to output triangle strips.
    //
    // Returns the maximum number of vertices that will be emitted.
    int emitEdgeGeometry(GrGLSLGeometryBuilder*, const char* emitVertexFn, const char* leftPt,
                         const char* rightPt, const char* distanceEquation = nullptr) const;

    // Defines an equation ("dot(vec3(pt, 1), distance_equation)") that is -1 on the outside border
    // of a conservative raster edge and 0 on the inside (see emitEdgeGeometry).
    void emitEdgeDistanceEquation(GrGLSLGeometryBuilder*, const char* leftPt, const char* rightPt,
                                  const char* outputDistanceEquation) const;

    // Defines a global vec2 array that contains MSAA sample locations as offsets from pixel center.
    // Subclasses can use this for software multisampling.
    //
    // Returns the number of samples.
    int defineSoftSampleLocations(GrGLSLFragmentBuilder*, const char* samplesName) const;

private:
    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor&,
                 FPCoordTransformIter&& transformIter) final {
        this->setTransformDataHelper(SkMatrix::I(), pdman, &transformIter);
    }

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) final;

    void emitVertexShader(const GrCCPRCoverageProcessor&, GrGLSLVertexBuilder*,
                          const TexelBufferHandle& pointsBuffer, const char* rtAdjust,
                          GrGPArgs* gpArgs) const;
    void emitGeometryShader(const GrCCPRCoverageProcessor&, GrGLSLGeometryBuilder*,
                            const char* rtAdjust) const;
    void emitCoverage(const GrCCPRCoverageProcessor&, GrGLSLFragmentBuilder*,
                      const char* outputColor, const char* outputCoverage) const;

    const CoverageType   fCoverageType;
    GrShaderVar          fGeomWind;
    GrGLSLGeoToFrag      fFragWind;
    GrGLSLGeoToFrag      fFragCoverageTimesWind;

    typedef GrGLSLGeometryProcessor INHERITED;
};

#endif
