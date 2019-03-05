/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAAConvexPathRenderer.h"
#include "GrCaps.h"
#include "GrDrawOpTest.h"
#include "GrGeometryProcessor.h"
#include "GrPathUtils.h"
#include "GrProcessor.h"
#include "GrRenderTargetContext.h"
#include "GrShape.h"
#include "GrSimpleMeshDrawOpHelper.h"
#include "GrVertexWriter.h"
#include "SkGeometry.h"
#include "SkPathPriv.h"
#include "SkPointPriv.h"
#include "SkString.h"
#include "SkTypes.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"
#include "ops/GrMeshDrawOp.h"

GrAAConvexPathRenderer::GrAAConvexPathRenderer() {
}

struct Segment {
    enum {
        // These enum values are assumed in member functions below.
        kLine = 0,
        kQuad = 1,
    } fType;

    // line uses one pt, quad uses 2 pts
    SkPoint fPts[2];
    // normal to edge ending at each pt
    SkVector fNorms[2];
    // is the corner where the previous segment meets this segment
    // sharp. If so, fMid is a normalized bisector facing outward.
    SkVector fMid;

    int countPoints() {
        GR_STATIC_ASSERT(0 == kLine && 1 == kQuad);
        return fType + 1;
    }
    const SkPoint& endPt() const {
        GR_STATIC_ASSERT(0 == kLine && 1 == kQuad);
        return fPts[fType];
    }
    const SkPoint& endNorm() const {
        GR_STATIC_ASSERT(0 == kLine && 1 == kQuad);
        return fNorms[fType];
    }
};

typedef SkTArray<Segment, true> SegmentArray;

static bool center_of_mass(const SegmentArray& segments, SkPoint* c) {
    SkScalar area = 0;
    SkPoint center = {0, 0};
    int count = segments.count();
    SkPoint p0 = {0, 0};
    if (count > 2) {
        // We translate the polygon so that the first point is at the origin.
        // This avoids some precision issues with small area polygons far away
        // from the origin.
        p0 = segments[0].endPt();
        SkPoint pi;
        SkPoint pj;
        // the first and last iteration of the below loop would compute
        // zeros since the starting / ending point is (0,0). So instead we start
        // at i=1 and make the last iteration i=count-2.
        pj = segments[1].endPt() - p0;
        for (int i = 1; i < count - 1; ++i) {
            pi = pj;
            pj = segments[i + 1].endPt() - p0;

            SkScalar t = SkPoint::CrossProduct(pi, pj);
            area += t;
            center.fX += (pi.fX + pj.fX) * t;
            center.fY += (pi.fY + pj.fY) * t;
        }
    }

    // If the poly has no area then we instead return the average of
    // its points.
    if (SkScalarNearlyZero(area)) {
        SkPoint avg;
        avg.set(0, 0);
        for (int i = 0; i < count; ++i) {
            const SkPoint& pt = segments[i].endPt();
            avg.fX += pt.fX;
            avg.fY += pt.fY;
        }
        SkScalar denom = SK_Scalar1 / count;
        avg.scale(denom);
        *c = avg;
    } else {
        area *= 3;
        area = SkScalarInvert(area);
        center.scale(area);
        // undo the translate of p0 to the origin.
        *c = center + p0;
    }
    return !SkScalarIsNaN(c->fX) && !SkScalarIsNaN(c->fY) && c->isFinite();
}

