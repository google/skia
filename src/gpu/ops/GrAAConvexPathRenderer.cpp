/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAAConvexPathRenderer.h"

#include "GrAAConvexTessellator.h"
#include "GrCaps.h"
#include "GrContext.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrDrawOpTest.h"
#include "GrGeometryProcessor.h"
#include "GrOpFlushState.h"
#include "GrPathUtils.h"
#include "GrProcessor.h"
#include "GrSimpleMeshDrawOpHelper.h"
#include "SkGeometry.h"
#include "SkPathPriv.h"
#include "SkString.h"
#include "SkTraceEvent.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLVertexShaderBuilder.h"
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

static void center_of_mass(const SegmentArray& segments, SkPoint* c) {
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
    SkASSERT(!SkScalarIsNaN(c->fX) && !SkScalarIsNaN(c->fY));
}

static void compute_vectors(SegmentArray* segments,
                            SkPoint* fanPt,
                            SkPathPriv::FirstDirection dir,
                            int* vCount,
                            int* iCount) {
    center_of_mass(*segments, fanPt);
    int count = segments->count();

    // Make the normals point towards the outside
    SkPoint::Side normSide;
    if (dir == SkPathPriv::kCCW_FirstDirection) {
        normSide = SkPoint::kRight_Side;
    } else {
        normSide = SkPoint::kLeft_Side;
    }

    *vCount = 0;
    *iCount = 0;
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
            segb.fNorms[p].setOrthog(segb.fNorms[p], normSide);
            prevPt = &segb.fPts[p];
        }
        if (Segment::kLine == segb.fType) {
            *vCount += 5;
            *iCount += 9;
        } else {
            *vCount += 6;
            *iCount += 12;
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
        *vCount += 4;
        *iCount += 6;
    }
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
            if (pt.distanceToSqd(data->fFirstPoint) > kCloseSqd) {
                data->fLineNormal = pt - data->fFirstPoint;
                data->fLineNormal.normalize();
                data->fLineNormal.setOrthog(data->fLineNormal);
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
            SkFAIL("Unexpected degenerate test stage.");
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
    if (pts[0].distanceToSqd(pts[1]) < kCloseSqd || pts[1].distanceToSqd(pts[2]) < kCloseSqd) {
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
                const SkPoint* quadPts = converter.computeQuads(pts, weight, 0.5f);
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
            };
            case SkPath::kDone_Verb:
                if (degenerateData.isDegenerate()) {
                    return false;
                } else {
                    compute_vectors(segments, fanPt, dir, vCount, iCount);
                    return true;
                }
            default:
                break;
        }
    }
}

struct QuadVertex {
    SkPoint  fPos;
    GrColor  fColor;
    SkPoint  fUV;
    SkScalar fD0;
    SkScalar fD1;
};

struct Draw {
    Draw() : fVertexCnt(0), fIndexCnt(0) {}
    int fVertexCnt;
    int fIndexCnt;
};

typedef SkTArray<Draw, true> DrawArray;

