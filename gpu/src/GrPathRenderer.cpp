#include "GrPathRenderer.h"

#include "GrPoint.h"
#include "GrDrawTarget.h"
#include "GrPathIter.h"
#include "GrMemory.h"
#include "GrTexture.h"



GrDefaultPathRenderer::GrDefaultPathRenderer(bool singlePassWindingStencil)
    : fSinglePassWindingStencil(singlePassWindingStencil) {

}

////////////////////////////////////////////////////////////////////////////////
// Helpers for draw Path

#define STENCIL_OFF     0   // Always disable stencil (even when needed)
static const GrScalar gTolerance = GR_Scalar1;

static const uint32_t MAX_POINTS_PER_CURVE = 1 << 10;

static uint32_t quadratic_point_count(const GrPoint points[], GrScalar tol) {
    GrScalar d = points[1].distanceToLineSegmentBetween(points[0], points[2]);
    if (d < tol) {
        return 1;
    } else {
        // Each time we subdivide, d should be cut in 4. So we need to
        // subdivide x = log4(d/tol) times. x subdivisions creates 2^(x)
        // points.
        // 2^(log4(x)) = sqrt(x);
        d = ceilf(sqrtf(d/tol));
        return GrMin(GrNextPow2((uint32_t)d), MAX_POINTS_PER_CURVE);
    }
}

static uint32_t generate_quadratic_points(const GrPoint& p0,
                                          const GrPoint& p1,
                                          const GrPoint& p2,
                                          GrScalar tolSqd,
                                          GrPoint** points,
                                          uint32_t pointsLeft) {
    if (pointsLeft < 2 ||
        (p1.distanceToLineSegmentBetweenSqd(p0, p2)) < tolSqd) {
        (*points)[0] = p2;
        *points += 1;
        return 1;
    }

    GrPoint q[] = {
        GrPoint(GrScalarAve(p0.fX, p1.fX), GrScalarAve(p0.fY, p1.fY)),
        GrPoint(GrScalarAve(p1.fX, p2.fX), GrScalarAve(p1.fY, p2.fY)),
    };
    GrPoint r(GrScalarAve(q[0].fX, q[1].fX), GrScalarAve(q[0].fY, q[1].fY));

    pointsLeft >>= 1;
    uint32_t a = generate_quadratic_points(p0, q[0], r, tolSqd, points, pointsLeft);
    uint32_t b = generate_quadratic_points(r, q[1], p2, tolSqd, points, pointsLeft);
    return a + b;
}

static uint32_t cubic_point_count(const GrPoint points[], GrScalar tol) {
    GrScalar d = GrMax(points[1].distanceToLineSegmentBetweenSqd(points[0], points[3]),
                       points[2].distanceToLineSegmentBetweenSqd(points[0], points[3]));
    d = sqrtf(d);
    if (d < tol) {
        return 1;
    } else {
        d = ceilf(sqrtf(d/tol));
        return GrMin(GrNextPow2((uint32_t)d), MAX_POINTS_PER_CURVE);
    }
}

static uint32_t generate_cubic_points(const GrPoint& p0,
                                      const GrPoint& p1,
                                      const GrPoint& p2,
                                      const GrPoint& p3,
                                      GrScalar tolSqd,
                                      GrPoint** points,
                                      uint32_t pointsLeft) {
    if (pointsLeft < 2 ||
        (p1.distanceToLineSegmentBetweenSqd(p0, p3) < tolSqd &&
         p2.distanceToLineSegmentBetweenSqd(p0, p3) < tolSqd)) {
            (*points)[0] = p3;
            *points += 1;
            return 1;
        }
    GrPoint q[] = {
        GrPoint(GrScalarAve(p0.fX, p1.fX), GrScalarAve(p0.fY, p1.fY)),
        GrPoint(GrScalarAve(p1.fX, p2.fX), GrScalarAve(p1.fY, p2.fY)),
        GrPoint(GrScalarAve(p2.fX, p3.fX), GrScalarAve(p2.fY, p3.fY))
    };
    GrPoint r[] = {
        GrPoint(GrScalarAve(q[0].fX, q[1].fX), GrScalarAve(q[0].fY, q[1].fY)),
        GrPoint(GrScalarAve(q[1].fX, q[2].fX), GrScalarAve(q[1].fY, q[2].fY))
    };
    GrPoint s(GrScalarAve(r[0].fX, r[1].fX), GrScalarAve(r[0].fY, r[1].fY));
    pointsLeft >>= 1;
    uint32_t a = generate_cubic_points(p0, q[0], r[0], s, tolSqd, points, pointsLeft);
    uint32_t b = generate_cubic_points(s, r[1], q[2], p3, tolSqd, points, pointsLeft);
    return a + b;
}

