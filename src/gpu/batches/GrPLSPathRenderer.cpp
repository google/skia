/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPLSPathRenderer.h"

#include "SkChunkAlloc.h"
#include "SkGeometry.h"
#include "SkPathPriv.h"
#include "SkString.h"
#include "SkTSort.h"
#include "SkTraceEvent.h"
#include "GrBatchFlushState.h"
#include "GrBatchTest.h"
#include "GrCaps.h"
#include "GrContext.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrPLSGeometryProcessor.h"
#include "GrInvariantOutput.h"
#include "GrPathUtils.h"
#include "GrProcessor.h"
#include "GrPipelineBuilder.h"
#include "GrStyle.h"
#include "GrTessellator.h"
#include "batches/GrVertexBatch.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"
#include "glsl/GrGLSLPLSPathRendering.h"

GrPLSPathRenderer::GrPLSPathRenderer() {
}

struct PLSVertex {
    SkPoint  fPos;
    // for triangles, these are the three triangle vertices
    // for quads, vert1 is the texture UV coords, and vert2 and vert3 are the line segment
    // comprising the flat edge of the quad
    SkPoint  fVert1;
    SkPoint  fVert2;
    SkPoint  fVert3;
    int fWinding;
};
typedef SkTArray<PLSVertex, true> PLSVertices;

typedef SkTArray<SkPoint, true> FinishVertices;

static const float kCubicTolerance = 0.5f;
static const float kConicTolerance = 0.5f;

static const float kBloatSize = 1.0f;

static const float kBloatLimit = 640000.0f;

#define kQuadNumVertices 5
static void add_quad(SkPoint pts[3], PLSVertices& vertices) {
    SkPoint normal = SkPoint::Make(pts[0].fY - pts[2].fY,
                                   pts[2].fX - pts[0].fX);
    normal.setLength(kBloatSize);
    SkScalar cross = (pts[1] - pts[0]).cross(pts[2] - pts[0]);
    if (cross < 0) {
        normal = -normal;
    }
    PLSVertex quad[kQuadNumVertices];
    quad[0].fPos = pts[0] + normal;
    quad[1].fPos = pts[0] - normal;
    quad[2].fPos = pts[1] - normal;
    quad[3].fPos = pts[2] - normal;
    quad[4].fPos = pts[2] + normal;
    for (int i = 0; i < kQuadNumVertices; i++) {
        quad[i].fWinding = cross < 0 ? 1 : -1;
        if (cross > 0.0) {
            quad[i].fVert2 = pts[0];
            quad[i].fVert3 = pts[2];
        }
        else {
            quad[i].fVert2 = pts[2];
            quad[i].fVert3 = pts[0];
        }
    }
    GrPathUtils::QuadUVMatrix DevToUV(pts);
    DevToUV.apply<kQuadNumVertices, sizeof(PLSVertex), sizeof(SkPoint)>(quad);
    for (int i = 2; i < kQuadNumVertices; i++) {
        vertices.push_back(quad[0]);
        vertices.push_back(quad[i - 1]);
        vertices.push_back(quad[i]);
    }
}

/* Used by bloat_tri; outsets a single point. */
static bool outset(SkPoint* p1, SkPoint line1, SkPoint line2) {
    // rotate the two line vectors 90 degrees to form the normals, and compute
    // the dot product of the normals
    SkScalar dotProd = line1.fY * line2.fY + line1.fX * line2.fX;
    SkScalar lengthSq = 1.0f / ((1.0f - dotProd) / 2.0f);
    if (lengthSq > kBloatLimit) {
        return false;
    }
    SkPoint bisector = line1 + line2;
    bisector.setLength(SkScalarSqrt(lengthSq) * kBloatSize);
    *p1 += bisector;
    return true;
}

/* Bloats a triangle so as to create a border kBloatSize pixels wide all around it. */
static bool bloat_tri(SkPoint pts[3]) {
    SkPoint line1 = pts[0] - pts[1];
    line1.normalize();
    SkPoint line2 = pts[0] - pts[2];
    line2.normalize();
    SkPoint line3 = pts[1] - pts[2];
    line3.normalize();

    SkPoint result[3];
    result[0] = pts[0];
    if (!outset(&result[0], line1, line2)) {
        return false;
    }
    result[1] = pts[1];
    if (!outset(&result[1], -line1, line3)) {
        return false;
    }
    result[2] = pts[2];
    if (!outset(&result[2], -line3, -line2)) {
        return false;
    }
    pts[0] = result[0];
    pts[1] = result[1];
    pts[2] = result[2];
    return true;
}

