#include "GrPathRenderer.h"

#include "GrPoint.h"
#include "GrDrawTarget.h"
#include "GrPathUtils.h"
#include "GrMemory.h"
#include "GrTexture.h"

GrPathRenderer::GrPathRenderer()
    : fCurveTolerance (GR_Scalar1) {

}
   
GrDefaultPathRenderer::GrDefaultPathRenderer(bool separateStencilSupport,
                                             bool stencilWrapOpsSupport)
    : fSeparateStencil(separateStencilSupport)
    , fStencilWrapOps(stencilWrapOpsSupport) {

}

////////////////////////////////////////////////////////////////////////////////
// Stencil rules for paths

////// Even/Odd

static const GrStencilSettings gEOStencilPass = {
    kInvert_StencilOp,           kInvert_StencilOp,
    kKeep_StencilOp,             kKeep_StencilOp,
    kAlwaysIfInClip_StencilFunc, kAlwaysIfInClip_StencilFunc,
    0xffffffff,                  0xffffffff,
    0xffffffff,                  0xffffffff,
    0xffffffff,                  0xffffffff
};

// ok not to check clip b/c stencil pass only wrote inside clip
static const GrStencilSettings gEOColorPass = {
    kZero_StencilOp,          kZero_StencilOp,
    kZero_StencilOp,          kZero_StencilOp,
    kNotEqual_StencilFunc,    kNotEqual_StencilFunc,
    0xffffffff,               0xffffffff,
    0x0,                      0x0,
    0xffffffff,               0xffffffff
};

// have to check clip b/c outside clip will always be zero.
static const GrStencilSettings gInvEOColorPass = {
    kZero_StencilOp,            kZero_StencilOp,
    kZero_StencilOp,            kZero_StencilOp,
    kEqualIfInClip_StencilFunc, kEqualIfInClip_StencilFunc,
    0xffffffff,                 0xffffffff,
    0x0,                        0x0,
    0xffffffff,                 0xffffffff
};

////// Winding

// when we have separate stencil we increment front faces / decrement back faces
// when we don't have wrap incr and decr we use the stencil test to simulate
// them.

static const GrStencilSettings gWindStencilSeparateWithWrap = {
    kIncWrap_StencilOp,             kDecWrap_StencilOp,
    kKeep_StencilOp,                kKeep_StencilOp,
    kAlwaysIfInClip_StencilFunc,    kAlwaysIfInClip_StencilFunc,
    0xffffffff,                     0xffffffff,
    0xffffffff,                     0xffffffff,
    0xffffffff,                     0xffffffff
};

// if inc'ing the max value, invert to make 0
// if dec'ing zero invert to make all ones.
// we can't avoid touching the stencil on both passing and
// failing, so we can't resctrict ourselves to the clip.
static const GrStencilSettings gWindStencilSeparateNoWrap = {
    kInvert_StencilOp,              kInvert_StencilOp,
    kIncClamp_StencilOp,            kDecClamp_StencilOp,
    kEqual_StencilFunc,             kEqual_StencilFunc,
    0xffffffff,                     0xffffffff,
    0xffffffff,                     0x0,
    0xffffffff,                     0xffffffff
};

// When there are no separate faces we do two passes to setup the winding rule
// stencil. First we draw the front faces and inc, then we draw the back faces
// and dec. These are same as the above two split into the incrementing and
// decrementing passes.
static const GrStencilSettings gWindSingleStencilWithWrapInc = {
    kIncWrap_StencilOp,             kIncWrap_StencilOp,
    kKeep_StencilOp,                kKeep_StencilOp,
    kAlwaysIfInClip_StencilFunc,    kAlwaysIfInClip_StencilFunc,
    0xffffffff,                     0xffffffff,
    0xffffffff,                     0xffffffff,
    0xffffffff,                     0xffffffff
};
static const GrStencilSettings gWindSingleStencilWithWrapDec = {
    kDecWrap_StencilOp,             kDecWrap_StencilOp,
    kKeep_StencilOp,                kKeep_StencilOp,
    kAlwaysIfInClip_StencilFunc,    kAlwaysIfInClip_StencilFunc,
    0xffffffff,                     0xffffffff,
    0xffffffff,                     0xffffffff,
    0xffffffff,                     0xffffffff
};
static const GrStencilSettings gWindSingleStencilNoWrapInc = {
    kInvert_StencilOp,              kInvert_StencilOp,
    kIncClamp_StencilOp,            kIncClamp_StencilOp,
    kEqual_StencilFunc,             kEqual_StencilFunc,
    0xffffffff,                     0xffffffff,
    0xffffffff,                     0xffffffff,
    0xffffffff,                     0xffffffff
};
static const GrStencilSettings gWindSingleStencilNoWrapDec = {
    kInvert_StencilOp,              kInvert_StencilOp,
    kDecClamp_StencilOp,            kDecClamp_StencilOp,
    kEqual_StencilFunc,             kEqual_StencilFunc,
    0xffffffff,                     0xffffffff,
    0x0,                            0x0,
    0xffffffff,                     0xffffffff
};