static bool compute_vectors(SegmentArray* segments,
                            SkPoint* fanPt,
                            SkPathPriv::FirstDirection dir,
                            int* vCount,
                            int* iCount) {
    if (!center_of_mass(*segments, fanPt)) {
        return false;
    }
    int count = segments->count();

    // Make the normals point towards the outside
    SkPointPriv::Side normSide;
    if (dir == SkPathPriv::kCCW_FirstDirection) {
        normSide = SkPointPriv::kRight_Side;
    } else {
        normSide = SkPointPriv::kLeft_Side;
    }

    int64_t vCount64 = 0;
    int64_t iCount64 = 0;
    // compute normals at all points
    for (int a = 0; a < count; ++a) {
        Segment& sega = (*segments)[a];
        int b = (a + 1) % count;
        Segment& segb = (*segments)[b];

        const SkPoint* prevPt = &sega.endPt();
        int n = segb.countPoints();
        for (int p = 0; p < n; ++p) {
            segb.fNorms[p] = segb.fPts[p] - *prevPt;
            segb.fNorms[p].normalize();
            segb.fNorms[p] = SkPointPriv::MakeOrthog(segb.fNorms[p], normSide);
            prevPt = &segb.fPts[p];
        }
        if (Segment::kLine == segb.fType) {
            vCount64 += 5;
            iCount64 += 9;
        } else {
            vCount64 += 6;
            iCount64 += 12;
        }
    }

    // compute mid-vectors where segments meet. TODO: Detect shallow corners
    // and leave out the wedges and close gaps by stitching segments together.
    for (int a = 0; a < count; ++a) {
        const Segment& sega = (*segments)[a];
        int b = (a + 1) % count;
        Segment& segb = (*segments)[b];
        segb.fMid = segb.fNorms[0] + sega.endNorm();
        segb.fMid.normalize();
        // corner wedges
        vCount64 += 4;
        iCount64 += 6;
    }
    if (vCount64 > SK_MaxS32 || iCount64 > SK_MaxS32) {
        return false;
    }
    *vCount = vCount64;
    *iCount = iCount64;
    return true;
}

struct DegenerateTestData {
    DegenerateTestData() { fStage = kInitial; }
    bool isDegenerate() const { return kNonDegenerate != fStage; }
    enum {
        kInitial,
        kPoint,
        kLine,
        kNonDegenerate
    }           fStage;
    SkPoint     fFirstPoint;
    SkVector    fLineNormal;
    SkScalar    fLineC;
};

static const SkScalar kClose = (SK_Scalar1 / 16);
static const SkScalar kCloseSqd = kClose * kClose;

static void update_degenerate_test(DegenerateTestData* data, const SkPoint& pt) {
    switch (data->fStage) {
        case DegenerateTestData::kInitial:
            data->fFirstPoint = pt;
            data->fStage = DegenerateTestData::kPoint;
            break;
        case DegenerateTestData::kPoint:
            if (SkPointPriv::DistanceToSqd(pt, data->fFirstPoint) > kCloseSqd) {
                data->fLineNormal = pt - data->fFirstPoint;
                data->fLineNormal.normalize();
                data->fLineNormal = SkPointPriv::MakeOrthog(data->fLineNormal);
                data->fLineC = -data->fLineNormal.dot(data->fFirstPoint);
                data->fStage = DegenerateTestData::kLine;
            }
            break;
        case DegenerateTestData::kLine:
            if (SkScalarAbs(data->fLineNormal.dot(pt) + data->fLineC) > kClose) {
                data->fStage = DegenerateTestData::kNonDegenerate;
            }
        case DegenerateTestData::kNonDegenerate:
            break;
        default:
            SK_ABORT("Unexpected degenerate test stage.");
    }
}

static inline bool get_direction(const SkPath& path, const SkMatrix& m,
                                 SkPathPriv::FirstDirection* dir) {
    if (!SkPathPriv::CheapComputeFirstDirection(path, dir)) {
        return false;
    }
    // check whether m reverses the orientation
    SkASSERT(!m.hasPerspective());
    SkScalar det2x2 = m.get(SkMatrix::kMScaleX) * m.get(SkMatrix::kMScaleY) -
                      m.get(SkMatrix::kMSkewX)  * m.get(SkMatrix::kMSkewY);
    if (det2x2 < 0) {
        *dir = SkPathPriv::OppositeFirstDirection(*dir);
    }
    return true;
}

static inline void add_line_to_segment(const SkPoint& pt,
                                       SegmentArray* segments) {
    segments->push_back();
    segments->back().fType = Segment::kLine;
    segments->back().fPts[0] = pt;
}

static inline void add_quad_segment(const SkPoint pts[3],
                                    SegmentArray* segments) {
    if (SkPointPriv::DistanceToSqd(pts[0], pts[1]) < kCloseSqd ||
        SkPointPriv::DistanceToSqd(pts[1], pts[2]) < kCloseSqd) {
        if (pts[0] != pts[2]) {
            add_line_to_segment(pts[2], segments);
        }
    } else {
        segments->push_back();
        segments->back().fType = Segment::kQuad;
        segments->back().fPts[0] = pts[1];
        segments->back().fPts[1] = pts[2];
    }
}

