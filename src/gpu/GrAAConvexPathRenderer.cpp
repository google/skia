
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAAConvexPathRenderer.h"

#include "GrContext.h"
#include "GrDrawState.h"
#include "GrPathUtils.h"
#include "SkString.h"
#include "SkTrace.h"


GrAAConvexPathRenderer::GrAAConvexPathRenderer() {
}

bool GrAAConvexPathRenderer::canDrawPath(const GrDrawTarget::Caps& targetCaps,
                                         const SkPath& path,
                                         GrPathFill fill,
                                         bool antiAlias) const {
    return targetCaps.fShaderDerivativeSupport && antiAlias &&
           kHairLine_PathFill != fill && !GrIsFillInverted(fill) &&
           path.isConvex();
}

namespace {


struct Segment {
    enum {
        kLine,
        kQuad
    } fType;
    // line uses a, quad uses a and b (first point comes from prev. segment)
    GrPoint fA, fB;
    // normal to edge ending at a and b
    GrVec fANorm, fBNorm;
    // mid vector at a that splits angle with previous edge
    GrVec fPrevMid;
};

typedef SkTArray<Segment, true> SegmentArray;

bool is_path_degenerate(const GrPath& path) {
    int n = path.countPoints();
    if (n < 3) {
        return true;
    }

    // compute a line from the first two points that are not equal, look for
    // a third pt that is off the line.
    static const SkScalar TOL = (SK_Scalar1 / 16);
    bool foundLine = false;
    GrPoint firstPoint = path.getPoint(0);
    GrVec lineV;
    SkScalar lineC;
    int i = 1;

    do {
        GrPoint pt = path.getPoint(i);
        if (!foundLine) {
            if (pt != firstPoint) {
                lineV = pt - firstPoint;
                lineV.normalize();
                lineV.setOrthog(lineV);
                lineC = lineV.dot(firstPoint);
                foundLine = true;
            }
        } else {
            if (SkScalarAbs(lineV.dot(pt) - lineC) > TOL) {
                return false;
            }
        }
        ++i;
    } while (i < n);
    return true;
}

bool get_segments(const GrPath& path,
                 SegmentArray* segments,
                 int* quadCnt,
                 int* lineCnt) {
    *quadCnt = 0;
    *lineCnt = 0;
    SkPath::Iter iter(path, true);
    // This renderer overemphasis very thin paths (every pixel intersected by
    // the path will be at least 1/2 on). When the path degenerates to a line
    // this makes the path draw as a hairline. This is a pretty glaring error
    // so we detect this case and will not draw.
    if (is_path_degenerate(path)) {
        return false;
    }
    for (;;) {
        GrPoint pts[4];
        GrPathCmd cmd = (GrPathCmd)iter.next(pts);
        switch (cmd) {
            case kLine_PathCmd: {
                segments->push_back();
                segments->back().fType = Segment::kLine;
                segments->back().fA = pts[1];
                ++(*lineCnt);
                break;
            }
            case kQuadratic_PathCmd:
                segments->push_back();
                segments->back().fType = Segment::kQuad;
                segments->back().fA = pts[1];
                segments->back().fB = pts[2];
                ++(*quadCnt);
                break;
            case kCubic_PathCmd: {
                SkSTArray<15, SkPoint, true> quads;
                GrPathUtils::convertCubicToQuads(pts, SK_Scalar1, &quads);
                int count = quads.count();
                for (int q = 0; q < count; q += 3) {
                    segments->push_back();
                    segments->back().fType = Segment::kQuad;
                    segments->back().fA = quads[q + 1];
                    segments->back().fB = quads[q + 2];
                    ++(*quadCnt);
                }
                break;
            };
            case kEnd_PathCmd:
                GrAssert(*quadCnt + *lineCnt == segments->count());
                return true;
            default:
                break;
        }
    }
}

struct QuadVertex {
    GrPoint  fPos;
    union {
        GrPoint fQuadUV;
        GrScalar fEdge[4];
    };
};

void get_counts(int quadCount, int lineCount, int* vCount, int* iCount) {
    *vCount = 9 * lineCount + 11 * quadCount;
    *iCount = 15  * lineCount + 24 * quadCount;
}

// for visual debugging, exagerate the AA smear at the edges
// requires modifying the distance calc in the shader actually shade differently
//#define STRETCH_AA
#define STRETCH_FACTOR (20 * SK_Scalar1)

void create_vertices(SegmentArray* segments,
                     const GrPoint& fanPt,
                     QuadVertex* verts,
                     uint16_t* idxs) {
    int count = segments->count();
    GrAssert(count > 1);
    int prevS = count - 1;
    const Segment& lastSeg = (*segments)[prevS];

    // walk the segments and compute normals to each edge and
    // bisectors at vertices. The loop relies on having the end point and normal
    // from previous segment so we first compute that. Also, we determine
    // whether normals point left or right to face outside the path.
    GrVec prevPt;
    GrPoint prevPrevPt;
    GrVec prevNorm;
    if (Segment::kLine == lastSeg.fType) {
        prevPt = lastSeg.fA;
        const Segment& secondLastSeg = (*segments)[prevS - 1];
        prevPrevPt = (Segment::kLine == secondLastSeg.fType) ?
                                                      secondLastSeg.fA :
                                                      secondLastSeg.fB;
    } else {
        prevPt = lastSeg.fB;
        prevPrevPt = lastSeg.fA;
    }
    GrVec::Side outside;
    // we will compute our edge vectors so that they are pointing along the
    // direction in which we are iterating the path. So here we take an opposite
    // vector and get the side that the fan pt lies relative to it.
    fanPt.distanceToLineBetweenSqd(prevPrevPt, prevPt, &outside);
    prevNorm = prevPt - prevPrevPt;
    prevNorm.normalize();
    prevNorm.setOrthog(prevNorm, outside);
#ifdef STRETCH_AA
    prevNorm.scale(STRETCH_FACTOR);
#endif

    // compute the normals and bisectors
    for (int s = 0; s < count; ++s, ++prevS) {
        Segment& curr = (*segments)[s];

        GrVec currVec = curr.fA - prevPt;
        currVec.normalize();
        curr.fANorm.setOrthog(currVec, outside);
#ifdef STRETCH_AA
        curr.fANorm.scale(STRETCH_FACTOR);
#endif
        curr.fPrevMid = prevNorm + curr.fANorm;
        curr.fPrevMid.normalize();
#ifdef STRETCH_AA
        curr.fPrevMid.scale(STRETCH_FACTOR);
#endif
        if (Segment::kLine == curr.fType) {
            prevPt = curr.fA;
            prevNorm = curr.fANorm;
        } else {
            currVec = curr.fB - curr.fA;
            currVec.normalize();
            curr.fBNorm.setOrthog(currVec, outside);
#ifdef STRETCH_AA
            curr.fBNorm.scale(STRETCH_FACTOR);
#endif
            prevPt = curr.fB;
            prevNorm = curr.fBNorm;
        }
    }

    // compute the vertices / indices
    if (Segment::kLine == lastSeg.fType) {
        prevPt = lastSeg.fA;
        prevNorm = lastSeg.fANorm;
    } else {
        prevPt = lastSeg.fB;
        prevNorm = lastSeg.fBNorm;
    }
    int v = 0;
    int i = 0;
    for (int s = 0; s < count; ++s, ++prevS) {
        Segment& curr = (*segments)[s];
        verts[v + 0].fPos = prevPt;
        verts[v + 1].fPos = prevPt + prevNorm;
        verts[v + 2].fPos = prevPt + curr.fPrevMid;
        verts[v + 3].fPos = prevPt + curr.fANorm;
        verts[v + 0].fQuadUV.set(0, 0);
        verts[v + 1].fQuadUV.set(0, -SK_Scalar1);
        verts[v + 2].fQuadUV.set(0, -SK_Scalar1);
        verts[v + 3].fQuadUV.set(0, -SK_Scalar1);

        idxs[i + 0] = v + 0;
        idxs[i + 1] = v + 1;
        idxs[i + 2] = v + 2;
        idxs[i + 3] = v + 0;
        idxs[i + 4] = v + 2;
        idxs[i + 5] = v + 3;

        v += 4;
        i += 6;

        if (Segment::kLine == curr.fType) {
            verts[v + 0].fPos = fanPt;
            verts[v + 1].fPos = prevPt;
            verts[v + 2].fPos = curr.fA;
            verts[v + 3].fPos = prevPt + curr.fANorm;
            verts[v + 4].fPos = curr.fA + curr.fANorm;
            GrScalar lineC = -curr.fANorm.dot(curr.fA);
            GrScalar fanDist = curr.fANorm.dot(fanPt) - lineC;
            verts[v + 0].fQuadUV.set(0, SkScalarAbs(fanDist));
            verts[v + 1].fQuadUV.set(0, 0);
            verts[v + 2].fQuadUV.set(0, 0);
            verts[v + 3].fQuadUV.set(0, -GR_Scalar1);
            verts[v + 4].fQuadUV.set(0, -GR_Scalar1);

            idxs[i + 0] = v + 0;
            idxs[i + 1] = v + 1;
            idxs[i + 2] = v + 2;
            idxs[i + 3] = v + 1;
            idxs[i + 4] = v + 3;
            idxs[i + 5] = v + 4;
            idxs[i + 6] = v + 1;
            idxs[i + 7] = v + 4;
            idxs[i + 8] = v + 2;

            i += 9;
            v += 5;

            prevPt = curr.fA;
            prevNorm = curr.fANorm;
        } else {
            GrVec splitVec = curr.fANorm + curr.fBNorm;
            splitVec.normalize();
#ifdef STRETCH_AA
            splitVec.scale(STRETCH_FACTOR);
#endif

            verts[v + 0].fPos = prevPt;
            verts[v + 1].fPos = curr.fA;
            verts[v + 2].fPos = curr.fB;
            verts[v + 3].fPos = fanPt;
            verts[v + 4].fPos = prevPt + curr.fANorm;
            verts[v + 5].fPos = curr.fA + splitVec;
            verts[v + 6].fPos = curr.fB + curr.fBNorm;

            verts[v + 0].fQuadUV.set(0, 0);
            verts[v + 1].fQuadUV.set(GR_ScalarHalf, 0);
            verts[v + 2].fQuadUV.set(GR_Scalar1, GR_Scalar1);
            GrMatrix toUV;
            GrPoint pts[] = {prevPt, curr.fA, curr.fB};
            GrPathUtils::quadDesignSpaceToUVCoordsMatrix(pts, &toUV);
            toUV.mapPointsWithStride(&verts[v + 3].fQuadUV,
                                     &verts[v + 3].fPos,
                                     sizeof(QuadVertex), 4);

            idxs[i +  0] = v + 3;
            idxs[i +  1] = v + 0;
            idxs[i +  2] = v + 1;
            idxs[i +  3] = v + 3;
            idxs[i +  4] = v + 1;
            idxs[i +  5] = v + 2;
            idxs[i +  6] = v + 0;
            idxs[i +  7] = v + 4;
            idxs[i +  8] = v + 1;
            idxs[i +  9] = v + 4;
            idxs[i + 10] = v + 1;
            idxs[i + 11] = v + 5;
            idxs[i + 12] = v + 5;
            idxs[i + 13] = v + 1;
            idxs[i + 14] = v + 2;
            idxs[i + 15] = v + 5;
            idxs[i + 16] = v + 2;
            idxs[i + 17] = v + 6;

            i += 18;
            v += 7;
            prevPt = curr.fB;
            prevNorm = curr.fBNorm;
        }
    }
}

}

