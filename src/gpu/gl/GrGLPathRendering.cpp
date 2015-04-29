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

#define GL_CALL(X) GR_GL_CALL(fGpu->glInterface(), X)
#define GL_CALL_RET(RET, X) GR_GL_CALL_RET(fGpu->glInterface(), RET, X)


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
    : fGpu(gpu) {
    const GrGLInterface* glInterface = gpu->glInterface();
    fCaps.stencilThenCoverSupport =
        NULL != glInterface->fFunctions.fStencilThenCoverFillPath &&
        NULL != glInterface->fFunctions.fStencilThenCoverStrokePath &&
        NULL != glInterface->fFunctions.fStencilThenCoverFillPathInstanced &&
        NULL != glInterface->fFunctions.fStencilThenCoverStrokePathInstanced;
    fCaps.fragmentInputGenSupport =
        NULL != glInterface->fFunctions.fProgramPathFragmentInputGen;
    fCaps.glyphLoadingSupport =
        NULL != glInterface->fFunctions.fPathMemoryGlyphIndexArray;

    SkASSERT(fCaps.fragmentInputGenSupport);
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

    SkASSERT(fCaps.fragmentInputGenSupport);
    fHWPathStencilSettings.invalidate();
}

GrPath* GrGLPathRendering::createPath(const SkPath& inPath, const SkStrokeRec& stroke) {
    return SkNEW_ARGS(GrGLPath, (fGpu, inPath, stroke));
}

GrPathRange* GrGLPathRendering::createPathRange(GrPathRange::PathGenerator* pathGenerator,
                                                const SkStrokeRec& stroke) {
    return SkNEW_ARGS(GrGLPathRange, (fGpu, pathGenerator, stroke));
}

GrPathRange* GrGLPathRendering::createGlyphs(const SkTypeface* typeface,
                                             const SkDescriptor* desc,
                                             const SkStrokeRec& stroke) {
    if (NULL != desc || !caps().glyphLoadingSupport) {
        return GrPathRendering::createGlyphs(typeface, desc, stroke);
    }

    if (NULL == typeface) {
        typeface = SkTypeface::GetDefaultTypeface();
        SkASSERT(NULL != typeface);
    }

    int faceIndex;
    SkAutoTDelete<SkStream> fontStream(typeface->openStream(&faceIndex));

    const size_t fontDataLength = fontStream->getLength();
    if (0 == fontDataLength) {
        return GrPathRendering::createGlyphs(typeface, NULL, stroke);
    }

    SkTArray<uint8_t> fontTempBuffer;
    const void* fontData = fontStream->getMemoryBase();
    if (NULL == fontData) {
        // TODO: Find a more efficient way to pass the font data (e.g. open file descriptor).
        fontTempBuffer.reset(SkToInt(fontDataLength));
        fontStream->read(&fontTempBuffer.front(), fontDataLength);
        fontData = &fontTempBuffer.front();
    }

    const int numPaths = typeface->countGlyphs();
    const GrGLuint basePathID = this->genPaths(numPaths);
    SkAutoTUnref<GrGLPath> templatePath(SkNEW_ARGS(GrGLPath, (fGpu, SkPath(), stroke)));

    GrGLenum status;
    GL_CALL_RET(status, PathMemoryGlyphIndexArray(basePathID, GR_GL_STANDARD_FONT_FORMAT,
                                                  fontDataLength, fontData, faceIndex, 0,
                                                  numPaths, templatePath->pathID(),
                                                  SkPaint::kCanonicalTextSizeForPaths));

    if (GR_GL_FONT_GLYPHS_AVAILABLE != status) {
        this->deletePaths(basePathID, numPaths);
        return GrPathRendering::createGlyphs(typeface, NULL, stroke);
    }

    // This is a crude approximation. We may want to consider giving this class
    // a pseudo PathGenerator whose sole purpose is to track the approximate gpu
    // memory size.
    const size_t gpuMemorySize = fontDataLength / 4;
    return SkNEW_ARGS(GrGLPathRange, (fGpu, basePathID, numPaths, gpuMemorySize, stroke));
}

void GrGLPathRendering::stencilPath(const GrPath* path, const GrStencilSettings& stencilSettings) {
    GrGLuint id = static_cast<const GrGLPath*>(path)->pathID();

    this->flushPathStencilSettings(stencilSettings);
    SkASSERT(!fHWPathStencilSettings.isTwoSided());

    const SkStrokeRec& stroke = path->getStroke();

    GrGLenum fillMode =
        gr_stencil_op_to_gl_path_rendering_fill_mode(fHWPathStencilSettings.passOp(GrStencilSettings::kFront_Face));
    GrGLint writeMask = fHWPathStencilSettings.writeMask(GrStencilSettings::kFront_Face);

    if (stroke.isFillStyle() || SkStrokeRec::kStrokeAndFill_Style == stroke.getStyle()) {
        GL_CALL(StencilFillPath(id, fillMode, writeMask));
    }
    if (stroke.needToApply()) {
        GL_CALL(StencilStrokePath(id, 0xffff, writeMask));
    }
}