static int worst_case_point_count(GrPathIter* path,
                                  int* subpaths,
                                  GrScalar tol) {
    int pointCount = 0;
    *subpaths = 1;

    bool first = true;

    GrPathIter::Command cmd;

    GrPoint pts[4];
    while ((cmd = path->next(pts)) != GrPathIter::kEnd_Command) {

        switch (cmd) {
            case GrPathIter::kLine_Command:
                pointCount += 1;
                break;
            case GrPathIter::kQuadratic_Command:
                pointCount += quadratic_point_count(pts, tol);
                break;
            case GrPathIter::kCubic_Command:
                pointCount += cubic_point_count(pts, tol);
                break;
            case GrPathIter::kMove_Command:
                pointCount += 1;
                if (!first) {
                    ++(*subpaths);
                }
                break;
            default:
                break;
        }
        first = false;
    }
    return pointCount;
}

static inline bool single_pass_path(const GrPathIter& path,
                                    GrPathFill fill,
                                    const GrDrawTarget& target) {
#if STENCIL_OFF
    return true;
#else
    if (kEvenOdd_PathFill == fill) {
        GrPathIter::ConvexHint hint = path.hint();
        return hint == GrPathIter::kConvex_ConvexHint ||
               hint == GrPathIter::kNonOverlappingConvexPieces_ConvexHint;
    } else if (kWinding_PathFill == fill) {
        GrPathIter::ConvexHint hint = path.hint();
        return hint == GrPathIter::kConvex_ConvexHint ||
               hint == GrPathIter::kNonOverlappingConvexPieces_ConvexHint ||
               (hint == GrPathIter::kSameWindingConvexPieces_ConvexHint &&
                target.canDisableBlend() && !target.isDitherState());

    }
    return false;
#endif
}