static inline void add_cubic_segments(const SkPoint pts[4],
                                      SkPathPriv::FirstDirection dir,
                                      SegmentArray* segments) {
    SkSTArray<15, SkPoint, true> quads;
    GrPathUtils::convertCubicToQuadsConstrainToTangents(pts, SK_Scalar1, dir, &quads);
    int count = quads.count();
    for (int q = 0; q < count; q += 3) {
        add_quad_segment(&quads[q], segments);
    }
}

static bool get_segments(const SkPath& path,
                         const SkMatrix& m,
                         SegmentArray* segments,
                         SkPoint* fanPt,
                         int* vCount,
                         int* iCount) {
    SkPath::Iter iter(path, true);
    // This renderer over-emphasizes very thin path regions. We use the distance
    // to the path from the sample to compute coverage. Every pixel intersected
    // by the path will be hit and the maximum distance is sqrt(2)/2. We don't
    // notice that the sample may be close to a very thin area of the path and
    // thus should be very light. This is particularly egregious for degenerate
    // line paths. We detect paths that are very close to a line (zero area) and
    // draw nothing.
    DegenerateTestData degenerateData;
    SkPathPriv::FirstDirection dir;
    // get_direction can fail for some degenerate paths.
    if (!get_direction(path, m, &dir)) {
        return false;
    }

    for (;;) {
        SkPoint pts[4];
        SkPath::Verb verb = iter.next(pts, true, true);
        switch (verb) {
            case SkPath::kMove_Verb:
                m.mapPoints(pts, 1);
                update_degenerate_test(&degenerateData, pts[0]);
                break;
            case SkPath::kLine_Verb: {
                m.mapPoints(&pts[1], 1);
                update_degenerate_test(&degenerateData, pts[1]);
                add_line_to_segment(pts[1], segments);
                break;
            }
            case SkPath::kQuad_Verb:
                m.mapPoints(pts, 3);
                update_degenerate_test(&degenerateData, pts[1]);
                update_degenerate_test(&degenerateData, pts[2]);
                add_quad_segment(pts, segments);
                break;
            case SkPath::kConic_Verb: {
                m.mapPoints(pts, 3);
                SkScalar weight = iter.conicWeight();
                SkAutoConicToQuads converter;
                const SkPoint* quadPts = converter.computeQuads(pts, weight, 0.25f);
                for (int i = 0; i < converter.countQuads(); ++i) {
                    update_degenerate_test(&degenerateData, quadPts[2*i + 1]);
                    update_degenerate_test(&degenerateData, quadPts[2*i + 2]);
                    add_quad_segment(quadPts + 2*i, segments);
                }
                break;
            }
            case SkPath::kCubic_Verb: {
                m.mapPoints(pts, 4);
                update_degenerate_test(&degenerateData, pts[1]);
                update_degenerate_test(&degenerateData, pts[2]);
                update_degenerate_test(&degenerateData, pts[3]);
                add_cubic_segments(pts, dir, segments);
                break;
            }
            case SkPath::kDone_Verb:
                if (degenerateData.isDegenerate()) {
                    return false;
                } else {
                    return compute_vectors(segments, fanPt, dir, vCount, iCount);
                }
            default:
                break;
        }
    }
}

struct Draw {
    Draw() : fVertexCnt(0), fIndexCnt(0) {}
    int fVertexCnt;
    int fIndexCnt;
};

typedef SkTArray<Draw, true> DrawArray;

