
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDefaultPathRenderer.h"

#include "GrContext.h"
#include "GrDrawState.h"
#include "GrPathUtils.h"
#include "SkString.h"
#include "SkTrace.h"


GrDefaultPathRenderer::GrDefaultPathRenderer(bool separateStencilSupport,
                                             bool stencilWrapOpsSupport)
    : fSeparateStencil(separateStencilSupport)
    , fStencilWrapOps(stencilWrapOpsSupport)
    , fSubpathCount(0)
    , fSubpathVertCount(0)
    , fPreviousSrcTol(-GR_Scalar1)
    , fPreviousStages(-1) {
    fTarget = NULL;
}

bool GrDefaultPathRenderer::canDrawPath(const GrDrawTarget::Caps& targetCaps,
                                        const SkPath& path,
                                        GrPathFill fill,
                                        bool antiAlias) const {
    // this class can draw any path with any fill but doesn't do any 
    // anti-aliasing.
    return !antiAlias; 
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
                !target.drawWillReadDst() && !target.isDitherState());

    }
    return false;
#endif
}

bool GrDefaultPathRenderer::requiresStencilPass(const GrDrawTarget* target,
                                                const GrPath& path,
                                                GrPathFill fill) const {
    return !single_pass_path(*target, path, fill);
}

void GrDefaultPathRenderer::pathWillClear() {
    fSubpathVertCount.reset(0);
    fTarget->resetVertexSource();
    if (fUseIndexedDraw) {
        fTarget->resetIndexSource();
    }
    fPreviousSrcTol = -GR_Scalar1;
    fPreviousStages = -1;
}

static inline void append_countour_edge_indices(GrPathFill fillType,
                                                uint16_t fanCenterIdx,
                                                uint16_t edgeV0Idx,
                                                uint16_t** indices) {
    // when drawing lines we're appending line segments along
    // the contour. When applying the other fill rules we're
    // drawing triangle fans around fanCenterIdx.
    if (kHairLine_PathFill != fillType) {
        *((*indices)++) = fanCenterIdx;
    }
    *((*indices)++) = edgeV0Idx;
    *((*indices)++) = edgeV0Idx + 1;
}

