/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCCoverageProcessor_DEFINED
#define GrCCCoverageProcessor_DEFINED

#include "GrCaps.h"
#include "GrGeometryProcessor.h"
#include "GrShaderCaps.h"
#include "SkNx.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLVarying.h"

class GrGLSLFPFragmentBuilder;
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
    // Defines a single primitive shape with 3 input points (i.e. Triangles and Quadratics).
    // X,Y point values are transposed.
    struct TriPointInstance {
        float fX[3];
        float fY[3];

        void set(const SkPoint[3], const Sk2f& trans);
        void set(const SkPoint&, const SkPoint&, const SkPoint&, const Sk2f& trans);
    };

    // Defines a single primitive shape with 4 input points, or 3 input points plus a W parameter
    // duplicated in both 4th components (i.e. Cubics or Triangles with a custom winding number).
    // X,Y point values are transposed.
    struct QuadPointInstance {
        float fX[4];
        float fY[4];

        void set(const SkPoint[4], float dx, float dy);
        void set(const SkPoint&, const SkPoint&, const SkPoint&, const Sk2f& trans, float w);
    };

    // All primitive shapes (triangles and closed, convex bezier curves) may require two render
    // passes: One to draw a rough outline of the shape, and a second pass to touch up the corners.
    // Check DoesRenderPass() before attempting to draw a given RenderPass. Here we enumerate every
    // possible render pass needed in order to produce a complete coverage count mask. This is an
    // exhaustive list of all ccpr coverage shaders.
    enum class RenderPass {
        kTriangles,
        kTriangleCorners,
        kQuadratics,
        kQuadraticCorners,
        kCubics,
        kCubicCorners
    };
    static bool RenderPassIsCubic(RenderPass);
    static const char* RenderPassName(RenderPass);

    constexpr static bool DoesRenderPass(RenderPass renderPass, const GrCaps& caps) {
        return RenderPass::kTriangleCorners != renderPass ||
               caps.shaderCaps()->geometryShaderSupport();
    }

    enum class WindMethod : bool {
        kCrossProduct, // Calculate wind = +/-1 by sign of the cross product.
        kInstanceData // Instance data provides custom, signed wind values of any magnitude.
                      // (For tightly-wound tessellated triangles.)
    };

    GrCCCoverageProcessor(GrResourceProvider* rp, RenderPass pass, WindMethod windMethod)
            : INHERITED(kGrCCCoverageProcessor_ClassID)
            , fRenderPass(pass)
            , fWindMethod(windMethod)
            , fImpl(rp->caps()->shaderCaps()->geometryShaderSupport() ? Impl::kGeometryShader
                                                                      : Impl::kVertexShader) {
        SkASSERT(DoesRenderPass(pass, *rp->caps()));
        if (Impl::kGeometryShader == fImpl) {
            this->initGS();
        } else {
            this->initVS(rp);
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

    // Appends a GrMesh that will draw the provided instances. The instanceBuffer must be an array
    // of either TriPointInstance or QuadPointInstance, depending on this processor's RendererPass,
    // with coordinates in the desired shape's final atlas-space position.
    void appendMesh(GrBuffer* instanceBuffer, int instanceCount, int baseInstance,
                    SkTArray<GrMesh>* out) const {
        if (Impl::kGeometryShader == fImpl) {
            this->appendGSMesh(instanceBuffer, instanceCount, baseInstance, out);
        } else {
            this->appendVSMesh(instanceBuffer, instanceCount, baseInstance, out);
        }
    }

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

        void emitVaryings(GrGLSLVaryingHandler* varyingHandler, GrGLSLVarying::Scope scope,
                          SkString* code, const char* position, const char* coverage,
                          const char* attenuatedCoverage, const char* wind) {
            SkASSERT(GrGLSLVarying::Scope::kVertToGeo != scope);
            this->onEmitVaryings(varyingHandler, scope, code, position, coverage,
                                 attenuatedCoverage, wind);
        }

        void emitFragmentCode(const GrCCCoverageProcessor&, GrGLSLFPFragmentBuilder*,
                              const char* skOutputColor, const char* skOutputCoverage) const;

        // Defines an equation ("dot(float3(pt, 1), distance_equation)") that is -1 on the outside
        // border of a conservative raster edge and 0 on the inside. 'leftPt' and 'rightPt' must be
        // ordered clockwise.
        static void EmitEdgeDistanceEquation(GrGLSLVertexGeoBuilder*, const char* leftPt,
                                             const char* rightPt,
                                             const char* outputDistanceEquation);

        // Calculates an edge's coverage at a conservative raster vertex. The edge is defined by two
        // clockwise-ordered points, 'leftPt' and 'rightPt'. 'rasterVertexDir' is a pair of +/-1
        // values that point in the direction of conservative raster bloat, starting from an
        // endpoint.
        //
        // Coverage values ramp from -1 (completely outside the edge) to 0 (completely inside).
        static void CalcEdgeCoverageAtBloatVertex(GrGLSLVertexGeoBuilder*, const char* leftPt,
                                                  const char* rightPt, const char* rasterVertexDir,
                                                  const char* outputCoverage);

        // Calculates an edge's coverage at two conservative raster vertices.
        // (See CalcEdgeCoverageAtBloatVertex).
        static void CalcEdgeCoveragesAtBloatVertices(GrGLSLVertexGeoBuilder*, const char* leftPt,
                                                     const char* rightPt, const char* bloatDir1,
                                                     const char* bloatDir2,
                                                     const char* outputCoverages);

        // Corner boxes require an additional "attenuation" varying that is multiplied by the
        // regular (linearly-interpolated) coverage. This function calculates the attenuation value
        // to use in the single, outermost vertex. The remaining three vertices of the corner box
        // all use an attenuation value of 1.
        static void CalcCornerCoverageAttenuation(GrGLSLVertexGeoBuilder*, const char* leftDir,
                                                  const char* rightDir,
                                                  const char* outputAttenuation);

        virtual ~Shader() {}

    protected:
        // Here the subclass adds its internal varyings to the handler and produces code to
        // initialize those varyings from a given position, input coverage value, and wind.
        //
        // NOTE: the coverage inputs are only relevant for triangles. Otherwise they are null.
        virtual void onEmitVaryings(GrGLSLVaryingHandler*, GrGLSLVarying::Scope, SkString* code,
                                    const char* position, const char* coverage,
                                    const char* attenuatedCoverage, const char* wind) = 0;

        // Emits the fragment code that calculates a pixel's signed coverage value.
        virtual void onEmitFragmentCode(GrGLSLFPFragmentBuilder*,
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
        static int DefineSoftSampleLocations(GrGLSLFPFragmentBuilder* f, const char* samplesName);
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
    const WindMethod fWindMethod;
    const Impl fImpl;
    SkDEBUGCODE(float fDebugBloat = 0);

    // Used by VSImpl.
    sk_sp<const GrBuffer> fVertexBuffer;
    sk_sp<const GrBuffer> fIndexBuffer;
    int fNumIndicesPerInstance;
    GrPrimitiveType fPrimitiveType;

    typedef GrGeometryProcessor INHERITED;
};

inline void GrCCCoverageProcessor::TriPointInstance::set(const SkPoint p[3], const Sk2f& trans) {
    this->set(p[0], p[1], p[2], trans);
}

inline void GrCCCoverageProcessor::TriPointInstance::set(const SkPoint& p0, const SkPoint& p1,
                                                         const SkPoint& p2, const Sk2f& trans) {
    Sk2f P0 = Sk2f::Load(&p0) + trans;
    Sk2f P1 = Sk2f::Load(&p1) + trans;
    Sk2f P2 = Sk2f::Load(&p2) + trans;
    Sk2f::Store3(this, P0, P1, P2);
}

inline void GrCCCoverageProcessor::QuadPointInstance::set(const SkPoint p[4], float dx, float dy) {
    Sk4f X,Y;
    Sk4f::Load2(p, &X, &Y);
    (X + dx).store(&fX);
    (Y + dy).store(&fY);
}

inline void GrCCCoverageProcessor::QuadPointInstance::set(const SkPoint& p0, const SkPoint& p1,
                                                          const SkPoint& p2, const Sk2f& trans,
                                                          float w) {
    Sk2f P0 = Sk2f::Load(&p0) + trans;
    Sk2f P1 = Sk2f::Load(&p1) + trans;
    Sk2f P2 = Sk2f::Load(&p2) + trans;
    Sk2f W = Sk2f(w);
    Sk2f::Store4(this, P0, P1, P2, W);
}

inline bool GrCCCoverageProcessor::RenderPassIsCubic(RenderPass pass) {
    switch (pass) {
        case RenderPass::kTriangles:
        case RenderPass::kTriangleCorners:
        case RenderPass::kQuadratics:
        case RenderPass::kQuadraticCorners:
            return false;
        case RenderPass::kCubics:
        case RenderPass::kCubicCorners:
            return true;
    }
    SK_ABORT("Invalid RenderPass");
    return false;
}

inline const char* GrCCCoverageProcessor::RenderPassName(RenderPass pass) {
    switch (pass) {
        case RenderPass::kTriangles: return "kTriangles";
        case RenderPass::kTriangleCorners: return "kTriangleCorners";
        case RenderPass::kQuadratics: return "kQuadratics";
        case RenderPass::kQuadraticCorners: return "kQuadraticCorners";
        case RenderPass::kCubics: return "kCubics";
        case RenderPass::kCubicCorners: return "kCubicCorners";
    }
    SK_ABORT("Invalid RenderPass");
    return "";
}

#endif