static void create_vertices(const SegmentArray& segments,
                            const SkPoint& fanPt,
                            GrColor color,
                            DrawArray* draws,
                            QuadVertex* verts,
                            uint16_t* idxs) {
    Draw* draw = &draws->push_back();
    // alias just to make vert/index assignments easier to read.
    int* v = &draw->fVertexCnt;
    int* i = &draw->fIndexCnt;

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
            verts += *v;
            idxs += *i;
            draw = &draws->push_back();
            v = &draw->fVertexCnt;
            i = &draw->fIndexCnt;
        }

        // FIXME: These tris are inset in the 1 unit arc around the corner
        verts[*v + 0].fPos = sega.endPt();
        verts[*v + 1].fPos = verts[*v + 0].fPos + sega.endNorm();
        verts[*v + 2].fPos = verts[*v + 0].fPos + segb.fMid;
        verts[*v + 3].fPos = verts[*v + 0].fPos + segb.fNorms[0];
        verts[*v + 0].fColor = color;
        verts[*v + 1].fColor = color;
        verts[*v + 2].fColor = color;
        verts[*v + 3].fColor = color;
        verts[*v + 0].fUV.set(0,0);
        verts[*v + 1].fUV.set(0,-SK_Scalar1);
        verts[*v + 2].fUV.set(0,-SK_Scalar1);
        verts[*v + 3].fUV.set(0,-SK_Scalar1);
        verts[*v + 0].fD0 = verts[*v + 0].fD1 = -SK_Scalar1;
        verts[*v + 1].fD0 = verts[*v + 1].fD1 = -SK_Scalar1;
        verts[*v + 2].fD0 = verts[*v + 2].fD1 = -SK_Scalar1;
        verts[*v + 3].fD0 = verts[*v + 3].fD1 = -SK_Scalar1;

        idxs[*i + 0] = *v + 0;
        idxs[*i + 1] = *v + 2;
        idxs[*i + 2] = *v + 1;
        idxs[*i + 3] = *v + 0;
        idxs[*i + 4] = *v + 3;
        idxs[*i + 5] = *v + 2;

        *v += 4;
        *i += 6;

        if (Segment::kLine == segb.fType) {
            verts[*v + 0].fPos = fanPt;
            verts[*v + 1].fPos = sega.endPt();
            verts[*v + 2].fPos = segb.fPts[0];

            verts[*v + 3].fPos = verts[*v + 1].fPos + segb.fNorms[0];
            verts[*v + 4].fPos = verts[*v + 2].fPos + segb.fNorms[0];

            verts[*v + 0].fColor = color;
            verts[*v + 1].fColor = color;
            verts[*v + 2].fColor = color;
            verts[*v + 3].fColor = color;
            verts[*v + 4].fColor = color;

            // we draw the line edge as a degenerate quad (u is 0, v is the
            // signed distance to the edge)
            SkScalar dist = fanPt.distanceToLineBetween(verts[*v + 1].fPos,
                                                        verts[*v + 2].fPos);
            verts[*v + 0].fUV.set(0, dist);
            verts[*v + 1].fUV.set(0, 0);
            verts[*v + 2].fUV.set(0, 0);
            verts[*v + 3].fUV.set(0, -SK_Scalar1);
            verts[*v + 4].fUV.set(0, -SK_Scalar1);

            verts[*v + 0].fD0 = verts[*v + 0].fD1 = -SK_Scalar1;
            verts[*v + 1].fD0 = verts[*v + 1].fD1 = -SK_Scalar1;
            verts[*v + 2].fD0 = verts[*v + 2].fD1 = -SK_Scalar1;
            verts[*v + 3].fD0 = verts[*v + 3].fD1 = -SK_Scalar1;
            verts[*v + 4].fD0 = verts[*v + 4].fD1 = -SK_Scalar1;

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
            SkPoint qpts[] = {sega.endPt(), segb.fPts[0], segb.fPts[1]};

            SkVector midVec = segb.fNorms[0] + segb.fNorms[1];
            midVec.normalize();

            verts[*v + 0].fPos = fanPt;
            verts[*v + 1].fPos = qpts[0];
            verts[*v + 2].fPos = qpts[2];
            verts[*v + 3].fPos = qpts[0] + segb.fNorms[0];
            verts[*v + 4].fPos = qpts[2] + segb.fNorms[1];
            verts[*v + 5].fPos = qpts[1] + midVec;

            verts[*v + 0].fColor = color;
            verts[*v + 1].fColor = color;
            verts[*v + 2].fColor = color;
            verts[*v + 3].fColor = color;
            verts[*v + 4].fColor = color;
            verts[*v + 5].fColor = color;

            SkScalar c = segb.fNorms[0].dot(qpts[0]);
            verts[*v + 0].fD0 =  -segb.fNorms[0].dot(fanPt) + c;
            verts[*v + 1].fD0 =  0.f;
            verts[*v + 2].fD0 =  -segb.fNorms[0].dot(qpts[2]) + c;
            verts[*v + 3].fD0 = -SK_ScalarMax/100;
            verts[*v + 4].fD0 = -SK_ScalarMax/100;
            verts[*v + 5].fD0 = -SK_ScalarMax/100;

            c = segb.fNorms[1].dot(qpts[2]);
            verts[*v + 0].fD1 =  -segb.fNorms[1].dot(fanPt) + c;
            verts[*v + 1].fD1 =  -segb.fNorms[1].dot(qpts[0]) + c;
            verts[*v + 2].fD1 =  0.f;
            verts[*v + 3].fD1 = -SK_ScalarMax/100;
            verts[*v + 4].fD1 = -SK_ScalarMax/100;
            verts[*v + 5].fD1 = -SK_ScalarMax/100;

            GrPathUtils::QuadUVMatrix toUV(qpts);
            toUV.apply<6, sizeof(QuadVertex), offsetof(QuadVertex, fUV)>(verts + *v);

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
    static sk_sp<GrGeometryProcessor> Make(const SkMatrix& localMatrix, bool usesLocalCoords) {
        return sk_sp<GrGeometryProcessor>(new QuadEdgeEffect(localMatrix, usesLocalCoords));
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

            GrGLSLVertToFrag v(kVec4f_GrSLType);
            varyingHandler->addVarying("QuadEdge", &v);
            vertBuilder->codeAppendf("%s = %s;", v.vsOut(), qe.fInQuadEdge->fName);

            // Setup pass through color
            varyingHandler->addPassThroughAttribute(qe.fInColor, args.fOutputColor);

            GrGLSLPPFragmentBuilder* fragBuilder = args.fFragBuilder;

            // Setup position
            this->setupPosition(vertBuilder, gpArgs, qe.fInPosition->fName);

            // emit transforms
            this->emitTransforms(vertBuilder,
                                 varyingHandler,
                                 uniformHandler,
                                 gpArgs->fPositionVar,
                                 qe.fInPosition->fName,
                                 qe.fLocalMatrix,
                                 args.fFPCoordTransformHandler);

            fragBuilder->codeAppendf("float edgeAlpha;");

            // keep the derivative instructions outside the conditional
            fragBuilder->codeAppendf("vec2 duvdx = dFdx(%s.xy);", v.fsIn());
            fragBuilder->codeAppendf("vec2 duvdy = dFdy(%s.xy);", v.fsIn());
            fragBuilder->codeAppendf("if (%s.z > 0.0 && %s.w > 0.0) {", v.fsIn(), v.fsIn());
            // today we know z and w are in device space. We could use derivatives
            fragBuilder->codeAppendf("edgeAlpha = min(min(%s.z, %s.w) + 0.5, 1.0);", v.fsIn(),
                                     v.fsIn());
            fragBuilder->codeAppendf ("} else {");
            fragBuilder->codeAppendf("vec2 gF = vec2(2.0*%s.x*duvdx.x - duvdx.y,"
                                     "               2.0*%s.x*duvdy.x - duvdy.y);",
                                     v.fsIn(), v.fsIn());
            fragBuilder->codeAppendf("edgeAlpha = (%s.x*%s.x - %s.y);", v.fsIn(), v.fsIn(),
                                     v.fsIn());
            fragBuilder->codeAppendf("edgeAlpha = "
                                     "clamp(0.5 - edgeAlpha / length(gF), 0.0, 1.0);}");

            fragBuilder->codeAppendf("%s = vec4(edgeAlpha);", args.fOutputCoverage);
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
    QuadEdgeEffect(const SkMatrix& localMatrix, bool usesLocalCoords)
            : fLocalMatrix(localMatrix), fUsesLocalCoords(usesLocalCoords) {
        this->initClassID<QuadEdgeEffect>();
        fInPosition = &this->addVertexAttrib("inPosition", kVec2f_GrVertexAttribType);
        fInColor = &this->addVertexAttrib("inColor", kVec4ub_GrVertexAttribType);
        fInQuadEdge = &this->addVertexAttrib("inQuadEdge", kVec4f_GrVertexAttribType);
    }

    const Attribute* fInPosition;
    const Attribute* fInQuadEdge;
    const Attribute* fInColor;
    SkMatrix         fLocalMatrix;
    bool             fUsesLocalCoords;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST

    typedef GrGeometryProcessor INHERITED;
};

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(QuadEdgeEffect);

#if GR_TEST_UTILS
sk_sp<GrGeometryProcessor> QuadEdgeEffect::TestCreate(GrProcessorTestData* d) {
    // Doesn't work without derivative instructions.
    return d->caps()->shaderCaps()->shaderDerivativeSupport()
                   ? QuadEdgeEffect::Make(GrTest::TestMatrix(d->fRandom), d->fRandom->nextBool())
                   : nullptr;
}
#endif

///////////////////////////////////////////////////////////////////////////////

bool GrAAConvexPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    return (args.fCaps->shaderCaps()->shaderDerivativeSupport() &&
            (GrAAType::kCoverage == args.fAAType) && args.fShape->style().isSimpleFill() &&
            !args.fShape->inverseFilled() && args.fShape->knownToBeConvex());
}

