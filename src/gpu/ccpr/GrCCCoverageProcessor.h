/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCCoverageProcessor_DEFINED
#define GrCCCoverageProcessor_DEFINED

#include "include/private/SkNx.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLVarying.h"

class GrGLSLFPFragmentBuilder;
class GrGLSLVertexGeoBuilder;
class GrMesh;
class GrOpFlushState;

/**
 * This is the geometry processor for the simple convex primitive shapes (triangles and closed,
 * convex bezier curves) from which ccpr paths are composed. The output is a single-channel alpha
 * value, positive for clockwise shapes and negative for counter-clockwise, that indicates coverage.
 *
 * The caller is responsible to draw all primitives as produced by GrCCGeometry into a cleared,
 * floating point, alpha-only render target using SkBlendMode::kPlus. Once all of a path's
 * primitives have been drawn, the render target contains a composite coverage count that can then
 * be used to draw the path (see GrCCPathProcessor).
 *
 * To draw primitives, use appendMesh() and draw() (defined below).
 */
class GrCCCoverageProcessor : public GrGeometryProcessor {
public:
    enum class PrimitiveType {
        kTriangles,
        kWeightedTriangles,  // Triangles (from the tessellator) whose winding magnitude > 1.
        kQuadratics,
        kCubics,
        kConics
    };
    static const char* PrimitiveTypeName(PrimitiveType);

    // Defines a single primitive shape with 3 input points (i.e. Triangles and Quadratics).
    // X,Y point values are transposed.
    struct TriPointInstance {
        float fValues[6];

        enum class Ordering : bool {
            kXYTransposed,
            kXYInterleaved,
        };

        void set(const SkPoint[3], const Sk2f& translate, Ordering);
        void set(const SkPoint&, const SkPoint&, const SkPoint&, const Sk2f& translate, Ordering);
        void set(const Sk2f& P0, const Sk2f& P1, const Sk2f& P2, const Sk2f& translate, Ordering);
    };

    // Defines a single primitive shape with 4 input points, or 3 input points plus a "weight"
    // parameter duplicated in both lanes of the 4th input (i.e. Cubics, Conics, and Triangles with
    // a weighted winding number). X,Y point values are transposed.
    struct QuadPointInstance {
        float fX[4];
        float fY[4];

        void set(const SkPoint[4], float dx, float dy);
        void setW(const SkPoint[3], const Sk2f& trans, float w);
        void setW(const SkPoint&, const SkPoint&, const SkPoint&, const Sk2f& trans, float w);
        void setW(const Sk2f& P0, const Sk2f& P1, const Sk2f& P2, const Sk2f& trans, float w);
    };

    virtual void reset(PrimitiveType, GrResourceProvider*) = 0;

    PrimitiveType primitiveType() const { return fPrimitiveType; }

    // Number of bezier points for curves, or 3 for triangles.
    int numInputPoints() const { return PrimitiveType::kCubics == fPrimitiveType ? 4 : 3; }

    bool isTriangles() const {
        return PrimitiveType::kTriangles == fPrimitiveType ||
               PrimitiveType::kWeightedTriangles == fPrimitiveType;
    }

    int hasInputWeight() const {
        return PrimitiveType::kWeightedTriangles == fPrimitiveType ||
               PrimitiveType::kConics == fPrimitiveType;
    }

    // GrPrimitiveProcessor overrides.
    const char* name() const override { return PrimitiveTypeName(fPrimitiveType); }
#ifdef SK_DEBUG
    SkString dumpInfo() const override {
        return SkStringPrintf("%s\n%s", this->name(), this->INHERITED::dumpInfo().c_str());
    }
#endif
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        SkDEBUGCODE(this->getDebugBloatKey(b));
        b->add32((int)fPrimitiveType);
    }
    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const final;

