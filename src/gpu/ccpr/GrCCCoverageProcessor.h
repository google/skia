/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCCoverageProcessor_DEFINED
#define GrCCCoverageProcessor_DEFINED

#include "GrGeometryProcessor.h"
#include "GrShaderCaps.h"
#include "SkNx.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLVarying.h"

class GrGLSLPPFragmentBuilder;
class GrGLSLVertexGeoBuilder;
class GrMesh;

/**
 * This is the geometry processor for the simple convex primitive shapes (triangles and closed,
 * convex bezier curves) from which ccpr paths are composed. The output is a single-channel alpha
 * value, positive for clockwise shapes and negative for counter-clockwise, that indicates coverage.
 *
 * The caller is responsible to execute all render passes for all applicable primitives into a
 * cleared, floating point, alpha-only render target using SkBlendMode::kPlus (see RenderPass
 * below). Once all of a path's primitives have been drawn, the render target contains a composite
 * coverage count that can then be used to draw the path (see GrCCPathProcessor).
 *
 * To draw a renderer pass, see appendMesh below.
 */
class GrCCCoverageProcessor : public GrGeometryProcessor {
public:
    // Defines a single triangle or closed quadratic bezier, with transposed x,y point values.
    struct TriangleInstance {
        float fX[3];
        float fY[3];

        void set(const SkPoint[3], const Sk2f& trans);
        void set(const SkPoint&, const SkPoint&, const SkPoint&, const Sk2f& trans);
    };

    // Defines a single closed cubic bezier, with transposed x,y point values.
    struct CubicInstance {
        float fX[4];
        float fY[4];

        void set(const SkPoint[4], float dx, float dy);
    };

    // All primitive shapes (triangles and closed, convex bezier curves) require more than one
    // render pass. Here we enumerate every render pass needed in order to produce a complete
    // coverage count mask. This is an exhaustive list of all ccpr coverage shaders.
    //
    // During a render pass, the "Impl" (GSImpl or VSimpl) generates conservative geometry for
    // rasterization, and the Shader decides the coverage value at each pixel.
    enum class RenderPass {
        // For a Hull, the Impl generates a "conservative raster hull" around the input points. This
        // is the geometry that causes a pixel to be rasterized if it is touched anywhere by the
        // input polygon. The initial coverage values sent to the Shader at each vertex are either
        // null, or +1 all around if the Impl combines this pass with kTriangleEdges. Logically,
        // the conservative raster hull is equivalent to the convex hull of pixel size boxes
        // centered on each input point.
        kTriangleHulls,
        kQuadraticHulls,
        kCubicHulls,

        // For Edges, the Impl generates conservative rasters around every input edge (i.e. convex
        // hulls of two pixel-size boxes centered on both of the edge's endpoints). The initial
        // coverage values sent to the Shader at each vertex are -1 on the outside border of the
        // edge geometry and 0 on the inside. This is the only geometry type that associates
        // coverage values with the output vertices. Interpolated, these coverage values convert
        // jagged conservative raster edges into a smooth antialiased edge.
        //
        // NOTE: The Impl may combine this pass with kTriangleHulls, in which case DoesRenderPass()
        // will be false for kTriangleEdges and it must not be used.
        kTriangleEdges,

        // For Corners, the Impl Generates the conservative rasters of corner points (i.e.
        // pixel-size boxes). It generates 3 corner boxes for triangles and 2 for curves. The Shader
        // specifies which corners. Initial coverage values sent to the Shader will be null.
        kTriangleCorners,
        kQuadraticCorners,
        kCubicCorners
    };
    static bool RenderPassIsCubic(RenderPass);
    static const char* RenderPassName(RenderPass);

    constexpr static bool DoesRenderPass(RenderPass renderPass, const GrShaderCaps& caps) {
        return RenderPass::kTriangleEdges != renderPass || caps.geometryShaderSupport();
    }

