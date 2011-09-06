#include "GrAAHairLinePathRenderer.h"

#include "GrContext.h"
#include "GrGpu.h"
#include "GrIndexBuffer.h"
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
    if (CanBeUsed(context)) {
        const GrIndexBuffer* lIdxBuffer = context->getQuadIndexBuffer();
        if (NULL == lIdxBuffer) {
            return NULL;
        }
        GrGpu* gpu = context->getGpu();
        GrIndexBuffer* qIdxBuf = gpu->createIndexBuffer(kQuadIdxSBufize, false);
        SkAutoTUnref<GrIndexBuffer> qIdxBuffer(qIdxBuf); // cons will take a ref
        if (NULL == qIdxBuf ||
            !push_quad_index_data(qIdxBuffer.get())) {
            return NULL;
        }
        return new GrAAHairLinePathRenderer(context,
                                            lIdxBuffer,
                                            qIdxBuf);
    } else {
        return NULL;
    }
}

bool GrAAHairLinePathRenderer::CanBeUsed(const GrContext* context) {
    return context->getGpu()->supportsShaderDerivatives();

}

GrAAHairLinePathRenderer::GrAAHairLinePathRenderer(
                                        const GrContext* context,
                                        const GrIndexBuffer* linesIndexBuffer,
                                        const GrIndexBuffer* quadsIndexBuffer) {
    GrAssert(CanBeUsed(context));
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

bool GrAAHairLinePathRenderer::supportsAA(GrDrawTarget* target,
                                          const SkPath& path,
                                          GrPathFill fill) { 
    return kHairLine_PathFill == fill;
}

bool GrAAHairLinePathRenderer::canDrawPath(const GrDrawTarget* target,
                                           const SkPath& path,
                                           GrPathFill fill) const {
    // TODO: support perspective
    return kHairLine_PathFill == fill &&
           !target->getViewMatrix().hasPerspective();
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

typedef GrTArray<SkPoint, true> PtArray;
typedef GrTArray<int, true> IntArray;

/**
 * We convert cubics to quadratics (for now).
 */
void convert_noninflect_cubic_to_quads(const SkPoint p[4],
                                       PtArray* quads,
                                       int sublevel = 0) {
    SkVector ab = p[1];
    ab -= p[0];
    SkVector dc = p[2];
    dc -= p[3];

    static const SkScalar gLengthScale = 3 * SK_Scalar1 / 2;
    static const SkScalar gDistanceSqdTol = 2 * SK_Scalar1;
    static const int kMaxSubdivs = 30;

    ab.scale(gLengthScale);
    dc.scale(gLengthScale);

    SkVector c0 = p[0];
    c0 += ab;
    SkVector c1 = p[3];
    c1 += dc;

    SkScalar dSqd = c0.distanceToSqd(c1);
    if (sublevel > kMaxSubdivs || dSqd <= gDistanceSqdTol) {
        SkPoint cAvg = c0;
        cAvg += c1;
        cAvg.scale(SK_ScalarHalf);

        int idx = quads->count();
        quads->push_back_n(3);
        (*quads)[idx+0] = p[0];
        (*quads)[idx+1] = cAvg;
        (*quads)[idx+2] = p[3];

        return;
    } else {
        SkPoint choppedPts[7];
        SkChopCubicAtHalf(p, choppedPts);
        convert_noninflect_cubic_to_quads(choppedPts + 0, quads, sublevel + 1);
        convert_noninflect_cubic_to_quads(choppedPts + 3, quads, sublevel + 1);
    }
}

void convert_cubic_to_quads(const SkPoint p[4], PtArray* quads) {
    SkPoint chopped[13];
    int count = SkChopCubicAtInflections(p, chopped);

    for (int i = 0; i < count; ++i) {
        SkPoint* cubic = chopped + 3*i;
        convert_noninflect_cubic_to_quads(cubic, quads);
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
    return (((*(int*)&x) & 0x7f800000) >> 23) - 127;
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

int get_lines_and_quads(const SkPath& path, const SkMatrix& m, GrIRect clip,
                        PtArray* lines, PtArray* quads,
                        IntArray* quadSubdivCnts) {
    SkPath::Iter iter(path, false);

    int totalQuadCount = 0;
    GrRect bounds;
    GrIRect ibounds;
    for (;;) {
        GrPoint pts[4];
        GrPathCmd cmd = (GrPathCmd)iter.next(pts);
        switch (cmd) {
            case kMove_PathCmd:
                break;
            case kLine_PathCmd:
                m.mapPoints(pts,2);
                bounds.setBounds(pts, 2);
                bounds.outset(SK_Scalar1, SK_Scalar1);
                bounds.roundOut(&ibounds);
                if (SkIRect::Intersects(clip, ibounds)) {
                    lines->push_back() = pts[0];
                    lines->push_back() = pts[1];
                }
                break;
            case kQuadratic_PathCmd: {
                bounds.setBounds(pts, 3);
                bounds.outset(SK_Scalar1, SK_Scalar1);
                bounds.roundOut(&ibounds);
                if (SkIRect::Intersects(clip, ibounds)) {
                    m.mapPoints(pts, 3);
                    int subdiv = num_quad_subdivs(pts);
                    GrAssert(subdiv >= -1);
                    if (-1 == subdiv) {
                        lines->push_back() = pts[0];
                        lines->push_back() = pts[1];
                        lines->push_back() = pts[1];
                        lines->push_back() = pts[2];
                    } else {
                        quads->push_back() = pts[0];
                        quads->push_back() = pts[1];
                        quads->push_back() = pts[2];
                        quadSubdivCnts->push_back() = subdiv;
                        totalQuadCount += 1 << subdiv;
                    }
                }
            } break;
            case kCubic_PathCmd: {
                bounds.setBounds(pts, 4);
                bounds.outset(SK_Scalar1, SK_Scalar1);
                bounds.roundOut(&ibounds);
                if (SkIRect::Intersects(clip, ibounds)) {
                    m.mapPoints(pts, 4);
                    SkPoint stackStorage[32];
                    PtArray q((void*)stackStorage, 32);
                    convert_cubic_to_quads(pts, &q);
                    for (int i = 0; i < q.count(); i += 3) {
                        bounds.setBounds(&q[i], 3);
                        bounds.outset(SK_Scalar1, SK_Scalar1);
                        bounds.roundOut(&ibounds);
                        if (SkIRect::Intersects(clip, ibounds)) {
                            int subdiv = num_quad_subdivs(&q[i]);
                            GrAssert(subdiv >= -1);
                            if (-1 == subdiv) {
                                lines->push_back() = q[0 + i];
                                lines->push_back() = q[1 + i];
                                lines->push_back() = q[1 + i];
                                lines->push_back() = q[2 + i];
                            } else {
                                quads->push_back() = q[0 + i];
                                quads->push_back() = q[1 + i];
                                quads->push_back() = q[2 + i];
                                quadSubdivCnts->push_back() = subdiv;
                                totalQuadCount += 1 << subdiv;
                            }
                        }
                    }
                }
            } break;
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

void bloat_quad(const SkPoint qpts[3], Vertex verts[kVertsPerQuad]) {
    // original quad is specified by tri a,b,c
    const SkPoint& a = qpts[0];
    const SkPoint& b = qpts[1];
    const SkPoint& c = qpts[2];
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

    // compute a matrix that goes from device coords to U,V quad params
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

    DevToUV.mapPointsWithStride(&verts[0].fQuadCoord,
                                &verts[0].fPos, sizeof(Vertex), kVertsPerQuad);
}

void add_quads(const SkPoint p[3],
               int subdiv,
               Vertex** vert) {
    GrAssert(subdiv >= 0);
    if (subdiv) {
        SkPoint newP[5];
        SkChopQuadAtHalf(p, newP);
        add_quads(newP + 0, subdiv-1, vert);
        add_quads(newP + 2, subdiv-1, vert);
    } else {
        bloat_quad(p, *vert);
        *vert += kVertsPerQuad;
    }
}

void add_line(const SkPoint p[2],
              int rtHeight,
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

    if (stages == fPreviousStages &&
        fPreviousViewMatrix == fTarget->getViewMatrix() &&
        rtHeight == fPreviousRTHeight &&
        fClipRect == clip) {
        return true;
    }

    GrVertexLayout layout = GrDrawTarget::kEdge_VertexLayoutBit;
    for (int s = 0; s < GrDrawTarget::kNumStages; ++s) {
        if ((1 << s) & stages) {
            layout |= GrDrawTarget::StagePosAsTexCoordVertexLayoutBit(s);
        }
    }

    GrMatrix viewM = fTarget->getViewMatrix();
    viewM.preTranslate(fTranslate.fX, fTranslate.fY);

    GrAlignedSTStorage<128, GrPoint> lineStorage;
    GrAlignedSTStorage<128, GrPoint> quadStorage;
    PtArray lines(&lineStorage);
    PtArray quads(&quadStorage);
    IntArray qSubdivs;
    fQuadCnt = get_lines_and_quads(*fPath, viewM, clip,
                                   &lines, &quads, &qSubdivs);

    fLineSegmentCnt = lines.count() / 2;
    int vertCnt = kVertsPerLineSeg * fLineSegmentCnt + kVertsPerQuad * fQuadCnt;

    GrAssert(sizeof(Vertex) == GrDrawTarget::VertexSize(layout));

    Vertex* verts;
    if (!fTarget->reserveVertexSpace(layout, vertCnt, (void**)&verts)) {
        return false;
    }

    for (int i = 0; i < fLineSegmentCnt; ++i) {
        add_line(&lines[2*i], rtHeight, &verts);
    }
    int unsubdivQuadCnt = quads.count() / 3;
    for (int i = 0; i < unsubdivQuadCnt; ++i) {
        GrAssert(qSubdivs[i] >= 0);
        add_quads(&quads[3*i], qSubdivs[i], &verts);
    }

    fPreviousStages = stages;
    fPreviousViewMatrix = fTarget->getViewMatrix();
    fPreviousRTHeight = rtHeight;
    fClipRect = clip;
    return true;
}

void GrAAHairLinePathRenderer::drawPath(GrDrawTarget::StageBitfield stages) {
    GrDrawTarget::AutoStateRestore asr(fTarget);

    GrMatrix ivm;

    if (!this->createGeom(stages)) {
        return;
    }

    if (fTarget->getViewInverse(&ivm)) {
        fTarget->preConcatSamplerMatrices(stages, ivm);
    }

    fTarget->setViewMatrix(GrMatrix::I());

    // TODO: See whether rendering lines as degenerate quads improves perf
    // when we have a mix
    fTarget->setIndexSourceToBuffer(fLinesIndexBuffer);
    int lines = 0;
    int nBufLines = fLinesIndexBuffer->maxQuads();
    while (lines < fLineSegmentCnt) {
        int n = GrMin(fLineSegmentCnt-lines, nBufLines);
        fTarget->setVertexEdgeType(GrDrawTarget::kHairLine_EdgeType);
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
        fTarget->setVertexEdgeType(GrDrawTarget::kHairQuad_EdgeType);
        fTarget->drawIndexed(kTriangles_PrimitiveType,
                             4*fLineSegmentCnt + kVertsPerQuad*quads, // startV
                             0,                                       // startI
                             kVertsPerQuad*n,                         // vCount
                             kIdxsPerQuad*n);                         // iCount
        quads += n;
    }

}