static void create_vertices(const SegmentArray& segments,
                            const SkPoint& fanPt,
                            const GrVertexColor& color,
                            DrawArray* draws,
                            GrVertexWriter& verts,
                            uint16_t* idxs,
                            size_t vertexStride) {
    Draw* draw = &draws->push_back();
    // alias just to make vert/index assignments easier to read.
    int* v = &draw->fVertexCnt;
    int* i = &draw->fIndexCnt;
    const size_t uvOffset = sizeof(SkPoint) + color.size();

    int count = segments.count();
    for (int a = 0; a < count; ++a) {
        const Segment& sega = segments[a];
        int b = (a + 1) % count;
        const Segment& segb = segments[b];

        // Check whether adding the verts for this segment to the current draw would cause index
        // values to overflow.
        int vCount = 4;
        if (Segment::kLine == segb.fType) {
            vCount += 5;
        } else {
            vCount += 6;
        }
        if (draw->fVertexCnt + vCount > (1 << 16)) {
            idxs += *i;
            draw = &draws->push_back();
            v = &draw->fVertexCnt;
            i = &draw->fIndexCnt;
        }

        const SkScalar negOneDists[2] = { -SK_Scalar1, -SK_Scalar1 };

        // FIXME: These tris are inset in the 1 unit arc around the corner
        SkPoint p0 = sega.endPt();
        // Position, Color, UV, D0, D1
        verts.write(p0,                  color, SkPoint{0, 0},           negOneDists);
        verts.write(p0 + sega.endNorm(), color, SkPoint{0, -SK_Scalar1}, negOneDists);
        verts.write(p0 + segb.fMid,      color, SkPoint{0, -SK_Scalar1}, negOneDists);
        verts.write(p0 + segb.fNorms[0], color, SkPoint{0, -SK_Scalar1}, negOneDists);

        idxs[*i + 0] = *v + 0;
        idxs[*i + 1] = *v + 2;
        idxs[*i + 2] = *v + 1;
        idxs[*i + 3] = *v + 0;
        idxs[*i + 4] = *v + 3;
        idxs[*i + 5] = *v + 2;

        *v += 4;
        *i += 6;

        if (Segment::kLine == segb.fType) {
            // we draw the line edge as a degenerate quad (u is 0, v is the
            // signed distance to the edge)
            SkPoint v1Pos = sega.endPt();
            SkPoint v2Pos = segb.fPts[0];
            SkScalar dist = SkPointPriv::DistanceToLineBetween(fanPt, v1Pos, v2Pos);

            verts.write(fanPt,                  color, SkPoint{0, dist},        negOneDists);
            verts.write(v1Pos,                  color, SkPoint{0, 0},           negOneDists);
            verts.write(v2Pos,                  color, SkPoint{0, 0},           negOneDists);
            verts.write(v1Pos + segb.fNorms[0], color, SkPoint{0, -SK_Scalar1}, negOneDists);
            verts.write(v2Pos + segb.fNorms[0], color, SkPoint{0, -SK_Scalar1}, negOneDists);

            idxs[*i + 0] = *v + 3;
            idxs[*i + 1] = *v + 1;
            idxs[*i + 2] = *v + 2;

            idxs[*i + 3] = *v + 4;
            idxs[*i + 4] = *v + 3;
            idxs[*i + 5] = *v + 2;

            *i += 6;

            // Draw the interior fan if it exists.
            // TODO: Detect and combine colinear segments. This will ensure we catch every case
            // with no interior, and that the resulting shared edge uses the same endpoints.
            if (count >= 3) {
                idxs[*i + 0] = *v + 0;
                idxs[*i + 1] = *v + 2;
                idxs[*i + 2] = *v + 1;

                *i += 3;
            }

            *v += 5;
        } else {
            void* quadVertsBegin = verts.fPtr;

            SkPoint qpts[] = {sega.endPt(), segb.fPts[0], segb.fPts[1]};

            SkScalar c0 = segb.fNorms[0].dot(qpts[0]);
            SkScalar c1 = segb.fNorms[1].dot(qpts[2]);
            GrVertexWriter::Skip<SkPoint> skipUVs;

            verts.write(fanPt,
                        color, skipUVs,
                        -segb.fNorms[0].dot(fanPt) + c0,
                        -segb.fNorms[1].dot(fanPt) + c1);

            verts.write(qpts[0],
                        color, skipUVs,
                        0.0f,
                        -segb.fNorms[1].dot(qpts[0]) + c1);

            verts.write(qpts[2],
                        color, skipUVs,
                        -segb.fNorms[0].dot(qpts[2]) + c0,
                        0.0f);

            verts.write(qpts[0] + segb.fNorms[0],
                        color, skipUVs,
                        -SK_ScalarMax/100,
                        -SK_ScalarMax/100);

            verts.write(qpts[2] + segb.fNorms[1],
                        color, skipUVs,
                        -SK_ScalarMax/100,
                        -SK_ScalarMax/100);

            SkVector midVec = segb.fNorms[0] + segb.fNorms[1];
            midVec.normalize();

            verts.write(qpts[1] + midVec,
                        color, skipUVs,
                        -SK_ScalarMax/100,
                        -SK_ScalarMax/100);

            GrPathUtils::QuadUVMatrix toUV(qpts);
            toUV.apply(quadVertsBegin, 6, vertexStride, uvOffset);

            idxs[*i + 0] = *v + 3;
            idxs[*i + 1] = *v + 1;
            idxs[*i + 2] = *v + 2;
            idxs[*i + 3] = *v + 4;
            idxs[*i + 4] = *v + 3;
            idxs[*i + 5] = *v + 2;

            idxs[*i + 6] = *v + 5;
            idxs[*i + 7] = *v + 3;
            idxs[*i + 8] = *v + 4;

            *i += 9;

            // Draw the interior fan if it exists.
            // TODO: Detect and combine colinear segments. This will ensure we catch every case
            // with no interior, and that the resulting shared edge uses the same endpoints.
            if (count >= 3) {
                idxs[*i + 0] = *v + 0;
                idxs[*i + 1] = *v + 2;
                idxs[*i + 2] = *v + 1;

                *i += 3;
            }

            *v += 6;
        }
    }
}

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