static bool get_geometry(const SkPath& path, const SkMatrix& m, PLSVertices& triVertices,
                         PLSVertices& quadVertices, GrResourceProvider* resourceProvider,
                         SkRect bounds) {
    SkScalar screenSpaceTol = GrPathUtils::kDefaultTolerance;
    SkScalar tol = GrPathUtils::scaleToleranceToSrc(screenSpaceTol, m, bounds);
    int contourCnt;
    int maxPts = GrPathUtils::worstCasePointCount(path, &contourCnt, tol);
    if (maxPts <= 0) {
        return 0;
    }
    SkPath linesOnlyPath;
    linesOnlyPath.setFillType(path.getFillType());
    SkSTArray<15, SkPoint, true> quadPoints;
    SkPath::Iter iter(path, true);
    bool done = false;
    while (!done) {
        SkPoint pts[4];
        SkPath::Verb verb = iter.next(pts);
        switch (verb) {
            case SkPath::kMove_Verb:
                SkASSERT(quadPoints.count() % 3 == 0);
                for (int i = 0; i < quadPoints.count(); i += 3) {
                    add_quad(&quadPoints[i], quadVertices);
                }
                quadPoints.reset();
                m.mapPoints(&pts[0], 1);
                linesOnlyPath.moveTo(pts[0]);
                break;
            case SkPath::kLine_Verb:
                m.mapPoints(&pts[1], 1);
                linesOnlyPath.lineTo(pts[1]);
                break;
            case SkPath::kQuad_Verb:
                m.mapPoints(pts, 3);
                linesOnlyPath.lineTo(pts[2]);
                quadPoints.push_back(pts[0]);
                quadPoints.push_back(pts[1]);
                quadPoints.push_back(pts[2]);
                break;
            case SkPath::kCubic_Verb: {
                m.mapPoints(pts, 4);
                SkSTArray<15, SkPoint, true> quads;
                GrPathUtils::convertCubicToQuads(pts, kCubicTolerance, &quads);
                int count = quads.count();
                for (int q = 0; q < count; q += 3) {
                    linesOnlyPath.lineTo(quads[q + 2]);
                    quadPoints.push_back(quads[q]);
                    quadPoints.push_back(quads[q + 1]);
                    quadPoints.push_back(quads[q + 2]);
                }
                break;
            }
            case SkPath::kConic_Verb: {
                m.mapPoints(pts, 3);
                SkScalar weight = iter.conicWeight();
                SkAutoConicToQuads converter;
                const SkPoint* quads = converter.computeQuads(pts, weight, kConicTolerance);
                int count = converter.countQuads();
                for (int i = 0; i < count; ++i) {
                    linesOnlyPath.lineTo(quads[2 * i + 2]);
                    quadPoints.push_back(quads[2 * i]);
                    quadPoints.push_back(quads[2 * i + 1]);
                    quadPoints.push_back(quads[2 * i + 2]);
                }
                break;
            }
            case SkPath::kClose_Verb:
                linesOnlyPath.close();
                break;
            case SkPath::kDone_Verb:
                done = true;
                break;
            default: SkASSERT(false);
        }
    }
    SkASSERT(quadPoints.count() % 3 == 0);
    for (int i = 0; i < quadPoints.count(); i += 3) {
        add_quad(&quadPoints[i], quadVertices);
    }

    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey key;
    GrUniqueKey::Builder builder(&key, kDomain, 2);
    builder[0] = path.getGenerationID();
    builder[1] = path.getFillType();
    builder.finish();
    GrTessellator::WindingVertex* windingVertices;
    int triVertexCount = GrTessellator::PathToVertices(linesOnlyPath, 0, bounds, &windingVertices);
    if (triVertexCount > 0) {
        for (int i = 0; i < triVertexCount; i += 3) {
            SkPoint p1 = windingVertices[i].fPos;
            SkPoint p2 = windingVertices[i + 1].fPos;
            SkPoint p3 = windingVertices[i + 2].fPos;
            int winding = windingVertices[i].fWinding;
            SkASSERT(windingVertices[i + 1].fWinding == winding);
            SkASSERT(windingVertices[i + 2].fWinding == winding);
            SkScalar cross = (p2 - p1).cross(p3 - p1);
            SkPoint bloated[3] = { p1, p2, p3 };
            if (cross < 0.0f) {
                SkTSwap(p1, p3);
            }
            if (bloat_tri(bloated)) {
                triVertices.push_back({ bloated[0], p1, p2, p3, winding });
                triVertices.push_back({ bloated[1], p1, p2, p3, winding });
                triVertices.push_back({ bloated[2], p1, p2, p3, winding });
            }
            else {
                SkScalar minX = SkTMin(p1.fX, SkTMin(p2.fX, p3.fX)) - 1.0f;
                SkScalar minY = SkTMin(p1.fY, SkTMin(p2.fY, p3.fY)) - 1.0f;
                SkScalar maxX = SkTMax(p1.fX, SkTMax(p2.fX, p3.fX)) + 1.0f;
                SkScalar maxY = SkTMax(p1.fY, SkTMax(p2.fY, p3.fY)) + 1.0f;
                triVertices.push_back({ { minX, minY }, p1, p2, p3, winding });
                triVertices.push_back({ { maxX, minY }, p1, p2, p3, winding });
                triVertices.push_back({ { minX, maxY }, p1, p2, p3, winding });
                triVertices.push_back({ { maxX, minY }, p1, p2, p3, winding });
                triVertices.push_back({ { maxX, maxY }, p1, p2, p3, winding });
                triVertices.push_back({ { minX, maxY }, p1, p2, p3, winding });
            }
        }
        delete[] windingVertices;
    }
    return triVertexCount > 0 || quadVertices.count() > 0;
}

class PLSAATriangleEffect : public GrPLSGeometryProcessor {
public:

    static GrPLSGeometryProcessor* Create(const SkMatrix& localMatrix,
                                          bool usesLocalCoords) {
        return new PLSAATriangleEffect(localMatrix, usesLocalCoords);
    }

    virtual ~PLSAATriangleEffect() {}

    const char* name() const override { return "PLSAATriangle"; }

    const Attribute* inPosition() const { return fInPosition; }
    const Attribute* inVertex1() const { return fInVertex1; }
    const Attribute* inVertex2() const { return fInVertex2; }
    const Attribute* inVertex3() const { return fInVertex3; }
    const Attribute* inWindings() const { return fInWindings; }
    const SkMatrix& localMatrix() const { return fLocalMatrix; }
    bool usesLocalCoords() const { return fUsesLocalCoords; }

    class GLSLProcessor : public GrGLSLGeometryProcessor {
    public:
        GLSLProcessor(const GrGeometryProcessor&) {}

        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
            const PLSAATriangleEffect& te = args.fGP.cast<PLSAATriangleEffect>();
            GrGLSLVertexBuilder* vsBuilder = args.fVertBuilder;
            GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            varyingHandler->emitAttributes(te);

