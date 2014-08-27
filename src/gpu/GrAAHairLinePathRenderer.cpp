/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAAHairLinePathRenderer.h"

#include "GrContext.h"
#include "GrDrawState.h"
#include "GrDrawTargetCaps.h"
#include "GrEffect.h"
#include "GrGpu.h"
#include "GrIndexBuffer.h"
#include "GrPathUtils.h"
#include "GrTBackendEffectFactory.h"
#include "SkGeometry.h"
#include "SkStroke.h"
#include "SkTemplates.h"

#include "effects/GrBezierEffect.h"

namespace {
// quadratics are rendered as 5-sided polys in order to bound the
// AA stroke around the center-curve. See comments in push_quad_index_buffer and
// bloat_quad. Quadratics and conics share an index buffer
static const int kVertsPerQuad = 5;
static const int kIdxsPerQuad = 9;

// lines are rendered as:
//      *______________*
//      |\ -_______   /|
//      | \        \ / |
//      |  *--------*  |
//      | /  ______/ \ |
//      */_-__________\*
// For: 6 vertices and 18 indices (for 6 triangles)
static const int kVertsPerLineSeg = 6;
static const int kIdxsPerLineSeg = 18;

static const int kNumQuadsInIdxBuffer = 256;
static const size_t kQuadIdxSBufize = kIdxsPerQuad *
                                      sizeof(uint16_t) *
                                      kNumQuadsInIdxBuffer;

static const int kNumLineSegsInIdxBuffer = 256;
static const size_t kLineSegIdxSBufize = kIdxsPerLineSeg *
                                         sizeof(uint16_t) *
                                         kNumLineSegsInIdxBuffer;

static bool push_quad_index_data(GrIndexBuffer* qIdxBuffer) {
    uint16_t* data = (uint16_t*) qIdxBuffer->map();
    bool tempData = NULL == data;
    if (tempData) {
        data = SkNEW_ARRAY(uint16_t, kNumQuadsInIdxBuffer * kIdxsPerQuad);
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
        // Each is drawn as three triangles specified by these 9 indices:
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
        qIdxBuffer->unmap();
        return true;
    }
}

static bool push_line_index_data(GrIndexBuffer* lIdxBuffer) {
    uint16_t* data = (uint16_t*) lIdxBuffer->map();
    bool tempData = NULL == data;
    if (tempData) {
        data = SkNEW_ARRAY(uint16_t, kNumLineSegsInIdxBuffer * kIdxsPerLineSeg);
    }
    for (int i = 0; i < kNumLineSegsInIdxBuffer; ++i) {
        // Each line segment is rendered as two quads and two triangles.
        // p0 and p1 have alpha = 1 while all other points have alpha = 0.
        // The four external points are offset 1 pixel perpendicular to the
        // line and half a pixel parallel to the line.
        //
        // p4                  p5
        //      p0         p1
        // p2                  p3
        //
        // Each is drawn as six triangles specified by these 18 indices:
        int baseIdx = i * kIdxsPerLineSeg;
        uint16_t baseVert = (uint16_t)(i * kVertsPerLineSeg);
        data[0 + baseIdx] = baseVert + 0;
        data[1 + baseIdx] = baseVert + 1;
        data[2 + baseIdx] = baseVert + 3;

        data[3 + baseIdx] = baseVert + 0;
        data[4 + baseIdx] = baseVert + 3;
        data[5 + baseIdx] = baseVert + 2;

        data[6 + baseIdx] = baseVert + 0;
        data[7 + baseIdx] = baseVert + 4;
        data[8 + baseIdx] = baseVert + 5;

        data[9 + baseIdx] = baseVert + 0;
        data[10+ baseIdx] = baseVert + 5;
        data[11+ baseIdx] = baseVert + 1;

        data[12 + baseIdx] = baseVert + 0;
        data[13 + baseIdx] = baseVert + 2;
        data[14 + baseIdx] = baseVert + 4;

        data[15 + baseIdx] = baseVert + 1;
        data[16 + baseIdx] = baseVert + 5;
        data[17 + baseIdx] = baseVert + 3;
    }
    if (tempData) {
        bool ret = lIdxBuffer->updateData(data, kLineSegIdxSBufize);
        delete[] data;
        return ret;
    } else {
        lIdxBuffer->unmap();
        return true;
    }
}
}