class QuadEdgeEffect : public GrGeometryProcessor {
public:
    static sk_sp<GrGeometryProcessor> Make(const SkMatrix& localMatrix, bool usesLocalCoords,
                                           bool wideColor) {
        return sk_sp<GrGeometryProcessor>(
                new QuadEdgeEffect(localMatrix, usesLocalCoords, wideColor));
    }

    ~QuadEdgeEffect() override {}

    const char* name() const override { return "QuadEdge"; }

    class GLSLProcessor : public GrGLSLGeometryProcessor {
    public:
        GLSLProcessor() {}

        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
            const QuadEdgeEffect& qe = args.fGP.cast<QuadEdgeEffect>();
            GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
            GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            // emit attributes
            varyingHandler->emitAttributes(qe);

            GrGLSLVarying v(kHalf4_GrSLType);
            varyingHandler->addVarying("QuadEdge", &v);
            vertBuilder->codeAppendf("%s = %s;", v.vsOut(), qe.fInQuadEdge.name());

            // Setup pass through color
            varyingHandler->addPassThroughAttribute(qe.fInColor, args.fOutputColor);

            GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

            // Setup position
            this->writeOutputPosition(vertBuilder, gpArgs, qe.fInPosition.name());

            // emit transforms
            this->emitTransforms(vertBuilder,
                                 varyingHandler,
                                 uniformHandler,
                                 qe.fInPosition.asShaderVar(),
                                 qe.fLocalMatrix,
                                 args.fFPCoordTransformHandler);

            fragBuilder->codeAppendf("half edgeAlpha;");

            // keep the derivative instructions outside the conditional
            fragBuilder->codeAppendf("half2 duvdx = half2(dFdx(%s.xy));", v.fsIn());
            fragBuilder->codeAppendf("half2 duvdy = half2(dFdy(%s.xy));", v.fsIn());
            fragBuilder->codeAppendf("if (%s.z > 0.0 && %s.w > 0.0) {", v.fsIn(), v.fsIn());
            // today we know z and w are in device space. We could use derivatives
            fragBuilder->codeAppendf("edgeAlpha = min(min(%s.z, %s.w) + 0.5, 1.0);", v.fsIn(),
                                     v.fsIn());
            fragBuilder->codeAppendf ("} else {");
            fragBuilder->codeAppendf("half2 gF = half2(2.0*%s.x*duvdx.x - duvdx.y,"
                                     "               2.0*%s.x*duvdy.x - duvdy.y);",
                                     v.fsIn(), v.fsIn());
            fragBuilder->codeAppendf("edgeAlpha = (%s.x*%s.x - %s.y);", v.fsIn(), v.fsIn(),
                                     v.fsIn());
            fragBuilder->codeAppendf("edgeAlpha = "
                                     "saturate(0.5 - edgeAlpha / length(gF));}");

            fragBuilder->codeAppendf("%s = half4(edgeAlpha);", args.fOutputCoverage);
        }

        static inline void GenKey(const GrGeometryProcessor& gp,
                                  const GrShaderCaps&,
                                  GrProcessorKeyBuilder* b) {
            const QuadEdgeEffect& qee = gp.cast<QuadEdgeEffect>();
            b->add32(SkToBool(qee.fUsesLocalCoords && qee.fLocalMatrix.hasPerspective()));
        }

