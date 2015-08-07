
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAAConvexPathRenderer.h"

#include "GrAAConvexTessellator.h"
#include "GrBatchTarget.h"
#include "GrBatchTest.h"
#include "GrCaps.h"
#include "GrContext.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrGeometryProcessor.h"
#include "GrInvariantOutput.h"
#include "GrPathUtils.h"
#include "GrProcessor.h"
#include "GrPipelineBuilder.h"
#include "GrStrokeInfo.h"
#include "SkGeometry.h"
#include "SkPathPriv.h"
#include "SkString.h"
#include "SkTraceEvent.h"
#include "batches/GrBatch.h"
#include "gl/GrGLProcessor.h"
#include "gl/GrGLGeometryProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"

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
    };
    const SkPoint& endNorm() const {
        GR_STATIC_ASSERT(0 == kLine && 1 == kQuad);
        return fNorms[fType];
    };
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
            const SkPoint pj = segments[i + 1].endPt() - p0;

            SkScalar t = SkScalarMul(pi.fX, pj.fY) - SkScalarMul(pj.fX, pi.fY);
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
        center.fX = SkScalarMul(center.fX, area);
        center.fY = SkScalarMul(center.fY, area);
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
static const SkScalar kCloseSqd = SkScalarMul(kClose, kClose);

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
    SkScalar det2x2 = SkScalarMul(m.get(SkMatrix::kMScaleX), m.get(SkMatrix::kMScaleY)) -
                      SkScalarMul(m.get(SkMatrix::kMSkewX), m.get(SkMatrix::kMSkewY));
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
    GrPathUtils::convertCubicToQuads(pts, SK_Scalar1, true, dir, &quads);
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
        SkPath::Verb verb = iter.next(pts);
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

