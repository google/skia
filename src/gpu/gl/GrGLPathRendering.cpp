/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkMatrixPriv.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrRenderTargetProxy.h"
#include "src/gpu/gl/GrGLGpu.h"
#include "src/gpu/gl/GrGLPath.h"
#include "src/gpu/gl/GrGLPathRendering.h"
#include "src/gpu/gl/GrGLUtil.h"

#define GL_CALL(X) GR_GL_CALL(this->gpu()->glInterface(), X)
#define GL_CALL_RET(RET, X) GR_GL_CALL_RET(this->gpu()->glInterface(), RET, X)

// Number of paths to allocate per glGenPaths call. The call can be overly slow on command buffer GL
// implementation. The call has a result value, and thus waiting for the call completion is needed.
static const GrGLsizei kPathIDPreallocationAmount = 65536;

static_assert(0 == GrPathRendering::kNone_PathTransformType);
static_assert(1 == GrPathRendering::kTranslateX_PathTransformType);
static_assert(2 == GrPathRendering::kTranslateY_PathTransformType);
static_assert(3 == GrPathRendering::kTranslate_PathTransformType);
static_assert(4 == GrPathRendering::kAffine_PathTransformType);
static_assert(GrPathRendering::kAffine_PathTransformType == GrPathRendering::kLast_PathTransformType);

#ifdef SK_DEBUG

static void verify_floats(const float* floats, int count) {
    for (int i = 0; i < count; ++i) {
        SkASSERT(!SkScalarIsNaN(SkFloatToScalar(floats[i])));
    }
}
#endif

static GrGLenum gr_stencil_op_to_gl_path_rendering_fill_mode(GrStencilOp op) {
    switch (op) {
        default:
            SK_ABORT("Unexpected path fill.");
            /* fallthrough */
        case GrStencilOp::kIncWrap:
            return GR_GL_COUNT_UP;
        case GrStencilOp::kInvert:
            return GR_GL_INVERT;
    }
}

GrGLPathRendering::GrGLPathRendering(GrGLGpu* gpu)
    : GrPathRendering(gpu)
    , fPreallocatedPathCount(0) {
    const GrGLInterface* glInterface = gpu->glInterface();
    fCaps.bindFragmentInputSupport = (bool)glInterface->fFunctions.fBindFragmentInputLocation;
}

GrGLPathRendering::~GrGLPathRendering() {
    if (fPreallocatedPathCount > 0) {
        this->deletePaths(fFirstPreallocatedPathID, fPreallocatedPathCount);
    }
}

void GrGLPathRendering::disconnect(GrGpu::DisconnectType type) {
    if (GrGpu::DisconnectType::kCleanup == type) {
        this->deletePaths(fFirstPreallocatedPathID, fPreallocatedPathCount);
    }
    fPreallocatedPathCount = 0;
}

void GrGLPathRendering::resetContext() {
    fHWProjectionMatrixState.invalidate();
    // we don't use the model view matrix.
    GL_CALL(MatrixLoadIdentity(GR_GL_PATH_MODELVIEW));

    fHWPathStencilSettings.invalidate();
}

sk_sp<GrPath> GrGLPathRendering::createPath(const SkPath& inPath, const GrStyle& style) {
    return sk_make_sp<GrGLPath>(this->gpu(), inPath, style);
}

