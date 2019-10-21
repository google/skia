/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPoint3.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkPointPriv.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkStroke.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrBuffer.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrDefaultGeoProcFactory.h"
#include "src/gpu/GrDrawOpTest.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrProcessor.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/effects/GrBezierEffect.h"
#include "src/gpu/geometry/GrPathUtils.h"
#include "src/gpu/geometry/GrShape.h"
#include "src/gpu/ops/GrAAHairLinePathRenderer.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"

#define PREALLOC_PTARRAY(N) SkSTArray<(N),SkPoint, true>

// quadratics are rendered as 5-sided polys in order to bound the
// AA stroke around the center-curve. See comments in push_quad_index_buffer and
// bloat_quad. Quadratics and conics share an index buffer

// lines are rendered as:
//      *______________*
//      |\ -_______   /|
//      | \        \ / |
//      |  *--------*  |
//      | /  ______/ \ |
//      */_-__________\*
// For: 6 vertices and 18 indices (for 6 triangles)

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
// Each is drawn as three triangles ((a0,a1,b0), (b0,c1,c0), (a1,c1,b0))
// specified by these 9 indices:
static const uint16_t kQuadIdxBufPattern[] = {
    0, 1, 2,
    2, 4, 3,
    1, 4, 2
};

static const int kIdxsPerQuad = SK_ARRAY_COUNT(kQuadIdxBufPattern);
static const int kQuadNumVertices = 5;
static const int kQuadsNumInIdxBuffer = 256;
GR_DECLARE_STATIC_UNIQUE_KEY(gQuadsIndexBufferKey);

static sk_sp<const GrBuffer> get_quads_index_buffer(GrResourceProvider* resourceProvider) {
    GR_DEFINE_STATIC_UNIQUE_KEY(gQuadsIndexBufferKey);
    return resourceProvider->findOrCreatePatternedIndexBuffer(
        kQuadIdxBufPattern, kIdxsPerQuad, kQuadsNumInIdxBuffer, kQuadNumVertices,
        gQuadsIndexBufferKey);
}


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

static const uint16_t kLineSegIdxBufPattern[] = {
    0, 1, 3,
    0, 3, 2,
    0, 4, 5,
    0, 5, 1,
    0, 2, 4,
    1, 5, 3
};

static const int kIdxsPerLineSeg = SK_ARRAY_COUNT(kLineSegIdxBufPattern);
static const int kLineSegNumVertices = 6;
static const int kLineSegsNumInIdxBuffer = 256;

GR_DECLARE_STATIC_UNIQUE_KEY(gLinesIndexBufferKey);

static sk_sp<const GrBuffer> get_lines_index_buffer(GrResourceProvider* resourceProvider) {
    GR_DEFINE_STATIC_UNIQUE_KEY(gLinesIndexBufferKey);
    return resourceProvider->findOrCreatePatternedIndexBuffer(
        kLineSegIdxBufPattern, kIdxsPerLineSeg,  kLineSegsNumInIdxBuffer, kLineSegNumVertices,
        gLinesIndexBufferKey);
}