void GrAAConvexPathRenderer::drawPath(GrDrawState::StageMask stageMask) {
    GrAssert(fPath->isConvex());
    if (fPath->isEmpty()) {
        return;
    }
    GrDrawState* drawState = fTarget->drawState();

    GrDrawTarget::AutoStateRestore asr;
    GrMatrix vm = drawState->getViewMatrix();
    vm.postTranslate(fTranslate.fX, fTranslate.fY);
    asr.set(fTarget);
    GrMatrix ivm;
    if (vm.invert(&ivm)) {
        drawState->preConcatSamplerMatrices(stageMask, ivm);
    }
    drawState->setViewMatrix(GrMatrix::I());


    SkPath path;
    fPath->transform(vm, &path);

    SkPoint fanPt = {path.getBounds().centerX(),
                     path.getBounds().centerY()};

    GrVertexLayout layout = 0;
    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        if ((1 << s) & stageMask) {
            layout |= GrDrawTarget::StagePosAsTexCoordVertexLayoutBit(s);
        }
    }
    layout |= GrDrawTarget::kEdge_VertexLayoutBit;

    QuadVertex *verts;
    uint16_t* idxs;

    int nQuads;
    int nLines;
    SegmentArray segments;
    if (!get_segments(path, &segments, &nQuads, &nLines)) {
        return;
    }
    int vCount;
    int iCount;
    get_counts(nQuads, nLines, &vCount, &iCount);

    if (!fTarget->reserveVertexSpace(layout,
                                     vCount,
                                     reinterpret_cast<void**>(&verts))) {
        return;
    }
    if (!fTarget->reserveIndexSpace(iCount, reinterpret_cast<void**>(&idxs))) {
        fTarget->resetVertexSource();
        return;
    }

    create_vertices(&segments, fanPt, verts, idxs);

    drawState->setVertexEdgeType(GrDrawState::kQuad_EdgeType);
    fTarget->drawIndexed(kTriangles_PrimitiveType,
                         0,        // start vertex
                         0,        // start index
                         vCount,
                         iCount);
}