// extract the result vertices and indices from the GrAAConvexTessellator
static void extract_lines_only_verts(const GrAAConvexTessellator& tess,
                                     void* vertices,
                                     size_t vertexStride,
                                     GrColor color,
                                     uint16_t* idxs,
                                     bool tweakAlphaForCoverage) {
    intptr_t verts = reinterpret_cast<intptr_t>(vertices);

    for (int i = 0; i < tess.numPts(); ++i) {
        *((SkPoint*)((intptr_t)verts + i * vertexStride)) = tess.point(i);
    }

    // Make 'verts' point to the colors
    verts += sizeof(SkPoint);
    for (int i = 0; i < tess.numPts(); ++i) {
        if (tweakAlphaForCoverage) {
            SkASSERT(SkScalarRoundToInt(255.0f * tess.coverage(i)) <= 255);
            unsigned scale = SkScalarRoundToInt(255.0f * tess.coverage(i));
            GrColor scaledColor = (0xff == scale) ? color : SkAlphaMulQ(color, scale);
            *reinterpret_cast<GrColor*>(verts + i * vertexStride) = scaledColor;
        } else {
            *reinterpret_cast<GrColor*>(verts + i * vertexStride) = color;
            *reinterpret_cast<float*>(verts + i * vertexStride + sizeof(GrColor)) =
                    tess.coverage(i);
        }
    }

    for (int i = 0; i < tess.numIndices(); ++i) {
        idxs[i] = tess.index(i);
    }
}

