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

class GrGLSLPPFragmentBuilder;
class GrGLSLShaderBuilder;

/**
 * This is the geometry processor for the simple convex primitive shapes (triangles and closed curve
 * segments) from which ccpr paths are composed. The output is a single-channel alpha value,
 * positive for clockwise shapes and negative for counter-clockwise, that indicates coverage.
 *
 * The caller is responsible to execute all render passes for all applicable primitives into a
 * cleared, floating point, alpha-only render target using SkBlendMode::kPlus (see RenderPass
 * below). Once all of a path's primitives have been drawn, the render target contains a composite
 * coverage count that can then be used to draw the path (see GrCCPRPathProcessor).
 *
 * Caller provides the primitives' (x,y) input points in an fp32x2 (RG) texel buffer, and an
 * instance buffer with a single int32x4 attrib (for triangles) or int32x2 (for curves) defined
 * below. There are no vertex attribs.
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

    struct TriangleInstance {
        int32_t fPt0Idx;
        int32_t fPt1Idx;
        int32_t fPt2Idx;
        int32_t fPackedAtlasOffset; // (offsetY << 16) | (offsetX & 0xffff)
    };

    GR_STATIC_ASSERT(4 * 4 == sizeof(TriangleInstance));

    struct CurveInstance {
        int32_t fPtsIdx;
        int32_t fPackedAtlasOffset; // (offsetY << 16) | (offsetX & 0xffff)
    };

    GR_STATIC_ASSERT(2 * 4 == sizeof(CurveInstance));

    /**
     * All primitive shapes (triangles and convex closed curve segments) require more than one
     * render pass. Here we enumerate every render pass needed in order to produce a complete
     * coverage count mask. This is an exhaustive list of all ccpr coverage shaders.
     */
    enum class RenderPass {
        // Triangles.
        kTriangleHulls,
        kTriangleEdges,
        kTriangleCorners,

        // Quadratics.
        kQuadraticHulls,
        kQuadraticCorners,

        // Cubics.
        kSerpentineHulls,
        kLoopHulls,
        kSerpentineCorners,
        kLoopCorners
    };

    static const char* GetRenderPassName(RenderPass);

    /**
     * This serves as the base class for each RenderPass's Shader. It indicates what type of
     * geometry the Impl should generate and provides implementation-independent code to process
     * the inputs and calculate coverage in the fragment Shader.
     */
    class Shader {
    public:
        using TexelBufferHandle = GrGLSLGeometryProcessor::TexelBufferHandle;

        // This enum specifies the type of geometry that should be generated for a Shader instance.
        // Subclasses are limited to three built-in types of geometry to choose from:
        enum class GeometryType {
            // Generates a conservative raster hull around the input points. This is the geometry
            // that causes a pixel to be rasterized if it is touched anywhere by the input polygon.
            // Coverage is +1 all around.
            //
            // Logically, the conservative raster hull is equivalent to the convex hull of pixel
            // size boxes centered around each input point.
            kHull,

            // Generates the conservative rasters of the input edges (i.e. convex hull of two
            // pixel-size boxes centered on both endpoints). Coverage is -1 on the outside border of
            // the edge geometry and 0 on the inside. This is the only geometry type that associates
            // coverage values with the output points. It effectively converts a jagged conservative
            // raster edge into a smooth antialiased edge.
            kEdges,

            // Generates the conservative rasters of the corners specified by the geometry provider
            // (i.e. pixel-size box centered on the corner point). Coverage is +1 all around.
            kCorners
        };

        virtual GeometryType getGeometryType() const = 0;
        virtual int getNumInputPoints() const = 0;

        // Returns the number of independent geometric segments to generate for the render pass
        // (number of wedges for a hull, number of edges, or number of corners.)
        virtual int getNumSegments() const = 0;

        // Appends an expression that fetches input point # "pointId" from the texel buffer.
        virtual void appendInputPointFetch(const GrCCPRCoverageProcessor&, GrGLSLShaderBuilder*,
                                           const TexelBufferHandle& pointsBuffer,
                                           const char* pointId) const = 0;

        // Determines the winding direction of the primitive. The subclass must write a value of
        // either -1, 0, or +1 to "outputWind" (e.g. "sign(area)"). Fractional values are not valid.
        virtual void emitWind(GrGLSLShaderBuilder*, const char* pts, const char* rtAdjust,
                              const char* outputWind) const = 0;

        union GeometryVars {
            struct {
                const char* fAlternatePoints; // floatNx2 (if left null, will use input points).
                const char* fAlternateMidpoint; // float2 (if left null, finds euclidean midpoint).
            } fHullVars;

            struct {
                const char* fPoint; // float2
            } fCornerVars;

            GeometryVars() { memset(this, 0, sizeof(*this)); }
        };

        // Called before generating geometry. Subclasses must fill out the applicable fields in
        // GeometryVars (if any), and may also use this opportunity to setup internal member
        // variables that will be needed during onEmitVaryings (e.g. transformation matrices).
        virtual void emitSetupCode(GrGLSLShaderBuilder*, const char* pts, const char* segmentId,
                                   const char* bloat, const char* wind, const char* rtAdjust,
                                   GeometryVars*) const {}

        void emitVaryings(GrGLSLVaryingHandler*, SkString* code, const char* position,
                          const char* coverage, const char* wind);

        void emitFragmentCode(const GrCCPRCoverageProcessor& proc, GrGLSLPPFragmentBuilder*,
                              const char* skOutputColor, const char* skOutputCoverage) const;

        // Defines an equation ("dot(float3(pt, 1), distance_equation)") that is -1 on the outside
        // border of a conservative raster edge and 0 on the inside (see emitEdgeGeometry).
        static void EmitEdgeDistanceEquation(GrGLSLShaderBuilder*, const char* leftPt,
                                             const char* rightPt,
                                             const char* outputDistanceEquation);

        // Defines a global float2 array that contains MSAA sample locations as offsets from pixel
        // center. Subclasses can use this for software multisampling.
        //
        // Returns the number of samples.
        static int DefineSoftSampleLocations(GrGLSLPPFragmentBuilder* f, const char* samplesName);

        virtual ~Shader() {}

    protected:
        enum class WindHandling : bool {
            kHandled,
            kNotHandled
        };

        // Here the subclass adds its internal varyings to the handler and produces code to
        // initialize those varyings from a given position, coverage value, and wind.
        //
        // Returns whether the subclass will handle wind modulation or if this base class should
        // take charge of multiplying the final coverage output by "wind".
        //
        // NOTE: the coverage parameter is only relevant for edges (see comments in GeometryType).
        // Otherwise it is +1 all around.
        virtual WindHandling onEmitVaryings(GrGLSLVaryingHandler*, SkString* code,
                                            const char* position, const char* coverage,
                                            const char* wind) = 0;

        // Emits the fragment code that calculates a pixel's coverage value. If using
        // WindHandling::kHandled, this value must be signed appropriately.
        virtual void onEmitFragmentCode(GrGLSLPPFragmentBuilder*,
                                        const char* outputCoverage) const = 0;

    private:
        GrGLSLGeoToFrag fWind{kHalf_GrSLType};
    };

    GrCCPRCoverageProcessor(RenderPass, GrBuffer* pointsBuffer);

    const char* instanceAttrib() const { return fInstanceAttrib.fName; }
    const char* name() const override { return GetRenderPassName(fRenderPass); }
    SkString dumpInfo() const override {
        return SkStringPrintf("%s\n%s", this->name(), this->INHERITED::dumpInfo().c_str());
    }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override;

