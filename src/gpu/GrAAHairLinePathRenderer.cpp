
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAAHairLinePathRenderer.h"

#include "GrContext.h"
#include "GrDrawState.h"
#include "GrGpu.h"
#include "GrIndexBuffer.h"
#include "GrPathUtils.h"
#include "SkGeometry.h"
#include "SkTemplates.h"

namespace {
// quadratics are rendered as 5-sided polys in order to bound the
// AA stroke around the center-curve. See comments in push_quad_index_buffer and
// bloat_quad.
static const int kVertsPerQuad = 5;
static const int kIdxsPerQuad = 9;

static const int kVertsPerLineSeg = 4;
static const int kIdxsPerLineSeg = 6;

static const int kNumQuadsInIdxBuffer = 256;
static const size_t kQuadIdxSBufize = kIdxsPerQuad *
                                      sizeof(uint16_t) *
                                      kNumQuadsInIdxBuffer;

bool push_quad_index_data(GrIndexBuffer* qIdxBuffer) {
    uint16_t* data = (uint16_t*) qIdxBuffer->lock();
    bool tempData = NULL == data;
    if (tempData) {
        data = new uint16_t[kNumQuadsInIdxBuffer * kIdxsPerQuad];
    }
    for (int i = 0; i < kNumQuadsInIdxBuffer; ++i) {

        // Each quadratic is rendered as a five sided polygon. This poly bounds
        // the quadratic's bounding triangle but has been expanded so that the
        // 1-pixel wide area around the curve is inside the poly.
        // If a,b,c are the original control points then the poly a0,b0,c0,c1,a1
        // that is rendered would look like this:
        //              b0
        //              b
        //
        //     a0              c0
        //      a            c
        //       a1       c1
        // Each is drawn as three triagnles specified by these 9 indices:
        int baseIdx = i * kIdxsPerQuad;
        uint16_t baseVert = (uint16_t)(i * kVertsPerQuad);
        data[0 + baseIdx] = baseVert + 0; // a0
        data[1 + baseIdx] = baseVert + 1; // a1
        data[2 + baseIdx] = baseVert + 2; // b0
        data[3 + baseIdx] = baseVert + 2; // b0
        data[4 + baseIdx] = baseVert + 4; // c1
        data[5 + baseIdx] = baseVert + 3; // c0
        data[6 + baseIdx] = baseVert + 1; // a1
        data[7 + baseIdx] = baseVert + 4; // c1
        data[8 + baseIdx] = baseVert + 2; // b0
    }
    if (tempData) {
        bool ret = qIdxBuffer->updateData(data, kQuadIdxSBufize);
        delete[] data;
        return ret;
    } else {
        qIdxBuffer->unlock();
        return true;
    }
}
}

GrPathRenderer* GrAAHairLinePathRenderer::Create(GrContext* context) {
    const GrIndexBuffer* lIdxBuffer = context->getQuadIndexBuffer();
    if (NULL == lIdxBuffer) {
        return NULL;
    }
    GrGpu* gpu = context->getGpu();
    GrIndexBuffer* qIdxBuf = gpu->createIndexBuffer(kQuadIdxSBufize, false);
    SkAutoTUnref<GrIndexBuffer> qIdxBuffer(qIdxBuf);
    if (NULL == qIdxBuf ||
        !push_quad_index_data(qIdxBuf)) {
        return NULL;
    }
    return new GrAAHairLinePathRenderer(context,
                                        lIdxBuffer,
                                        qIdxBuf);
}

GrAAHairLinePathRenderer::GrAAHairLinePathRenderer(
                                        const GrContext* context,
                                        const GrIndexBuffer* linesIndexBuffer,
                                        const GrIndexBuffer* quadsIndexBuffer) {
    fLinesIndexBuffer = linesIndexBuffer;
    linesIndexBuffer->ref();
    fQuadsIndexBuffer = quadsIndexBuffer;
    quadsIndexBuffer->ref();
    this->resetGeom();
}