#ifdef SK_DEBUG
    // Increases the 1/2 pixel AA bloat by a factor of debugBloat.
    void enableDebugBloat(float debugBloat) { fDebugBloat = debugBloat; }
    bool debugBloatEnabled() const { return fDebugBloat > 0; }
    float debugBloat() const { SkASSERT(this->debugBloatEnabled()); return fDebugBloat; }
    void getDebugBloatKey(GrProcessorKeyBuilder* b) const {
        uint32_t bloatBits;
        memcpy(&bloatBits, &fDebugBloat, 4);
        b->add32(bloatBits);
    }
#endif

    // Appends a GrMesh that will draw the provided instances. The instanceBuffer must be an array
    // of either TriPointInstance or QuadPointInstance, depending on this processor's RendererPass,
    // with coordinates in the desired shape's final atlas-space position.
    virtual void appendMesh(sk_sp<const GrGpuBuffer> instanceBuffer, int instanceCount,
                            int baseInstance, SkTArray<GrMesh>* out) const = 0;

    virtual void draw(GrOpFlushState*, const GrPipeline&, const SkIRect scissorRects[],
                      const GrMesh[], int meshCount, const SkRect& drawBounds) const;

    // The Shader provides code to calculate each pixel's coverage in a RenderPass. It also
    // provides details about shape-specific geometry.
    class Shader {
    public:
        // Returns true if the Impl should not calculate the coverage argument for emitVaryings().
        // If true, then "coverage" will have a signed magnitude of 1.
        virtual bool calculatesOwnEdgeCoverage() const { return false; }

        // Called before generating geometry. Subclasses may set up internal member variables during
        // this time that will be needed during onEmitVaryings (e.g. transformation matrices).
        //
        // If the 'outHull4' parameter is provided, and there are not 4 input points, the subclass
        // is required to fill it with the name of a 4-point hull around which the Impl can generate
        // its geometry. If it is left unchanged, the Impl will use the regular input points.
        virtual void emitSetupCode(
                GrGLSLVertexGeoBuilder*, const char* pts, const char** outHull4 = nullptr) const {
            SkASSERT(!outHull4);
        }

        void emitVaryings(
                GrGLSLVaryingHandler* varyingHandler, GrGLSLVarying::Scope scope, SkString* code,
                const char* position, const char* coverage, const char* cornerCoverage,
                const char* wind) {
            SkASSERT(GrGLSLVarying::Scope::kVertToGeo != scope);
            this->onEmitVaryings(
                    varyingHandler, scope, code, position, coverage, cornerCoverage, wind);
        }

        // Writes the signed coverage value at the current pixel to "outputCoverage".
        virtual void emitFragmentCoverageCode(
                GrGLSLFPFragmentBuilder*, const char* outputCoverage) const = 0;

        // Assigns the built-in sample mask at the current pixel.
        virtual void emitSampleMaskCode(GrGLSLFPFragmentBuilder*) const = 0;

        // Calculates the winding direction of the input points (+1, -1, or 0). Wind for extremely
        // thin triangles gets rounded to zero.
        static void CalcWind(const GrCCCoverageProcessor&, GrGLSLVertexGeoBuilder*, const char* pts,
                             const char* outputWind);

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
        static void CalcCornerAttenuation(GrGLSLVertexGeoBuilder*, const char* leftDir,
                                          const char* rightDir, const char* outputAttenuation);

        virtual ~Shader() {}

    protected:
        // Here the subclass adds its internal varyings to the handler and produces code to
        // initialize those varyings from a given position and coverage values.
        //
        // NOTE: the coverage values are signed appropriately for wind.
        //       'coverage' will only be +1 or -1 on curves.
        virtual void onEmitVaryings(
                GrGLSLVaryingHandler*, GrGLSLVarying::Scope, SkString* code, const char* position,
                const char* coverage, const char* cornerCoverage, const char* wind) = 0;

        // Returns the name of a Shader's internal varying at the point where where its value is
        // assigned. This is intended to work whether called for a vertex or a geometry shader.
        const char* OutName(const GrGLSLVarying& varying) const {
            using Scope = GrGLSLVarying::Scope;
            SkASSERT(Scope::kVertToGeo != varying.scope());
            return Scope::kGeoToFrag == varying.scope() ? varying.gsOut() : varying.vsOut();
        }

        // Our friendship with GrGLSLShaderBuilder does not propagate to subclasses.
        inline static SkString& AccessCodeString(GrGLSLShaderBuilder* s) { return s->code(); }
    };

