/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/ops/AAConvexPathRenderer.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkMath.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTDArray.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkColorData.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkPathEnums.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkPointPriv.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/ganesh/GrAppliedClip.h"
#include "src/gpu/ganesh/GrAuditTrail.h"
#include "src/gpu/ganesh/GrBuffer.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDrawOpTest.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrMeshDrawTarget.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/GrProcessorAnalysis.h"
#include "src/gpu/ganesh/GrProcessorSet.h"
#include "src/gpu/ganesh/GrProcessorUnitTest.h"
#include "src/gpu/ganesh/GrProgramInfo.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/GrSimpleMesh.h"
#include "src/gpu/ganesh/GrStyle.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/geometry/GrPathUtils.h"
#include "src/gpu/ganesh/geometry/GrStyledShape.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLVarying.h"
#include "src/gpu/ganesh/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/ganesh/ops/GrMeshDrawOp.h"
#include "src/gpu/ganesh/ops/GrOp.h"
#include "src/gpu/ganesh/ops/GrSimpleMeshDrawOpHelperWithStencil.h"

#if defined(GPU_TEST_UTILS)
#include "src/base/SkRandom.h"
#include "src/gpu/ganesh/GrTestUtils.h"
#endif

#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

class GrDstProxyView;
class GrGLSLProgramDataManager;
class GrGLSLUniformHandler;
class GrSurfaceProxyView;
enum class GrXferBarrierFlags;
struct GrUserStencilSettings;
struct SkRect;

using namespace skia_private;

namespace skgpu::ganesh {

namespace {

struct Segment {
    enum {
        kLine = 0,
        kQuad = 1,
    } fType;
    // These enum values are assumed in member functions below.
    static_assert(0 == kLine && 1 == kQuad);

    // line uses one pt, quad uses 2 pts
    SkPoint fPts[2];
    // normal to edge ending at each pt
    SkVector fNorms[2];
    // is the corner where the previous segment meets this segment
    // sharp. If so, fMid is a normalized bisector facing outward.
    SkVector fMid;