// Takes 178th time of logf on Z600 / VC2010
static int get_float_exp(float x) {
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
static int split_conic(const SkPoint src[3], SkConic dst[2], const SkScalar weight) {
    SkScalar t = SkFindQuadMaxCurvature(src);
    if (t == 0 || t == 1) {
        if (dst) {
            dst[0].set(src, weight);
        }
        return 1;
    } else {
        if (dst) {
            SkConic conic;
            conic.set(src, weight);
            if (!conic.chopAt(t, dst)) {
                dst[0].set(src, weight);
                return 1;
            }
        }
        return 2;
    }
}

// Calls split_conic on the entire conic and then once more on each subsection.
// Most cases will result in either 1 conic (chop point is not within t range)
// or 3 points (split once and then one subsection is split again).
static int chop_conic(const SkPoint src[3], SkConic dst[4], const SkScalar weight) {
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
static int is_degen_quad_or_conic(const SkPoint p[3], SkScalar* dsqd) {
    static const SkScalar gDegenerateToLineTol = GrPathUtils::kDefaultTolerance;
    static const SkScalar gDegenerateToLineTolSqd =
        gDegenerateToLineTol * gDegenerateToLineTol;

    if (SkPointPriv::DistanceToSqd(p[0], p[1]) < gDegenerateToLineTolSqd ||
        SkPointPriv::DistanceToSqd(p[1], p[2]) < gDegenerateToLineTolSqd) {
        return 1;
    }

    *dsqd = SkPointPriv::DistanceToLineBetweenSqd(p[1], p[0], p[2]);
    if (*dsqd < gDegenerateToLineTolSqd) {
        return 1;
    }

    if (SkPointPriv::DistanceToLineBetweenSqd(p[2], p[1], p[0]) < gDegenerateToLineTolSqd) {
        return 1;
    }
    return 0;
}

static int is_degen_quad_or_conic(const SkPoint p[3]) {
    SkScalar dsqd;
    return is_degen_quad_or_conic(p, &dsqd);
}

// we subdivide the quads to avoid huge overfill
// if it returns -1 then should be drawn as lines
static int num_quad_subdivs(const SkPoint p[3]) {
    SkScalar dsqd;
    if (is_degen_quad_or_conic(p, &dsqd)) {
        return -1;
    }

    // tolerance of triangle height in pixels
    // tuned on windows  Quadro FX 380 / Z600
    // trade off of fill vs cpu time on verts
    // maybe different when do this using gpu (geo or tess shaders)
    static const SkScalar gSubdivTol = 175 * SK_Scalar1;

    if (dsqd <= gSubdivTol * gSubdivTol) {
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
static int gather_lines_and_quads(const SkPath& path,
                                  const SkMatrix& m,
                                  const SkIRect& devClipBounds,
                                  SkScalar capLength,
                                  bool convertConicsToQuads,
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

    // Whenever a degenerate, zero-length contour is encountered, this code will insert a
    // 'capLength' x-aligned line segment. Since this is rendering hairlines it is hoped this will
    // suffice for AA square & circle capping.
    int verbsInContour = 0; // Does not count moves
    bool seenZeroLengthVerb = false;
    SkPoint zeroVerbPt;

    // Adds a quad that has already been chopped to the list and checks for quads that are close to
    // lines. Also does a bounding box check. It takes points that are in src space and device
    // space. The src points are only required if the view matrix has perspective.
    auto addChoppedQuad = [&](const SkPoint srcPts[3], const SkPoint devPts[4],
                              bool isContourStart) {
        SkRect bounds;
        SkIRect ibounds;
        bounds.setBounds(devPts, 3);
        bounds.outset(SK_Scalar1, SK_Scalar1);
        bounds.roundOut(&ibounds);
        // We only need the src space space pts when not in perspective.
        SkASSERT(srcPts || !persp);
        if (SkIRect::Intersects(devClipBounds, ibounds)) {
            int subdiv = num_quad_subdivs(devPts);
            SkASSERT(subdiv >= -1);
            if (-1 == subdiv) {
                SkPoint* pts = lines->push_back_n(4);
                pts[0] = devPts[0];
                pts[1] = devPts[1];
                pts[2] = devPts[1];
                pts[3] = devPts[2];
                if (isContourStart && pts[0] == pts[1] && pts[2] == pts[3]) {
                    seenZeroLengthVerb = true;
                    zeroVerbPt = pts[0];
                }
            } else {
                // when in perspective keep quads in src space
                const SkPoint* qPts = persp ? srcPts : devPts;
                SkPoint* pts = quads->push_back_n(3);
                pts[0] = qPts[0];
                pts[1] = qPts[1];
                pts[2] = qPts[2];
                quadSubdivCnts->push_back() = subdiv;
                totalQuadCount += 1 << subdiv;
            }
        }
    };

    // Applies the view matrix to quad src points and calls the above helper.
    auto addSrcChoppedQuad = [&](const SkPoint srcSpaceQuadPts[3], bool isContourStart) {
        SkPoint devPts[3];
        m.mapPoints(devPts, srcSpaceQuadPts, 3);
        addChoppedQuad(srcSpaceQuadPts, devPts, isContourStart);
    };

    for (;;) {
        SkPoint pathPts[4];
        SkPath::Verb verb = iter.next(pathPts);
        switch (verb) {
            case SkPath::kConic_Verb:
                if (convertConicsToQuads) {
                    SkScalar weight = iter.conicWeight();
                    SkAutoConicToQuads converter;
                    const SkPoint* quadPts = converter.computeQuads(pathPts, weight, 0.25f);
                    for (int i = 0; i < converter.countQuads(); ++i) {
                        addSrcChoppedQuad(quadPts + 2 * i, !verbsInContour && 0 == i);
                    }
                } else {
                    SkConic dst[4];
                    // We chop the conics to create tighter clipping to hide error
                    // that appears near max curvature of very thin conics. Thin
                    // hyperbolas with high weight still show error.
                    int conicCnt = chop_conic(pathPts, dst, iter.conicWeight());
                    for (int i = 0; i < conicCnt; ++i) {
                        SkPoint devPts[4];
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
                                if (verbsInContour == 0 && i == 0 && pts[0] == pts[1] &&
                                    pts[2] == pts[3]) {
                                    seenZeroLengthVerb = true;
                                    zeroVerbPt = pts[0];
                                }
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
                }
                verbsInContour++;
                break;
            case SkPath::kMove_Verb:
                // New contour (and last one was unclosed). If it was just a zero length drawing
                // operation, and we're supposed to draw caps, then add a tiny line.
                if (seenZeroLengthVerb && verbsInContour == 1 && capLength > 0) {
                    SkPoint* pts = lines->push_back_n(2);
                    pts[0] = SkPoint::Make(zeroVerbPt.fX - capLength, zeroVerbPt.fY);
                    pts[1] = SkPoint::Make(zeroVerbPt.fX + capLength, zeroVerbPt.fY);
                }
                verbsInContour = 0;
                seenZeroLengthVerb = false;
                break;
            case SkPath::kLine_Verb: {
                SkPoint devPts[2];
                m.mapPoints(devPts, pathPts, 2);
                bounds.setBounds(devPts, 2);
                bounds.outset(SK_Scalar1, SK_Scalar1);
                bounds.roundOut(&ibounds);
                if (SkIRect::Intersects(devClipBounds, ibounds)) {
                    SkPoint* pts = lines->push_back_n(2);
                    pts[0] = devPts[0];
                    pts[1] = devPts[1];
                    if (verbsInContour == 0 && pts[0] == pts[1]) {
                        seenZeroLengthVerb = true;
                        zeroVerbPt = pts[0];
                    }
                }
                verbsInContour++;
                break;
            }
            case SkPath::kQuad_Verb: {
                SkPoint choppedPts[5];
                // Chopping the quad helps when the quad is either degenerate or nearly degenerate.
                // When it is degenerate it allows the approximation with lines to work since the
                // chop point (if there is one) will be at the parabola's vertex. In the nearly
                // degenerate the QuadUVMatrix computed for the points is almost singular which
                // can cause rendering artifacts.
                int n = SkChopQuadAtMaxCurvature(pathPts, choppedPts);
                for (int i = 0; i < n; ++i) {
                    addSrcChoppedQuad(choppedPts + i * 2, !verbsInContour && 0 == i);
                }
                verbsInContour++;
                break;
            }
            case SkPath::kCubic_Verb: {
                SkPoint devPts[4];
                m.mapPoints(devPts, pathPts, 4);
                bounds.setBounds(devPts, 4);
                bounds.outset(SK_Scalar1, SK_Scalar1);
                bounds.roundOut(&ibounds);
                if (SkIRect::Intersects(devClipBounds, ibounds)) {
                    PREALLOC_PTARRAY(32) q;
                    // We convert cubics to quadratics (for now).
                    // In perspective have to do conversion in src space.
                    if (persp) {
                        SkScalar tolScale =
                            GrPathUtils::scaleToleranceToSrc(SK_Scalar1, m, path.getBounds());
                        GrPathUtils::convertCubicToQuads(pathPts, tolScale, &q);
                    } else {
                        GrPathUtils::convertCubicToQuads(devPts, SK_Scalar1, &q);
                    }
                    for (int i = 0; i < q.count(); i += 3) {
                        if (persp) {
                            addSrcChoppedQuad(&q[i], !verbsInContour && 0 == i);
                        } else {
                            addChoppedQuad(nullptr, &q[i], !verbsInContour && 0 == i);
                        }
                    }
                }
                verbsInContour++;
                break;
            }
            case SkPath::kClose_Verb:
                // Contour is closed, so we don't need to grow the starting line, unless it's
                // *just* a zero length subpath. (SVG Spec 11.4, 'stroke').
                if (capLength > 0) {
                    if (seenZeroLengthVerb && verbsInContour == 1) {
                        SkPoint* pts = lines->push_back_n(2);
                        pts[0] = SkPoint::Make(zeroVerbPt.fX - capLength, zeroVerbPt.fY);
                        pts[1] = SkPoint::Make(zeroVerbPt.fX + capLength, zeroVerbPt.fY);
                    } else if (verbsInContour == 0) {
                        // Contour was (moveTo, close). Add a line.
                        SkPoint devPts[2];
                        m.mapPoints(devPts, pathPts, 1);
                        devPts[1] = devPts[0];
                        bounds.setBounds(devPts, 2);
                        bounds.outset(SK_Scalar1, SK_Scalar1);
                        bounds.roundOut(&ibounds);
                        if (SkIRect::Intersects(devClipBounds, ibounds)) {
                            SkPoint* pts = lines->push_back_n(2);
                            pts[0] = SkPoint::Make(devPts[0].fX - capLength, devPts[0].fY);
                            pts[1] = SkPoint::Make(devPts[1].fX + capLength, devPts[1].fY);
                        }
                    }
                }
                break;
            case SkPath::kDone_Verb:
                if (seenZeroLengthVerb && verbsInContour == 1 && capLength > 0) {
                    // Path ended with a dangling (moveTo, line|quad|etc). If the final verb is
                    // degenerate, we need to draw a line.
                    SkPoint* pts = lines->push_back_n(2);
                    pts[0] = SkPoint::Make(zeroVerbPt.fX - capLength, zeroVerbPt.fY);
                    pts[1] = SkPoint::Make(zeroVerbPt.fX + capLength, zeroVerbPt.fY);
                }
                return totalQuadCount;
        }
    }
}

struct LineVertex {
    SkPoint fPos;
    float fCoverage;
};

struct BezierVertex {
    SkPoint fPos;
    union {
        struct {
            SkScalar fKLM[3];
        } fConic;
        SkVector   fQuadCoord;
        struct {
            SkScalar fBogus[4];
        };
    };
};

GR_STATIC_ASSERT(sizeof(BezierVertex) == 3 * sizeof(SkPoint));

static void intersect_lines(const SkPoint& ptA, const SkVector& normA,
                            const SkPoint& ptB, const SkVector& normB,
                            SkPoint* result) {

    SkScalar lineAW = -normA.dot(ptA);
    SkScalar lineBW = -normB.dot(ptB);

    SkScalar wInv = normA.fX * normB.fY - normA.fY * normB.fX;
    wInv = SkScalarInvert(wInv);
    if (!SkScalarIsFinite(wInv)) {
        // lines are parallel, pick the point in between
        *result = (ptA + ptB)*SK_ScalarHalf;
        *result += normA;
    } else {
        result->fX = normA.fY * lineBW - lineAW * normB.fY;
        result->fX *= wInv;

        result->fY = lineAW * normB.fX - normA.fX * lineBW;
        result->fY *= wInv;
    }
}

static void set_uv_quad(const SkPoint qpts[3], BezierVertex verts[kQuadNumVertices]) {
    // this should be in the src space, not dev coords, when we have perspective
    GrPathUtils::QuadUVMatrix DevToUV(qpts);
    DevToUV.apply(verts, kQuadNumVertices, sizeof(BezierVertex), sizeof(SkPoint));
}

static void bloat_quad(const SkPoint qpts[3], const SkMatrix* toDevice,
                       const SkMatrix* toSrc, BezierVertex verts[kQuadNumVertices]) {
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

    // After the transform we might have a line, try to do something reasonable
    if (toDevice && SkPointPriv::LengthSqd(ab) <= SK_ScalarNearlyZero*SK_ScalarNearlyZero) {
        ab = cb;
    }
    if (toDevice && SkPointPriv::LengthSqd(cb) <= SK_ScalarNearlyZero*SK_ScalarNearlyZero) {
        cb = ab;
    }

    // We should have already handled degenerates
    SkASSERT(toDevice || (ab.length() > 0 && cb.length() > 0));

    ab.normalize();
    SkVector abN = SkPointPriv::MakeOrthog(ab, SkPointPriv::kLeft_Side);
    if (abN.dot(ac) > 0) {
        abN.negate();
    }

    cb.normalize();
    SkVector cbN = SkPointPriv::MakeOrthog(cb, SkPointPriv::kLeft_Side);
    if (cbN.dot(ac) < 0) {
        cbN.negate();
    }

    a0.fPos = a;
    a0.fPos += abN;
    a1.fPos = a;
    a1.fPos -= abN;

    if (toDevice && SkPointPriv::LengthSqd(ac) <= SK_ScalarNearlyZero*SK_ScalarNearlyZero) {
        c = b;
    }
    c0.fPos = c;
    c0.fPos += cbN;
    c1.fPos = c;
    c1.fPos -= cbN;

    intersect_lines(a0.fPos, abN, c0.fPos, cbN, &b0.fPos);

    if (toSrc) {
        SkMatrixPriv::MapPointsWithStride(*toSrc, &verts[0].fPos, sizeof(BezierVertex),
                                          kQuadNumVertices);
    }
}

// Equations based off of Loop-Blinn Quadratic GPU Rendering
// Input Parametric:
// P(t) = (P0*(1-t)^2 + 2*w*P1*t*(1-t) + P2*t^2) / (1-t)^2 + 2*w*t*(1-t) + t^2)
// Output Implicit:
// f(x, y, w) = f(P) = K^2 - LM
// K = dot(k, P), L = dot(l, P), M = dot(m, P)
// k, l, m are calculated in function GrPathUtils::getConicKLM
static void set_conic_coeffs(const SkPoint p[3], BezierVertex verts[kQuadNumVertices],
                             const SkScalar weight) {
    SkMatrix klm;

    GrPathUtils::getConicKLM(p, weight, &klm);

    for (int i = 0; i < kQuadNumVertices; ++i) {
        const SkPoint3 pt3 = {verts[i].fPos.x(), verts[i].fPos.y(), 1.f};
        klm.mapHomogeneousPoints((SkPoint3* ) verts[i].fConic.fKLM, &pt3, 1);
    }
}

static void add_conics(const SkPoint p[3],
                       const SkScalar weight,
                       const SkMatrix* toDevice,
                       const SkMatrix* toSrc,
                       BezierVertex** vert) {
    bloat_quad(p, toDevice, toSrc, *vert);
    set_conic_coeffs(p, *vert, weight);
    *vert += kQuadNumVertices;
}

static void add_quads(const SkPoint p[3],
                      int subdiv,
                      const SkMatrix* toDevice,
                      const SkMatrix* toSrc,
                      BezierVertex** vert) {
    SkASSERT(subdiv >= 0);
    if (subdiv) {
        SkPoint newP[5];
        SkChopQuadAtHalf(p, newP);
        add_quads(newP + 0, subdiv-1, toDevice, toSrc, vert);
        add_quads(newP + 2, subdiv-1, toDevice, toSrc, vert);
    } else {
        bloat_quad(p, toDevice, toSrc, *vert);
        set_uv_quad(p, *vert);
        *vert += kQuadNumVertices;
    }
}

static void add_line(const SkPoint p[2],
                     const SkMatrix* toSrc,
                     uint8_t coverage,
                     LineVertex** vert) {
    const SkPoint& a = p[0];
    const SkPoint& b = p[1];

    SkVector ortho, vec = b;
    vec -= a;

    SkScalar lengthSqd = SkPointPriv::LengthSqd(vec);

    if (vec.setLength(SK_ScalarHalf)) {
        // Create a vector orthogonal to 'vec' and of unit length
        ortho.fX = 2.0f * vec.fY;
        ortho.fY = -2.0f * vec.fX;

        float floatCoverage = GrNormalizeByteToFloat(coverage);

        if (lengthSqd >= 1.0f) {
            // Relative to points a and b:
            // The inner vertices are inset half a pixel along the line a,b
            (*vert)[0].fPos = a + vec;
            (*vert)[0].fCoverage = floatCoverage;
            (*vert)[1].fPos = b - vec;
            (*vert)[1].fCoverage = floatCoverage;
        } else {
            // The inner vertices are inset a distance of length(a,b) from the outer edge of
            // geometry. For the "a" inset this is the same as insetting from b by half a pixel.
            // The coverage is then modulated by the length. This gives us the correct
            // coverage for rects shorter than a pixel as they get translated subpixel amounts
            // inside of a pixel.
            SkScalar length = SkScalarSqrt(lengthSqd);
            (*vert)[0].fPos = b - vec;
            (*vert)[0].fCoverage = floatCoverage * length;
            (*vert)[1].fPos = a + vec;
            (*vert)[1].fCoverage = floatCoverage * length;
        }
        // Relative to points a and b:
        // The outer vertices are outset half a pixel along the line a,b and then a whole pixel
        // orthogonally.
        (*vert)[2].fPos = a - vec + ortho;
        (*vert)[2].fCoverage = 0;
        (*vert)[3].fPos = b + vec + ortho;
        (*vert)[3].fCoverage = 0;
        (*vert)[4].fPos = a - vec - ortho;
        (*vert)[4].fCoverage = 0;
        (*vert)[5].fPos = b + vec - ortho;
        (*vert)[5].fCoverage = 0;

        if (toSrc) {
            SkMatrixPriv::MapPointsWithStride(*toSrc, &(*vert)->fPos, sizeof(LineVertex),
                                              kLineSegNumVertices);
        }
    } else {
        // just make it degenerate and likely offscreen
        for (int i = 0; i < kLineSegNumVertices; ++i) {
            (*vert)[i].fPos.set(SK_ScalarMax, SK_ScalarMax);
        }
    }

    *vert += kLineSegNumVertices;
}

///////////////////////////////////////////////////////////////////////////////

GrPathRenderer::CanDrawPath
GrAAHairLinePathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    if (GrAAType::kCoverage != args.fAAType) {
        return CanDrawPath::kNo;
    }

    if (!IsStrokeHairlineOrEquivalent(args.fShape->style(), *args.fViewMatrix, nullptr)) {
        return CanDrawPath::kNo;
    }

    // We don't currently handle dashing in this class though perhaps we should.
    if (args.fShape->style().pathEffect()) {
        return CanDrawPath::kNo;
    }

    if (SkPath::kLine_SegmentMask == args.fShape->segmentMask() ||
        args.fCaps->shaderCaps()->shaderDerivativeSupport()) {
        return CanDrawPath::kYes;
    }

    return CanDrawPath::kNo;
}

template <class VertexType>
bool check_bounds(const SkMatrix& viewMatrix, const SkRect& devBounds, void* vertices, int vCount)
{
    SkRect tolDevBounds = devBounds;
    // The bounds ought to be tight, but in perspective the below code runs the verts
    // through the view matrix to get back to dev coords, which can introduce imprecision.
    if (viewMatrix.hasPerspective()) {
        tolDevBounds.outset(SK_Scalar1 / 1000, SK_Scalar1 / 1000);
    } else {
        // Non-persp matrices cause this path renderer to draw in device space.
        SkASSERT(viewMatrix.isIdentity());
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
        viewMatrix.mapPoints(&pos, 1);
        if (first) {
            actualBounds.setLTRB(pos.fX, pos.fY, pos.fX, pos.fY);
            first = false;
        } else {
            SkRectPriv::GrowToInclude(&actualBounds, pos);
        }
    }
    if (!first) {
        return tolDevBounds.contains(actualBounds);
    }

    return true;
}

namespace {

class AAHairlineOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelperWithStencil;

public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                                          GrPaint&& paint,
                                          const SkMatrix& viewMatrix,
                                          const SkPath& path,
                                          const GrStyle& style,
                                          const SkIRect& devClipBounds,
                                          const GrUserStencilSettings* stencilSettings) {
        SkScalar hairlineCoverage;
        uint8_t newCoverage = 0xff;
        if (GrPathRenderer::IsStrokeHairlineOrEquivalent(style, viewMatrix, &hairlineCoverage)) {
            newCoverage = SkScalarRoundToInt(hairlineCoverage * 0xff);
        }

        const SkStrokeRec& stroke = style.strokeRec();
        SkScalar capLength = SkPaint::kButt_Cap != stroke.getCap() ? hairlineCoverage * 0.5f : 0.0f;

        return Helper::FactoryHelper<AAHairlineOp>(context, std::move(paint), newCoverage,
                                                   viewMatrix, path,
                                                   devClipBounds, capLength, stencilSettings);
    }

    AAHairlineOp(const Helper::MakeArgs& helperArgs,
                 const SkPMColor4f& color,
                 uint8_t coverage,
                 const SkMatrix& viewMatrix,
                 const SkPath& path,
                 SkIRect devClipBounds,
                 SkScalar capLength,
                 const GrUserStencilSettings* stencilSettings)
            : INHERITED(ClassID())
            , fHelper(helperArgs, GrAAType::kCoverage, stencilSettings)
            , fColor(color)
            , fCoverage(coverage) {
        fPaths.emplace_back(PathData{viewMatrix, path, devClipBounds, capLength});

        this->setTransformedBounds(path.getBounds(), viewMatrix, HasAABloat::kYes,
                                   IsHairline::kYes);
    }

    const char* name() const override { return "AAHairlineOp"; }

    void visitProxies(const VisitProxyFunc& func) const override {
        fHelper.visitProxies(func);
    }

#ifdef SK_DEBUG
    SkString dumpInfo() const override {
        SkString string;
        string.appendf("Color: 0x%08x Coverage: 0x%02x, Count: %d\n", fColor.toBytes_RGBA(),
                       fCoverage, fPaths.count());
        string += INHERITED::dumpInfo();
        string += fHelper.dumpInfo();
        return string;
    }
#endif

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

    GrProcessorSet::Analysis finalize(
            const GrCaps& caps, const GrAppliedClip* clip, bool hasMixedSampledCoverage,
            GrClampType clampType) override {
        // This Op uses uniform (not vertex) color, so doesn't need to track wide color.
        return fHelper.finalizeProcessors(caps, clip, hasMixedSampledCoverage, clampType,
                                          GrProcessorAnalysisCoverage::kSingleChannel, &fColor,
                                          nullptr);
    }

private:
    void onPrepareDraws(Target*) override;
    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    typedef SkTArray<SkPoint, true> PtArray;
    typedef SkTArray<int, true> IntArray;
    typedef SkTArray<float, true> FloatArray;

    CombineResult onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        AAHairlineOp* that = t->cast<AAHairlineOp>();

        if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
            return CombineResult::kCannotCombine;
        }

        if (this->viewMatrix().hasPerspective() != that->viewMatrix().hasPerspective()) {
            return CombineResult::kCannotCombine;
        }

        // We go to identity if we don't have perspective
        if (this->viewMatrix().hasPerspective() &&
            !this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return CombineResult::kCannotCombine;
        }

        // TODO we can actually combine hairlines if they are the same color in a kind of bulk
        // method but we haven't implemented this yet
        // TODO investigate going to vertex color and coverage?
        if (this->coverage() != that->coverage()) {
            return CombineResult::kCannotCombine;
        }

        if (this->color() != that->color()) {
            return CombineResult::kCannotCombine;
        }

        if (fHelper.usesLocalCoords() && !this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return CombineResult::kCannotCombine;
        }

        fPaths.push_back_n(that->fPaths.count(), that->fPaths.begin());
        return CombineResult::kMerged;
    }

    const SkPMColor4f& color() const { return fColor; }
    uint8_t coverage() const { return fCoverage; }
    const SkMatrix& viewMatrix() const { return fPaths[0].fViewMatrix; }

    struct PathData {
        SkMatrix fViewMatrix;
        SkPath fPath;
        SkIRect fDevClipBounds;
        SkScalar fCapLength;
    };

    SkSTArray<1, PathData, true> fPaths;
    Helper fHelper;
    SkPMColor4f fColor;
    uint8_t fCoverage;

    typedef GrMeshDrawOp INHERITED;
};

}  // anonymous namespace