        void setData(const GrGLSLProgramDataManager& pdman,
                     const GrPrimitiveProcessor& gp,
                     FPCoordTransformIter&& transformIter) override {
            const QuadEdgeEffect& qe = gp.cast<QuadEdgeEffect>();
            this->setTransformDataHelper(qe.fLocalMatrix, pdman, &transformIter);
        }

    private:
        typedef GrGLSLGeometryProcessor INHERITED;
    };

    void getGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {
        GLSLProcessor::GenKey(*this, caps, b);
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override {
        return new GLSLProcessor();
    }

private:
    QuadEdgeEffect(const SkMatrix& localMatrix, bool usesLocalCoords, bool wideColor)
            : INHERITED(kQuadEdgeEffect_ClassID)
            , fLocalMatrix(localMatrix)
            , fUsesLocalCoords(usesLocalCoords) {
        fInPosition = {"inPosition", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
        fInColor = MakeColorAttribute("inColor", wideColor);
        fInQuadEdge = {"inQuadEdge", kFloat4_GrVertexAttribType, kHalf4_GrSLType};
        this->setVertexAttributes(&fInPosition, 3);
    }

    Attribute fInPosition;
    Attribute fInColor;
    Attribute fInQuadEdge;

    SkMatrix fLocalMatrix;
    bool fUsesLocalCoords;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST

    typedef GrGeometryProcessor INHERITED;
};

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(QuadEdgeEffect);

#if GR_TEST_UTILS
sk_sp<GrGeometryProcessor> QuadEdgeEffect::TestCreate(GrProcessorTestData* d) {
    // Doesn't work without derivative instructions.
    return d->caps()->shaderCaps()->shaderDerivativeSupport()
                   ? QuadEdgeEffect::Make(GrTest::TestMatrix(d->fRandom), d->fRandom->nextBool(),
                                          d->fRandom->nextBool())
                   : nullptr;
}
#endif

///////////////////////////////////////////////////////////////////////////////

GrPathRenderer::CanDrawPath
GrAAConvexPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    if (args.fCaps->shaderCaps()->shaderDerivativeSupport() &&
        (GrAAType::kCoverage == args.fAAType) && args.fShape->style().isSimpleFill() &&
        !args.fShape->inverseFilled() && args.fShape->knownToBeConvex()) {
        return CanDrawPath::kYes;
    }
    return CanDrawPath::kNo;
}

namespace {

class AAConvexPathOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelperWithStencil;

public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                                          GrPaint&& paint,
                                          const SkMatrix& viewMatrix,
                                          const SkPath& path,
                                          const GrUserStencilSettings* stencilSettings) {
        return Helper::FactoryHelper<AAConvexPathOp>(context, std::move(paint), viewMatrix, path,
                                                     stencilSettings);
    }

    AAConvexPathOp(const Helper::MakeArgs& helperArgs, const SkPMColor4f& color,
                   const SkMatrix& viewMatrix, const SkPath& path,
                   const GrUserStencilSettings* stencilSettings)
            : INHERITED(ClassID()), fHelper(helperArgs, GrAAType::kCoverage, stencilSettings) {
        fPaths.emplace_back(PathData{viewMatrix, path, color});
        this->setTransformedBounds(path.getBounds(), viewMatrix, HasAABloat::kYes, IsZeroArea::kNo);
        fWideColor = !SkPMColor4fFitsInBytes(color);
    }

    const char* name() const override { return "AAConvexPathOp"; }

    void visitProxies(const VisitProxyFunc& func, VisitorType) const override {
        fHelper.visitProxies(func);
    }

#ifdef SK_DEBUG
    SkString dumpInfo() const override {
        SkString string;
        string.appendf("Count: %d\n", fPaths.count());
        string += fHelper.dumpInfo();
        string += INHERITED::dumpInfo();
        return string;
    }
#endif

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

    GrProcessorSet::Analysis finalize(
            const GrCaps& caps, const GrAppliedClip* clip, GrFSAAType fsaaType) override {
        return fHelper.finalizeProcessors(
                caps, clip, fsaaType, GrProcessorAnalysisCoverage::kSingleChannel,
                &fPaths.back().fColor);
    }