    GrCCCoverageProcessor(GrResourceProvider* rp, RenderPass pass, const GrShaderCaps& caps)
            : INHERITED(kGrCCCoverageProcessor_ClassID)
            , fRenderPass(pass)
            , fImpl(caps.geometryShaderSupport() ?  Impl::kGeometryShader : Impl::kVertexShader) {
        SkASSERT(DoesRenderPass(pass, caps));
        if (Impl::kGeometryShader == fImpl) {
            this->initGS();
        } else {
            this->initVS(rp);
        }
    }

    // Appends a GrMesh that will draw the provided instances. The instanceBuffer must be an array
    // of either TriangleInstance or CubicInstance, depending on this processor's RendererPass, with
    // coordinates in the desired shape's final atlas-space position.
    //
    // NOTE: Quadratics use TriangleInstance since both have 3 points.
    void appendMesh(GrBuffer* instanceBuffer, int instanceCount, int baseInstance,
                    SkTArray<GrMesh>* out) {
        if (Impl::kGeometryShader == fImpl) {
            this->appendGSMesh(instanceBuffer, instanceCount, baseInstance, out);
        } else {
            this->appendVSMesh(instanceBuffer, instanceCount, baseInstance, out);
        }
    }

    // GrPrimitiveProcessor overrides.
    const char* name() const override { return RenderPassName(fRenderPass); }
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
#endif

    // The Shader provides code to calculate each pixel's coverage in a RenderPass. It also
    // provides details about shape-specific geometry.
    class Shader {
    public:
        union GeometryVars {
            struct {
                const char* fAlternatePoints; // floatNx2 (if left null, will use input points).
            } fHullVars;

            struct {
                const char* fPoint; // float2
            } fCornerVars;

            GeometryVars() { memset(this, 0, sizeof(*this)); }
        };

        // Called before generating geometry. Subclasses must fill out the applicable fields in
        // GeometryVars (if any), and may also use this opportunity to setup internal member
        // variables that will be needed during onEmitVaryings (e.g. transformation matrices).
        //
        // repetitionID is a 0-based index and indicates which edge or corner is being generated.
        // It will be null when generating a hull.
        virtual void emitSetupCode(GrGLSLVertexGeoBuilder*, const char* pts,
                                   const char* repetitionID, const char* wind,
                                   GeometryVars*) const {}

        void emitVaryings(GrGLSLVaryingHandler*, GrGLSLVarying::Scope, SkString* code,
                          const char* position, const char* coverage, const char* wind);

        void emitFragmentCode(const GrCCCoverageProcessor& proc, GrGLSLPPFragmentBuilder*,
                              const char* skOutputColor, const char* skOutputCoverage) const;

        // Defines an equation ("dot(float3(pt, 1), distance_equation)") that is -1 on the outside
        // border of a conservative raster edge and 0 on the inside. 'leftPt' and 'rightPt' must be
        // ordered clockwise.
        static void EmitEdgeDistanceEquation(GrGLSLVertexGeoBuilder*, const char* leftPt,
                                             const char* rightPt,
                                             const char* outputDistanceEquation);

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
        // NOTE: the coverage parameter is only relevant for edges (see comments in RenderPass).
        // Otherwise it is +1 all around.
        virtual WindHandling onEmitVaryings(GrGLSLVaryingHandler*, GrGLSLVarying::Scope,
                                            SkString* code, const char* position,
                                            const char* coverage, const char* wind) = 0;

        // Emits the fragment code that calculates a pixel's coverage value. If using
        // WindHandling::kHandled, this value must be signed appropriately.
        virtual void onEmitFragmentCode(GrGLSLPPFragmentBuilder*,
                                        const char* outputCoverage) const = 0;

        // Returns the name of a Shader's internal varying at the point where where its value is
        // assigned. This is intended to work whether called for a vertex or a geometry shader.
        const char* OutName(const GrGLSLVarying& varying) const {
            using Scope = GrGLSLVarying::Scope;
            SkASSERT(Scope::kVertToGeo != varying.scope());
            return Scope::kGeoToFrag == varying.scope() ? varying.gsOut() : varying.vsOut();
        }