            this->setupPosition(vsBuilder, gpArgs, te.inPosition()->fName);

            GrGLSLVertToFrag v1(kVec2f_GrSLType);
            varyingHandler->addVarying("Vertex1", &v1, kHigh_GrSLPrecision);
            vsBuilder->codeAppendf("%s = vec2(%s.x, %s.y);",
                                   v1.vsOut(),
                                   te.inVertex1()->fName,
                                   te.inVertex1()->fName);

            GrGLSLVertToFrag v2(kVec2f_GrSLType);
            varyingHandler->addVarying("Vertex2", &v2, kHigh_GrSLPrecision);
            vsBuilder->codeAppendf("%s = vec2(%s.x, %s.y);",
                                   v2.vsOut(),
                                   te.inVertex2()->fName,
                                   te.inVertex2()->fName);

            GrGLSLVertToFrag v3(kVec2f_GrSLType);
            varyingHandler->addVarying("Vertex3", &v3, kHigh_GrSLPrecision);
            vsBuilder->codeAppendf("%s = vec2(%s.x, %s.y);",
                                   v3.vsOut(),
                                   te.inVertex3()->fName,
                                   te.inVertex3()->fName);

            GrGLSLVertToFrag delta1(kVec2f_GrSLType);
            varyingHandler->addVarying("delta1", &delta1, kHigh_GrSLPrecision);
            vsBuilder->codeAppendf("%s = vec2(%s.x - %s.x, %s.y - %s.y) * 0.5;",
                                   delta1.vsOut(), v1.vsOut(), v2.vsOut(), v2.vsOut(), v1.vsOut());

            GrGLSLVertToFrag delta2(kVec2f_GrSLType);
            varyingHandler->addVarying("delta2", &delta2, kHigh_GrSLPrecision);
            vsBuilder->codeAppendf("%s = vec2(%s.x - %s.x, %s.y - %s.y) * 0.5;",
                                   delta2.vsOut(), v2.vsOut(), v3.vsOut(), v3.vsOut(), v2.vsOut());

            GrGLSLVertToFrag delta3(kVec2f_GrSLType);
            varyingHandler->addVarying("delta3", &delta3, kHigh_GrSLPrecision);
            vsBuilder->codeAppendf("%s = vec2(%s.x - %s.x, %s.y - %s.y) * 0.5;",
                                   delta3.vsOut(), v3.vsOut(), v1.vsOut(), v1.vsOut(), v3.vsOut());

            GrGLSLVertToFrag windings(kInt_GrSLType);
            varyingHandler->addFlatVarying("windings", &windings, kLow_GrSLPrecision);
            vsBuilder->codeAppendf("%s = %s;",
                                   windings.vsOut(), te.inWindings()->fName);

            // emit transforms
            this->emitTransforms(vsBuilder, varyingHandler, uniformHandler, gpArgs->fPositionVar,
                                 te.inPosition()->fName, te.localMatrix(), args.fTransformsIn,
                                 args.fTransformsOut);

            GrGLSLPPFragmentBuilder* fsBuilder = args.fFragBuilder;
            SkAssertResult(fsBuilder->enableFeature(
                           GrGLSLFragmentShaderBuilder::kPixelLocalStorage_GLSLFeature));
            SkAssertResult(fsBuilder->enableFeature(
                    GrGLSLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));
            fsBuilder->declAppendf(GR_GL_PLS_PATH_DATA_DECL);
            // Compute four subsamples, each shifted a quarter pixel along x and y from
            // gl_FragCoord. The oriented box positioning of the subsamples is of course not
            // optimal, but it greatly simplifies the math and this simplification is necessary for
            // performance reasons.
            fsBuilder->codeAppendf("highp vec2 firstSample = %s.xy - vec2(0.25);",
                                   fsBuilder->fragmentPosition());
            fsBuilder->codeAppendf("highp vec2 delta1 = %s;", delta1.fsIn());
            fsBuilder->codeAppendf("highp vec2 delta2 = %s;", delta2.fsIn());
            fsBuilder->codeAppendf("highp vec2 delta3 = %s;", delta3.fsIn());
            // Check whether first sample is inside the triangle by computing three dot products. If
            // all are < 0, we're inside. The first vector in each case is half of what it is
            // "supposed" to be, because we re-use them later as adjustment factors for which half
            // is the correct value, so we multiply the dots by two to compensate.
            fsBuilder->codeAppendf("highp float d1 = dot(delta1, (firstSample - %s).yx) * 2.0;",
                                   v1.fsIn());
            fsBuilder->codeAppendf("highp float d2 = dot(delta2, (firstSample - %s).yx) * 2.0;",
                                   v2.fsIn());
            fsBuilder->codeAppendf("highp float d3 = dot(delta3, (firstSample - %s).yx) * 2.0;",
                                   v3.fsIn());
            fsBuilder->codeAppend("highp float dmax = max(d1, max(d2, d3));");
            fsBuilder->codeAppendf("pls.windings[0] += (dmax <= 0.0) ? %s : 0;", windings.fsIn());
            // for subsequent samples, we don't recalculate the entire dot product -- just adjust it
            // to the value it would have if we did recompute it.
            fsBuilder->codeAppend("d1 += delta1.x;");
            fsBuilder->codeAppend("d2 += delta2.x;");
            fsBuilder->codeAppend("d3 += delta3.x;");
            fsBuilder->codeAppend("dmax = max(d1, max(d2, d3));");
            fsBuilder->codeAppendf("pls.windings[1] += (dmax <= 0.0) ? %s : 0;", windings.fsIn());
            fsBuilder->codeAppend("d1 += delta1.y;");
            fsBuilder->codeAppend("d2 += delta2.y;");
            fsBuilder->codeAppend("d3 += delta3.y;");
            fsBuilder->codeAppend("dmax = max(d1, max(d2, d3));");
            fsBuilder->codeAppendf("pls.windings[2] += (dmax <= 0.0) ? %s : 0;", windings.fsIn());
            fsBuilder->codeAppend("d1 -= delta1.x;");
            fsBuilder->codeAppend("d2 -= delta2.x;");
            fsBuilder->codeAppend("d3 -= delta3.x;");
            fsBuilder->codeAppend("dmax = max(d1, max(d2, d3));");
            fsBuilder->codeAppendf("pls.windings[3] += (dmax <= 0.0) ? %s : 0;", windings.fsIn());
        }