protected:
    // Slightly undershoot a bloat radius of 0.5 so vertices that fall on integer boundaries don't
    // accidentally bleed into neighbor pixels.
    static constexpr float kAABloatRadius = 0.491111f;

    GrCCCoverageProcessor(ClassID classID) : INHERITED(classID) {}

    virtual GrGLSLPrimitiveProcessor* onCreateGLSLInstance(std::unique_ptr<Shader>) const = 0;

    // Our friendship with GrGLSLShaderBuilder does not propagate to subclasses.
    inline static SkString& AccessCodeString(GrGLSLShaderBuilder* s) { return s->code(); }

    PrimitiveType fPrimitiveType;
    SkDEBUGCODE(float fDebugBloat = 0);

    class TriangleShader;

    typedef GrGeometryProcessor INHERITED;
};

inline const char* GrCCCoverageProcessor::PrimitiveTypeName(PrimitiveType type) {
    switch (type) {
        case PrimitiveType::kTriangles: return "kTriangles";
        case PrimitiveType::kWeightedTriangles: return "kWeightedTriangles";
        case PrimitiveType::kQuadratics: return "kQuadratics";
        case PrimitiveType::kCubics: return "kCubics";
        case PrimitiveType::kConics: return "kConics";
    }
    SK_ABORT("Invalid PrimitiveType");
}

inline void GrCCCoverageProcessor::TriPointInstance::set(
        const SkPoint p[3], const Sk2f& translate, Ordering ordering) {
    this->set(p[0], p[1], p[2], translate, ordering);
}

inline void GrCCCoverageProcessor::TriPointInstance::set(
        const SkPoint& p0, const SkPoint& p1, const SkPoint& p2, const Sk2f& translate,
        Ordering ordering) {
    Sk2f P0 = Sk2f::Load(&p0);
    Sk2f P1 = Sk2f::Load(&p1);
    Sk2f P2 = Sk2f::Load(&p2);
    this->set(P0, P1, P2, translate, ordering);
}

inline void GrCCCoverageProcessor::TriPointInstance::set(
        const Sk2f& P0, const Sk2f& P1, const Sk2f& P2, const Sk2f& translate, Ordering ordering) {
    if (Ordering::kXYTransposed == ordering) {
        Sk2f::Store3(fValues, P0 + translate, P1 + translate, P2 + translate);
    } else {
        (P0 + translate).store(fValues);
        (P1 + translate).store(fValues + 2);
        (P2 + translate).store(fValues + 4);
    }
}

inline void GrCCCoverageProcessor::QuadPointInstance::set(const SkPoint p[4], float dx, float dy) {
    Sk4f X,Y;
    Sk4f::Load2(p, &X, &Y);
    (X + dx).store(&fX);
    (Y + dy).store(&fY);
}

inline void GrCCCoverageProcessor::QuadPointInstance::setW(const SkPoint p[3], const Sk2f& trans,
                                                           float w) {
    this->setW(p[0], p[1], p[2], trans, w);
}

inline void GrCCCoverageProcessor::QuadPointInstance::setW(const SkPoint& p0, const SkPoint& p1,
                                                           const SkPoint& p2, const Sk2f& trans,
                                                           float w) {
    Sk2f P0 = Sk2f::Load(&p0);
    Sk2f P1 = Sk2f::Load(&p1);
    Sk2f P2 = Sk2f::Load(&p2);
    this->setW(P0, P1, P2, trans, w);
}

inline void GrCCCoverageProcessor::QuadPointInstance::setW(const Sk2f& P0, const Sk2f& P1,
                                                           const Sk2f& P2, const Sk2f& trans,
                                                           float w) {
    Sk2f W = Sk2f(w);
    Sk2f::Store4(this, P0 + trans, P1 + trans, P2 + trans, W);
}

#endif