static sk_sp<GrGeometryProcessor> make_lines_only_gp(bool tweakAlphaForCoverage,
                                                     const SkMatrix& viewMatrix,
                                                     bool usesLocalCoords) {
    using namespace GrDefaultGeoProcFactory;

    Coverage::Type coverageType;
    if (tweakAlphaForCoverage) {
        coverageType = Coverage::kSolid_Type;
    } else {
        coverageType = Coverage::kAttribute_Type;
    }
    LocalCoords::Type localCoordsType =
            usesLocalCoords ? LocalCoords::kUsePosition_Type : LocalCoords::kUnused_Type;
    return MakeForDeviceSpace(Color::kPremulGrColorAttribute_Type, coverageType, localCoordsType,
                              viewMatrix);
}

namespace {

class AAConvexPathOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelperWithStencil;

public:
    DEFINE_OP_CLASS_ID
    static std::unique_ptr<GrDrawOp> Make(GrPaint&& paint, const SkMatrix& viewMatrix,
                                          const SkPath& path,
                                          const GrUserStencilSettings* stencilSettings) {
        return Helper::FactoryHelper<AAConvexPathOp>(std::move(paint), viewMatrix, path,
                                                     stencilSettings);
    }

    AAConvexPathOp(const Helper::MakeArgs& helperArgs, GrColor color, const SkMatrix& viewMatrix,
                   const SkPath& path, const GrUserStencilSettings* stencilSettings)
            : INHERITED(ClassID()), fHelper(helperArgs, GrAAType::kCoverage, stencilSettings) {
        fPaths.emplace_back(PathData{viewMatrix, path, color});
        this->setTransformedBounds(path.getBounds(), viewMatrix, HasAABloat::kYes, IsZeroArea::kNo);
        fLinesOnly = SkPath::kLine_SegmentMask == path.getSegmentMasks();
    }

    const char* name() const override { return "AAConvexPathOp"; }

    SkString dumpInfo() const override {
        SkString string;
        string.appendf("Count: %d\n", fPaths.count());
        string += fHelper.dumpInfo();
        string += INHERITED::dumpInfo();
        return string;
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip) override {
        return fHelper.xpRequiresDstTexture(caps, clip, GrProcessorAnalysisCoverage::kSingleChannel,
                                            &fPaths.back().fColor);
    }

private:
    void prepareLinesOnlyDraws(Target* target) const {
        // Setup GrGeometryProcessor
        sk_sp<GrGeometryProcessor> gp(make_lines_only_gp(fHelper.compatibleWithAlphaAsCoverage(),
                                                         fPaths.back().fViewMatrix,
                                                         fHelper.usesLocalCoords()));
        if (!gp) {
            SkDebugf("Could not create GrGeometryProcessor\n");
            return;
        }

        size_t vertexStride = gp->getVertexStride();

        SkASSERT(fHelper.compatibleWithAlphaAsCoverage()
                         ? vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorAttr)
                         : vertexStride ==
                                   sizeof(GrDefaultGeoProcFactory::PositionColorCoverageAttr));

        GrAAConvexTessellator tess;