        static inline void GenKey(const GrGeometryProcessor& gp,
                                  const GrGLSLCaps&,
                                  GrProcessorKeyBuilder* b) {
            const PLSAATriangleEffect& te = gp.cast<PLSAATriangleEffect>();
            uint32_t key = 0;
            key |= te.localMatrix().hasPerspective() ? 0x1 : 0x0;
            b->add32(key);
        }

        virtual void setData(const GrGLSLProgramDataManager& pdman,
                             const GrPrimitiveProcessor& gp) override {
        }

        void setTransformData(const GrPrimitiveProcessor& primProc,
                              const GrGLSLProgramDataManager& pdman,
                              int index,
                              const SkTArray<const GrCoordTransform*, true>& transforms) override {
            this->setTransformDataHelper<PLSAATriangleEffect>(primProc, pdman, index, transforms);
        }

    private:
        typedef GrGLSLGeometryProcessor INHERITED;
    };

    virtual void getGLSLProcessorKey(const GrGLSLCaps& caps,
                                   GrProcessorKeyBuilder* b) const override {
        GLSLProcessor::GenKey(*this, caps, b);
    }

    virtual GrGLSLPrimitiveProcessor* createGLSLInstance(const GrGLSLCaps&) const override {
        return new GLSLProcessor(*this);
    }

private:
    PLSAATriangleEffect(const SkMatrix& localMatrix, bool usesLocalCoords)
        : fLocalMatrix(localMatrix)
        , fUsesLocalCoords(usesLocalCoords) {
        this->initClassID<PLSAATriangleEffect>();
        fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType,
                                                       kHigh_GrSLPrecision));
        fInVertex1 = &this->addVertexAttrib(Attribute("inVertex1", kVec2f_GrVertexAttribType,
                                                      kHigh_GrSLPrecision));
        fInVertex2 = &this->addVertexAttrib(Attribute("inVertex2", kVec2f_GrVertexAttribType,
                                                      kHigh_GrSLPrecision));
        fInVertex3 = &this->addVertexAttrib(Attribute("inVertex3", kVec2f_GrVertexAttribType,
                                                      kHigh_GrSLPrecision));
        fInWindings = &this->addVertexAttrib(Attribute("inWindings", kInt_GrVertexAttribType,
                                                       kLow_GrSLPrecision));
        this->setWillReadFragmentPosition();
    }

    const Attribute* fInPosition;
    const Attribute* fInVertex1;
    const Attribute* fInVertex2;
    const Attribute* fInVertex3;
    const Attribute* fInWindings;
    SkMatrix         fLocalMatrix;
    bool             fUsesLocalCoords;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Quadratic specified by 0=u^2-v canonical coords. u and v are the first
 * two components of the vertex attribute. Coverage is based on signed
 * distance with negative being inside, positive outside. The edge is specified in
 * window space (y-down). If either the third or fourth component of the interpolated
 * vertex coord is > 0 then the pixel is considered outside the edge. This is used to
 * attempt to trim to a portion of the infinite quad.
 * Requires shader derivative instruction support.
 */

class PLSQuadEdgeEffect : public GrPLSGeometryProcessor {
public:

    static GrPLSGeometryProcessor* Create(const SkMatrix& localMatrix,
                                          bool usesLocalCoords) {
        return new PLSQuadEdgeEffect(localMatrix, usesLocalCoords);
    }

    virtual ~PLSQuadEdgeEffect() {}

    const char* name() const override { return "PLSQuadEdge"; }

    const Attribute* inPosition() const { return fInPosition; }
    const Attribute* inUV() const { return fInUV; }
    const Attribute* inEndpoint1() const { return fInEndpoint1; }
    const Attribute* inEndpoint2() const { return fInEndpoint2; }
    const Attribute* inWindings() const { return fInWindings; }
    const SkMatrix& localMatrix() const { return fLocalMatrix; }
    bool usesLocalCoords() const { return fUsesLocalCoords; }

    class GLSLProcessor : public GrGLSLGeometryProcessor {
    public:
        GLSLProcessor(const GrGeometryProcessor&) {}

        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
            const PLSQuadEdgeEffect& qe = args.fGP.cast<PLSQuadEdgeEffect>();
            GrGLSLVertexBuilder* vsBuilder = args.fVertBuilder;
            GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            // emit attributes
            varyingHandler->emitAttributes(qe);

            GrGLSLVertToFrag uv(kVec2f_GrSLType);
            varyingHandler->addVarying("uv", &uv, kHigh_GrSLPrecision);
            vsBuilder->codeAppendf("%s = %s;", uv.vsOut(), qe.inUV()->fName);

            GrGLSLVertToFrag ep1(kVec2f_GrSLType);
            varyingHandler->addVarying("endpoint1", &ep1, kHigh_GrSLPrecision);
            vsBuilder->codeAppendf("%s = vec2(%s.x, %s.y);", ep1.vsOut(),
                                  qe.inEndpoint1()->fName, qe.inEndpoint1()->fName);