        // Defines a global float2 array that contains MSAA sample locations as offsets from pixel
        // center. Subclasses can use this for software multisampling.
        //
        // Returns the number of samples.
        static int DefineSoftSampleLocations(GrGLSLPPFragmentBuilder* f, const char* samplesName);

    private:
        GrGLSLVarying fWind;
    };

    class GSImpl;
    class VSImpl;

private:
    // Slightly undershoot a bloat radius of 0.5 so vertices that fall on integer boundaries don't
    // accidentally bleed into neighbor pixels.
    static constexpr float kAABloatRadius = 0.491111f;

    // Number of bezier points for curves, or 3 for triangles.
    int numInputPoints() const { return RenderPassIsCubic(fRenderPass) ? 4 : 3; }

    enum class Impl : bool {
        kGeometryShader,
        kVertexShader
    };

    void initGS();
    void initVS(GrResourceProvider*);

    void appendGSMesh(GrBuffer* instanceBuffer, int instanceCount, int baseInstance,
                      SkTArray<GrMesh>* out) const;
    void appendVSMesh(GrBuffer* instanceBuffer, int instanceCount, int baseInstance,
                      SkTArray<GrMesh>* out) const;

    GrGLSLPrimitiveProcessor* createGSImpl(std::unique_ptr<Shader>) const;
    GrGLSLPrimitiveProcessor* createVSImpl(std::unique_ptr<Shader>) const;

    const RenderPass fRenderPass;
    const Impl fImpl;
    sk_sp<const GrBuffer> fVertexBuffer; // Used by VSImpl.
    sk_sp<const GrBuffer> fIndexBuffer; // Used by VSImpl.
    SkDEBUGCODE(float fDebugBloat = 0);

    typedef GrGeometryProcessor INHERITED;
};

inline void GrCCCoverageProcessor::TriangleInstance::set(const SkPoint p[3], const Sk2f& trans) {
    this->set(p[0], p[1], p[2], trans);
}

inline void GrCCCoverageProcessor::TriangleInstance::set(const SkPoint& p0, const SkPoint& p1,
                                                         const SkPoint& p2, const Sk2f& trans) {
    Sk2f P0 = Sk2f::Load(&p0) + trans;
    Sk2f P1 = Sk2f::Load(&p1) + trans;
    Sk2f P2 = Sk2f::Load(&p2) + trans;
    Sk2f::Store3(this, P0, P1, P2);
}

inline void GrCCCoverageProcessor::CubicInstance::set(const SkPoint p[4], float dx, float dy) {
    Sk4f X,Y;
    Sk4f::Load2(p, &X, &Y);
    (X + dx).store(&fX);
    (Y + dy).store(&fY);
}

inline bool GrCCCoverageProcessor::RenderPassIsCubic(RenderPass pass) {
    switch (pass) {
        case RenderPass::kTriangleHulls:
        case RenderPass::kTriangleEdges:
        case RenderPass::kTriangleCorners:
        case RenderPass::kQuadraticHulls:
        case RenderPass::kQuadraticCorners:
            return false;
        case RenderPass::kCubicHulls:
        case RenderPass::kCubicCorners:
            return true;
    }
    SK_ABORT("Invalid RenderPass");
    return false;
}

inline const char* GrCCCoverageProcessor::RenderPassName(RenderPass pass) {
    switch (pass) {
        case RenderPass::kTriangleHulls: return "kTriangleHulls";
        case RenderPass::kTriangleEdges: return "kTriangleEdges";
        case RenderPass::kTriangleCorners: return "kTriangleCorners";
        case RenderPass::kQuadraticHulls: return "kQuadraticHulls";
        case RenderPass::kQuadraticCorners: return "kQuadraticCorners";
        case RenderPass::kCubicHulls: return "kCubicHulls";
        case RenderPass::kCubicCorners: return "kCubicCorners";
    }
    SK_ABORT("Invalid RenderPass");
    return "";
}

#endif