        int instanceCount = fPaths.count();
        const GrPipeline* pipeline = fHelper.makePipeline(target);
        for (int i = 0; i < instanceCount; i++) {
            tess.rewind();

            const PathData& args = fPaths[i];

            if (!tess.tessellate(args.fViewMatrix, args.fPath)) {
                continue;
            }

            const GrBuffer* vertexBuffer;
            int firstVertex;

            void* verts = target->makeVertexSpace(vertexStride, tess.numPts(), &vertexBuffer,
                                                  &firstVertex);
            if (!verts) {
                SkDebugf("Could not allocate vertices\n");
                return;
            }

            const GrBuffer* indexBuffer;
            int firstIndex;

            uint16_t* idxs = target->makeIndexSpace(tess.numIndices(), &indexBuffer, &firstIndex);
            if (!idxs) {
                SkDebugf("Could not allocate indices\n");
                return;
            }

            extract_lines_only_verts(tess, verts, vertexStride, args.fColor, idxs,
                                     fHelper.compatibleWithAlphaAsCoverage());

            GrMesh mesh(GrPrimitiveType::kTriangles);
            mesh.setIndexed(indexBuffer, tess.numIndices(), firstIndex, 0, tess.numPts() - 1);
            mesh.setVertexData(vertexBuffer, firstVertex);
            target->draw(gp.get(), pipeline, mesh);
        }
    }

    void onPrepareDraws(Target* target) const override {
#ifndef SK_IGNORE_LINEONLY_AA_CONVEX_PATH_OPTS
        if (fLinesOnly) {
            this->prepareLinesOnlyDraws(target);
            return;
        }
#endif
        const GrPipeline* pipeline = fHelper.makePipeline(target);
        int instanceCount = fPaths.count();

        SkMatrix invert;
        if (fHelper.usesLocalCoords() && !fPaths.back().fViewMatrix.invert(&invert)) {
            SkDebugf("Could not invert viewmatrix\n");
            return;
        }

        // Setup GrGeometryProcessor
        sk_sp<GrGeometryProcessor> quadProcessor(
                QuadEdgeEffect::Make(invert, fHelper.usesLocalCoords()));

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

            const GrBuffer* vertexBuffer;
            int firstVertex;

            size_t vertexStride = quadProcessor->getVertexStride();
            QuadVertex* verts = reinterpret_cast<QuadVertex*>(target->makeVertexSpace(
                vertexStride, vertexCount, &vertexBuffer, &firstVertex));

            if (!verts) {
                SkDebugf("Could not allocate vertices\n");
                return;
            }

            const GrBuffer* indexBuffer;
            int firstIndex;

            uint16_t *idxs = target->makeIndexSpace(indexCount, &indexBuffer, &firstIndex);
            if (!idxs) {
                SkDebugf("Could not allocate indices\n");
                return;
            }

            SkSTArray<kPreallocDrawCnt, Draw, true> draws;
            create_vertices(segments, fanPt, args.fColor, &draws, verts, idxs);

            GrMesh mesh(GrPrimitiveType::kTriangles);

            for (int j = 0; j < draws.count(); ++j) {
                const Draw& draw = draws[j];
                mesh.setIndexed(indexBuffer, draw.fIndexCnt, firstIndex, 0, draw.fVertexCnt - 1);
                mesh.setVertexData(vertexBuffer, firstVertex);
                target->draw(quadProcessor.get(), pipeline, mesh);
                firstIndex += draw.fIndexCnt;
                firstVertex += draw.fVertexCnt;
            }
        }
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        AAConvexPathOp* that = t->cast<AAConvexPathOp>();
        if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
            return false;
        }
        if (fHelper.usesLocalCoords() &&
            !fPaths[0].fViewMatrix.cheapEqualTo(that->fPaths[0].fViewMatrix)) {
            return false;
        }

        if (fLinesOnly != that->fLinesOnly) {
            return false;
        }

        fPaths.push_back_n(that->fPaths.count(), that->fPaths.begin());
        this->joinBounds(*that);
        return true;
    }

    struct PathData {
        SkMatrix fViewMatrix;
        SkPath fPath;
        GrColor fColor;
    };

    Helper fHelper;
    SkSTArray<1, PathData, true> fPaths;
    bool fLinesOnly;

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

    std::unique_ptr<GrDrawOp> op = AAConvexPathOp::Make(std::move(args.fPaint), *args.fViewMatrix,
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
    return AAConvexPathOp::Make(std::move(paint), viewMatrix, path, stencilSettings);
}

#endif