static void create_vertices(const SegmentArray&  segments,
                            const SkPoint& fanPt,
                            DrawArray*     draws,
                            QuadVertex*    verts,
                            uint16_t*      idxs) {
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
            toUV.apply<6, sizeof(QuadVertex), sizeof(SkPoint)>(verts + *v);

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

    static GrGeometryProcessor* Create(GrColor color, const SkMatrix& localMatrix,
                                       bool usesLocalCoords) {
        return SkNEW_ARGS(QuadEdgeEffect, (color, localMatrix, usesLocalCoords));
    }

    virtual ~QuadEdgeEffect() {}

    const char* name() const override { return "QuadEdge"; }

    const Attribute* inPosition() const { return fInPosition; }
    const Attribute* inQuadEdge() const { return fInQuadEdge; }
    GrColor color() const { return fColor; }
    bool colorIgnored() const { return GrColor_ILLEGAL == fColor; }
    const SkMatrix& localMatrix() const { return fLocalMatrix; }
    bool usesLocalCoords() const { return fUsesLocalCoords; }

    class GLProcessor : public GrGLGeometryProcessor {
    public:
        GLProcessor(const GrGeometryProcessor&,
                    const GrBatchTracker&)
            : fColor(GrColor_ILLEGAL) {}

        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
            const QuadEdgeEffect& qe = args.fGP.cast<QuadEdgeEffect>();
            GrGLGPBuilder* pb = args.fPB;
            GrGLVertexBuilder* vsBuilder = pb->getVertexShaderBuilder();

            // emit attributes
            vsBuilder->emitAttributes(qe);

            GrGLVertToFrag v(kVec4f_GrSLType);
            args.fPB->addVarying("QuadEdge", &v);
            vsBuilder->codeAppendf("%s = %s;", v.vsOut(), qe.inQuadEdge()->fName);

            // Setup pass through color
            if (!qe.colorIgnored()) {
                this->setupUniformColor(pb, args.fOutputColor, &fColorUniform);
            }

            // Setup position
            this->setupPosition(pb, gpArgs, qe.inPosition()->fName);

            // emit transforms
            this->emitTransforms(args.fPB, gpArgs->fPositionVar, qe.inPosition()->fName,
                                 qe.localMatrix(), args.fTransformsIn, args.fTransformsOut);

            GrGLFragmentBuilder* fsBuilder = args.fPB->getFragmentShaderBuilder();

            SkAssertResult(fsBuilder->enableFeature(
                    GrGLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));
            fsBuilder->codeAppendf("float edgeAlpha;");

            // keep the derivative instructions outside the conditional
            fsBuilder->codeAppendf("vec2 duvdx = dFdx(%s.xy);", v.fsIn());
            fsBuilder->codeAppendf("vec2 duvdy = dFdy(%s.xy);", v.fsIn());
            fsBuilder->codeAppendf("if (%s.z > 0.0 && %s.w > 0.0) {", v.fsIn(), v.fsIn());
            // today we know z and w are in device space. We could use derivatives
            fsBuilder->codeAppendf("edgeAlpha = min(min(%s.z, %s.w) + 0.5, 1.0);", v.fsIn(),
                                    v.fsIn());
            fsBuilder->codeAppendf ("} else {");
            fsBuilder->codeAppendf("vec2 gF = vec2(2.0*%s.x*duvdx.x - duvdx.y,"
                                   "               2.0*%s.x*duvdy.x - duvdy.y);",
                                   v.fsIn(), v.fsIn());
            fsBuilder->codeAppendf("edgeAlpha = (%s.x*%s.x - %s.y);", v.fsIn(), v.fsIn(),
                                    v.fsIn());
            fsBuilder->codeAppendf("edgeAlpha = "
                                   "clamp(0.5 - edgeAlpha / length(gF), 0.0, 1.0);}");

            fsBuilder->codeAppendf("%s = vec4(edgeAlpha);", args.fOutputCoverage);
        }

        static inline void GenKey(const GrGeometryProcessor& gp,
                                  const GrBatchTracker& bt,
                                  const GrGLSLCaps&,
                                  GrProcessorKeyBuilder* b) {
            const QuadEdgeEffect& qee = gp.cast<QuadEdgeEffect>();
            uint32_t key = 0;
            key |= qee.usesLocalCoords() && qee.localMatrix().hasPerspective() ? 0x1 : 0x0;
            key |= qee.colorIgnored() ? 0x2 : 0x0;
            b->add32(key);
        }

        virtual void setData(const GrGLProgramDataManager& pdman,
                             const GrPrimitiveProcessor& gp,
                             const GrBatchTracker& bt) override {
            const QuadEdgeEffect& qe = gp.cast<QuadEdgeEffect>();
            if (qe.color() != fColor) {
                GrGLfloat c[4];
                GrColorToRGBAFloat(qe.color(), c);
                pdman.set4fv(fColorUniform, 1, c);
                fColor = qe.color();
            }
        }

        void setTransformData(const GrPrimitiveProcessor& primProc,
                              const GrGLProgramDataManager& pdman,
                              int index,
                              const SkTArray<const GrCoordTransform*, true>& transforms) override {
            this->setTransformDataHelper<QuadEdgeEffect>(primProc, pdman, index, transforms);
        }

    private:
        GrColor fColor;
        UniformHandle fColorUniform;

        typedef GrGLGeometryProcessor INHERITED;
    };

    virtual void getGLProcessorKey(const GrBatchTracker& bt,
                                   const GrGLSLCaps& caps,
                                   GrProcessorKeyBuilder* b) const override {
        GLProcessor::GenKey(*this, bt, caps, b);
    }

    virtual GrGLPrimitiveProcessor* createGLInstance(const GrBatchTracker& bt,
                                                     const GrGLSLCaps&) const override {
        return SkNEW_ARGS(GLProcessor, (*this, bt));
    }

