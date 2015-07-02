/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/GrGLPathRendering.h"
#include "gl/GrGLNameAllocator.h"
#include "gl/GrGLUtil.h"
#include "gl/GrGLGpu.h"

#include "GrGLPath.h"
#include "GrGLPathRange.h"
#include "GrGLPathRendering.h"

#include "SkStream.h"
#include "SkTypeface.h"

#define GL_CALL(X) GR_GL_CALL(this->gpu()->glInterface(), X)
#define GL_CALL_RET(RET, X) GR_GL_CALL_RET(this->gpu()->glInterface(), RET, X)


static const GrGLenum gIndexType2GLType[] = {
    GR_GL_UNSIGNED_BYTE,
    GR_GL_UNSIGNED_SHORT,
    GR_GL_UNSIGNED_INT
};

GR_STATIC_ASSERT(0 == GrPathRange::kU8_PathIndexType);
GR_STATIC_ASSERT(1 == GrPathRange::kU16_PathIndexType);
GR_STATIC_ASSERT(2 == GrPathRange::kU32_PathIndexType);
GR_STATIC_ASSERT(GrPathRange::kU32_PathIndexType == GrPathRange::kLast_PathIndexType);

static const GrGLenum gXformType2GLType[] = {
    GR_GL_NONE,
    GR_GL_TRANSLATE_X,
    GR_GL_TRANSLATE_Y,
    GR_GL_TRANSLATE_2D,
    GR_GL_TRANSPOSE_AFFINE_2D
};

GR_STATIC_ASSERT(0 == GrPathRendering::kNone_PathTransformType);
GR_STATIC_ASSERT(1 == GrPathRendering::kTranslateX_PathTransformType);
GR_STATIC_ASSERT(2 == GrPathRendering::kTranslateY_PathTransformType);
GR_STATIC_ASSERT(3 == GrPathRendering::kTranslate_PathTransformType);
GR_STATIC_ASSERT(4 == GrPathRendering::kAffine_PathTransformType);
GR_STATIC_ASSERT(GrPathRendering::kAffine_PathTransformType == GrPathRendering::kLast_PathTransformType);

static GrGLenum gr_stencil_op_to_gl_path_rendering_fill_mode(GrStencilOp op) {
    switch (op) {
        default:
            SkFAIL("Unexpected path fill.");
            /* fallthrough */;
        case kIncClamp_StencilOp:
            return GR_GL_COUNT_UP;
        case kInvert_StencilOp:
            return GR_GL_INVERT;
    }
}

GrGLPathRendering::GrGLPathRendering(GrGLGpu* gpu)
    : GrPathRendering(gpu) {
    const GrGLInterface* glInterface = gpu->glInterface();
    fCaps.bindFragmentInputSupport =
        NULL != glInterface->fFunctions.fBindFragmentInputLocation;
}

GrGLPathRendering::~GrGLPathRendering() {
}

void GrGLPathRendering::abandonGpuResources() {
    fPathNameAllocator.reset(NULL);
}

void GrGLPathRendering::resetContext() {
    fHWProjectionMatrixState.invalidate();
    // we don't use the model view matrix.
    GL_CALL(MatrixLoadIdentity(GR_GL_PATH_MODELVIEW));

    fHWPathStencilSettings.invalidate();
}

GrPath* GrGLPathRendering::createPath(const SkPath& inPath, const GrStrokeInfo& stroke) {
    return SkNEW_ARGS(GrGLPath, (this->gpu(), inPath, stroke));
}

GrPathRange* GrGLPathRendering::createPathRange(GrPathRange::PathGenerator* pathGenerator,
                                                const GrStrokeInfo& stroke) {
    return SkNEW_ARGS(GrGLPathRange, (this->gpu(), pathGenerator, stroke));
}