    int countPoints() const {
        SkASSERT(fType == kLine || fType == kQuad);
        return fType + 1;
    }
    const SkPoint& endPt() const {
        SkASSERT(fType == kLine || fType == kQuad);
        return fPts[fType];
    }
    const SkPoint& endNorm() const {
        SkASSERT(fType == kLine || fType == kQuad);
        return fNorms[fType];
    }
};

typedef TArray<Segment, true> SegmentArray;

bool center_of_mass(const SegmentArray& segments, SkPoint* c) {
    SkScalar area = 0;
    SkPoint center = {0, 0};
    int count = segments.size();
    if (count <= 0) {
        return false;
    }
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
    return !SkIsNaN(c->fX) && !SkIsNaN(c->fY) && c->isFinite();
}

bool compute_vectors(SegmentArray* segments,
                     SkPoint* fanPt,
                     SkPathFirstDirection dir,
                     int* vCount,
                     int* iCount) {
    if (!center_of_mass(*segments, fanPt)) {
        return false;
    }
    int count = segments->size();

    // Make the normals point towards the outside
    SkPointPriv::Side normSide;
    if (dir == SkPathFirstDirection::kCCW) {
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
    bool isDegenerate() const { return Stage::kNonDegenerate != fStage; }
    enum class Stage {
        kInitial,
        kPoint,
        kLine,
        kNonDegenerate,
    };
    Stage       fStage = Stage::kInitial;
    SkPoint     fFirstPoint;
    SkVector    fLineNormal;
    SkScalar    fLineC;
};

static const SkScalar kClose = (SK_Scalar1 / 16);
static const SkScalar kCloseSqd = kClose * kClose;

void update_degenerate_test(DegenerateTestData* data, const SkPoint& pt) {
    switch (data->fStage) {
        case DegenerateTestData::Stage::kInitial:
            data->fFirstPoint = pt;
            data->fStage = DegenerateTestData::Stage::kPoint;
            break;
        case DegenerateTestData::Stage::kPoint:
            if (SkPointPriv::DistanceToSqd(pt, data->fFirstPoint) > kCloseSqd) {
                data->fLineNormal = pt - data->fFirstPoint;
                data->fLineNormal.normalize();
                data->fLineNormal = SkPointPriv::MakeOrthog(data->fLineNormal);
                data->fLineC = -data->fLineNormal.dot(data->fFirstPoint);
                data->fStage = DegenerateTestData::Stage::kLine;
            }
            break;
        case DegenerateTestData::Stage::kLine:
            if (SkScalarAbs(data->fLineNormal.dot(pt) + data->fLineC) > kClose) {
                data->fStage = DegenerateTestData::Stage::kNonDegenerate;
            }
            break;
        case DegenerateTestData::Stage::kNonDegenerate:
            break;
        default:
            SK_ABORT("Unexpected degenerate test stage.");
    }
}

inline bool get_direction(const SkPath& path, const SkMatrix& m, SkPathFirstDirection* dir) {
    // At this point, we've already returned true from canDraw(), which checked that the path's
    // direction could be determined, so this should just be fetching the cached direction.
    // However, if perspective is involved, we're operating on a transformed path, which may no
    // longer have a computable direction.
    *dir = SkPathPriv::ComputeFirstDirection(path);
    if (*dir == SkPathFirstDirection::kUnknown) {
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

inline void add_line_to_segment(const SkPoint& pt, SegmentArray* segments) {
    segments->push_back();
    segments->back().fType = Segment::kLine;
    segments->back().fPts[0] = pt;
}

inline void add_quad_segment(const SkPoint pts[3], SegmentArray* segments) {
    if (SkPointPriv::DistanceToLineSegmentBetweenSqd(pts[1], pts[0], pts[2]) < kCloseSqd) {
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

inline void add_cubic_segments(const SkPoint pts[4],
                               SkPathFirstDirection dir,
                               SegmentArray* segments) {
    STArray<15, SkPoint, true> quads;
    GrPathUtils::convertCubicToQuadsConstrainToTangents(pts, SK_Scalar1, dir, &quads);
    int count = quads.size();
    for (int q = 0; q < count; q += 3) {
        add_quad_segment(&quads[q], segments);
    }
}

bool get_segments(const SkPath& path,
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
    SkPathFirstDirection dir;
    if (!get_direction(path, m, &dir)) {
        return false;
    }

    for (;;) {
        SkPoint pts[4];
        SkPath::Verb verb = iter.next(pts);
        switch (verb) {
            case SkPath::kMove_Verb:
                m.mapPoints({pts, 1});
                update_degenerate_test(&degenerateData, pts[0]);
                break;
            case SkPath::kLine_Verb: {
                if (!SkPathPriv::AllPointsEq(pts, 2)) {
                    m.mapPoints({&pts[1], 1});
                    update_degenerate_test(&degenerateData, pts[1]);
                    add_line_to_segment(pts[1], segments);
                }
                break;
            }
            case SkPath::kQuad_Verb:
                if (!SkPathPriv::AllPointsEq(pts, 3)) {
                    m.mapPoints({pts, 3});
                    update_degenerate_test(&degenerateData, pts[1]);
                    update_degenerate_test(&degenerateData, pts[2]);
                    add_quad_segment(pts, segments);
                }
                break;
            case SkPath::kConic_Verb: {
                if (!SkPathPriv::AllPointsEq(pts, 3)) {
                    m.mapPoints({pts, 3});
                    SkScalar weight = iter.conicWeight();
                    SkAutoConicToQuads converter;
                    const SkPoint* quadPts = converter.computeQuads(pts, weight, 0.25f);
                    for (int i = 0; i < converter.countQuads(); ++i) {
                        update_degenerate_test(&degenerateData, quadPts[2*i + 1]);
                        update_degenerate_test(&degenerateData, quadPts[2*i + 2]);
                        add_quad_segment(quadPts + 2*i, segments);
                    }
                }
                break;
            }
            case SkPath::kCubic_Verb: {
                if (!SkPathPriv::AllPointsEq(pts, 4)) {
                    m.mapPoints({pts, 4});
                    update_degenerate_test(&degenerateData, pts[1]);
                    update_degenerate_test(&degenerateData, pts[2]);
                    update_degenerate_test(&degenerateData, pts[3]);
                    add_cubic_segments(pts, dir, segments);
                }
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

typedef TArray<Draw, true> DrawArray;

void create_vertices(const SegmentArray& segments,
                     const SkPoint& fanPt,
                     const VertexColor& color,
                     DrawArray* draws,
                     VertexWriter& verts,
                     uint16_t* idxs,
                     size_t vertexStride) {
    Draw* draw = &draws->push_back();
    // alias just to make vert/index assignments easier to read.
    int* v = &draw->fVertexCnt;
    int* i = &draw->fIndexCnt;

    int count = segments.size();
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
        verts << p0                    << color << SkPoint{0, 0}           << negOneDists;
        verts << (p0 + sega.endNorm()) << color << SkPoint{0, -SK_Scalar1} << negOneDists;
        verts << (p0 + segb.fMid)      << color << SkPoint{0, -SK_Scalar1} << negOneDists;
        verts << (p0 + segb.fNorms[0]) << color << SkPoint{0, -SK_Scalar1} << negOneDists;

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

            verts << fanPt                    << color << SkPoint{0, dist}        << negOneDists;
            verts << v1Pos                    << color << SkPoint{0, 0}           << negOneDists;
            verts << v2Pos                    << color << SkPoint{0, 0}           << negOneDists;
            verts << (v1Pos + segb.fNorms[0]) << color << SkPoint{0, -SK_Scalar1} << negOneDists;
            verts << (v2Pos + segb.fNorms[0]) << color << SkPoint{0, -SK_Scalar1} << negOneDists;

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

            SkScalar c0 = segb.fNorms[0].dot(qpts[0]);
            SkScalar c1 = segb.fNorms[1].dot(qpts[2]);

            // We must transform the positions into UV in cpu memory and then copy them to the gpu
            // buffer. If we write the position first into the gpu buffer then calculate the UVs, it
            // will cause us to read from the GPU buffer which can be very slow.
            struct PosAndUV {
                SkPoint fPos;
                SkPoint fUV;
            };
            PosAndUV posAndUVPoints[6];
            posAndUVPoints[0].fPos = fanPt;
            posAndUVPoints[1].fPos = qpts[0];
            posAndUVPoints[2].fPos = qpts[2];
            posAndUVPoints[3].fPos = qpts[0] + segb.fNorms[0];
            posAndUVPoints[4].fPos = qpts[2] + segb.fNorms[1];
            SkVector midVec = segb.fNorms[0] + segb.fNorms[1];
            midVec.normalize();
            posAndUVPoints[5].fPos = qpts[1] + midVec;

            GrPathUtils::QuadUVMatrix toUV(qpts);
            toUV.apply(posAndUVPoints, 6, sizeof(PosAndUV), sizeof(SkPoint));

            verts << posAndUVPoints[0].fPos << color << posAndUVPoints[0].fUV
                  << (-segb.fNorms[0].dot(fanPt) + c0)
                  << (-segb.fNorms[1].dot(fanPt) + c1);

            verts << posAndUVPoints[1].fPos << color << posAndUVPoints[1].fUV
                  << 0.0f
                  << (-segb.fNorms[1].dot(qpts[0]) + c1);

            verts << posAndUVPoints[2].fPos << color << posAndUVPoints[2].fUV
                  << (-segb.fNorms[0].dot(qpts[2]) + c0)
                  << 0.0f;
            // We need a negative value that is very large that it won't effect results if it is
            // interpolated with. However, the value can't be too large of a negative that it
            // effects numerical precision on less powerful GPUs.
            static const SkScalar kStableLargeNegativeValue = -SK_ScalarMax/1000000;
            verts << posAndUVPoints[3].fPos << color << posAndUVPoints[3].fUV
                  << kStableLargeNegativeValue
                  << kStableLargeNegativeValue;

            verts << posAndUVPoints[4].fPos << color << posAndUVPoints[4].fUV
                  << kStableLargeNegativeValue
                  << kStableLargeNegativeValue;

            verts << posAndUVPoints[5].fPos << color << posAndUVPoints[5].fUV
                  << kStableLargeNegativeValue
                  << kStableLargeNegativeValue;

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
    static GrGeometryProcessor* Make(SkArenaAlloc* arena,
                                     const SkMatrix& localMatrix,
                                     bool usesLocalCoords,
                                     bool wideColor) {
        return arena->make([&](void* ptr) {
            return new (ptr) QuadEdgeEffect(localMatrix, usesLocalCoords, wideColor);
        });
    }

    ~QuadEdgeEffect() override {}

    const char* name() const override { return "QuadEdge"; }

    void addToKey(const GrShaderCaps& caps, KeyBuilder* b) const override {
        b->addBool(fUsesLocalCoords, "usesLocalCoords");
        b->addBits(ProgramImpl::kMatrixKeyBits,
                   ProgramImpl::ComputeMatrixKey(caps, fLocalMatrix),
                   "localMatrixType");
    }

    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const override;

private:
    QuadEdgeEffect(const SkMatrix& localMatrix, bool usesLocalCoords, bool wideColor)
            : INHERITED(kQuadEdgeEffect_ClassID)
            , fLocalMatrix(localMatrix)
            , fUsesLocalCoords(usesLocalCoords) {
        fInPosition = {"inPosition", kFloat2_GrVertexAttribType, SkSLType::kFloat2};
        fInColor = MakeColorAttribute("inColor", wideColor);
        // GL on iOS 14 needs more precision for the quadedge attributes
        fInQuadEdge = {"inQuadEdge", kFloat4_GrVertexAttribType, SkSLType::kFloat4};
        this->setVertexAttributesWithImplicitOffsets(&fInPosition, 3);
    }

    Attribute fInPosition;
    Attribute fInColor;
    Attribute fInQuadEdge;

    SkMatrix fLocalMatrix;
    bool fUsesLocalCoords;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST

    using INHERITED = GrGeometryProcessor;
};

std::unique_ptr<GrGeometryProcessor::ProgramImpl> QuadEdgeEffect::makeProgramImpl(
        const GrShaderCaps&) const {
    class Impl : public ProgramImpl {
    public:
        void setData(const GrGLSLProgramDataManager& pdman,
                     const GrShaderCaps& shaderCaps,
                     const GrGeometryProcessor& geomProc) override {
            const QuadEdgeEffect& qe = geomProc.cast<QuadEdgeEffect>();
            SetTransform(pdman, shaderCaps, fLocalMatrixUniform, qe.fLocalMatrix, &fLocalMatrix);
        }

    private:
        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
            const QuadEdgeEffect& qe = args.fGeomProc.cast<QuadEdgeEffect>();
            GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
            GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
            GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            // emit attributes
            varyingHandler->emitAttributes(qe);

            // GL on iOS 14 needs more precision for the quadedge attributes
            // We might as well enable it everywhere
            GrGLSLVarying v(SkSLType::kFloat4);
            varyingHandler->addVarying("QuadEdge", &v);
            vertBuilder->codeAppendf("%s = %s;", v.vsOut(), qe.fInQuadEdge.name());

            // Setup pass through color
            fragBuilder->codeAppendf("half4 %s;", args.fOutputColor);
            varyingHandler->addPassThroughAttribute(qe.fInColor.asShaderVar(), args.fOutputColor);

            // Setup position
            WriteOutputPosition(vertBuilder, gpArgs, qe.fInPosition.name());
            if (qe.fUsesLocalCoords) {
                WriteLocalCoord(vertBuilder,
                                uniformHandler,
                                *args.fShaderCaps,
                                gpArgs,
                                qe.fInPosition.asShaderVar(),
                                qe.fLocalMatrix,
                                &fLocalMatrixUniform);
            }

            fragBuilder->codeAppendf("half edgeAlpha;");

            // keep the derivative instructions outside the conditional
            fragBuilder->codeAppendf("half2 duvdx = half2(dFdx(%s.xy));", v.fsIn());
            fragBuilder->codeAppendf("half2 duvdy = half2(dFdy(%s.xy));", v.fsIn());
            fragBuilder->codeAppendf("if (%s.z > 0.0 && %s.w > 0.0) {", v.fsIn(), v.fsIn());
            // today we know z and w are in device space. We could use derivatives
            fragBuilder->codeAppendf("edgeAlpha = half(min(min(%s.z, %s.w) + 0.5, 1.0));", v.fsIn(),
                                     v.fsIn());
            fragBuilder->codeAppendf ("} else {");
            fragBuilder->codeAppendf("half2 gF = half2(half(2.0*%s.x*duvdx.x - duvdx.y),"
                                     "                 half(2.0*%s.x*duvdy.x - duvdy.y));",
                                     v.fsIn(), v.fsIn());
            fragBuilder->codeAppendf("edgeAlpha = half(%s.x*%s.x - %s.y);", v.fsIn(), v.fsIn(),
                                     v.fsIn());
            fragBuilder->codeAppendf("edgeAlpha = "
                                     "saturate(0.5 - edgeAlpha / length(gF));}");

            fragBuilder->codeAppendf("half4 %s = half4(edgeAlpha);", args.fOutputCoverage);
        }

    private:
        SkMatrix fLocalMatrix = SkMatrix::InvalidMatrix();

        UniformHandle fLocalMatrixUniform;
    };

    return std::make_unique<Impl>();
}

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(QuadEdgeEffect)

#if defined(GPU_TEST_UTILS)
GrGeometryProcessor* QuadEdgeEffect::TestCreate(GrProcessorTestData* d) {
    SkMatrix localMatrix = GrTest::TestMatrix(d->fRandom);
    bool usesLocalCoords = d->fRandom->nextBool();
    bool wideColor = d->fRandom->nextBool();
    // Doesn't work without derivative instructions.
    return d->caps()->shaderCaps()->fShaderDerivativeSupport
                   ? QuadEdgeEffect::Make(d->allocator(), localMatrix, usesLocalCoords, wideColor)
                   : nullptr;
}
#endif

class AAConvexPathOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelperWithStencil;

public:
    DEFINE_OP_CLASS_ID

    static GrOp::Owner Make(GrRecordingContext* context,
                            GrPaint&& paint,
                            const SkMatrix& viewMatrix,
                            const SkPath& path,
                            const GrUserStencilSettings* stencilSettings) {
        return Helper::FactoryHelper<AAConvexPathOp>(context, std::move(paint), viewMatrix, path,
                                                     stencilSettings);
    }

    AAConvexPathOp(GrProcessorSet* processorSet, const SkPMColor4f& color,
                   const SkMatrix& viewMatrix, const SkPath& path,
                   const GrUserStencilSettings* stencilSettings)
            : INHERITED(ClassID()), fHelper(processorSet, GrAAType::kCoverage, stencilSettings) {
        fPaths.emplace_back(PathData{viewMatrix, path, color});
        this->setTransformedBounds(path.getBounds(), viewMatrix, HasAABloat::kYes,
                                   IsHairline::kNo);
    }

    const char* name() const override { return "AAConvexPathOp"; }

    void visitProxies(const GrVisitProxyFunc& func) const override {
        if (fProgramInfo) {
            fProgramInfo->visitFPProxies(func);
        } else {
            fHelper.visitProxies(func);
        }
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

    GrProcessorSet::Analysis finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                      GrClampType clampType) override {
        return fHelper.finalizeProcessors(
                caps, clip, clampType, GrProcessorAnalysisCoverage::kSingleChannel,
                &fPaths.back().fColor, &fWideColor);
    }

private:
    GrProgramInfo* programInfo() override { return fProgramInfo; }

    void onCreateProgramInfo(const GrCaps* caps,
                             SkArenaAlloc* arena,
                             const GrSurfaceProxyView& writeView,
                             bool usesMSAASurface,
                             GrAppliedClip&& appliedClip,
                             const GrDstProxyView& dstProxyView,
                             GrXferBarrierFlags renderPassXferBarriers,
                             GrLoadOp colorLoadOp) override {
        SkMatrix invert;
        if (fHelper.usesLocalCoords() && !fPaths.back().fViewMatrix.invert(&invert)) {
            return;
        }

        GrGeometryProcessor* quadProcessor = QuadEdgeEffect::Make(arena, invert,
                                                                  fHelper.usesLocalCoords(),
                                                                  fWideColor);

        fProgramInfo = fHelper.createProgramInfoWithStencil(caps, arena, writeView, usesMSAASurface,
                                                            std::move(appliedClip),
                                                            dstProxyView, quadProcessor,
                                                            GrPrimitiveType::kTriangles,
                                                            renderPassXferBarriers, colorLoadOp);
    }

    void onPrepareDraws(GrMeshDrawTarget* target) override {
        int instanceCount = fPaths.size();

        if (!fProgramInfo) {
            this->createProgramInfo(target);
            if (!fProgramInfo) {
                return;
            }
        }

        const size_t kVertexStride = fProgramInfo->geomProc().vertexStride();

        fDraws.reserve(instanceCount);

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
            static constexpr size_t kPreallocSegmentCnt = 512 / sizeof(Segment);
            static constexpr size_t kPreallocDrawCnt = 4;

            STArray<kPreallocSegmentCnt, Segment, true> segments;
            SkPoint fanPt;

            if (!get_segments(*pathPtr, *viewMatrix, &segments, &fanPt, &vertexCount,
                              &indexCount)) {
                continue;
            }

            sk_sp<const GrBuffer> vertexBuffer;
            int firstVertex;

            VertexWriter verts = target->makeVertexWriter(kVertexStride,
                                                          vertexCount,
                                                          &vertexBuffer,
                                                          &firstVertex);

            if (!verts) {
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

            STArray<kPreallocDrawCnt, Draw, true> draws;
            VertexColor color(args.fColor, fWideColor);
            create_vertices(segments, fanPt, color, &draws, verts, idxs, kVertexStride);

            GrSimpleMesh* meshes = target->allocMeshes(draws.size());
            for (int j = 0; j < draws.size(); ++j) {
                const Draw& draw = draws[j];
                meshes[j].setIndexed(indexBuffer, draw.fIndexCnt, firstIndex, 0,
                                     draw.fVertexCnt - 1, GrPrimitiveRestart::kNo, vertexBuffer,
                                     firstVertex);
                firstIndex += draw.fIndexCnt;
                firstVertex += draw.fVertexCnt;
            }

            fDraws.push_back({ meshes, draws.size() });
        }
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        if (!fProgramInfo || fDraws.empty()) {
            return;
        }

        flushState->bindPipelineAndScissorClip(*fProgramInfo, chainBounds);
        flushState->bindTextures(fProgramInfo->geomProc(), nullptr, fProgramInfo->pipeline());
        for (int i = 0; i < fDraws.size(); ++i) {
            for (int j = 0; j < fDraws[i].fMeshCount; ++j) {
                flushState->drawMesh(fDraws[i].fMeshes[j]);
            }
        }
    }

    CombineResult onCombineIfPossible(GrOp* t, SkArenaAlloc*, const GrCaps& caps) override {
        AAConvexPathOp* that = t->cast<AAConvexPathOp>();
        if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
            return CombineResult::kCannotCombine;
        }
        if (fHelper.usesLocalCoords() &&
            !SkMatrixPriv::CheapEqual(fPaths[0].fViewMatrix, that->fPaths[0].fViewMatrix)) {
            return CombineResult::kCannotCombine;
        }

        fPaths.push_back_n(that->fPaths.size(), that->fPaths.begin());
        fWideColor |= that->fWideColor;
        return CombineResult::kMerged;
    }

#if defined(GPU_TEST_UTILS)
    SkString onDumpInfo() const override {
        return SkStringPrintf("Count: %d\n%s", fPaths.size(), fHelper.dumpInfo().c_str());
    }
#endif

    struct PathData {
        SkMatrix    fViewMatrix;
        SkPath      fPath;
        SkPMColor4f fColor;
    };

    Helper fHelper;
    STArray<1, PathData, true> fPaths;
    bool fWideColor;

    struct MeshDraw {
        GrSimpleMesh* fMeshes;
        int fMeshCount;
    };

    SkTDArray<MeshDraw> fDraws;
    GrProgramInfo*      fProgramInfo = nullptr;

    using INHERITED = GrMeshDrawOp;
};

} // anonymous namespace

///////////////////////////////////////////////////////////////////////////////

PathRenderer::CanDrawPath AAConvexPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    // This check requires convexity and known direction, since the direction is used to build
    // the geometry segments. Degenerate convex paths will fall through to some other path renderer.
    if (args.fCaps->shaderCaps()->fShaderDerivativeSupport &&
        (GrAAType::kCoverage == args.fAAType) && args.fShape->style().isSimpleFill() &&
        !args.fShape->inverseFilled() && args.fShape->knownToBeConvex() &&
        args.fShape->knownDirection()) {
        return CanDrawPath::kYes;
    }
    return CanDrawPath::kNo;
}

bool AAConvexPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fContext->priv().auditTrail(),
                              "AAConvexPathRenderer::onDrawPath");
    SkASSERT(args.fSurfaceDrawContext->numSamples() <= 1);
    SkASSERT(!args.fShape->isEmpty());

    SkPath path;
    args.fShape->asPath(&path);

    GrOp::Owner op = AAConvexPathOp::Make(args.fContext, std::move(args.fPaint),
                                          *args.fViewMatrix,
                                          path, args.fUserStencilSettings);
    args.fSurfaceDrawContext->addDrawOp(args.fClip, std::move(op));
    return true;
}

}  // namespace skgpu::ganesh

#if defined(GPU_TEST_UTILS)

GR_DRAW_OP_TEST_DEFINE(AAConvexPathOp) {
    SkMatrix viewMatrix = GrTest::TestMatrixInvertible(random);
    const SkPath& path = GrTest::TestPathConvex(random);
    const GrUserStencilSettings* stencilSettings = GrGetRandomStencil(random, context);
    return skgpu::ganesh::AAConvexPathOp::Make(
            context, std::move(paint), viewMatrix, path, stencilSettings);
}

#endif