private:
    QuadEdgeEffect(GrColor color, const SkMatrix& localMatrix, bool usesLocalCoords)
        : fColor(color)
        , fLocalMatrix(localMatrix)
        , fUsesLocalCoords(usesLocalCoords) {
        this->initClassID<QuadEdgeEffect>();
        fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType));
        fInQuadEdge = &this->addVertexAttrib(Attribute("inQuadEdge", kVec4f_GrVertexAttribType));
    }

    const Attribute* fInPosition;
    const Attribute* fInQuadEdge;
    GrColor          fColor;
    SkMatrix         fLocalMatrix;
    bool             fUsesLocalCoords;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(QuadEdgeEffect);

GrGeometryProcessor* QuadEdgeEffect::TestCreate(GrProcessorTestData* d) {
    // Doesn't work without derivative instructions.
    return d->fCaps->shaderCaps()->shaderDerivativeSupport() ?
           QuadEdgeEffect::Create(GrRandomColor(d->fRandom),
                                  GrTest::TestMatrix(d->fRandom),
                                  d->fRandom->nextBool()) : NULL;
}

///////////////////////////////////////////////////////////////////////////////

bool GrAAConvexPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    return (args.fTarget->caps()->shaderCaps()->shaderDerivativeSupport() && args.fAntiAlias &&
            args.fStroke->isFillStyle() && !args.fPath->isInverseFillType() &&
            args.fPath->isConvex());
}