GrAAHairLinePathRenderer::~GrAAHairLinePathRenderer() {
    fLinesIndexBuffer->unref();
    fQuadsIndexBuffer->unref();
}

bool GrAAHairLinePathRenderer::canDrawPath(const GrDrawTarget::Caps& targetCaps,
                                           const SkPath& path,
                                           GrPathFill fill,
                                           bool antiAlias) const {
    static const uint32_t gReqDerivMask = SkPath::kCubic_SegmentMask |
                                          SkPath::kQuad_SegmentMask;
    return (kHairLine_PathFill == fill &&
            antiAlias &&
            (targetCaps.fShaderDerivativeSupport ||
             !(gReqDerivMask & path.getSegmentMasks())));
}

void GrAAHairLinePathRenderer::pathWillClear() {
    this->resetGeom();
}

void GrAAHairLinePathRenderer::resetGeom() {
    fPreviousStages = ~0;
    fPreviousRTHeight = ~0;
    fPreviousViewMatrix = GrMatrix::InvalidMatrix();
    fLineSegmentCnt = 0;
    fQuadCnt = 0; 
    if ((fQuadCnt || fLineSegmentCnt) && NULL != fTarget) {
        fTarget->resetVertexSource();
    }
}

namespace {

typedef SkTArray<SkPoint, true> PtArray;
#define PREALLOC_PTARRAY(N) SkSTArray<(N),SkPoint, true>
typedef SkTArray<int, true> IntArray;

/**
 * We convert cubics to quadratics (for now).
 */
void convert_noninflect_cubic_to_quads(const SkPoint p[4],
                                       SkScalar tolScale,
                                       PtArray* quads,
                                       int sublevel = 0) {
    SkVector ab = p[1];
    ab -= p[0];
    SkVector dc = p[2];
    dc -= p[3];

    static const SkScalar gLengthScale = 3 * SK_Scalar1 / 2;
    // base tolerance is 2 pixels in dev coords.
    const SkScalar distanceSqdTol = SkScalarMul(tolScale, 2 * SK_Scalar1);
    static const int kMaxSubdivs = 10;

    ab.scale(gLengthScale);
    dc.scale(gLengthScale);

    SkVector c0 = p[0];
    c0 += ab;
    SkVector c1 = p[3];
    c1 += dc;

    SkScalar dSqd = c0.distanceToSqd(c1);
    if (sublevel > kMaxSubdivs || dSqd <= distanceSqdTol) {
        SkPoint cAvg = c0;
        cAvg += c1;
        cAvg.scale(SK_ScalarHalf);

        SkPoint* pts = quads->push_back_n(3);
        pts[0] = p[0];
        pts[1] = cAvg;
        pts[2] = p[3];

        return;
    } else {
        SkPoint choppedPts[7];
        SkChopCubicAtHalf(p, choppedPts);
        convert_noninflect_cubic_to_quads(choppedPts + 0, tolScale, 
                                          quads, sublevel + 1);
        convert_noninflect_cubic_to_quads(choppedPts + 3, tolScale,
                                          quads, sublevel + 1);
    }
}

void convert_cubic_to_quads(const SkPoint p[4],
                            SkScalar tolScale,
                            PtArray* quads) {
    SkPoint chopped[13];
    int count = SkChopCubicAtInflections(p, chopped);

    for (int i = 0; i < count; ++i) {
        SkPoint* cubic = chopped + 3*i;
        convert_noninflect_cubic_to_quads(cubic, tolScale, quads);
    }
}

// Takes 178th time of logf on Z600 / VC2010
int get_float_exp(float x) {
    GR_STATIC_ASSERT(sizeof(int) == sizeof(float));
#if GR_DEBUG
    static bool tested;
    if (!tested) {
        tested = true;
        GrAssert(get_float_exp(0.25f) == -2);
        GrAssert(get_float_exp(0.3f) == -2);
        GrAssert(get_float_exp(0.5f) == -1);
        GrAssert(get_float_exp(1.f) == 0);
        GrAssert(get_float_exp(2.f) == 1);
        GrAssert(get_float_exp(2.5f) == 1);
        GrAssert(get_float_exp(8.f) == 3);
        GrAssert(get_float_exp(100.f) == 6);
        GrAssert(get_float_exp(1000.f) == 9);
        GrAssert(get_float_exp(1024.f) == 10);
        GrAssert(get_float_exp(3000000.f) == 21);
    }
#endif
    const int* iptr = (const int*)&x;
    return (((*iptr) & 0x7f800000) >> 23) - 127;
}

// we subdivide the quads to avoid huge overfill
// if it returns -1 then should be drawn as lines
int num_quad_subdivs(const SkPoint p[3]) {
    static const SkScalar gDegenerateToLineTol = SK_Scalar1;
    static const SkScalar gDegenerateToLineTolSqd = 
        SkScalarMul(gDegenerateToLineTol, gDegenerateToLineTol);

    if (p[0].distanceToSqd(p[1]) < gDegenerateToLineTolSqd ||
        p[1].distanceToSqd(p[2]) < gDegenerateToLineTolSqd) {
        return -1;
    }

    GrScalar dsqd = p[1].distanceToLineBetweenSqd(p[0], p[2]);
    if (dsqd < gDegenerateToLineTolSqd) {
        return -1;
    }

    if (p[2].distanceToLineBetweenSqd(p[1], p[0]) < gDegenerateToLineTolSqd) {
        return -1;
    }

    static const int kMaxSub = 4;
    // tolerance of triangle height in pixels
    // tuned on windows  Quadro FX 380 / Z600
    // trade off of fill vs cpu time on verts
    // maybe different when do this using gpu (geo or tess shaders)
    static const SkScalar gSubdivTol = 175 * SK_Scalar1;

    if (dsqd <= gSubdivTol*gSubdivTol) {
        return 0;
    } else {
        // subdividing the quad reduces d by 4. so we want x = log4(d/tol)
        // = log4(d*d/tol*tol)/2
        // = log2(d*d/tol*tol)

#ifdef SK_SCALAR_IS_FLOAT
        // +1 since we're ignoring the mantissa contribution.
        int log = get_float_exp(dsqd/(gSubdivTol*gSubdivTol)) + 1;
        log = GrMin(GrMax(0, log), kMaxSub);
        return log;
#else
        SkScalar log = SkScalarLog(SkScalarDiv(dsqd,gSubdivTol*gSubdivTol));
        static const SkScalar conv = SkScalarInvert(SkScalarLog(2));
        log = SkScalarMul(log, conv);
        return  GrMin(GrMax(0, SkScalarCeilToInt(log)),kMaxSub);
#endif
    }
}

/**
 * Generates the lines and quads to be rendered. Lines are always recorded in
 * device space. We will do a device space bloat to account for the 1pixel
 * thickness.
 * Quads are recorded in device space unless m contains
 * perspective, then in they are in src space. We do this because we will
 * subdivide large quads to reduce over-fill. This subdivision has to be
 * performed before applying the perspective matrix.
 */
int generate_lines_and_quads(const SkPath& path,
                             const SkMatrix& m,
                             const SkVector& translate,
                             GrIRect clip,
                             PtArray* lines,
                             PtArray* quads,
                             IntArray* quadSubdivCnts) {
    SkPath::Iter iter(path, false);

    int totalQuadCount = 0;
    GrRect bounds;
    GrIRect ibounds;

    bool persp = m.hasPerspective();

    for (;;) {
        GrPoint pts[4];
        GrPoint devPts[4];
        GrPathCmd cmd = (GrPathCmd)iter.next(pts);
        switch (cmd) {
            case kMove_PathCmd:
                break;
            case kLine_PathCmd:
                SkPoint::Offset(pts, 2, translate);
                m.mapPoints(devPts, pts, 2);
                bounds.setBounds(devPts, 2);
                bounds.outset(SK_Scalar1, SK_Scalar1);
                bounds.roundOut(&ibounds);
                if (SkIRect::Intersects(clip, ibounds)) {
                    SkPoint* pts = lines->push_back_n(2);
                    pts[0] = devPts[0];
                    pts[1] = devPts[1];
                }
                break;
            case kQuadratic_PathCmd:
                SkPoint::Offset(pts, 3, translate);
                m.mapPoints(devPts, pts, 3);
                bounds.setBounds(devPts, 3);
                bounds.outset(SK_Scalar1, SK_Scalar1);
                bounds.roundOut(&ibounds);
                if (SkIRect::Intersects(clip, ibounds)) {
                    int subdiv = num_quad_subdivs(devPts);
                    GrAssert(subdiv >= -1);
                    if (-1 == subdiv) {
                        SkPoint* pts = lines->push_back_n(4);
                        pts[0] = devPts[0];
                        pts[1] = devPts[1];
                        pts[2] = devPts[1];
                        pts[3] = devPts[2];
                    } else {
                        // when in perspective keep quads in src space
                        SkPoint* qPts = persp ? pts : devPts;
                        SkPoint* pts = quads->push_back_n(3);
                        pts[0] = qPts[0];
                        pts[1] = qPts[1];
                        pts[2] = qPts[2];
                        quadSubdivCnts->push_back() = subdiv;
                        totalQuadCount += 1 << subdiv;
                    }
                }
            break;
            case kCubic_PathCmd:
                SkPoint::Offset(pts, 4, translate);
                m.mapPoints(devPts, pts, 4);
                bounds.setBounds(devPts, 4);
                bounds.outset(SK_Scalar1, SK_Scalar1);
                bounds.roundOut(&ibounds);
                if (SkIRect::Intersects(clip, ibounds)) {
                    PREALLOC_PTARRAY(32) q;
                    // in perspective have to do conversion in src space
                    if (persp) {
                        SkScalar tolScale = 
                            GrPathUtils::scaleToleranceToSrc(SK_Scalar1, m,
                                                             path.getBounds());
                        convert_cubic_to_quads(pts, tolScale, &q);
                    } else {
                        convert_cubic_to_quads(devPts, SK_Scalar1, &q);
                    }
                    for (int i = 0; i < q.count(); i += 3) {
                        SkPoint* qInDevSpace;
                        // bounds has to be calculated in device space, but q is
                        // in src space when there is perspective.
                        if (persp) {
                            m.mapPoints(devPts, &q[i], 3);
                            bounds.setBounds(devPts, 3);
                            qInDevSpace = devPts;
                        } else {
                            bounds.setBounds(&q[i], 3);
                            qInDevSpace = &q[i];
                        }
                        bounds.outset(SK_Scalar1, SK_Scalar1);
                        bounds.roundOut(&ibounds);
                        if (SkIRect::Intersects(clip, ibounds)) {
                            int subdiv = num_quad_subdivs(qInDevSpace);
                            GrAssert(subdiv >= -1);
                            if (-1 == subdiv) {
                                SkPoint* pts = lines->push_back_n(4);
                                // lines should always be in device coords
                                pts[0] = qInDevSpace[0];
                                pts[1] = qInDevSpace[1];
                                pts[2] = qInDevSpace[1];
                                pts[3] = qInDevSpace[2];
                            } else {
                                SkPoint* pts = quads->push_back_n(3);
                                // q is already in src space when there is no
                                // perspective and dev coords otherwise.
                                pts[0] = q[0 + i];
                                pts[1] = q[1 + i];
                                pts[2] = q[2 + i];
                                quadSubdivCnts->push_back() = subdiv;
                                totalQuadCount += 1 << subdiv;
                            }
                        }
                    }
                }
            break;
            case kClose_PathCmd:
                break;
            case kEnd_PathCmd:
                return totalQuadCount;
        }
    }
}

struct Vertex {
    GrPoint fPos;
    union {
        struct {
            GrScalar fA;
            GrScalar fB;
            GrScalar fC;
        } fLine;
        GrVec   fQuadCoord;
        struct {
            GrScalar fBogus[4];
        };
    };
};
GR_STATIC_ASSERT(sizeof(Vertex) == 3 * sizeof(GrPoint));

void intersect_lines(const SkPoint& ptA, const SkVector& normA,
                     const SkPoint& ptB, const SkVector& normB,
                     SkPoint* result) {

    SkScalar lineAW = -normA.dot(ptA);
    SkScalar lineBW = -normB.dot(ptB);

    SkScalar wInv = SkScalarMul(normA.fX, normB.fY) -
                    SkScalarMul(normA.fY, normB.fX);
    wInv = SkScalarInvert(wInv);

    result->fX = SkScalarMul(normA.fY, lineBW) - SkScalarMul(lineAW, normB.fY);
    result->fX = SkScalarMul(result->fX, wInv);
    
    result->fY = SkScalarMul(lineAW, normB.fX) - SkScalarMul(normA.fX, lineBW);
    result->fY = SkScalarMul(result->fY, wInv);
}

void bloat_quad(const SkPoint qpts[3], const GrMatrix* toDevice,
                const GrMatrix* toSrc, Vertex verts[kVertsPerQuad]) {
    GrAssert(!toDevice == !toSrc);
    // original quad is specified by tri a,b,c
    SkPoint a = qpts[0];
    SkPoint b = qpts[1];
    SkPoint c = qpts[2];

    // compute a matrix that goes from device coords to U,V quad params
    // this should be in the src space, not dev coords, when we have perspective
    SkMatrix DevToUV;
    DevToUV.setAll(a.fX,           b.fX,          c.fX,
                   a.fY,           b.fY,          c.fY,
                   SK_Scalar1,     SK_Scalar1,    SK_Scalar1);
    DevToUV.invert(&DevToUV);
    // can't make this static, no cons :(
    SkMatrix UVpts;
    UVpts.setAll(0,                 SK_ScalarHalf,  SK_Scalar1,
                 0,                 0,              SK_Scalar1,
                 SK_Scalar1,        SK_Scalar1,     SK_Scalar1);
    DevToUV.postConcat(UVpts);

    // We really want to avoid perspective matrix muls.
    // These may wind up really close to zero
    DevToUV.setPerspX(0);
    DevToUV.setPerspY(0);

    if (toDevice) {
        toDevice->mapPoints(&a, 1);
        toDevice->mapPoints(&b, 1);
        toDevice->mapPoints(&c, 1);
    }
    // make a new poly where we replace a and c by a 1-pixel wide edges orthog
    // to edges ab and bc:
    //
    //   before       |        after
    //                |              b0
    //         b      |
    //                |
    //                |     a0            c0
    // a         c    |        a1       c1
    //
    // edges a0->b0 and b0->c0 are parallel to original edges a->b and b->c,
    // respectively.
    Vertex& a0 = verts[0];
    Vertex& a1 = verts[1];
    Vertex& b0 = verts[2];
    Vertex& c0 = verts[3];
    Vertex& c1 = verts[4];

    SkVector ab = b;
    ab -= a;
    SkVector ac = c;
    ac -= a;
    SkVector cb = b;
    cb -= c;

    // We should have already handled degenerates
    GrAssert(ab.length() > 0 && cb.length() > 0);

    ab.normalize();
    SkVector abN;
    abN.setOrthog(ab, SkVector::kLeft_Side);
    if (abN.dot(ac) > 0) {
        abN.negate();
    }

    cb.normalize();
    SkVector cbN;
    cbN.setOrthog(cb, SkVector::kLeft_Side);
    if (cbN.dot(ac) < 0) {
        cbN.negate();
    }

    a0.fPos = a;
    a0.fPos += abN;
    a1.fPos = a;
    a1.fPos -= abN;

    c0.fPos = c;
    c0.fPos += cbN;
    c1.fPos = c;
    c1.fPos -= cbN;

    intersect_lines(a0.fPos, abN, c0.fPos, cbN, &b0.fPos);

    if (toSrc) {
        toSrc->mapPointsWithStride(&verts[0].fPos, sizeof(Vertex), kVertsPerQuad);
    }
    DevToUV.mapPointsWithStride(&verts[0].fQuadCoord,
                                &verts[0].fPos, sizeof(Vertex), kVertsPerQuad);
}

void add_quads(const SkPoint p[3],
               int subdiv,
               const GrMatrix* toDevice,
               const GrMatrix* toSrc,
               Vertex** vert) {
    GrAssert(subdiv >= 0);
    if (subdiv) {
        SkPoint newP[5];
        SkChopQuadAtHalf(p, newP);
        add_quads(newP + 0, subdiv-1, toDevice, toSrc, vert);
        add_quads(newP + 2, subdiv-1, toDevice, toSrc, vert);
    } else {
        bloat_quad(p, toDevice, toSrc, *vert);
        *vert += kVertsPerQuad;
    }
}

void add_line(const SkPoint p[2],
              int rtHeight,
              const SkMatrix* toSrc,
              Vertex** vert) {
    const SkPoint& a = p[0];
    const SkPoint& b = p[1];

    SkVector orthVec = b;
    orthVec -= a;

    if (orthVec.setLength(SK_Scalar1)) {
        orthVec.setOrthog(orthVec);

        // the values we pass down to the frag shader
        // have to be in y-points-up space;
        SkVector normal;
        normal.fX = orthVec.fX;
        normal.fY = -orthVec.fY;
        SkPoint aYDown;
        aYDown.fX = a.fX;
        aYDown.fY = rtHeight - a.fY;

        SkScalar lineC = -(aYDown.dot(normal));
        for (int i = 0; i < kVertsPerLineSeg; ++i) {
            (*vert)[i].fPos = (i < 2) ? a : b;
            if (0 == i || 3 == i) {
                (*vert)[i].fPos -= orthVec;
            } else {
                (*vert)[i].fPos += orthVec;
            }
            (*vert)[i].fLine.fA = normal.fX;
            (*vert)[i].fLine.fB = normal.fY;
            (*vert)[i].fLine.fC = lineC;
        }
        if (NULL != toSrc) {
            toSrc->mapPointsWithStride(&(*vert)->fPos,
                                       sizeof(Vertex),
                                       kVertsPerLineSeg);
        }
    } else {
        // just make it degenerate and likely offscreen
        (*vert)[0].fPos.set(SK_ScalarMax, SK_ScalarMax);
        (*vert)[1].fPos.set(SK_ScalarMax, SK_ScalarMax);
        (*vert)[2].fPos.set(SK_ScalarMax, SK_ScalarMax);
        (*vert)[3].fPos.set(SK_ScalarMax, SK_ScalarMax);
    }

    *vert += kVertsPerLineSeg;
}

}