            GrGLSLVertToFrag ep2(kVec2f_GrSLType);
            varyingHandler->addVarying("endpoint2", &ep2, kHigh_GrSLPrecision);
            vsBuilder->codeAppendf("%s = vec2(%s.x, %s.y);", ep2.vsOut(),
                                  qe.inEndpoint2()->fName, qe.inEndpoint2()->fName);

            GrGLSLVertToFrag delta(kVec2f_GrSLType);
            varyingHandler->addVarying("delta", &delta, kHigh_GrSLPrecision);
            vsBuilder->codeAppendf("%s = vec2(%s.x - %s.x, %s.y - %s.y) * 0.5;",
                                   delta.vsOut(), ep1.vsOut(), ep2.vsOut(), ep2.vsOut(),
                                   ep1.vsOut());

            GrGLSLVertToFrag windings(kInt_GrSLType);
            varyingHandler->addFlatVarying("windings", &windings, kLow_GrSLPrecision);
            vsBuilder->codeAppendf("%s = %s;",
                                   windings.vsOut(), qe.inWindings()->fName);

            // Setup position
            this->setupPosition(vsBuilder, gpArgs, qe.inPosition()->fName);

            // emit transforms
            this->emitTransforms(vsBuilder, varyingHandler, uniformHandler, gpArgs->fPositionVar,
                                 qe.inPosition()->fName, qe.localMatrix(), args.fTransformsIn,
                                 args.fTransformsOut);

            GrGLSLPPFragmentBuilder* fsBuilder = args.fFragBuilder;
            SkAssertResult(fsBuilder->enableFeature(
                           GrGLSLFragmentShaderBuilder::kPixelLocalStorage_GLSLFeature));
            SkAssertResult(fsBuilder->enableFeature(
                    GrGLSLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));
            static const int QUAD_ARGS = 2;
            GrGLSLShaderVar inQuadArgs[QUAD_ARGS] = {
                GrGLSLShaderVar("dot", kFloat_GrSLType, 0, kHigh_GrSLPrecision),
                GrGLSLShaderVar("uv", kVec2f_GrSLType, 0, kHigh_GrSLPrecision)
            };
            SkString inQuadName;

            const char* inQuadCode = "if (uv.x * uv.x <= uv.y) {"
                                     "return dot >= 0.0;"
                                     "} else {"
                                     "return false;"
                                     "}";
            fsBuilder->emitFunction(kBool_GrSLType, "in_quad", QUAD_ARGS, inQuadArgs, inQuadCode,
                                    &inQuadName);
            fsBuilder->declAppendf(GR_GL_PLS_PATH_DATA_DECL);
            // keep the derivative instructions outside the conditional
            fsBuilder->codeAppendf("highp vec2 uvdX = dFdx(%s);", uv.fsIn());
            fsBuilder->codeAppendf("highp vec2 uvdY = dFdy(%s);", uv.fsIn());
            fsBuilder->codeAppend("highp vec2 uvIncX = uvdX * 0.45 + uvdY * -0.1;");
            fsBuilder->codeAppend("highp vec2 uvIncY = uvdX * 0.1 + uvdY * 0.55;");
            fsBuilder->codeAppendf("highp vec2 uv = %s.xy - uvdX * 0.35 - uvdY * 0.25;",
                                   uv.fsIn());
            fsBuilder->codeAppendf("highp vec2 firstSample = %s.xy - vec2(0.25);",
                                   fsBuilder->fragmentPosition());
            fsBuilder->codeAppendf("highp float d = dot(%s, (firstSample - %s).yx) * 2.0;",
                                   delta.fsIn(), ep1.fsIn());
            fsBuilder->codeAppendf("pls.windings[0] += %s(d, uv) ? %s : 0;", inQuadName.c_str(),
                                   windings.fsIn());
            fsBuilder->codeAppend("uv += uvIncX;");
            fsBuilder->codeAppendf("d += %s.x;", delta.fsIn());
            fsBuilder->codeAppendf("pls.windings[1] += %s(d, uv) ? %s : 0;", inQuadName.c_str(),
                                   windings.fsIn());
            fsBuilder->codeAppend("uv += uvIncY;");
            fsBuilder->codeAppendf("d += %s.y;", delta.fsIn());
            fsBuilder->codeAppendf("pls.windings[2] += %s(d, uv) ? %s : 0;", inQuadName.c_str(),
                                   windings.fsIn());
            fsBuilder->codeAppend("uv -= uvIncX;");
            fsBuilder->codeAppendf("d -= %s.x;", delta.fsIn());
            fsBuilder->codeAppendf("pls.windings[3] += %s(d, uv) ? %s : 0;", inQuadName.c_str(),
                                   windings.fsIn());
        }

        static inline void GenKey(const GrGeometryProcessor& gp,
                                  const GrGLSLCaps&,
                                  GrProcessorKeyBuilder* b) {
            const PLSQuadEdgeEffect& qee = gp.cast<PLSQuadEdgeEffect>();
            uint32_t key = 0;
            key |= qee.usesLocalCoords() && qee.localMatrix().hasPerspective() ? 0x1 : 0x0;
            b->add32(key);
        }

        virtual void setData(const GrGLSLProgramDataManager& pdman,
                             const GrPrimitiveProcessor& gp) override {
        }

        void setTransformData(const GrPrimitiveProcessor& primProc,
                              const GrGLSLProgramDataManager& pdman,
                              int index,
                              const SkTArray<const GrCoordTransform*, true>& transforms) override {
            this->setTransformDataHelper<PLSQuadEdgeEffect>(primProc, pdman, index, transforms);
        }

    private:
        typedef GrGLSLGeometryProcessor INHERITED;
    };

    virtual void getGLSLProcessorKey(const GrGLSLCaps& caps,
                                   GrProcessorKeyBuilder* b) const override {
        GLSLProcessor::GenKey(*this, caps, b);
    }

    virtual GrGLSLPrimitiveProcessor* createGLSLInstance(const GrGLSLCaps&) const override {
        return new GLSLProcessor(*this);
    }