#ifdef SK_DEBUG
    // Increases the 1/2 pixel AA bloat by a factor of debugBloat and outputs color instead of
    // coverage (coverage=+1 -> green, coverage=0 -> black, coverage=-1 -> red).
    void enableDebugVisualizations(float debugBloat) { fDebugBloat = debugBloat; }
    bool debugVisualizationsEnabled() const { return fDebugBloat > 0; }
    float debugBloat() const { SkASSERT(this->debugVisualizationsEnabled()); return fDebugBloat; }

    static void Validate(GrRenderTargetProxy* atlasProxy);
#endif

    class GSImpl;

private:
    // Slightly undershoot a bloat radius of 0.5 so vertices that fall on integer boundaries don't
    // accidentally bleed into neighbor pixels.
    static constexpr float kAABloatRadius = 0.491111f;

    static GrGLSLPrimitiveProcessor* CreateGSImpl(std::unique_ptr<Shader>);

    int atlasOffsetIdx() const {
        SkASSERT(kInt2_GrVertexAttribType == fInstanceAttrib.fType ||
                 kInt4_GrVertexAttribType == fInstanceAttrib.fType);
        return kInt4_GrVertexAttribType == fInstanceAttrib.fType ? 3 : 1;
    }

    const RenderPass    fRenderPass;
    const Attribute&    fInstanceAttrib;
    BufferAccess        fPointsBufferAccess;
    SkDEBUGCODE(float   fDebugBloat = 0;)

    typedef GrGeometryProcessor INHERITED;
};

#endif