// extract the result vertices and indices from the GrAAConvexTessellator
static void extract_verts(const GrAAConvexTessellator& tess,
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

static const GrGeometryProcessor* create_fill_gp(bool tweakAlphaForCoverage,
                                                 const SkMatrix& viewMatrix,
                                                 bool usesLocalCoords,
                                                 bool coverageIgnored) {
    using namespace GrDefaultGeoProcFactory;

    Color color(Color::kAttribute_Type);
    Coverage::Type coverageType;
    // TODO remove coverage if coverage is ignored
    /*if (coverageIgnored) {
        coverageType = Coverage::kNone_Type;
    } else*/ if (tweakAlphaForCoverage) {
        coverageType = Coverage::kSolid_Type;
    } else {
        coverageType = Coverage::kAttribute_Type;
    }
    Coverage coverage(coverageType);
    LocalCoords localCoords(usesLocalCoords ? LocalCoords::kUsePosition_Type :
                                              LocalCoords::kUnused_Type);
    return CreateForDeviceSpace(color, coverage, localCoords, viewMatrix);
}

class AAConvexPathBatch : public GrBatch {
public:
    struct Geometry {
        GrColor fColor;
        SkMatrix fViewMatrix;
        SkPath fPath;
    };

    static GrBatch* Create(const Geometry& geometry) {
        return SkNEW_ARGS(AAConvexPathBatch, (geometry));
    }

    const char* name() const override { return "AAConvexBatch"; }

    void getInvariantOutputColor(GrInitInvariantOutput* out) const override {
        // When this is called on a batch, there is only one geometry bundle
        out->setKnownFourComponents(fGeoData[0].fColor);
    }
    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const override {
        out->setUnknownSingleComponent();
    }

    void initBatchTracker(const GrPipelineInfo& init) override {
        // Handle any color overrides
        if (!init.readsColor()) {
            fGeoData[0].fColor = GrColor_ILLEGAL;
        }
        init.getOverrideColorIfSet(&fGeoData[0].fColor);

        // setup batch properties
        fBatch.fColorIgnored = !init.readsColor();
        fBatch.fColor = fGeoData[0].fColor;
        fBatch.fUsesLocalCoords = init.readsLocalCoords();
        fBatch.fCoverageIgnored = !init.readsCoverage();
        fBatch.fLinesOnly = SkPath::kLine_SegmentMask == fGeoData[0].fPath.getSegmentMasks();
        fBatch.fCanTweakAlphaForCoverage = init.canTweakAlphaForCoverage();
    }

    void generateGeometryLinesOnly(GrBatchTarget* batchTarget) {
        bool canTweakAlphaForCoverage = this->canTweakAlphaForCoverage();

        // Setup GrGeometryProcessor
        SkAutoTUnref<const GrGeometryProcessor> gp(create_fill_gp(canTweakAlphaForCoverage,
                                                                  this->viewMatrix(),
                                                                  this->usesLocalCoords(),
                                                                  this->coverageIgnored()));
        if (!gp) {
            SkDebugf("Could not create GrGeometryProcessor\n");
            return;
        }

        batchTarget->initDraw(gp, this->pipeline());

        size_t vertexStride = gp->getVertexStride();

        SkASSERT(canTweakAlphaForCoverage ?
                 vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorAttr) :
                 vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorCoverageAttr));

        GrAAConvexTessellator tess;

        int instanceCount = fGeoData.count();

        for (int i = 0; i < instanceCount; i++) {
            tess.rewind();

            Geometry& args = fGeoData[i];

            if (!tess.tessellate(args.fViewMatrix, args.fPath)) {
                continue;
            }

            const GrVertexBuffer* vertexBuffer;
            int firstVertex;

            void* verts = batchTarget->makeVertSpace(vertexStride, tess.numPts(),
                                                     &vertexBuffer, &firstVertex);
            if (!verts) {
                SkDebugf("Could not allocate vertices\n");
                return;
            }

            const GrIndexBuffer* indexBuffer;
            int firstIndex;

            uint16_t* idxs = batchTarget->makeIndexSpace(tess.numIndices(),
                                                        &indexBuffer, &firstIndex);
            if (!idxs) {
                SkDebugf("Could not allocate indices\n");
                return;
            }

            extract_verts(tess, verts, vertexStride, args.fColor, idxs, canTweakAlphaForCoverage);

            GrVertices info;
            info.initIndexed(kTriangles_GrPrimitiveType,
                             vertexBuffer, indexBuffer,
                             firstVertex, firstIndex,
                             tess.numPts(), tess.numIndices());
            batchTarget->draw(info);
        }
    }

    void generateGeometry(GrBatchTarget* batchTarget) override {
#ifndef SK_IGNORE_LINEONLY_AA_CONVEX_PATH_OPTS
        if (this->linesOnly()) {
            this->generateGeometryLinesOnly(batchTarget);
            return;
        }
#endif

        int instanceCount = fGeoData.count();

        SkMatrix invert;
        if (this->usesLocalCoords() && !this->viewMatrix().invert(&invert)) {
            SkDebugf("Could not invert viewmatrix\n");
            return;
        }

        // Setup GrGeometryProcessor
        SkAutoTUnref<GrGeometryProcessor> quadProcessor(
                QuadEdgeEffect::Create(this->color(), invert, this->usesLocalCoords()));

        batchTarget->initDraw(quadProcessor, this->pipeline());

        // TODO generate all segments for all paths and use one vertex buffer
        for (int i = 0; i < instanceCount; i++) {
            Geometry& args = fGeoData[i];

            // We use the fact that SkPath::transform path does subdivision based on
            // perspective. Otherwise, we apply the view matrix when copying to the
            // segment representation.
            const SkMatrix* viewMatrix = &args.fViewMatrix;
            if (viewMatrix->hasPerspective()) {
                args.fPath.transform(*viewMatrix);
                viewMatrix = &SkMatrix::I();
            }

            int vertexCount;
            int indexCount;
            enum {
                kPreallocSegmentCnt = 512 / sizeof(Segment),
                kPreallocDrawCnt = 4,
            };
            SkSTArray<kPreallocSegmentCnt, Segment, true> segments;
            SkPoint fanPt;

            if (!get_segments(args.fPath, *viewMatrix, &segments, &fanPt, &vertexCount,
                              &indexCount)) {
                continue;
            }

            const GrVertexBuffer* vertexBuffer;
            int firstVertex;

            size_t vertexStride = quadProcessor->getVertexStride();
            QuadVertex* verts = reinterpret_cast<QuadVertex*>(batchTarget->makeVertSpace(
                vertexStride, vertexCount, &vertexBuffer, &firstVertex));

            if (!verts) {
                SkDebugf("Could not allocate vertices\n");
                return;
            }

            const GrIndexBuffer* indexBuffer;
            int firstIndex;

            uint16_t *idxs = batchTarget->makeIndexSpace(indexCount, &indexBuffer, &firstIndex);
            if (!idxs) {
                SkDebugf("Could not allocate indices\n");
                return;
            }

            SkSTArray<kPreallocDrawCnt, Draw, true> draws;
            create_vertices(segments, fanPt, &draws, verts, idxs);

            GrVertices vertices;

            for (int i = 0; i < draws.count(); ++i) {
                const Draw& draw = draws[i];
                vertices.initIndexed(kTriangles_GrPrimitiveType, vertexBuffer, indexBuffer,
                                     firstVertex, firstIndex, draw.fVertexCnt, draw.fIndexCnt);
                batchTarget->draw(vertices);
                firstVertex += draw.fVertexCnt;
                firstIndex += draw.fIndexCnt;
            }
        }
    }

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