private:
    PLSQuadEdgeEffect(const SkMatrix& localMatrix, bool usesLocalCoords)
        : fLocalMatrix(localMatrix)
        , fUsesLocalCoords(usesLocalCoords) {
        this->initClassID<PLSQuadEdgeEffect>();
        fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType,
                                                       kHigh_GrSLPrecision));
        fInUV = &this->addVertexAttrib(Attribute("inUV", kVec2f_GrVertexAttribType,
                                                 kHigh_GrSLPrecision));
        fInEndpoint1 = &this->addVertexAttrib(Attribute("inEndpoint1", kVec2f_GrVertexAttribType,
                                                        kHigh_GrSLPrecision));
        fInEndpoint2 = &this->addVertexAttrib(Attribute("inEndpoint2", kVec2f_GrVertexAttribType,
                                                        kHigh_GrSLPrecision));
        fInWindings  = &this->addVertexAttrib(Attribute("inWindings", kInt_GrVertexAttribType,
                                                        kLow_GrSLPrecision));
        this->setWillReadFragmentPosition();
    }

    const Attribute* fInPosition;
    const Attribute* fInUV;
    const Attribute* fInEndpoint1;
    const Attribute* fInEndpoint2;
    const Attribute* fInWindings;
    SkMatrix         fLocalMatrix;
    bool             fUsesLocalCoords;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

class PLSFinishEffect : public GrGeometryProcessor {
public:

    static GrGeometryProcessor* Create(GrColor color, bool useEvenOdd, const SkMatrix& localMatrix,
                                       bool usesLocalCoords) {
        return new PLSFinishEffect(color, useEvenOdd, localMatrix, usesLocalCoords);
    }

    virtual ~PLSFinishEffect() {}

    const char* name() const override { return "PLSFinish"; }

    const Attribute* inPosition() const { return fInPosition; }
    GrColor color() const { return fColor; }
    bool colorIgnored() const { return GrColor_ILLEGAL == fColor; }
    const SkMatrix& localMatrix() const { return fLocalMatrix; }
    bool usesLocalCoords() const { return fUsesLocalCoords; }

    GrPixelLocalStorageState getPixelLocalStorageState() const override {
        return GrPixelLocalStorageState::kFinish_GrPixelLocalStorageState;
    }

    const char* getDestColorOverride() const override {
        return GR_GL_PLS_DSTCOLOR_NAME;
    }

    class GLSLProcessor : public GrGLSLGeometryProcessor {
    public:
        GLSLProcessor(const GrGeometryProcessor&) {}

        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
            const PLSFinishEffect& fe = args.fGP.cast<PLSFinishEffect>();
            GrGLSLVertexBuilder* vsBuilder = args.fVertBuilder;
            GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            fUseEvenOdd = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                    kFloat_GrSLType, kLow_GrSLPrecision,
                                                    "useEvenOdd");
            const char* useEvenOdd = uniformHandler->getUniformCStr(fUseEvenOdd);

            varyingHandler->emitAttributes(fe);
            this->setupPosition(vsBuilder, gpArgs, fe.inPosition()->fName);
            this->emitTransforms(vsBuilder, varyingHandler, uniformHandler, gpArgs->fPositionVar,
                                 fe.inPosition()->fName, fe.localMatrix(), args.fTransformsIn,
                                 args.fTransformsOut);

            GrGLSLPPFragmentBuilder* fsBuilder = args.fFragBuilder;
            SkAssertResult(fsBuilder->enableFeature(
                           GrGLSLFragmentShaderBuilder::kPixelLocalStorage_GLSLFeature));
            fsBuilder->declAppendf(GR_GL_PLS_PATH_DATA_DECL);
            fsBuilder->codeAppend("float coverage;");
            fsBuilder->codeAppendf("if (%s != 0.0) {", useEvenOdd);
            fsBuilder->codeAppend("coverage = float(abs(pls.windings[0]) % 2) * 0.25;");
            fsBuilder->codeAppend("coverage += float(abs(pls.windings[1]) % 2) * 0.25;");
            fsBuilder->codeAppend("coverage += float(abs(pls.windings[2]) % 2) * 0.25;");
            fsBuilder->codeAppend("coverage += float(abs(pls.windings[3]) % 2) * 0.25;");
            fsBuilder->codeAppend("} else {");
            fsBuilder->codeAppend("coverage = pls.windings[0] != 0 ? 0.25 : 0.0;");
            fsBuilder->codeAppend("coverage += pls.windings[1] != 0 ? 0.25 : 0.0;");
            fsBuilder->codeAppend("coverage += pls.windings[2] != 0 ? 0.25 : 0.0;");
            fsBuilder->codeAppend("coverage += pls.windings[3] != 0 ? 0.25 : 0.0;");
            fsBuilder->codeAppend("}");
            if (!fe.colorIgnored()) {
                this->setupUniformColor(fsBuilder, uniformHandler, args.fOutputColor,
                                        &fColorUniform);
            }
            fsBuilder->codeAppendf("%s = vec4(coverage);", args.fOutputCoverage);
            fsBuilder->codeAppendf("%s = vec4(1.0, 0.0, 1.0, 1.0);", args.fOutputColor);
        }

        static inline void GenKey(const GrGeometryProcessor& gp,
                                  const GrGLSLCaps&,
                                  GrProcessorKeyBuilder* b) {
            const PLSFinishEffect& fe = gp.cast<PLSFinishEffect>();
            uint32_t key = 0;
            key |= fe.usesLocalCoords() && fe.localMatrix().hasPerspective() ? 0x1 : 0x0;
            b->add32(key);
        }

        virtual void setData(const GrGLSLProgramDataManager& pdman,
                             const GrPrimitiveProcessor& gp) override {
            const PLSFinishEffect& fe = gp.cast<PLSFinishEffect>();
            pdman.set1f(fUseEvenOdd, fe.fUseEvenOdd);
            if (fe.color() != fColor && !fe.colorIgnored()) {
                GrGLfloat c[4];
                GrColorToRGBAFloat(fe.color(), c);
                pdman.set4fv(fColorUniform, 1, c);
                fColor = fe.color();
            }
        }

        void setTransformData(const GrPrimitiveProcessor& primProc,
                              const GrGLSLProgramDataManager& pdman,
                              int index,
                              const SkTArray<const GrCoordTransform*, true>& transforms) override {
            this->setTransformDataHelper<PLSFinishEffect>(primProc, pdman, index, transforms);
        }

    private:
        GrColor fColor;
        UniformHandle fColorUniform;
        UniformHandle fUseEvenOdd;

        typedef GrGLSLGeometryProcessor INHERITED;
    };

    virtual void getGLSLProcessorKey(const GrGLSLCaps& caps,
                                   GrProcessorKeyBuilder* b) const override {
        GLSLProcessor::GenKey(*this, caps, b);
    }

    virtual GrGLSLPrimitiveProcessor* createGLSLInstance(const GrGLSLCaps&) const override {
        return new GLSLProcessor(*this);
    }