void AAHairlineOp::onPrepareDraws(Target* target) {
    // Setup the viewmatrix and localmatrix for the GrGeometryProcessor.
    SkMatrix invert;
    if (!this->viewMatrix().invert(&invert)) {
        return;
    }

    // we will transform to identity space if the viewmatrix does not have perspective
    bool hasPerspective = this->viewMatrix().hasPerspective();
    const SkMatrix* geometryProcessorViewM = &SkMatrix::I();
    const SkMatrix* geometryProcessorLocalM = &invert;
    const SkMatrix* toDevice = nullptr;
    const SkMatrix* toSrc = nullptr;
    if (hasPerspective) {
        geometryProcessorViewM = &this->viewMatrix();
        geometryProcessorLocalM = &SkMatrix::I();
        toDevice = &this->viewMatrix();
        toSrc = &invert;
    }

    // This is hand inlined for maximum performance.
    PREALLOC_PTARRAY(128) lines;
    PREALLOC_PTARRAY(128) quads;
    PREALLOC_PTARRAY(128) conics;
    IntArray qSubdivs;
    FloatArray cWeights;
    int quadCount = 0;

    int instanceCount = fPaths.count();
    bool convertConicsToQuads = !target->caps().shaderCaps()->floatIs32Bits();
    for (int i = 0; i < instanceCount; i++) {
        const PathData& args = fPaths[i];
        quadCount += gather_lines_and_quads(args.fPath, args.fViewMatrix, args.fDevClipBounds,
                                            args.fCapLength, convertConicsToQuads, &lines, &quads,
                                            &conics, &qSubdivs, &cWeights);
    }

    int lineCount = lines.count() / 2;
    int conicCount = conics.count() / 3;
    int quadAndConicCount = conicCount + quadCount;

    static constexpr int kMaxLines = SK_MaxS32 / kLineSegNumVertices;
    static constexpr int kMaxQuadsAndConics = SK_MaxS32 / kQuadNumVertices;
    if (lineCount > kMaxLines || quadAndConicCount > kMaxQuadsAndConics) {
        return;
    }

    // do lines first
    if (lineCount) {
        sk_sp<GrGeometryProcessor> lineGP;
        {
            using namespace GrDefaultGeoProcFactory;

            Color color(this->color());
            LocalCoords localCoords(fHelper.usesLocalCoords() ? LocalCoords::kUsePosition_Type
                                                              : LocalCoords::kUnused_Type);
            localCoords.fMatrix = geometryProcessorLocalM;
            lineGP = GrDefaultGeoProcFactory::Make(target->caps().shaderCaps(),
                                                   color, Coverage::kAttribute_Type, localCoords,
                                                   *geometryProcessorViewM);
        }

        sk_sp<const GrBuffer> linesIndexBuffer = get_lines_index_buffer(target->resourceProvider());

        sk_sp<const GrBuffer> vertexBuffer;
        int firstVertex;

        SkASSERT(sizeof(LineVertex) == lineGP->vertexStride());
        int vertexCount = kLineSegNumVertices * lineCount;
        LineVertex* verts = reinterpret_cast<LineVertex*>(target->makeVertexSpace(
                sizeof(LineVertex), vertexCount, &vertexBuffer, &firstVertex));

        if (!verts|| !linesIndexBuffer) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        for (int i = 0; i < lineCount; ++i) {
            add_line(&lines[2*i], toSrc, this->coverage(), &verts);
        }

        GrMesh* mesh = target->allocMesh(GrPrimitiveType::kTriangles);
        mesh->setIndexedPatterned(std::move(linesIndexBuffer), kIdxsPerLineSeg, kLineSegNumVertices,
                                  lineCount, kLineSegsNumInIdxBuffer);
        mesh->setVertexData(std::move(vertexBuffer), firstVertex);
        target->recordDraw(std::move(lineGP), mesh);
    }

    if (quadCount || conicCount) {
        sk_sp<GrGeometryProcessor> quadGP(GrQuadEffect::Make(this->color(),
                                                             *geometryProcessorViewM,
                                                             GrClipEdgeType::kHairlineAA,
                                                             target->caps(),
                                                             *geometryProcessorLocalM,
                                                             fHelper.usesLocalCoords(),
                                                             this->coverage()));

        sk_sp<GrGeometryProcessor> conicGP(GrConicEffect::Make(this->color(),
                                                               *geometryProcessorViewM,
                                                               GrClipEdgeType::kHairlineAA,
                                                               target->caps(),
                                                               *geometryProcessorLocalM,
                                                               fHelper.usesLocalCoords(),
                                                               this->coverage()));

        sk_sp<const GrBuffer> vertexBuffer;
        int firstVertex;

        sk_sp<const GrBuffer> quadsIndexBuffer = get_quads_index_buffer(target->resourceProvider());

        SkASSERT(sizeof(BezierVertex) == quadGP->vertexStride());
        SkASSERT(sizeof(BezierVertex) == conicGP->vertexStride());
        int vertexCount = kQuadNumVertices * quadAndConicCount;
        void* vertices = target->makeVertexSpace(sizeof(BezierVertex), vertexCount, &vertexBuffer,
                                                 &firstVertex);

        if (!vertices || !quadsIndexBuffer) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        // Setup vertices
        BezierVertex* bezVerts = reinterpret_cast<BezierVertex*>(vertices);

        int unsubdivQuadCnt = quads.count() / 3;
        for (int i = 0; i < unsubdivQuadCnt; ++i) {
            SkASSERT(qSubdivs[i] >= 0);
            add_quads(&quads[3*i], qSubdivs[i], toDevice, toSrc, &bezVerts);
        }

        // Start Conics
        for (int i = 0; i < conicCount; ++i) {
            add_conics(&conics[3*i], cWeights[i], toDevice, toSrc, &bezVerts);
        }

        if (quadCount > 0) {
            GrMesh* mesh = target->allocMesh(GrPrimitiveType::kTriangles);
            mesh->setIndexedPatterned(quadsIndexBuffer, kIdxsPerQuad, kQuadNumVertices, quadCount,
                                      kQuadsNumInIdxBuffer);
            mesh->setVertexData(vertexBuffer, firstVertex);
            target->recordDraw(std::move(quadGP), mesh);
            firstVertex += quadCount * kQuadNumVertices;
        }

        if (conicCount > 0) {
            GrMesh* mesh = target->allocMesh(GrPrimitiveType::kTriangles);
            mesh->setIndexedPatterned(std::move(quadsIndexBuffer), kIdxsPerQuad, kQuadNumVertices,
                                      conicCount, kQuadsNumInIdxBuffer);
            mesh->setVertexData(std::move(vertexBuffer), firstVertex);
            target->recordDraw(std::move(conicGP), mesh);
        }
    }
}

void AAHairlineOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    fHelper.executeDrawsAndUploads(this, flushState, chainBounds);
}

bool GrAAHairLinePathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fRenderTargetContext->auditTrail(),
                              "GrAAHairlinePathRenderer::onDrawPath");
    SkASSERT(args.fRenderTargetContext->numSamples() <= 1);

    SkIRect devClipBounds;
    args.fClip->getConservativeBounds(args.fRenderTargetContext->width(),
                                      args.fRenderTargetContext->height(),
                                      &devClipBounds);
    SkPath path;
    args.fShape->asPath(&path);
    std::unique_ptr<GrDrawOp> op =
            AAHairlineOp::Make(args.fContext, std::move(args.fPaint), *args.fViewMatrix, path,
                               args.fShape->style(), devClipBounds, args.fUserStencilSettings);
    args.fRenderTargetContext->addDrawOp(*args.fClip, std::move(op));
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS

GR_DRAW_OP_TEST_DEFINE(AAHairlineOp) {
    SkMatrix viewMatrix = GrTest::TestMatrix(random);
    SkPath path = GrTest::TestPath(random);
    SkIRect devClipBounds;
    devClipBounds.setEmpty();
    return AAHairlineOp::Make(context, std::move(paint), viewMatrix, path,
                              GrStyle::SimpleHairline(), devClipBounds,
                              GrGetRandomStencil(random, context));
}

#endif