void GrDefaultPathRenderer::drawPath(GrDrawTarget* target,
                                     GrDrawTarget::StageBitfield stages,
                                     GrPathIter* path,
                                     GrPathFill fill,
                                     const GrPoint* translate) {

    GrDrawTarget::AutoStateRestore asr(target);

    GrMatrix viewM = target->getViewMatrix();
    // In order to tesselate the path we get a bound on how much the matrix can
    // stretch when mapping to screen coordinates.
    GrScalar stretch = viewM.getMaxStretch();
    bool useStretch = stretch > 0;
    GrScalar tol = gTolerance;

    if (!useStretch) {
        // TODO: deal with perspective in some better way.
        tol /= 10;
    } else {
        GrScalar sinv = GR_Scalar1 / stretch;
        tol = GrMul(tol, sinv);
    }
    GrScalar tolSqd = GrMul(tol, tol);

    int subpathCnt;
    int maxPts = worst_case_point_count(path,
                                        &subpathCnt,
                                        tol);

    GrVertexLayout layout = 0;
    for (int s = 0; s < GrDrawTarget::kNumStages; ++s) {
        if ((1 << s) & stages) {
            layout |= GrDrawTarget::StagePosAsTexCoordVertexLayoutBit(s);
        }
    }

    // add 4 to hold the bounding rect
    GrDrawTarget::AutoReleaseGeometry arg(target, layout, maxPts + 4, 0);

    GrPoint* base = (GrPoint*) arg.vertices();
    GrPoint* vert = base;
    GrPoint* subpathBase = base;

    GrAutoSTMalloc<8, uint16_t> subpathVertCount(subpathCnt);

    path->rewind();

    // TODO: use primitve restart if available rather than multiple draws
    GrPrimitiveType             type;
    int                         passCount = 0;
    GrDrawTarget::StencilPass   passes[3];
    bool                        reverse = false;

    if (kHairLine_PathFill == fill) {
        type = kLineStrip_PrimitiveType;
        passCount = 1;
        passes[0] = GrDrawTarget::kNone_StencilPass;
    } else {
        type = kTriangleFan_PrimitiveType;
        if (single_pass_path(*path, fill, *target)) {
            passCount = 1;
            passes[0] = GrDrawTarget::kNone_StencilPass;
        } else {
            switch (fill) {
                case kInverseEvenOdd_PathFill:
                    reverse = true;
                    // fallthrough
                case kEvenOdd_PathFill:
                    passCount = 2;
                    passes[0] = GrDrawTarget::kEvenOddStencil_StencilPass;
                    passes[1] = GrDrawTarget::kEvenOddColor_StencilPass;
                    break;

                case kInverseWinding_PathFill:
                    reverse = true;
                    // fallthrough
                case kWinding_PathFill:
                    passes[0] = GrDrawTarget::kWindingStencil1_StencilPass;
                    if (fSinglePassWindingStencil) {
                        passes[1] = GrDrawTarget::kWindingColor_StencilPass;
                        passCount = 2;
                    } else {
                        passes[1] = GrDrawTarget::kWindingStencil2_StencilPass;
                        passes[2] = GrDrawTarget::kWindingColor_StencilPass;
                        passCount = 3;
                    }
                    break;
                default:
                    GrAssert(!"Unknown path fill!");
                    return;
            }
        }
    }
    target->setReverseFill(reverse);

    GrPoint pts[4];

    bool first = true;
    int subpath = 0;

    for (;;) {
        GrPathIter::Command cmd = path->next(pts);
        switch (cmd) {
            case GrPathIter::kMove_Command:
                if (!first) {
                    subpathVertCount[subpath] = vert-subpathBase;
                    subpathBase = vert;
                    ++subpath;
                }
                *vert = pts[0];
                vert++;
                break;
            case GrPathIter::kLine_Command:
                *vert = pts[1];
                vert++;
                break;
            case GrPathIter::kQuadratic_Command: {
                generate_quadratic_points(pts[0], pts[1], pts[2],
                                          tolSqd, &vert,
                                          quadratic_point_count(pts, tol));
                break;
            }
            case GrPathIter::kCubic_Command: {
                generate_cubic_points(pts[0], pts[1], pts[2], pts[3],
                                      tolSqd, &vert,
                                      cubic_point_count(pts, tol));
                break;
            }
            case GrPathIter::kClose_Command:
                break;
            case GrPathIter::kEnd_Command:
                subpathVertCount[subpath] = vert-subpathBase;
                ++subpath; // this could be only in debug
                goto FINISHED;
        }
        first = false;
    }
FINISHED:
    GrAssert(subpath == subpathCnt);
    GrAssert((vert - base) <= maxPts);

    if (translate) {
        int count = vert - base;
        for (int i = 0; i < count; i++) {
            base[i].offset(translate->fX, translate->fY);
        }
    }

    // arbitrary path complexity cutoff
    bool useBounds = fill != kHairLine_PathFill &&
                    (reverse || (vert - base) > 8);
    GrPoint* boundsVerts = base + maxPts;
    if (useBounds) {
        GrRect bounds;
        if (reverse) {
            GrAssert(NULL != target->getRenderTarget());
            // draw over the whole world.
            bounds.setLTRB(0, 0,
                           GrIntToScalar(target->getRenderTarget()->width()),
                           GrIntToScalar(target->getRenderTarget()->height()));
            GrMatrix vmi;
            if (target->getViewInverse(&vmi)) {
                vmi.mapRect(&bounds);
            }
        } else {
            bounds.setBounds((GrPoint*)base, vert - base);
        }
        boundsVerts[0].setRectFan(bounds.fLeft, bounds.fTop, bounds.fRight,
                                  bounds.fBottom);
    }

    for (int p = 0; p < passCount; ++p) {
        target->setStencilPass(passes[p]);
        if (useBounds && (GrDrawTarget::kEvenOddColor_StencilPass == passes[p] ||
                          GrDrawTarget::kWindingColor_StencilPass == passes[p])) {
            target->drawNonIndexed(kTriangleFan_PrimitiveType,
                                 maxPts, 4);

        } else {
            int baseVertex = 0;
            for (int sp = 0; sp < subpathCnt; ++sp) {
                target->drawNonIndexed(type,
                                     baseVertex,
                                     subpathVertCount[sp]);
                baseVertex += subpathVertCount[sp];
            }
        }
    }
}