GrPathRenderer* GrAAHairLinePathRenderer::Create(GrContext* context) {
    GrGpu* gpu = context->getGpu();
    GrIndexBuffer* qIdxBuf = gpu->createIndexBuffer(kQuadIdxSBufize, false);
    SkAutoTUnref<GrIndexBuffer> qIdxBuffer(qIdxBuf);
    if (NULL == qIdxBuf || !push_quad_index_data(qIdxBuf)) {
        return NULL;
    }
    GrIndexBuffer* lIdxBuf = gpu->createIndexBuffer(kLineSegIdxSBufize, false);
    SkAutoTUnref<GrIndexBuffer> lIdxBuffer(lIdxBuf);
    if (NULL == lIdxBuf || !push_line_index_data(lIdxBuf)) {
        return NULL;
    }
    return SkNEW_ARGS(GrAAHairLinePathRenderer,
                      (context, lIdxBuf, qIdxBuf));
}

GrAAHairLinePathRenderer::GrAAHairLinePathRenderer(
                                        const GrContext* context,
                                        const GrIndexBuffer* linesIndexBuffer,
                                        const GrIndexBuffer* quadsIndexBuffer) {
    fLinesIndexBuffer = linesIndexBuffer;
    linesIndexBuffer->ref();
    fQuadsIndexBuffer = quadsIndexBuffer;
    quadsIndexBuffer->ref();
}

GrAAHairLinePathRenderer::~GrAAHairLinePathRenderer() {
    fLinesIndexBuffer->unref();
    fQuadsIndexBuffer->unref();
}