private:
    void onPrepareDraws(Target* target) override {
        int instanceCount = fPaths.count();

        SkMatrix invert;
        if (fHelper.usesLocalCoords() && !fPaths.back().fViewMatrix.invert(&invert)) {
            return;
        }

        // Setup GrGeometryProcessor
        sk_sp<GrGeometryProcessor> quadProcessor(
                QuadEdgeEffect::Make(invert, fHelper.usesLocalCoords(), fWideColor));
        const size_t kVertexStride = quadProcessor->vertexStride();

        // TODO generate all segments for all paths and use one vertex buffer
        for (int i = 0; i < instanceCount; i++) {
            const PathData& args = fPaths[i];

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

            int vertexCount;
            int indexCount;
            enum {
                kPreallocSegmentCnt = 512 / sizeof(Segment),
                kPreallocDrawCnt = 4,
            };
            SkSTArray<kPreallocSegmentCnt, Segment, true> segments;
            SkPoint fanPt;

            if (!get_segments(*pathPtr, *viewMatrix, &segments, &fanPt, &vertexCount,
                              &indexCount)) {
                continue;
            }

            sk_sp<const GrBuffer> vertexBuffer;
            int firstVertex;

            GrVertexWriter verts{target->makeVertexSpace(kVertexStride, vertexCount,
                                                         &vertexBuffer, &firstVertex)};

            if (!verts.fPtr) {
                SkDebugf("Could not allocate vertices\n");
                return;
            }

            sk_sp<const GrBuffer> indexBuffer;
            int firstIndex;

            uint16_t *idxs = target->makeIndexSpace(indexCount, &indexBuffer, &firstIndex);
            if (!idxs) {
                SkDebugf("Could not allocate indices\n");
                return;
            }

            SkSTArray<kPreallocDrawCnt, Draw, true> draws;
            GrVertexColor color(args.fColor, fWideColor);
            create_vertices(segments, fanPt, color, &draws, verts, idxs, kVertexStride);

            GrMesh* meshes = target->allocMeshes(draws.count());
            for (int j = 0; j < draws.count(); ++j) {
                const Draw& draw = draws[j];
                meshes[j].setPrimitiveType(GrPrimitiveType::kTriangles);
                meshes[j].setIndexed(indexBuffer, draw.fIndexCnt, firstIndex, 0,
                                     draw.fVertexCnt - 1, GrPrimitiveRestart::kNo);
                meshes[j].setVertexData(vertexBuffer, firstVertex);
                firstIndex += draw.fIndexCnt;
                firstVertex += draw.fVertexCnt;
            }
            target->recordDraw(quadProcessor, meshes, draws.count());
        }
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        fHelper.executeDrawsAndUploads(this, flushState, chainBounds);
    }

    CombineResult onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        AAConvexPathOp* that = t->cast<AAConvexPathOp>();
        if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
            return CombineResult::kCannotCombine;
        }
        if (fHelper.usesLocalCoords() &&
            !fPaths[0].fViewMatrix.cheapEqualTo(that->fPaths[0].fViewMatrix)) {
            return CombineResult::kCannotCombine;
        }

        fPaths.push_back_n(that->fPaths.count(), that->fPaths.begin());
        fWideColor |= that->fWideColor;
        return CombineResult::kMerged;
    }

    struct PathData {
        SkMatrix fViewMatrix;
        SkPath fPath;
        SkPMColor4f fColor;
    };

    Helper fHelper;
    SkSTArray<1, PathData, true> fPaths;
    bool fWideColor;

    typedef GrMeshDrawOp INHERITED;
};

}  // anonymous namespace

bool GrAAConvexPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fRenderTargetContext->auditTrail(),
                              "GrAAConvexPathRenderer::onDrawPath");
    SkASSERT(GrFSAAType::kUnifiedMSAA != args.fRenderTargetContext->fsaaType());
    SkASSERT(!args.fShape->isEmpty());

    SkPath path;
    args.fShape->asPath(&path);

    std::unique_ptr<GrDrawOp> op = AAConvexPathOp::Make(args.fContext, std::move(args.fPaint),
                                                        *args.fViewMatrix,
                                                        path, args.fUserStencilSettings);
    args.fRenderTargetContext->addDrawOp(*args.fClip, std::move(op));
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS

GR_DRAW_OP_TEST_DEFINE(AAConvexPathOp) {
    SkMatrix viewMatrix = GrTest::TestMatrixInvertible(random);
    SkPath path = GrTest::TestPathConvex(random);
    const GrUserStencilSettings* stencilSettings = GrGetRandomStencil(random, context);
    return AAConvexPathOp::Make(context, std::move(paint), viewMatrix, path, stencilSettings);
}

#endif