void GrGLPathRendering::onStencilPath(const StencilPathArgs& args, const GrPath* path) {
    GrGLGpu* gpu = this->gpu();
    SkASSERT(gpu->caps()->shaderCaps()->pathRenderingSupport());
    gpu->flushColorWrite(false);

    GrGLRenderTarget* rt = static_cast<GrGLRenderTarget*>(args.fProxy->peekRenderTarget());
    SkISize dimensions = rt->dimensions();
    this->setProjectionMatrix(*args.fViewMatrix, dimensions, args.fOrigin);
    gpu->flushScissor(*args.fScissor, rt->height(), args.fOrigin);
    gpu->flushHWAAState(rt, args.fUseHWAA);
    gpu->flushRenderTarget(rt);

    const GrGLPath* glPath = static_cast<const GrGLPath*>(path);

    this->flushPathStencilSettings(*args.fStencil);

    GrGLenum fillMode = gr_stencil_op_to_gl_path_rendering_fill_mode(
            fHWPathStencilSettings.singleSidedFace().fPassOp);
    GrGLint writeMask = fHWPathStencilSettings.singleSidedFace().fWriteMask;

    if (glPath->shouldFill()) {
        GL_CALL(StencilFillPath(glPath->pathID(), fillMode, writeMask));
    }
    if (glPath->shouldStroke()) {
        GL_CALL(StencilStrokePath(glPath->pathID(), 0xffff, writeMask));
    }
}

void GrGLPathRendering::onDrawPath(const GrStencilSettings& stencilPassSettings,
                                   const GrPath* path) {
    const GrGLPath* glPath = static_cast<const GrGLPath*>(path);

    this->flushPathStencilSettings(stencilPassSettings);

    GrGLenum fillMode = gr_stencil_op_to_gl_path_rendering_fill_mode(
            fHWPathStencilSettings.singleSidedFace().fPassOp);
    GrGLint writeMask = fHWPathStencilSettings.singleSidedFace().fWriteMask;

    if (glPath->shouldStroke()) {
        if (glPath->shouldFill()) {
            GL_CALL(StencilFillPath(glPath->pathID(), fillMode, writeMask));
        }
        GL_CALL(StencilThenCoverStrokePath(glPath->pathID(), 0xffff, writeMask,
                                           GR_GL_BOUNDING_BOX));
    } else {
        GL_CALL(StencilThenCoverFillPath(glPath->pathID(), fillMode, writeMask,
                                         GR_GL_BOUNDING_BOX));
    }
}

void GrGLPathRendering::setProgramPathFragmentInputTransform(GrGLuint program, GrGLint location,
                                                             GrGLenum genMode, GrGLint components,
                                                             const SkMatrix& matrix) {
    float coefficients[3 * 3];
    SkASSERT(components >= 1 && components <= 3);

    coefficients[0] = SkScalarToFloat(matrix[SkMatrix::kMScaleX]);
    coefficients[1] = SkScalarToFloat(matrix[SkMatrix::kMSkewX]);
    coefficients[2] = SkScalarToFloat(matrix[SkMatrix::kMTransX]);

    if (components >= 2) {
        coefficients[3] = SkScalarToFloat(matrix[SkMatrix::kMSkewY]);
        coefficients[4] = SkScalarToFloat(matrix[SkMatrix::kMScaleY]);
        coefficients[5] = SkScalarToFloat(matrix[SkMatrix::kMTransY]);
    }

    if (components >= 3) {
        coefficients[6] = SkScalarToFloat(matrix[SkMatrix::kMPersp0]);
        coefficients[7] = SkScalarToFloat(matrix[SkMatrix::kMPersp1]);
        coefficients[8] = SkScalarToFloat(matrix[SkMatrix::kMPersp2]);
    }
    SkDEBUGCODE(verify_floats(coefficients, components * 3));

    GL_CALL(ProgramPathFragmentInputGen(program, location, genMode, components, coefficients));
}

void GrGLPathRendering::setProjectionMatrix(const SkMatrix& matrix,
                                            const SkISize& renderTargetSize,
                                            GrSurfaceOrigin renderTargetOrigin) {

    SkASSERT(this->gpu()->glCaps().shaderCaps()->pathRenderingSupport());

    if (renderTargetOrigin == fHWProjectionMatrixState.fRenderTargetOrigin &&
        renderTargetSize == fHWProjectionMatrixState.fRenderTargetSize &&
        SkMatrixPriv::CheapEqual(matrix, fHWProjectionMatrixState.fViewMatrix)) {
        return;
    }

    fHWProjectionMatrixState.fViewMatrix = matrix;
    fHWProjectionMatrixState.fRenderTargetSize = renderTargetSize;
    fHWProjectionMatrixState.fRenderTargetOrigin = renderTargetOrigin;

    float glMatrix[4 * 4];
    fHWProjectionMatrixState.getRTAdjustedGLMatrix(glMatrix);
    SkDEBUGCODE(verify_floats(glMatrix, SK_ARRAY_COUNT(glMatrix)));
    GL_CALL(MatrixLoadf(GR_GL_PATH_PROJECTION, glMatrix));
}