namespace {

#define PREALLOC_PTARRAY(N) SkSTArray<(N),SkPoint, true>

// Takes 178th time of logf on Z600 / VC2010
int get_float_exp(float x) {
    GR_STATIC_ASSERT(sizeof(int) == sizeof(float));
#ifdef SK_DEBUG
    static bool tested;
    if (!tested) {
        tested = true;
        SkASSERT(get_float_exp(0.25f) == -2);
        SkASSERT(get_float_exp(0.3f) == -2);
        SkASSERT(get_float_exp(0.5f) == -1);
        SkASSERT(get_float_exp(1.f) == 0);
        SkASSERT(get_float_exp(2.f) == 1);
        SkASSERT(get_float_exp(2.5f) == 1);
        SkASSERT(get_float_exp(8.f) == 3);
        SkASSERT(get_float_exp(100.f) == 6);
        SkASSERT(get_float_exp(1000.f) == 9);
        SkASSERT(get_float_exp(1024.f) == 10);
        SkASSERT(get_float_exp(3000000.f) == 21);
    }
#endif
    const int* iptr = (const int*)&x;
    return (((*iptr) & 0x7f800000) >> 23) - 127;
}

// Uses the max curvature function for quads to estimate
// where to chop the conic. If the max curvature is not
// found along the curve segment it will return 1 and
// dst[0] is the original conic. If it returns 2 the dst[0]
// and dst[1] are the two new conics.
int split_conic(const SkPoint src[3], SkConic dst[2], const SkScalar weight) {
    SkScalar t = SkFindQuadMaxCurvature(src);
    if (t == 0) {
        if (dst) {
            dst[0].set(src, weight);
        }
        return 1;
    } else {
        if (dst) {
            SkConic conic;
            conic.set(src, weight);
            conic.chopAt(t, dst);
        }
        return 2;
    }
}

// Calls split_conic on the entire conic and then once more on each subsection.
// Most cases will result in either 1 conic (chop point is not within t range)
// or 3 points (split once and then one subsection is split again).
int chop_conic(const SkPoint src[3], SkConic dst[4], const SkScalar weight) {
    SkConic dstTemp[2];
    int conicCnt = split_conic(src, dstTemp, weight);
    if (2 == conicCnt) {
        int conicCnt2 = split_conic(dstTemp[0].fPts, dst, dstTemp[0].fW);
        conicCnt = conicCnt2 + split_conic(dstTemp[1].fPts, &dst[conicCnt2], dstTemp[1].fW);
    } else {
        dst[0] = dstTemp[0];
    }
    return conicCnt;
}

// returns 0 if quad/conic is degen or close to it
// in this case approx the path with lines
// otherwise returns 1
int is_degen_quad_or_conic(const SkPoint p[3]) {
    static const SkScalar gDegenerateToLineTol = SK_Scalar1;
    static const SkScalar gDegenerateToLineTolSqd =
        SkScalarMul(gDegenerateToLineTol, gDegenerateToLineTol);

    if (p[0].distanceToSqd(p[1]) < gDegenerateToLineTolSqd ||
        p[1].distanceToSqd(p[2]) < gDegenerateToLineTolSqd) {
        return 1;
    }

    SkScalar dsqd = p[1].distanceToLineBetweenSqd(p[0], p[2]);
    if (dsqd < gDegenerateToLineTolSqd) {
        return 1;
    }

    if (p[2].distanceToLineBetweenSqd(p[1], p[0]) < gDegenerateToLineTolSqd) {
        return 1;
    }
    return 0;
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

    SkScalar dsqd = p[1].distanceToLineBetweenSqd(p[0], p[2]);
    if (dsqd < gDegenerateToLineTolSqd) {
        return -1;
    }

    if (p[2].distanceToLineBetweenSqd(p[1], p[0]) < gDegenerateToLineTolSqd) {
        return -1;
    }

    // tolerance of triangle height in pixels
    // tuned on windows  Quadro FX 380 / Z600
    // trade off of fill vs cpu time on verts
    // maybe different when do this using gpu (geo or tess shaders)
    static const SkScalar gSubdivTol = 175 * SK_Scalar1;

    if (dsqd <= SkScalarMul(gSubdivTol, gSubdivTol)) {
        return 0;
    } else {
        static const int kMaxSub = 4;
        // subdividing the quad reduces d by 4. so we want x = log4(d/tol)
        // = log4(d*d/tol*tol)/2
        // = log2(d*d/tol*tol)

        // +1 since we're ignoring the mantissa contribution.
        int log = get_float_exp(dsqd/(gSubdivTol*gSubdivTol)) + 1;
        log = SkTMin(SkTMax(0, log), kMaxSub);
        return log;
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
                             const SkIRect& devClipBounds,
                             GrAAHairLinePathRenderer::PtArray* lines,
                             GrAAHairLinePathRenderer::PtArray* quads,
                             GrAAHairLinePathRenderer::PtArray* conics,
                             GrAAHairLinePathRenderer::IntArray* quadSubdivCnts,
                             GrAAHairLinePathRenderer::FloatArray* conicWeights) {
    SkPath::Iter iter(path, false);

    int totalQuadCount = 0;
    SkRect bounds;
    SkIRect ibounds;

    bool persp = m.hasPerspective();

    for (;;) {
        SkPoint pathPts[4];
        SkPoint devPts[4];
        SkPath::Verb verb = iter.next(pathPts);
        switch (verb) {
            case SkPath::kConic_Verb: {
                SkConic dst[4];
                // We chop the conics to create tighter clipping to hide error
                // that appears near max curvature of very thin conics. Thin
                // hyperbolas with high weight still show error.
                int conicCnt = chop_conic(pathPts, dst, iter.conicWeight());
                for (int i = 0; i < conicCnt; ++i) {
                    SkPoint* chopPnts = dst[i].fPts;
                    m.mapPoints(devPts, chopPnts, 3);
                    bounds.setBounds(devPts, 3);
                    bounds.outset(SK_Scalar1, SK_Scalar1);
                    bounds.roundOut(&ibounds);
                    if (SkIRect::Intersects(devClipBounds, ibounds)) {
                        if (is_degen_quad_or_conic(devPts)) {
                            SkPoint* pts = lines->push_back_n(4);
                            pts[0] = devPts[0];
                            pts[1] = devPts[1];
                            pts[2] = devPts[1];
                            pts[3] = devPts[2];
                        } else {
                            // when in perspective keep conics in src space
                            SkPoint* cPts = persp ? chopPnts : devPts;
                            SkPoint* pts = conics->push_back_n(3);
                            pts[0] = cPts[0];
                            pts[1] = cPts[1];
                            pts[2] = cPts[2];
                            conicWeights->push_back() = dst[i].fW;
                        }
                    }
                }
                break;
            }
            case SkPath::kMove_Verb:
                break;
            case SkPath::kLine_Verb:
                m.mapPoints(devPts, pathPts, 2);
                bounds.setBounds(devPts, 2);
                bounds.outset(SK_Scalar1, SK_Scalar1);
                bounds.roundOut(&ibounds);
                if (SkIRect::Intersects(devClipBounds, ibounds)) {
                    SkPoint* pts = lines->push_back_n(2);
                    pts[0] = devPts[0];
                    pts[1] = devPts[1];
                }
                break;
            case SkPath::kQuad_Verb: {
                SkPoint choppedPts[5];
                // Chopping the quad helps when the quad is either degenerate or nearly degenerate.
                // When it is degenerate it allows the approximation with lines to work since the
                // chop point (if there is one) will be at the parabola's vertex. In the nearly
                // degenerate the QuadUVMatrix computed for the points is almost singular which
                // can cause rendering artifacts.
                int n = SkChopQuadAtMaxCurvature(pathPts, choppedPts);
                for (int i = 0; i < n; ++i) {
                    SkPoint* quadPts = choppedPts + i * 2;
                    m.mapPoints(devPts, quadPts, 3);
                    bounds.setBounds(devPts, 3);
                    bounds.outset(SK_Scalar1, SK_Scalar1);
                    bounds.roundOut(&ibounds);

                    if (SkIRect::Intersects(devClipBounds, ibounds)) {
                        int subdiv = num_quad_subdivs(devPts);
                        SkASSERT(subdiv >= -1);
                        if (-1 == subdiv) {
                            SkPoint* pts = lines->push_back_n(4);
                            pts[0] = devPts[0];
                            pts[1] = devPts[1];
                            pts[2] = devPts[1];
                            pts[3] = devPts[2];
                        } else {
                            // when in perspective keep quads in src space
                            SkPoint* qPts = persp ? quadPts : devPts;
                            SkPoint* pts = quads->push_back_n(3);
                            pts[0] = qPts[0];
                            pts[1] = qPts[1];
                            pts[2] = qPts[2];
                            quadSubdivCnts->push_back() = subdiv;
                            totalQuadCount += 1 << subdiv;
                        }
                    }
                }
                break;
            }
            case SkPath::kCubic_Verb:
                m.mapPoints(devPts, pathPts, 4);
                bounds.setBounds(devPts, 4);
                bounds.outset(SK_Scalar1, SK_Scalar1);
                bounds.roundOut(&ibounds);
                if (SkIRect::Intersects(devClipBounds, ibounds)) {
                    PREALLOC_PTARRAY(32) q;
                    // we don't need a direction if we aren't constraining the subdivision
                    static const SkPath::Direction kDummyDir = SkPath::kCCW_Direction;
                    // We convert cubics to quadratics (for now).
                    // In perspective have to do conversion in src space.
                    if (persp) {
                        SkScalar tolScale =
                            GrPathUtils::scaleToleranceToSrc(SK_Scalar1, m,
                                                             path.getBounds());
                        GrPathUtils::convertCubicToQuads(pathPts, tolScale, false, kDummyDir, &q);
                    } else {
                        GrPathUtils::convertCubicToQuads(devPts, SK_Scalar1, false, kDummyDir, &q);
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
                        if (SkIRect::Intersects(devClipBounds, ibounds)) {
                            int subdiv = num_quad_subdivs(qInDevSpace);
                            SkASSERT(subdiv >= -1);
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
            case SkPath::kClose_Verb:
                break;
            case SkPath::kDone_Verb:
                return totalQuadCount;
        }
    }
}

struct LineVertex {
    SkPoint fPos;
    GrColor fCoverage;
};

struct BezierVertex {
    SkPoint fPos;
    union {
        struct {
            SkScalar fK;
            SkScalar fL;
            SkScalar fM;
        } fConic;
        SkVector   fQuadCoord;
        struct {
            SkScalar fBogus[4];
        };
    };
};

GR_STATIC_ASSERT(sizeof(BezierVertex) == 3 * sizeof(SkPoint));

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

void set_uv_quad(const SkPoint qpts[3], BezierVertex verts[kVertsPerQuad]) {
    // this should be in the src space, not dev coords, when we have perspective
    GrPathUtils::QuadUVMatrix DevToUV(qpts);
    DevToUV.apply<kVertsPerQuad, sizeof(BezierVertex), sizeof(SkPoint)>(verts);
}

void bloat_quad(const SkPoint qpts[3], const SkMatrix* toDevice,
                const SkMatrix* toSrc, BezierVertex verts[kVertsPerQuad],
                SkRect* devBounds) {
    SkASSERT(!toDevice == !toSrc);
    // original quad is specified by tri a,b,c
    SkPoint a = qpts[0];
    SkPoint b = qpts[1];
    SkPoint c = qpts[2];

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
    BezierVertex& a0 = verts[0];
    BezierVertex& a1 = verts[1];
    BezierVertex& b0 = verts[2];
    BezierVertex& c0 = verts[3];
    BezierVertex& c1 = verts[4];

    SkVector ab = b;
    ab -= a;
    SkVector ac = c;
    ac -= a;
    SkVector cb = b;
    cb -= c;

    // We should have already handled degenerates
    SkASSERT(ab.length() > 0 && cb.length() > 0);

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
    devBounds->growToInclude(&verts[0].fPos, sizeof(BezierVertex), kVertsPerQuad);

    if (toSrc) {
        toSrc->mapPointsWithStride(&verts[0].fPos, sizeof(BezierVertex), kVertsPerQuad);
    }
}

// Equations based off of Loop-Blinn Quadratic GPU Rendering
// Input Parametric:
// P(t) = (P0*(1-t)^2 + 2*w*P1*t*(1-t) + P2*t^2) / (1-t)^2 + 2*w*t*(1-t) + t^2)
// Output Implicit:
// f(x, y, w) = f(P) = K^2 - LM
// K = dot(k, P), L = dot(l, P), M = dot(m, P)
// k, l, m are calculated in function GrPathUtils::getConicKLM
void set_conic_coeffs(const SkPoint p[3], BezierVertex verts[kVertsPerQuad],
                      const SkScalar weight) {
    SkScalar klm[9];

    GrPathUtils::getConicKLM(p, weight, klm);

    for (int i = 0; i < kVertsPerQuad; ++i) {
        const SkPoint pnt = verts[i].fPos;
        verts[i].fConic.fK = pnt.fX * klm[0] + pnt.fY * klm[1] + klm[2];
        verts[i].fConic.fL = pnt.fX * klm[3] + pnt.fY * klm[4] + klm[5];
        verts[i].fConic.fM = pnt.fX * klm[6] + pnt.fY * klm[7] + klm[8];
    }
}

void add_conics(const SkPoint p[3],
                const SkScalar weight,
                const SkMatrix* toDevice,
                const SkMatrix* toSrc,
                BezierVertex** vert,
                SkRect* devBounds) {
    bloat_quad(p, toDevice, toSrc, *vert, devBounds);
    set_conic_coeffs(p, *vert, weight);
    *vert += kVertsPerQuad;
}

void add_quads(const SkPoint p[3],
               int subdiv,
               const SkMatrix* toDevice,
               const SkMatrix* toSrc,
               BezierVertex** vert,
               SkRect* devBounds) {
    SkASSERT(subdiv >= 0);
    if (subdiv) {
        SkPoint newP[5];
        SkChopQuadAtHalf(p, newP);
        add_quads(newP + 0, subdiv-1, toDevice, toSrc, vert, devBounds);
        add_quads(newP + 2, subdiv-1, toDevice, toSrc, vert, devBounds);
    } else {
        bloat_quad(p, toDevice, toSrc, *vert, devBounds);
        set_uv_quad(p, *vert);
        *vert += kVertsPerQuad;
    }
}

void add_line(const SkPoint p[2],
              const SkMatrix* toSrc,
              GrColor coverage,
              LineVertex** vert) {
    const SkPoint& a = p[0];
    const SkPoint& b = p[1];

    SkVector ortho, vec = b;
    vec -= a;

    if (vec.setLength(SK_ScalarHalf)) {
        // Create a vector orthogonal to 'vec' and of unit length
        ortho.fX = 2.0f * vec.fY;
        ortho.fY = -2.0f * vec.fX;

        (*vert)[0].fPos = a;
        (*vert)[0].fCoverage = coverage;
        (*vert)[1].fPos = b;
        (*vert)[1].fCoverage = coverage;
        (*vert)[2].fPos = a - vec + ortho;
        (*vert)[2].fCoverage = 0;
        (*vert)[3].fPos = b + vec + ortho;
        (*vert)[3].fCoverage = 0;
        (*vert)[4].fPos = a - vec - ortho;
        (*vert)[4].fCoverage = 0;
        (*vert)[5].fPos = b + vec - ortho;
        (*vert)[5].fCoverage = 0;

        if (NULL != toSrc) {
            toSrc->mapPointsWithStride(&(*vert)->fPos,
                                       sizeof(LineVertex),
                                       kVertsPerLineSeg);
        }
    } else {
        // just make it degenerate and likely offscreen
        for (int i = 0; i < kVertsPerLineSeg; ++i) {
            (*vert)[i].fPos.set(SK_ScalarMax, SK_ScalarMax);
        }
    }

    *vert += kVertsPerLineSeg;
}

}

///////////////////////////////////////////////////////////////////////////////

namespace {

// position + edge
extern const GrVertexAttrib gHairlineBezierAttribs[] = {
    {kVec2f_GrVertexAttribType, 0,                  kPosition_GrVertexAttribBinding},
    {kVec4f_GrVertexAttribType, sizeof(SkPoint),    kEffect_GrVertexAttribBinding}
};

// position + coverage
extern const GrVertexAttrib gHairlineLineAttribs[] = {
    {kVec2f_GrVertexAttribType,  0,               kPosition_GrVertexAttribBinding},
    {kVec4ub_GrVertexAttribType, sizeof(SkPoint), kCoverage_GrVertexAttribBinding},
};

};

bool GrAAHairLinePathRenderer::createLineGeom(const SkPath& path,
                                              GrDrawTarget* target,
                                              const PtArray& lines,
                                              int lineCnt,
                                              GrDrawTarget::AutoReleaseGeometry* arg,
                                              SkRect* devBounds) {
    GrDrawState* drawState = target->drawState();

    const SkMatrix& viewM = drawState->getViewMatrix();

    int vertCnt = kVertsPerLineSeg * lineCnt;

    drawState->setVertexAttribs<gHairlineLineAttribs>(SK_ARRAY_COUNT(gHairlineLineAttribs));
    SkASSERT(sizeof(LineVertex) == drawState->getVertexSize());

    if (!arg->set(target, vertCnt, 0)) {
        return false;
    }

    LineVertex* verts = reinterpret_cast<LineVertex*>(arg->vertices());

    const SkMatrix* toSrc = NULL;
    SkMatrix ivm;

    if (viewM.hasPerspective()) {
        if (viewM.invert(&ivm)) {
            toSrc = &ivm;
        }
    }
    devBounds->set(lines.begin(), lines.count());
    for (int i = 0; i < lineCnt; ++i) {
        add_line(&lines[2*i], toSrc, drawState->getCoverageColor(), &verts);
    }
    // All the verts computed by add_line are within sqrt(1^2 + 0.5^2) of the end points.
    static const SkScalar kSqrtOfOneAndAQuarter = 1.118f;
    // Add a little extra to account for vector normalization precision.
    static const SkScalar kOutset = kSqrtOfOneAndAQuarter + SK_Scalar1 / 20;
    devBounds->outset(kOutset, kOutset);

    return true;
}

bool GrAAHairLinePathRenderer::createBezierGeom(
                                          const SkPath& path,
                                          GrDrawTarget* target,
                                          const PtArray& quads,
                                          int quadCnt,
                                          const PtArray& conics,
                                          int conicCnt,
                                          const IntArray& qSubdivs,
                                          const FloatArray& cWeights,
                                          GrDrawTarget::AutoReleaseGeometry* arg,
                                          SkRect* devBounds) {
    GrDrawState* drawState = target->drawState();

    const SkMatrix& viewM = drawState->getViewMatrix();

    int vertCnt = kVertsPerQuad * quadCnt + kVertsPerQuad * conicCnt;

    target->drawState()->setVertexAttribs<gHairlineBezierAttribs>(SK_ARRAY_COUNT(gHairlineBezierAttribs));
    SkASSERT(sizeof(BezierVertex) == target->getDrawState().getVertexSize());

    if (!arg->set(target, vertCnt, 0)) {
        return false;
    }

    BezierVertex* verts = reinterpret_cast<BezierVertex*>(arg->vertices());

    const SkMatrix* toDevice = NULL;
    const SkMatrix* toSrc = NULL;
    SkMatrix ivm;

    if (viewM.hasPerspective()) {
        if (viewM.invert(&ivm)) {
            toDevice = &viewM;
            toSrc = &ivm;
        }
    }

    // Seed the dev bounds with some pts known to be inside. Each quad and conic grows the bounding
    // box to include its vertices.
    SkPoint seedPts[2];
    if (quadCnt) {
        seedPts[0] = quads[0];
        seedPts[1] = quads[2];
    } else if (conicCnt) {
        seedPts[0] = conics[0];
        seedPts[1] = conics[2];
    }
    if (NULL != toDevice) {
        toDevice->mapPoints(seedPts, 2);
    }
    devBounds->set(seedPts[0], seedPts[1]);

    int unsubdivQuadCnt = quads.count() / 3;
    for (int i = 0; i < unsubdivQuadCnt; ++i) {
        SkASSERT(qSubdivs[i] >= 0);
        add_quads(&quads[3*i], qSubdivs[i], toDevice, toSrc, &verts, devBounds);
    }

    // Start Conics
    for (int i = 0; i < conicCnt; ++i) {
        add_conics(&conics[3*i], cWeights[i], toDevice, toSrc, &verts, devBounds);
    }
    return true;
}

bool GrAAHairLinePathRenderer::canDrawPath(const SkPath& path,
                                           const SkStrokeRec& stroke,
                                           const GrDrawTarget* target,
                                           bool antiAlias) const {
    if (!antiAlias) {
        return false;
    }

    if (!IsStrokeHairlineOrEquivalent(stroke,
                                      target->getDrawState().getViewMatrix(),
                                      NULL)) {
        return false;
    }

    if (SkPath::kLine_SegmentMask == path.getSegmentMasks() ||
        target->caps()->shaderDerivativeSupport()) {
        return true;
    }
    return false;
}

template <class VertexType>
bool check_bounds(GrDrawState* drawState, const SkRect& devBounds, void* vertices, int vCount)
{
    SkRect tolDevBounds = devBounds;
    // The bounds ought to be tight, but in perspective the below code runs the verts
    // through the view matrix to get back to dev coords, which can introduce imprecision.
    if (drawState->getViewMatrix().hasPerspective()) {
        tolDevBounds.outset(SK_Scalar1 / 1000, SK_Scalar1 / 1000);
    } else {
        // Non-persp matrices cause this path renderer to draw in device space.
        SkASSERT(drawState->getViewMatrix().isIdentity());
    }
    SkRect actualBounds;

    VertexType* verts = reinterpret_cast<VertexType*>(vertices);
    bool first = true;
    for (int i = 0; i < vCount; ++i) {
        SkPoint pos = verts[i].fPos;
        // This is a hack to workaround the fact that we move some degenerate segments offscreen.
        if (SK_ScalarMax == pos.fX) {
            continue;
        }
        drawState->getViewMatrix().mapPoints(&pos, 1);
        if (first) {
            actualBounds.set(pos.fX, pos.fY, pos.fX, pos.fY);
            first = false;
        } else {
            actualBounds.growToInclude(pos.fX, pos.fY);
        }
    }
    if (!first) {
        return tolDevBounds.contains(actualBounds);
    }

    return true;
}

bool GrAAHairLinePathRenderer::onDrawPath(const SkPath& path,
                                          const SkStrokeRec& stroke,
                                          GrDrawTarget* target,
                                          bool antiAlias) {
    GrDrawState* drawState = target->drawState();

    SkScalar hairlineCoverage;
    if (IsStrokeHairlineOrEquivalent(stroke,
                                     target->getDrawState().getViewMatrix(),
                                     &hairlineCoverage)) {
        uint8_t newCoverage = SkScalarRoundToInt(hairlineCoverage *
                                                 target->getDrawState().getCoverage());
        target->drawState()->setCoverage(newCoverage);
    }

    SkIRect devClipBounds;
    target->getClip()->getConservativeBounds(drawState->getRenderTarget(), &devClipBounds);

    int lineCnt;
    int quadCnt;
    int conicCnt;
    PREALLOC_PTARRAY(128) lines;
    PREALLOC_PTARRAY(128) quads;
    PREALLOC_PTARRAY(128) conics;
    IntArray qSubdivs;
    FloatArray cWeights;
    quadCnt = generate_lines_and_quads(path, drawState->getViewMatrix(), devClipBounds,
                                       &lines, &quads, &conics, &qSubdivs, &cWeights);
    lineCnt = lines.count() / 2;
    conicCnt = conics.count() / 3;

    // do lines first
    if (lineCnt) {
        GrDrawTarget::AutoReleaseGeometry arg;
        SkRect devBounds;

        if (!this->createLineGeom(path,
                                  target,
                                  lines,
                                  lineCnt,
                                  &arg,
                                  &devBounds)) {
            return false;
        }

        GrDrawTarget::AutoStateRestore asr;

        // createLineGeom transforms the geometry to device space when the matrix does not have
        // perspective.
        if (target->getDrawState().getViewMatrix().hasPerspective()) {
            asr.set(target, GrDrawTarget::kPreserve_ASRInit);
        } else if (!asr.setIdentity(target, GrDrawTarget::kPreserve_ASRInit)) {
            return false;
        }
        GrDrawState* drawState = target->drawState();

        // Check devBounds
        SkASSERT(check_bounds<LineVertex>(drawState, devBounds, arg.vertices(),
                                          kVertsPerLineSeg * lineCnt));

        {
            GrDrawState::AutoRestoreEffects are(drawState);
            target->setIndexSourceToBuffer(fLinesIndexBuffer);
            int lines = 0;
            while (lines < lineCnt) {
                int n = SkTMin(lineCnt - lines, kNumLineSegsInIdxBuffer);
                target->drawIndexed(kTriangles_GrPrimitiveType,
                                    kVertsPerLineSeg*lines,     // startV
                                    0,                          // startI
                                    kVertsPerLineSeg*n,         // vCount
                                    kIdxsPerLineSeg*n,          // iCount
                                    &devBounds);
                lines += n;
            }
        }
    }

    // then quadratics/conics
    if (quadCnt || conicCnt) {
        GrDrawTarget::AutoReleaseGeometry arg;
        SkRect devBounds;

        if (!this->createBezierGeom(path,
                                    target,
                                    quads,
                                    quadCnt,
                                    conics,
                                    conicCnt,
                                    qSubdivs,
                                    cWeights,
                                    &arg,
                                    &devBounds)) {
            return false;
        }

        GrDrawTarget::AutoStateRestore asr;

        // createGeom transforms the geometry to device space when the matrix does not have
        // perspective.
        if (target->getDrawState().getViewMatrix().hasPerspective()) {
            asr.set(target, GrDrawTarget::kPreserve_ASRInit);
        } else if (!asr.setIdentity(target, GrDrawTarget::kPreserve_ASRInit)) {
            return false;
        }
        GrDrawState* drawState = target->drawState();

        static const int kEdgeAttrIndex = 1;

        // Check devBounds
        SkASSERT(check_bounds<BezierVertex>(drawState, devBounds, arg.vertices(),
                                            kVertsPerQuad * quadCnt + kVertsPerQuad * conicCnt));

        if (quadCnt > 0) {
            GrEffect* hairQuadEffect = GrQuadEffect::Create(kHairlineAA_GrEffectEdgeType,
                                                            *target->caps());
            SkASSERT(NULL != hairQuadEffect);
            GrDrawState::AutoRestoreEffects are(drawState);
            target->setIndexSourceToBuffer(fQuadsIndexBuffer);
            drawState->addCoverageEffect(hairQuadEffect, kEdgeAttrIndex)->unref();
            int quads = 0;
            while (quads < quadCnt) {
                int n = SkTMin(quadCnt - quads, kNumQuadsInIdxBuffer);
                target->drawIndexed(kTriangles_GrPrimitiveType,
                                    kVertsPerQuad*quads,               // startV
                                    0,                                 // startI
                                    kVertsPerQuad*n,                   // vCount
                                    kIdxsPerQuad*n,                    // iCount
                                    &devBounds);
                quads += n;
            }
        }

        if (conicCnt > 0) {
            GrDrawState::AutoRestoreEffects are(drawState);
            GrEffect* hairConicEffect = GrConicEffect::Create(kHairlineAA_GrEffectEdgeType,
                                                              *target->caps());
            SkASSERT(NULL != hairConicEffect);
            drawState->addCoverageEffect(hairConicEffect, 1, 2)->unref();
            int conics = 0;
            while (conics < conicCnt) {
                int n = SkTMin(conicCnt - conics, kNumQuadsInIdxBuffer);
                target->drawIndexed(kTriangles_GrPrimitiveType,
                                    kVertsPerQuad*(quadCnt + conics),  // startV
                                    0,                                 // startI
                                    kVertsPerQuad*n,                   // vCount
                                    kIdxsPerQuad*n,                    // iCount
                                    &devBounds);
                conics += n;
            }
        }
    }

    target->resetIndexSource();

    return true;
}