bool GrAAHairLinePathRenderer::createGeom(GrDrawTarget::StageBitfield stages) {

    int rtHeight = fTarget->getRenderTarget()->height();

    GrIRect clip;
    if (fTarget->getClip().hasConservativeBounds()) {
        GrRect clipRect =  fTarget->getClip().getConservativeBounds();
        clipRect.roundOut(&clip);
    } else {
        clip.setLargest();
    }

    // If none of the inputs that affect generation of path geometry have
    // have changed since last previous path draw then we can reuse the
    // previous geoemtry.
    if (stages == fPreviousStages &&
        fPreviousViewMatrix == fTarget->getViewMatrix() &&
        fPreviousTranslate == fTranslate &&
        rtHeight == fPreviousRTHeight &&
        fClipRect == clip) {
        return true;
    }

    GrVertexLayout layout = GrDrawTarget::kEdge_VertexLayoutBit;
    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        if ((1 << s) & stages) {
            layout |= GrDrawTarget::StagePosAsTexCoordVertexLayoutBit(s);
        }
    }

    GrMatrix viewM = fTarget->getViewMatrix();

    PREALLOC_PTARRAY(128) lines;
    PREALLOC_PTARRAY(128) quads;
    IntArray qSubdivs;
    fQuadCnt = generate_lines_and_quads(*fPath, viewM, fTranslate, clip,
                                        &lines, &quads, &qSubdivs);

    fLineSegmentCnt = lines.count() / 2;
    int vertCnt = kVertsPerLineSeg * fLineSegmentCnt + kVertsPerQuad * fQuadCnt;

    GrAssert(sizeof(Vertex) == GrDrawTarget::VertexSize(layout));

    Vertex* verts;
    if (!fTarget->reserveVertexSpace(layout, vertCnt, (void**)&verts)) {
        return false;
    }
    Vertex* base = verts;

    const GrMatrix* toDevice = NULL;
    const GrMatrix* toSrc = NULL;
    GrMatrix ivm;

    if (viewM.hasPerspective()) {
        if (viewM.invert(&ivm)) {
            toDevice = &viewM;
            toSrc = &ivm;
        }
    }

    for (int i = 0; i < fLineSegmentCnt; ++i) {
        add_line(&lines[2*i], rtHeight, toSrc, &verts);
    }

    int unsubdivQuadCnt = quads.count() / 3;
    for (int i = 0; i < unsubdivQuadCnt; ++i) {
        GrAssert(qSubdivs[i] >= 0);
        add_quads(&quads[3*i], qSubdivs[i], toDevice, toSrc, &verts);
    }

    fPreviousStages = stages;
    fPreviousViewMatrix = fTarget->getViewMatrix();
    fPreviousRTHeight = rtHeight;
    fClipRect = clip;
    fPreviousTranslate = fTranslate;
    return true;
}