static const GrStencilSettings gWindColorPass = {
    kZero_StencilOp,                kZero_StencilOp,
    kZero_StencilOp,                kZero_StencilOp,
    kNonZeroIfInClip_StencilFunc,   kNonZeroIfInClip_StencilFunc,
    0xffffffff,                     0xffffffff,
    0x0,                            0x0,
    0xffffffff,                     0xffffffff
};

static const GrStencilSettings gInvWindColorPass = {
    kZero_StencilOp,                kZero_StencilOp,
    kZero_StencilOp,                kZero_StencilOp,
    kEqualIfInClip_StencilFunc,     kEqualIfInClip_StencilFunc,
    0xffffffff,                     0xffffffff,
    0x0,                            0x0,
    0xffffffff,                     0xffffffff
};

////// Normal render to stencil

// Sometimes the default path renderer can draw a path directly to the stencil
// buffer without having to first resolve the interior / exterior.
static const GrStencilSettings gDirectToStencil = {
    kZero_StencilOp,                kZero_StencilOp,
    kIncClamp_StencilOp,            kIncClamp_StencilOp,
    kAlwaysIfInClip_StencilFunc,    kAlwaysIfInClip_StencilFunc,
    0xffffffff,                     0xffffffff,
    0x0,                            0x0,
    0xffffffff,                     0xffffffff
};

////////////////////////////////////////////////////////////////////////////////
// Helpers for drawPath

static GrConvexHint getConvexHint(const SkPath& path) {
    return path.isConvex() ? kConvex_ConvexHint : kConcave_ConvexHint;
}

#define STENCIL_OFF     0   // Always disable stencil (even when needed)

static inline bool single_pass_path(const GrDrawTarget& target,
                                    const GrPath& path,
                                    GrPathFill fill) {
#if STENCIL_OFF
    return true;
#else
    if (kEvenOdd_PathFill == fill) {
        GrConvexHint hint = getConvexHint(path);
        return hint == kConvex_ConvexHint ||
               hint == kNonOverlappingConvexPieces_ConvexHint;
    } else if (kWinding_PathFill == fill) {
        GrConvexHint hint = getConvexHint(path);
        return hint == kConvex_ConvexHint ||
               hint == kNonOverlappingConvexPieces_ConvexHint ||
               (hint == kSameWindingConvexPieces_ConvexHint &&
                target.canDisableBlend() && !target.isDitherState());

    }
    return false;
#endif
}

bool GrDefaultPathRenderer::requiresStencilPass(const GrDrawTarget* target,
                                                const GrPath& path, 
                                                GrPathFill fill) const {
    return !single_pass_path(*target, path, fill);
}