void GrGLPathRendering::onStencilPath(const StencilPathArgs& args, const GrPath* path) {
    GrGLGpu* gpu = this->gpu();
    SkASSERT(gpu->caps()->shaderCaps()->pathRenderingSupport());
    gpu->flushColorWrite(false);
    gpu->flushDrawFace(GrPipelineBuilder::kBoth_DrawFace);

    GrGLRenderTarget* rt = static_cast<GrGLRenderTarget*>(args.fRenderTarget);
    SkISize size = SkISize::Make(rt->width(), rt->height());
    this->setProjectionMatrix(*args.fViewMatrix, size, rt->origin());
    gpu->flushScissor(*args.fScissor, rt->getViewport(), rt->origin());
    gpu->flushHWAAState(rt, args.fUseHWAA);
    gpu->flushRenderTarget(rt, NULL);

    const GrGLPath* glPath = static_cast<const GrGLPath*>(path);

    this->flushPathStencilSettings(*args.fStencil);
    SkASSERT(!fHWPathStencilSettings.isTwoSided());

    GrGLenum fillMode = gr_stencil_op_to_gl_path_rendering_fill_mode(
        fHWPathStencilSettings.passOp(GrStencilSettings::kFront_Face));
    GrGLint writeMask = fHWPathStencilSettings.writeMask(GrStencilSettings::kFront_Face);

    if (glPath->shouldFill()) {
        GL_CALL(StencilFillPath(glPath->pathID(), fillMode, writeMask));
    }
    if (glPath->shouldStroke()) {
        GL_CALL(StencilStrokePath(glPath->pathID(), 0xffff, writeMask));
    }
}