private:
    AAConvexPathBatch(const Geometry& geometry) {
        this->initClassID<AAConvexPathBatch>();
        fGeoData.push_back(geometry);

        // compute bounds
        fBounds = geometry.fPath.getBounds();
        geometry.fViewMatrix.mapRect(&fBounds);
    }

    bool onCombineIfPossible(GrBatch* t) override {
        if (!this->pipeline()->isEqual(*t->pipeline())) {
            return false;
        }

        AAConvexPathBatch* that = t->cast<AAConvexPathBatch>();

        if (this->color() != that->color()) {
            return false;
        }

        SkASSERT(this->usesLocalCoords() == that->usesLocalCoords());
        if (this->usesLocalCoords() && !this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return false;
        }

        if (this->linesOnly() != that->linesOnly()) {
            return false;
        }

        // In the event of two batches, one who can tweak, one who cannot, we just fall back to
        // not tweaking
        if (this->canTweakAlphaForCoverage() != that->canTweakAlphaForCoverage()) {
            fBatch.fCanTweakAlphaForCoverage = false;
        }

        fGeoData.push_back_n(that->geoData()->count(), that->geoData()->begin());
        this->joinBounds(that->bounds());
        return true;
    }

    GrColor color() const { return fBatch.fColor; }
    bool linesOnly() const { return fBatch.fLinesOnly; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }
    bool canTweakAlphaForCoverage() const { return fBatch.fCanTweakAlphaForCoverage; }
    const SkMatrix& viewMatrix() const { return fGeoData[0].fViewMatrix; }
    bool coverageIgnored() const { return fBatch.fCoverageIgnored; }

    struct BatchTracker {
        GrColor fColor;
        bool fUsesLocalCoords;
        bool fColorIgnored;
        bool fCoverageIgnored;
        bool fLinesOnly;
        bool fCanTweakAlphaForCoverage;
    };

    BatchTracker fBatch;
    SkSTArray<1, Geometry, true> fGeoData;
};

bool GrAAConvexPathRenderer::onDrawPath(const DrawPathArgs& args) {
    if (args.fPath->isEmpty()) {
        return true;
    }

    AAConvexPathBatch::Geometry geometry;
    geometry.fColor = args.fColor;
    geometry.fViewMatrix = *args.fViewMatrix;
    geometry.fPath = *args.fPath;

    SkAutoTUnref<GrBatch> batch(AAConvexPathBatch::Create(geometry));
    args.fTarget->drawBatch(*args.fPipelineBuilder, batch);

    return true;

}

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_TEST_UTILS

BATCH_TEST_DEFINE(AAConvexPathBatch) {
    AAConvexPathBatch::Geometry geometry;
    geometry.fColor = GrRandomColor(random);
    geometry.fViewMatrix = GrTest::TestMatrixInvertible(random);
    geometry.fPath = GrTest::TestPathConvex(random);

    return AAConvexPathBatch::Create(geometry);
}

#endif