void GrDefaultPathRenderer::onDrawPath(GrDrawTarget* target,
                                       GrDrawTarget::StageBitfield stages,
                                       const GrPath& path,
                                       GrPathFill fill,
                                       const GrPoint* translate,
                                       bool stencilOnly) {

    GrDrawTarget::AutoStateRestore asr(target);
    bool colorWritesWereDisabled = target->isColorWriteDisabled();
    // face culling doesn't make sense here
    GrAssert(GrDrawTarget::kBoth_DrawFace == target->getDrawFace());

    GrMatrix viewM = target->getViewMatrix();
    // In order to tesselate the path we get a bound on how much the matrix can
    // stretch when mapping to screen coordinates.
    GrScalar stretch = viewM.getMaxStretch();
    bool useStretch = stretch > 0;
    GrScalar tol = fCurveTolerance;

    if (!useStretch) {
        // TODO: deal with perspective in some better way.
        tol /= 10;
    } else {
        tol = GrScalarDiv(tol, stretch);
    }
    GrScalar tolSqd = GrMul(tol, tol);

    int subpathCnt;
    int maxPts = GrPathUtils::worstCasePointCount(path, &subpathCnt, tol);

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

    // TODO: use primitve restart if available rather than multiple draws
    GrPrimitiveType             type;
    int                         passCount = 0;
    const GrStencilSettings*    passes[3];
    GrDrawTarget::DrawFace      drawFace[3];
    bool                        reverse = false;
    bool                        lastPassIsBounds;

    if (kHairLine_PathFill == fill) {
        type = kLineStrip_PrimitiveType;
        passCount = 1;
        if (stencilOnly) {
            passes[0] = &gDirectToStencil;
        } else {
            passes[0] = NULL;
        }
        lastPassIsBounds = false;
        drawFace[0] = GrDrawTarget::kBoth_DrawFace;
    } else {
        type = kTriangleFan_PrimitiveType;
        if (single_pass_path(*target, path, fill)) {
            passCount = 1;
            if (stencilOnly) {
                passes[0] = &gDirectToStencil;
            } else {
                passes[0] = NULL;
            }
            drawFace[0] = GrDrawTarget::kBoth_DrawFace;
            lastPassIsBounds = false;
        } else {
            switch (fill) {
                case kInverseEvenOdd_PathFill:
                    reverse = true;
                    // fallthrough
                case kEvenOdd_PathFill:
                    passes[0] = &gEOStencilPass;
                    if (stencilOnly) {
                        passCount = 1;
                        lastPassIsBounds = false;
                    } else {
                        passCount = 2;
                        lastPassIsBounds = true;
                        if (reverse) {
                            passes[1] = &gInvEOColorPass;
                        } else {
                            passes[1] = &gEOColorPass;
                        }
                    }
                    drawFace[0] = drawFace[1] = GrDrawTarget::kBoth_DrawFace;
                    break;

                case kInverseWinding_PathFill:
                    reverse = true;
                    // fallthrough
                case kWinding_PathFill:
                    if (fSeparateStencil) {
                        if (fStencilWrapOps) {
                            passes[0] = &gWindStencilSeparateWithWrap;
                        } else {
                            passes[0] = &gWindStencilSeparateNoWrap;
                        }
                        passCount = 2;
                        drawFace[0] = GrDrawTarget::kBoth_DrawFace;
                    } else {
                        if (fStencilWrapOps) {
                            passes[0] = &gWindSingleStencilWithWrapInc;
                            passes[1] = &gWindSingleStencilWithWrapDec;
                        } else {
                            passes[0] = &gWindSingleStencilNoWrapInc;
                            passes[1] = &gWindSingleStencilNoWrapDec;
                        }
                        // which is cw and which is ccw is arbitrary.
                        drawFace[0] = GrDrawTarget::kCW_DrawFace;
                        drawFace[1] = GrDrawTarget::kCCW_DrawFace;
                        passCount = 3;
                    }
                    if (stencilOnly) {
                        lastPassIsBounds = false;
                        --passCount;
                    } else {
                        lastPassIsBounds = true;
                        drawFace[passCount-1] = GrDrawTarget::kBoth_DrawFace;
                        if (reverse) {
                            passes[passCount-1] = &gInvWindColorPass;
                        } else {
                            passes[passCount-1] = &gWindColorPass;
                        }
                    }
                    break;
                default:
                    GrAssert(!"Unknown path fill!");
                    return;
            }
        }
    }

    GrPoint pts[4];

    bool first = true;
    int subpath = 0;

    SkPath::Iter iter(path, false);

    for (;;) {
        GrPathCmd cmd = (GrPathCmd)iter.next(pts);
        switch (cmd) {
            case kMove_PathCmd:
                if (!first) {
                    subpathVertCount[subpath] = vert-subpathBase;
                    subpathBase = vert;
                    ++subpath;
                }
                *vert = pts[0];
                vert++;
                break;
            case kLine_PathCmd:
                *vert = pts[1];
                vert++;
                break;
            case kQuadratic_PathCmd: {
                GrPathUtils::generateQuadraticPoints(pts[0], pts[1], pts[2],
                                                     tolSqd, &vert,
                                                     GrPathUtils::quadraticPointCount(pts, tol));
                break;
            }
            case kCubic_PathCmd: {
                GrPathUtils::generateCubicPoints(pts[0], pts[1], pts[2], pts[3],
                                                 tolSqd, &vert,
                                                 GrPathUtils::cubicPointCount(pts, tol));
                break;
            }
            case kClose_PathCmd:
                break;
            case kEnd_PathCmd:
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

    // if we're stenciling we will follow with a pass that draws
    // a bounding rect to set the color. We're stenciling when
    // passCount > 1.
    const int& boundVertexStart = maxPts;
    GrPoint* boundsVerts = base + boundVertexStart;
    if (lastPassIsBounds) {
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
        target->setDrawFace(drawFace[p]);
        if (NULL != passes[p]) {
            target->setStencil(*passes[p]);
        }

        if (lastPassIsBounds && (p == passCount-1)) {
            if (!colorWritesWereDisabled) {
                target->disableState(GrDrawTarget::kNoColorWrites_StateBit);
            }
            target->drawNonIndexed(kTriangleFan_PrimitiveType,
                                   boundVertexStart, 4);

        } else {
            if (passCount > 1) {
                target->enableState(GrDrawTarget::kNoColorWrites_StateBit);
            }
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

void GrDefaultPathRenderer::drawPath(GrDrawTarget* target,
                                     GrDrawTarget::StageBitfield stages,
                                     const GrPath& path,
                                     GrPathFill fill,
                                     const GrPoint* translate) {
    this->onDrawPath(target, stages, path, fill, translate, false);
}

void GrDefaultPathRenderer::drawPathToStencil(GrDrawTarget* target,
                                              const GrPath& path,
                                              GrPathFill fill,
                                              const GrPoint* translate) {
    GrAssert(kInverseEvenOdd_PathFill != fill);
    GrAssert(kInverseWinding_PathFill != fill);
    this->onDrawPath(target, 0, path, fill, translate, true);
}