void GrGLPathRendering::onDrawPath(const DrawPathArgs& args, const GrPath* path) {
    if (!this->gpu()->flushGLState(args)) {
        return;
    }
    const GrGLPath* glPath = static_cast<const GrGLPath*>(path);

    this->flushPathStencilSettings(*args.fStencil);
    SkASSERT(!fHWPathStencilSettings.isTwoSided());

    GrGLenum fillMode = gr_stencil_op_to_gl_path_rendering_fill_mode(
        fHWPathStencilSettings.passOp(GrStencilSettings::kFront_Face));
    GrGLint writeMask = fHWPathStencilSettings.writeMask(GrStencilSettings::kFront_Face);

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

void GrGLPathRendering::onDrawPaths(const DrawPathArgs& args, const GrPathRange* pathRange,
                                    const void* indices, PathIndexType indexType,
                                    const float transformValues[], PathTransformType transformType,
                                    int count) {
    if (!this->gpu()->flushGLState(args)) {
        return;
    }
    this->flushPathStencilSettings(*args.fStencil);
    SkASSERT(!fHWPathStencilSettings.isTwoSided());


    const GrGLPathRange* glPathRange = static_cast<const GrGLPathRange*>(pathRange);

    GrGLenum fillMode =
        gr_stencil_op_to_gl_path_rendering_fill_mode(
            fHWPathStencilSettings.passOp(GrStencilSettings::kFront_Face));
    GrGLint writeMask =
        fHWPathStencilSettings.writeMask(GrStencilSettings::kFront_Face);

    if (glPathRange->shouldStroke()) {
        if (glPathRange->shouldFill()) {
            GL_CALL(StencilFillPathInstanced(
                            count, gIndexType2GLType[indexType], indices, glPathRange->basePathID(),
                            fillMode, writeMask, gXformType2GLType[transformType],
                            transformValues));
        }
        GL_CALL(StencilThenCoverStrokePathInstanced(
                            count, gIndexType2GLType[indexType], indices, glPathRange->basePathID(),
                            0xffff, writeMask, GR_GL_BOUNDING_BOX_OF_BOUNDING_BOXES,
                            gXformType2GLType[transformType], transformValues));
    } else {
        GL_CALL(StencilThenCoverFillPathInstanced(
                            count, gIndexType2GLType[indexType], indices, glPathRange->basePathID(),
                            fillMode, writeMask, GR_GL_BOUNDING_BOX_OF_BOUNDING_BOXES,
                            gXformType2GLType[transformType], transformValues));
    }
}

void GrGLPathRendering::setProgramPathFragmentInputTransform(GrGLuint program, GrGLint location,
                                                             GrGLenum genMode, GrGLint components,
                                                             const SkMatrix& matrix) {
    GrGLfloat coefficients[3 * 3];
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

    GL_CALL(ProgramPathFragmentInputGen(program, location, genMode, components, coefficients));
}

void GrGLPathRendering::setProjectionMatrix(const SkMatrix& matrix,
                                            const SkISize& renderTargetSize,
                                            GrSurfaceOrigin renderTargetOrigin) {

    SkASSERT(this->gpu()->glCaps().shaderCaps()->pathRenderingSupport());

    if (renderTargetOrigin == fHWProjectionMatrixState.fRenderTargetOrigin &&
        renderTargetSize == fHWProjectionMatrixState.fRenderTargetSize &&
        matrix.cheapEqualTo(fHWProjectionMatrixState.fViewMatrix)) {
        return;
    }

    fHWProjectionMatrixState.fViewMatrix = matrix;
    fHWProjectionMatrixState.fRenderTargetSize = renderTargetSize;
    fHWProjectionMatrixState.fRenderTargetOrigin = renderTargetOrigin;

    GrGLfloat glMatrix[4 * 4];
    fHWProjectionMatrixState.getRTAdjustedGLMatrix<4>(glMatrix);
    GL_CALL(MatrixLoadf(GR_GL_PATH_PROJECTION, glMatrix));
}

GrGLuint GrGLPathRendering::genPaths(GrGLsizei range) {
    if (range > 1) {
        GrGLuint name;
        GL_CALL_RET(name, GenPaths(range));
        return name;
    }

    if (NULL == fPathNameAllocator.get()) {
        static const int range = 65536;
        GrGLuint firstName;
        GL_CALL_RET(firstName, GenPaths(range));
        fPathNameAllocator.reset(SkNEW_ARGS(GrGLNameAllocator, (firstName, firstName + range)));
    }

    // When allocating names one at a time, pull from a client-side pool of
    // available names in order to save a round trip to the GL server.
    GrGLuint name = fPathNameAllocator->allocateName();

    if (0 == name) {
        // Our reserved path names are all in use. Fall back on GenPaths.
        GL_CALL_RET(name, GenPaths(1));
    }

    return name;
}

void GrGLPathRendering::deletePaths(GrGLuint path, GrGLsizei range) {
    if (range > 1) {
        // It is not supported to delete names in ranges that were allocated
        // individually using GrGLPathNameAllocator.
        SkASSERT(NULL == fPathNameAllocator.get() ||
                 path + range <= fPathNameAllocator->firstName() ||
                 path >= fPathNameAllocator->endName());
        GL_CALL(DeletePaths(path, range));
        return;
    }

    if (NULL == fPathNameAllocator.get() ||
        path < fPathNameAllocator->firstName() ||
        path >= fPathNameAllocator->endName()) {
        // If we aren't inside fPathNameAllocator's range then this name was
        // generated by the GenPaths fallback (or else was never allocated).
        GL_CALL(DeletePaths(path, 1));
        return;
    }

    // Make the path empty to save memory, but don't free the name in the driver.
    GL_CALL(PathCommands(path, 0, NULL, 0, GR_GL_FLOAT, NULL));
    fPathNameAllocator->free(path);
}

void GrGLPathRendering::flushPathStencilSettings(const GrStencilSettings& stencilSettings) {
    if (fHWPathStencilSettings != stencilSettings) {
        // Just the func, ref, and mask is set here. The op and write mask are params to the call
        // that draws the path to the SB (glStencilFillPath)
        GrGLenum func =
            GrToGLStencilFunc(stencilSettings.func(GrStencilSettings::kFront_Face));
        GL_CALL(PathStencilFunc(func, stencilSettings.funcRef(GrStencilSettings::kFront_Face),
                                stencilSettings.funcMask(GrStencilSettings::kFront_Face)));

        fHWPathStencilSettings = stencilSettings;
    }
}

inline GrGLGpu* GrGLPathRendering::gpu() {
    return static_cast<GrGLGpu*>(fGpu);
}