void GrAAHairLinePathRenderer::drawPath(GrDrawTarget::StageBitfield stages) {

    if (!this->createGeom(stages)) {
        return;
    }

    GrDrawTarget::AutoStateRestore asr;
    if (!fTarget->getViewMatrix().hasPerspective()) {
        asr.set(fTarget);
        GrMatrix ivm;
        if (fTarget->getViewInverse(&ivm)) {
            fTarget->preConcatSamplerMatrices(stages, ivm);
        }
        fTarget->setViewMatrix(GrMatrix::I());
    }

    // TODO: See whether rendering lines as degenerate quads improves perf
    // when we have a mix
    fTarget->setIndexSourceToBuffer(fLinesIndexBuffer);
    int lines = 0;
    int nBufLines = fLinesIndexBuffer->maxQuads();
    while (lines < fLineSegmentCnt) {
        int n = GrMin(fLineSegmentCnt-lines, nBufLines);
        fTarget->setVertexEdgeType(GrDrawState::kHairLine_EdgeType);
        fTarget->drawIndexed(kTriangles_PrimitiveType,
                             kVertsPerLineSeg*lines,    // startV
                             0,                         // startI
                             kVertsPerLineSeg*n,        // vCount
                             kIdxsPerLineSeg*n);        // iCount
        lines += n;
    }

    fTarget->setIndexSourceToBuffer(fQuadsIndexBuffer);
    int quads = 0;
    while (quads < fQuadCnt) {
        int n = GrMin(fQuadCnt-quads, kNumQuadsInIdxBuffer);
        fTarget->setVertexEdgeType(GrDrawState::kHairQuad_EdgeType);
        fTarget->drawIndexed(kTriangles_PrimitiveType,
                             4*fLineSegmentCnt + kVertsPerQuad*quads, // startV
                             0,                                       // startI
                             kVertsPerQuad*n,                         // vCount
                             kIdxsPerQuad*n);                         // iCount
        quads += n;
    }

}