GrGLuint GrGLPathRendering::genPaths(GrGLsizei range) {
    SkASSERT(range > 0);
    GrGLuint firstID;
    if (fPreallocatedPathCount >= range) {
        firstID = fFirstPreallocatedPathID;
        fPreallocatedPathCount -= range;
        fFirstPreallocatedPathID += range;
        return firstID;
    }
    // Allocate range + the amount to fill up preallocation amount. If succeed, either join with
    // the existing preallocation range or delete the existing and use the new (potentially partial)
    // preallocation range.
    GrGLsizei allocAmount = range + (kPathIDPreallocationAmount - fPreallocatedPathCount);
    if (allocAmount >= range) {
        GL_CALL_RET(firstID, GenPaths(allocAmount));

        if (firstID != 0) {
            if (fPreallocatedPathCount > 0 &&
                firstID == fFirstPreallocatedPathID + fPreallocatedPathCount) {
                firstID = fFirstPreallocatedPathID;
                fPreallocatedPathCount += allocAmount - range;
                fFirstPreallocatedPathID += range;
                return firstID;
            }

            if (allocAmount > range) {
                if (fPreallocatedPathCount > 0) {
                    this->deletePaths(fFirstPreallocatedPathID, fPreallocatedPathCount);
                }
                fFirstPreallocatedPathID = firstID + range;
                fPreallocatedPathCount = allocAmount - range;
            }
            // Special case: if allocAmount == range, we have full preallocated range.
            return firstID;
        }
    }
    // Failed to allocate with preallocation. Remove existing preallocation and try to allocate just
    // the range.
    if (fPreallocatedPathCount > 0) {
        this->deletePaths(fFirstPreallocatedPathID, fPreallocatedPathCount);
        fPreallocatedPathCount = 0;
    }

    GL_CALL_RET(firstID, GenPaths(range));
    if (firstID == 0) {
        SkDebugf("Warning: Failed to allocate path\n");
    }
    return firstID;
}

void GrGLPathRendering::deletePaths(GrGLuint path, GrGLsizei range) {
    GL_CALL(DeletePaths(path, range));
}

void GrGLPathRendering::flushPathStencilSettings(const GrStencilSettings& stencilSettings) {
    SkASSERT(!stencilSettings.isTwoSided());
    if (fHWPathStencilSettings != stencilSettings) {
        SkASSERT(stencilSettings.isValid());
        // Just the func, ref, and mask is set here. The op and write mask are params to the call
        // that draws the path to the SB (glStencilFillPath)
        uint16_t ref = stencilSettings.singleSidedFace().fRef;
        GrStencilTest test = stencilSettings.singleSidedFace().fTest;
        uint16_t testMask = stencilSettings.singleSidedFace().fTestMask;

        if (!fHWPathStencilSettings.isValid() ||
            ref != fHWPathStencilSettings.singleSidedFace().fRef ||
            test != fHWPathStencilSettings.singleSidedFace().fTest ||
            testMask != fHWPathStencilSettings.singleSidedFace().fTestMask) {
            GL_CALL(PathStencilFunc(GrToGLStencilFunc(test), ref, testMask));
        }
        fHWPathStencilSettings = stencilSettings;
    }
}

inline GrGLGpu* GrGLPathRendering::gpu() {
    return static_cast<GrGLGpu*>(fGpu);
}