void GrGLPathRendering::drawPath(const GrPath* path, const GrStencilSettings& stencilSettings) {
    GrGLuint id = static_cast<const GrGLPath*>(path)->pathID();

    this->flushPathStencilSettings(stencilSettings);
    SkASSERT(!fHWPathStencilSettings.isTwoSided());

    const SkStrokeRec& stroke = path->getStroke();

    GrGLenum fillMode =
        gr_stencil_op_to_gl_path_rendering_fill_mode(fHWPathStencilSettings.passOp(GrStencilSettings::kFront_Face));
    GrGLint writeMask = fHWPathStencilSettings.writeMask(GrStencilSettings::kFront_Face);

    if (stroke.needToApply()) {
        if (SkStrokeRec::kStrokeAndFill_Style == stroke.getStyle()) {
            GL_CALL(StencilFillPath(id, fillMode, writeMask));
        }
        this->stencilThenCoverStrokePath(id, 0xffff, writeMask, GR_GL_BOUNDING_BOX);
    } else {
        this->stencilThenCoverFillPath(id, fillMode, writeMask, GR_GL_BOUNDING_BOX);
    }
}

void GrGLPathRendering::drawPaths(const GrPathRange* pathRange,
                                  const void* indices, PathIndexType indexType,
                                  const float transformValues[], PathTransformType transformType,
                                  int count, const GrStencilSettings& stencilSettings) {
    SkASSERT(fGpu->caps()->shaderCaps()->pathRenderingSupport());

    GrGLuint baseID = static_cast<const GrGLPathRange*>(pathRange)->basePathID();

    this->flushPathStencilSettings(stencilSettings);
    SkASSERT(!fHWPathStencilSettings.isTwoSided());

    const SkStrokeRec& stroke = pathRange->getStroke();

    GrGLenum fillMode =
        gr_stencil_op_to_gl_path_rendering_fill_mode(
            fHWPathStencilSettings.passOp(GrStencilSettings::kFront_Face));
    GrGLint writeMask =
        fHWPathStencilSettings.writeMask(GrStencilSettings::kFront_Face);

    if (stroke.needToApply()) {
        if (SkStrokeRec::kStrokeAndFill_Style == stroke.getStyle()) {
            GL_CALL(StencilFillPathInstanced(
                            count, gIndexType2GLType[indexType], indices, baseID, fillMode,
                            writeMask, gXformType2GLType[transformType], transformValues));
        }
        this->stencilThenCoverStrokePathInstanced(
                            count, gIndexType2GLType[indexType], indices, baseID,
                            0xffff, writeMask, GR_GL_BOUNDING_BOX_OF_BOUNDING_BOXES,
                            gXformType2GLType[transformType], transformValues);
    } else {
        this->stencilThenCoverFillPathInstanced(
                            count, gIndexType2GLType[indexType], indices, baseID,
                            fillMode, writeMask, GR_GL_BOUNDING_BOX_OF_BOUNDING_BOXES,
                            gXformType2GLType[transformType], transformValues);
    }
}

void GrGLPathRendering::setProgramPathFragmentInputTransform(GrGLuint program, GrGLint location,
                                                             GrGLenum genMode, GrGLint components,
                                                             const SkMatrix& matrix) {
    SkASSERT(caps().fragmentInputGenSupport);
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

    SkASSERT(fGpu->glCaps().shaderCaps()->pathRenderingSupport());

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

inline void GrGLPathRendering::stencilThenCoverFillPath(GrGLuint path, GrGLenum fillMode,
                                                     GrGLuint mask, GrGLenum coverMode) {
    if (caps().stencilThenCoverSupport) {
        GL_CALL(StencilThenCoverFillPath(path, fillMode, mask, coverMode));
        return;
    }
    GL_CALL(StencilFillPath(path, fillMode, mask));
    GL_CALL(CoverFillPath(path, coverMode));
}

inline void GrGLPathRendering::stencilThenCoverStrokePath(GrGLuint path, GrGLint reference,
                                                       GrGLuint mask, GrGLenum coverMode) {
    if (caps().stencilThenCoverSupport) {
        GL_CALL(StencilThenCoverStrokePath(path, reference, mask, coverMode));
        return;
    }
    GL_CALL(StencilStrokePath(path, reference, mask));
    GL_CALL(CoverStrokePath(path, coverMode));
}

inline void GrGLPathRendering::stencilThenCoverFillPathInstanced(
             GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths,
             GrGLuint pathBase, GrGLenum fillMode, GrGLuint mask, GrGLenum coverMode,
             GrGLenum transformType, const GrGLfloat *transformValues) {
    if (caps().stencilThenCoverSupport) {
        GL_CALL(StencilThenCoverFillPathInstanced(numPaths, pathNameType, paths, pathBase, fillMode,
                                                  mask, coverMode, transformType, transformValues));
        return;
    }
    GL_CALL(StencilFillPathInstanced(numPaths, pathNameType, paths, pathBase,
                                     fillMode, mask, transformType, transformValues));
    GL_CALL(CoverFillPathInstanced(numPaths, pathNameType, paths, pathBase,
                                   coverMode, transformType, transformValues));
}

inline void GrGLPathRendering::stencilThenCoverStrokePathInstanced(
        GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths,
        GrGLuint pathBase, GrGLint reference, GrGLuint mask, GrGLenum coverMode,
        GrGLenum transformType, const GrGLfloat *transformValues) {
    if (caps().stencilThenCoverSupport) {
        GL_CALL(StencilThenCoverStrokePathInstanced(numPaths, pathNameType, paths, pathBase,
                                                    reference, mask, coverMode, transformType,
                                                    transformValues));
        return;
    }

    GL_CALL(StencilStrokePathInstanced(numPaths, pathNameType, paths, pathBase,
                                       reference, mask, transformType, transformValues));
    GL_CALL(CoverStrokePathInstanced(numPaths, pathNameType, paths, pathBase,
                                     coverMode, transformType, transformValues));
}