private:
    PLSFinishEffect(GrColor color, bool useEvenOdd, const SkMatrix& localMatrix,
                    bool usesLocalCoords)
        : fColor(color)
        , fUseEvenOdd(useEvenOdd)
        , fLocalMatrix(localMatrix)
        , fUsesLocalCoords(usesLocalCoords) {
        this->initClassID<PLSFinishEffect>();
        fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType,
                                                       kHigh_GrSLPrecision));
    }

    const Attribute* fInPosition;
    GrColor          fColor;
    bool             fUseEvenOdd;
    SkMatrix         fLocalMatrix;
    bool             fUsesLocalCoords;

    typedef GrGeometryProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

bool GrPLSPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    // We have support for even-odd rendering, but are having some troublesome
    // seams. Disable in the presence of even-odd for now.
    SkPath path;
    args.fShape->asPath(&path);
    return args.fShaderCaps->shaderDerivativeSupport() && args.fAntiAlias &&
            args.fShape->style().isSimpleFill() && !path.isInverseFillType() &&
            path.getFillType() == SkPath::FillType::kWinding_FillType;
}

class PLSPathBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID
    struct Geometry {
        GrColor fColor;
        SkMatrix fViewMatrix;
        SkPath fPath;
    };

    static GrDrawBatch* Create(const Geometry& geometry) {
        return new PLSPathBatch(geometry);
    }

    const char* name() const override { return "PLSBatch"; }

    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        // When this is called on a batch, there is only one geometry bundle
        color->setKnownFourComponents(fGeoData[0].fColor);
        coverage->setUnknownSingleComponent();
        overrides->fUsePLSDstRead = true;
    }

    void initBatchTracker(const GrXPOverridesForBatch& overrides) override {
        // Handle any color overrides
        if (!overrides.readsColor()) {
            fGeoData[0].fColor = GrColor_ILLEGAL;
        }
        overrides.getOverrideColorIfSet(&fGeoData[0].fColor);

        // setup batch properties
        fBatch.fColorIgnored = !overrides.readsColor();
        fBatch.fColor = fGeoData[0].fColor;
        fBatch.fUsesLocalCoords = overrides.readsLocalCoords();
        fBatch.fCoverageIgnored = !overrides.readsCoverage();
        fBatch.fCanTweakAlphaForCoverage = overrides.canTweakAlphaForCoverage();
    }

    void onPrepareDraws(Target* target) const override {
        int instanceCount = fGeoData.count();

        SkMatrix invert;
        if (this->usesLocalCoords() && !this->viewMatrix().invert(&invert)) {
            SkDebugf("Could not invert viewmatrix\n");
            return;
        }

        // Setup GrGeometryProcessors
        SkAutoTUnref<GrPLSGeometryProcessor> triangleProcessor(
                PLSAATriangleEffect::Create(invert, this->usesLocalCoords()));
        SkAutoTUnref<GrPLSGeometryProcessor> quadProcessor(
                PLSQuadEdgeEffect::Create(invert, this->usesLocalCoords()));

        GrResourceProvider* rp = target->resourceProvider();
        for (int i = 0; i < instanceCount; ++i) {
            const Geometry& args = fGeoData[i];
            SkRect bounds = args.fPath.getBounds();
            args.fViewMatrix.mapRect(&bounds);
            bounds.fLeft = SkScalarFloorToScalar(bounds.fLeft);
            bounds.fTop = SkScalarFloorToScalar(bounds.fTop);
            bounds.fRight = SkScalarCeilToScalar(bounds.fRight);
            bounds.fBottom = SkScalarCeilToScalar(bounds.fBottom);
            triangleProcessor->setBounds(bounds);
            quadProcessor->setBounds(bounds);

            // We use the fact that SkPath::transform path does subdivision based on
            // perspective. Otherwise, we apply the view matrix when copying to the
            // segment representation.
            const SkMatrix* viewMatrix = &args.fViewMatrix;

            // We avoid initializing the path unless we have to
            const SkPath* pathPtr = &args.fPath;
            SkTLazy<SkPath> tmpPath;
            if (viewMatrix->hasPerspective()) {
                SkPath* tmpPathPtr = tmpPath.init(*pathPtr);
                tmpPathPtr->setIsVolatile(true);
                tmpPathPtr->transform(*viewMatrix);
                viewMatrix = &SkMatrix::I();
                pathPtr = tmpPathPtr;
            }

            GrMesh mesh;

            PLSVertices triVertices;
            PLSVertices quadVertices;
            if (!get_geometry(*pathPtr, *viewMatrix, triVertices, quadVertices, rp, bounds)) {
                continue;
            }

            if (triVertices.count()) {
                const GrBuffer* triVertexBuffer;
                int firstTriVertex;
                size_t triStride = triangleProcessor->getVertexStride();
                PLSVertex* triVerts = reinterpret_cast<PLSVertex*>(target->makeVertexSpace(
                        triStride, triVertices.count(), &triVertexBuffer, &firstTriVertex));
                if (!triVerts) {
                    SkDebugf("Could not allocate vertices\n");
                    return;
                }
                for (int i = 0; i < triVertices.count(); ++i) {
                    triVerts[i] = triVertices[i];
                }
                mesh.init(kTriangles_GrPrimitiveType, triVertexBuffer, firstTriVertex,
                          triVertices.count());
                target->draw(triangleProcessor, mesh);
            }

            if (quadVertices.count()) {
                const GrBuffer* quadVertexBuffer;
                int firstQuadVertex;
                size_t quadStride = quadProcessor->getVertexStride();
                PLSVertex* quadVerts = reinterpret_cast<PLSVertex*>(target->makeVertexSpace(
                        quadStride, quadVertices.count(), &quadVertexBuffer, &firstQuadVertex));
                if (!quadVerts) {
                    SkDebugf("Could not allocate vertices\n");
                    return;
                }
                for (int i = 0; i < quadVertices.count(); ++i) {
                    quadVerts[i] = quadVertices[i];
                }
                mesh.init(kTriangles_GrPrimitiveType, quadVertexBuffer, firstQuadVertex,
                          quadVertices.count());
                target->draw(quadProcessor, mesh);
            }

            SkAutoTUnref<GrGeometryProcessor> finishProcessor(
                    PLSFinishEffect::Create(this->color(),
                                            pathPtr->getFillType() ==
                                                                SkPath::FillType::kEvenOdd_FillType,
                                            invert,
                                            this->usesLocalCoords()));
            const GrBuffer* rectVertexBuffer;
            size_t finishStride = finishProcessor->getVertexStride();
            int firstRectVertex;
            static const int kRectVertexCount = 6;
            SkPoint* rectVerts = reinterpret_cast<SkPoint*>(target->makeVertexSpace(
                    finishStride, kRectVertexCount, &rectVertexBuffer, &firstRectVertex));
            if (!rectVerts) {
                SkDebugf("Could not allocate vertices\n");
                return;
            }
            rectVerts[0] = { bounds.fLeft, bounds.fTop };
            rectVerts[1] = { bounds.fLeft, bounds.fBottom };
            rectVerts[2] = { bounds.fRight, bounds.fBottom };
            rectVerts[3] = { bounds.fLeft, bounds.fTop };
            rectVerts[4] = { bounds.fRight, bounds.fTop };
            rectVerts[5] = { bounds.fRight, bounds.fBottom };

            mesh.init(kTriangles_GrPrimitiveType, rectVertexBuffer, firstRectVertex,
                      kRectVertexCount);
            target->draw(finishProcessor, mesh);
        }
    }

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