bool GrDefaultPathRenderer::createGeom(GrScalar srcSpaceTol, 
                                       GrDrawTarget::StageBitfield stages) {
    {
    SK_TRACE_EVENT0("GrDefaultPathRenderer::createGeom");

    GrScalar srcSpaceTolSqd = GrMul(srcSpaceTol, srcSpaceTol);
    int maxPts = GrPathUtils::worstCasePointCount(*fPath, &fSubpathCount,
                                                  srcSpaceTol);

    if (maxPts <= 0) {
        return false;
    }
    if (maxPts > ((int)SK_MaxU16 + 1)) {
        GrPrintf("Path not rendered, too many verts (%d)\n", maxPts);
        return false;
    }

    GrVertexLayout layout = 0;
    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        if ((1 << s) & stages) {
            layout |= GrDrawTarget::StagePosAsTexCoordVertexLayoutBit(s);
        }
    }

    fUseIndexedDraw = fSubpathCount > 1;

    int maxIdxs = 0;
    if (kHairLine_PathFill == fFill) {
        if (fUseIndexedDraw) {
            maxIdxs = 2 * maxPts;
            fPrimitiveType = kLines_PrimitiveType;
        } else {
            fPrimitiveType = kLineStrip_PrimitiveType;
        }
    } else {
        if (fUseIndexedDraw) {
            maxIdxs = 3 * maxPts;
            fPrimitiveType = kTriangles_PrimitiveType;
        } else {
            fPrimitiveType = kTriangleFan_PrimitiveType;
        }
    }

    GrPoint* base;
    if (!fTarget->reserveVertexSpace(layout, maxPts, (void**)&base)) {
        return false;
    }
    GrAssert(NULL != base);
    GrPoint* vert = base;

    uint16_t* idxBase = NULL;
    uint16_t* idx = NULL;
    uint16_t subpathIdxStart = 0;
    if (fUseIndexedDraw) {
        if (!fTarget->reserveIndexSpace(maxIdxs, (void**)&idxBase)) {
            fTarget->resetVertexSource();
            return false;
        }
        GrAssert(NULL != idxBase);
        idx = idxBase;
    }

    fSubpathVertCount.reset(fSubpathCount);

    GrPoint pts[4];

    bool first = true;
    int subpath = 0;

    SkPath::Iter iter(*fPath, false);

    for (;;) {
        GrPathCmd cmd = (GrPathCmd)iter.next(pts);
        switch (cmd) {
            case kMove_PathCmd:
                if (!first) {
                    uint16_t currIdx = (uint16_t) (vert - base);
                    fSubpathVertCount[subpath] = currIdx - subpathIdxStart;
                    subpathIdxStart = currIdx;
                    ++subpath;
                }
                *vert = pts[0];
                vert++;
                break;
            case kLine_PathCmd:
                if (fUseIndexedDraw) {
                    uint16_t prevIdx = (uint16_t)(vert - base) - 1;
                    append_countour_edge_indices(fFill, subpathIdxStart,
                                                 prevIdx, &idx);
                }
                *(vert++) = pts[1];
                break;
            case kQuadratic_PathCmd: {
                // first pt of quad is the pt we ended on in previous step
                uint16_t firstQPtIdx = (uint16_t)(vert - base) - 1;
                uint16_t numPts =  (uint16_t) 
                    GrPathUtils::generateQuadraticPoints(
                            pts[0], pts[1], pts[2],
                            srcSpaceTolSqd, &vert,
                            GrPathUtils::quadraticPointCount(pts, srcSpaceTol));
                if (fUseIndexedDraw) {
                    for (uint16_t i = 0; i < numPts; ++i) {
                        append_countour_edge_indices(fFill, subpathIdxStart,
                                                     firstQPtIdx + i, &idx);
                    }
                }
                break;
            }
            case kCubic_PathCmd: {
                // first pt of cubic is the pt we ended on in previous step
                uint16_t firstCPtIdx = (uint16_t)(vert - base) - 1;
                uint16_t numPts = (uint16_t) GrPathUtils::generateCubicPoints(
                                pts[0], pts[1], pts[2], pts[3],
                                srcSpaceTolSqd, &vert,
                                GrPathUtils::cubicPointCount(pts, srcSpaceTol));
                if (fUseIndexedDraw) {
                    for (uint16_t i = 0; i < numPts; ++i) {
                        append_countour_edge_indices(fFill, subpathIdxStart,
                                                     firstCPtIdx + i, &idx);
                    }
                }
                break;
            }
            case kClose_PathCmd:
                break;
            case kEnd_PathCmd:
                uint16_t currIdx = (uint16_t) (vert - base);
                fSubpathVertCount[subpath] = currIdx - subpathIdxStart;
                goto FINISHED;
        }
        first = false;
    }
FINISHED:
    GrAssert((vert - base) <= maxPts);
    GrAssert((idx - idxBase) <= maxIdxs);

    fVertexCnt = vert - base;
    fIndexCnt = idx - idxBase;

    if (fTranslate.fX || fTranslate.fY) {
        int count = vert - base;
        for (int i = 0; i < count; i++) {
            base[i].offset(fTranslate.fX, fTranslate.fY);
        }
    }
    }
    // set these at the end so if we failed on first drawPath inside a
    // setPath/clearPath block we won't assume geom was created on a subsequent
    // drawPath in the same block.
    fPreviousSrcTol = srcSpaceTol;
    fPreviousStages = stages;
    return true;
}