private:
    PLSPathBatch(const Geometry& geometry) : INHERITED(ClassID()) {
        fGeoData.push_back(geometry);

        // compute bounds
        fBounds = geometry.fPath.getBounds();
        geometry.fViewMatrix.mapRect(&fBounds);
    }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override {
        return false;
    }

    GrColor color() const { return fBatch.fColor; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }
    bool canTweakAlphaForCoverage() const { return fBatch.fCanTweakAlphaForCoverage; }
    const SkMatrix& viewMatrix() const { return fGeoData[0].fViewMatrix; }
    bool coverageIgnored() const { return fBatch.fCoverageIgnored; }

    struct BatchTracker {
        GrColor fColor;
        bool fUsesLocalCoords;
        bool fColorIgnored;
        bool fCoverageIgnored;
        bool fCanTweakAlphaForCoverage;
    };

    BatchTracker fBatch;
    SkSTArray<1, Geometry, true> fGeoData;

    typedef GrVertexBatch INHERITED;
};

SkDEBUGCODE(bool inPLSDraw = false;)
bool GrPLSPathRenderer::onDrawPath(const DrawPathArgs& args) {
    SkASSERT(!args.fShape->isEmpty())
    SkASSERT(!inPLSDraw);
    SkDEBUGCODE(inPLSDraw = true;)
    PLSPathBatch::Geometry geometry;
    geometry.fColor = args.fColor;
    geometry.fViewMatrix = *args.fViewMatrix;
    args.fShape->asPath(&geometry.fPath);

    SkAutoTUnref<GrDrawBatch> batch(PLSPathBatch::Create(geometry));

    GrPipelineBuilder pipelineBuilder(*args.fPaint, args.fDrawContext->mustUseHWAA(*args.fPaint));
    pipelineBuilder.setUserStencil(args.fUserStencilSettings);

    args.fDrawContext->drawBatch(pipelineBuilder, *args.fClip, batch);

    SkDEBUGCODE(inPLSDraw = false;)
    return true;

}

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_TEST_UTILS

DRAW_BATCH_TEST_DEFINE(PLSPathBatch) {
    PLSPathBatch::Geometry geometry;
    geometry.fColor = GrRandomColor(random);
    geometry.fViewMatrix = GrTest::TestMatrixInvertible(random);
    geometry.fPath = GrTest::TestPathConvex(random);

    return PLSPathBatch::Create(geometry);
}

#endif