void GrDefaultPathRenderer::onDrawPath(GrDrawTarget::StageBitfield stages,
                                       bool stencilOnly) {

    GrMatrix viewM = fTarget->getViewMatrix();
    GrScalar tol = GR_Scalar1;
    tol = GrPathUtils::scaleToleranceToSrc(tol, viewM, fPath->getBounds());

    // FIXME: It's really dumb that we recreate the verts for a new vertex
    // layout. We only do that because the GrDrawTarget API doesn't allow
    // us to change the vertex layout after reserveVertexSpace(). We won't
    // actually change the vertex data when the layout changes since all the
    // stages reference the positions (rather than having separate tex coords)
    // and we don't ever have per-vert colors. In practice our call sites
    // won't change the stages in use inside a setPath / removePath pair. But
    // it is a silly limitation of the GrDrawTarget design that should be fixed.
    if (tol != fPreviousSrcTol ||
        stages != fPreviousStages) {
        if (!this->createGeom(tol, stages)) {
            return;
        }
    }

    GrAssert(NULL != fTarget);
    GrDrawTarget::AutoStateRestore asr(fTarget);
    bool colorWritesWereDisabled = fTarget->isColorWriteDisabled();
    // face culling doesn't make sense here
    GrAssert(GrDrawState::kBoth_DrawFace == fTarget->getDrawFace());

    int                         passCount = 0;
    const GrStencilSettings*    passes[3];
    GrDrawState::DrawFace       drawFace[3];
    bool                        reverse = false;
    bool                        lastPassIsBounds;

    if (kHairLine_PathFill == fFill) {
        passCount = 1;
        if (stencilOnly) {
            passes[0] = &gDirectToStencil;
        } else {
            passes[0] = NULL;
        }
        lastPassIsBounds = false;
        drawFace[0] = GrDrawState::kBoth_DrawFace;
    } else {
        if (single_pass_path(*fTarget, *fPath, fFill)) {
            passCount = 1;
            if (stencilOnly) {
                passes[0] = &gDirectToStencil;
            } else {
                passes[0] = NULL;
            }
            drawFace[0] = GrDrawState::kBoth_DrawFace;
            lastPassIsBounds = false;
        } else {
            switch (fFill) {
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
                    drawFace[0] = drawFace[1] = GrDrawState::kBoth_DrawFace;
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
                        drawFace[0] = GrDrawState::kBoth_DrawFace;
                    } else {
                        if (fStencilWrapOps) {
                            passes[0] = &gWindSingleStencilWithWrapInc;
                            passes[1] = &gWindSingleStencilWithWrapDec;
                        } else {
                            passes[0] = &gWindSingleStencilNoWrapInc;
                            passes[1] = &gWindSingleStencilNoWrapDec;
                        }
                        // which is cw and which is ccw is arbitrary.
                        drawFace[0] = GrDrawState::kCW_DrawFace;
                        drawFace[1] = GrDrawState::kCCW_DrawFace;
                        passCount = 3;
                    }
                    if (stencilOnly) {
                        lastPassIsBounds = false;
                        --passCount;
                    } else {
                        lastPassIsBounds = true;
                        drawFace[passCount-1] = GrDrawState::kBoth_DrawFace;
                        if (reverse) {
                            passes[passCount-1] = &gInvWindColorPass;
                        } else {
                            passes[passCount-1] = &gWindColorPass;
                        }
                    }
                    break;
                default:
                    GrAssert(!"Unknown path fFill!");
                    return;
            }
        }
    }

    {
    for (int p = 0; p < passCount; ++p) {
        fTarget->setDrawFace(drawFace[p]);
        if (NULL != passes[p]) {
            fTarget->setStencil(*passes[p]);
        }

        if (lastPassIsBounds && (p == passCount-1)) {
            if (!colorWritesWereDisabled) {
                fTarget->disableState(GrDrawTarget::kNoColorWrites_StateBit);
            }
            GrRect bounds;
            if (reverse) {
                GrAssert(NULL != fTarget->getRenderTarget());
                // draw over the whole world.
                bounds.setLTRB(0, 0,
                               GrIntToScalar(fTarget->getRenderTarget()->width()),
                               GrIntToScalar(fTarget->getRenderTarget()->height()));
                GrMatrix vmi;
                // mapRect through persp matrix may not be correct
                if (!fTarget->getViewMatrix().hasPerspective() &&
                    fTarget->getViewInverse(&vmi)) {
                    vmi.mapRect(&bounds);
                } else {
                    if (stages) {
                        if (!fTarget->getViewInverse(&vmi)) {
                            GrPrintf("Could not invert matrix.");
                            return;
                        }
                        fTarget->preConcatSamplerMatrices(stages, vmi);
                    }
                    fTarget->setViewMatrix(GrMatrix::I());
                }
            } else {
                bounds = fPath->getBounds();
                bounds.offset(fTranslate);
            }
            GrDrawTarget::AutoGeometryPush agp(fTarget);
            fTarget->drawSimpleRect(bounds, NULL, stages);
        } else {
            if (passCount > 1) {
                fTarget->enableState(GrDrawTarget::kNoColorWrites_StateBit);
            }
            if (fUseIndexedDraw) {
                fTarget->drawIndexed(fPrimitiveType, 0, 0, 
                                     fVertexCnt, fIndexCnt);
            } else {
                int baseVertex = 0;
                for (int sp = 0; sp < fSubpathCount; ++sp) {
                    fTarget->drawNonIndexed(fPrimitiveType, baseVertex,
                                            fSubpathVertCount[sp]);
                    baseVertex += fSubpathVertCount[sp];
                }
            }
        }
    }
    }
}

void GrDefaultPathRenderer::drawPath(GrDrawTarget::StageBitfield stages) {
    this->onDrawPath(stages, false);
}

void GrDefaultPathRenderer::drawPathToStencil() {
    GrAssert(kInverseEvenOdd_PathFill != fFill);
    GrAssert(kInverseWinding_PathFill != fFill);
    this->onDrawPath(0, true);
}
