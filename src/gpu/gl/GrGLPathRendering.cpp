/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/GrGLPathRendering.h"
#include "gl/GrGLNameAllocator.h"
#include "gl/GrGLUtil.h"
#include "gl/GrGpuGL.h"

#include "GrGLPath.h"
#include "GrGLPathRange.h"
#include "GrGLPathRendering.h"

#define GL_CALL(X) GR_GL_CALL(fGpu->glInterface(), X)
#define GL_CALL_RET(RET, X) GR_GL_CALL_RET(fGpu->glInterface(), RET, X)


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

class GrGLPathRenderingV12 : public GrGLPathRendering {
public:
    GrGLPathRenderingV12(GrGpuGL* gpu)
        : GrGLPathRendering(gpu) {
    }

    virtual GrGLvoid stencilThenCoverFillPath(GrGLuint path, GrGLenum fillMode,
                                              GrGLuint mask, GrGLenum coverMode) SK_OVERRIDE;
    virtual GrGLvoid stencilThenCoverStrokePath(GrGLuint path, GrGLint reference,
                                                GrGLuint mask, GrGLenum coverMode) SK_OVERRIDE;
    virtual GrGLvoid stencilThenCoverFillPathInstanced(
                         GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths,
                         GrGLuint pathBase, GrGLenum fillMode, GrGLuint mask, GrGLenum coverMode,
                         GrGLenum transformType, const GrGLfloat *transformValues) SK_OVERRIDE;
    virtual GrGLvoid stencilThenCoverStrokePathInstanced(
                         GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths,
                         GrGLuint pathBase, GrGLint reference, GrGLuint mask, GrGLenum coverMode,
                         GrGLenum transformType, const GrGLfloat *transformValues) SK_OVERRIDE;
};

class GrGLPathRenderingV13 : public GrGLPathRenderingV12 {
public:
    GrGLPathRenderingV13(GrGpuGL* gpu)
        : GrGLPathRenderingV12(gpu) {
        fCaps.fragmentInputGenSupport = true;
    }

    virtual GrGLvoid programPathFragmentInputGen(GrGLuint program, GrGLint location,
                                                 GrGLenum genMode, GrGLint components,
                                                 const GrGLfloat *coeffs) SK_OVERRIDE;
};


GrGLPathRendering* GrGLPathRendering::Create(GrGpuGL* gpu) {
    const GrGLInterface* glInterface = gpu->glInterface();
    if (NULL == glInterface->fFunctions.fStencilThenCoverFillPath ||
        NULL == glInterface->fFunctions.fStencilThenCoverStrokePath ||
        NULL == glInterface->fFunctions.fStencilThenCoverFillPathInstanced ||
        NULL == glInterface->fFunctions.fStencilThenCoverStrokePathInstanced) {
        return new GrGLPathRendering(gpu);
    }

    if (NULL == glInterface->fFunctions.fProgramPathFragmentInputGen) {
        return new GrGLPathRenderingV12(gpu);
    }

    return new GrGLPathRenderingV13(gpu);
}

GrGLPathRendering::GrGLPathRendering(GrGpuGL* gpu)
    : fGpu(gpu) {
    memset(&fCaps, 0, sizeof(fCaps));
    fHWPathTexGenSettings.reset(fGpu->glCaps().maxFixedFunctionTextureCoords());
}

GrGLPathRendering::~GrGLPathRendering() {
}

void GrGLPathRendering::abandonGpuResources() {
    fPathNameAllocator.reset(NULL);
}

void GrGLPathRendering::resetContext() {
    fHWProjectionMatrixState.invalidate();
    // we don't use the model view matrix.
    GL_CALL(MatrixLoadIdentity(GR_GL_MODELVIEW));

    for (int i = 0; i < fGpu->glCaps().maxFixedFunctionTextureCoords(); ++i) {
        this->pathTexGen(GR_GL_TEXTURE0 + i, GR_GL_NONE, 0, NULL);
        fHWPathTexGenSettings[i].fMode = GR_GL_NONE;
        fHWPathTexGenSettings[i].fNumComponents = 0;
    }
    fHWActivePathTexGenSets = 0;
    fHWPathStencilSettings.invalidate();
}

GrPath* GrGLPathRendering::createPath(const SkPath& inPath, const SkStrokeRec& stroke) {
    return SkNEW_ARGS(GrGLPath, (fGpu, inPath, stroke));
}

GrPathRange* GrGLPathRendering::createPathRange(size_t size, const SkStrokeRec& stroke) {
    return SkNEW_ARGS(GrGLPathRange, (fGpu, size, stroke));
}

void GrGLPathRendering::enablePathTexGen(int unitIdx, PathTexGenComponents components,
                                         const GrGLfloat* coefficients) {
    SkASSERT(components >= kS_PathTexGenComponents &&
             components <= kSTR_PathTexGenComponents);
    SkASSERT(fGpu->glCaps().maxFixedFunctionTextureCoords() >= unitIdx);

    if (GR_GL_OBJECT_LINEAR == fHWPathTexGenSettings[unitIdx].fMode &&
        components == fHWPathTexGenSettings[unitIdx].fNumComponents &&
        !memcmp(coefficients, fHWPathTexGenSettings[unitIdx].fCoefficients,
                3 * components * sizeof(GrGLfloat))) {
        return;
    }

    fGpu->setTextureUnit(unitIdx);

    fHWPathTexGenSettings[unitIdx].fNumComponents = components;
    this->pathTexGen(GR_GL_TEXTURE0 + unitIdx, GR_GL_OBJECT_LINEAR, components, coefficients);

    memcpy(fHWPathTexGenSettings[unitIdx].fCoefficients, coefficients,
           3 * components * sizeof(GrGLfloat));
}

void GrGLPathRendering::enablePathTexGen(int unitIdx, PathTexGenComponents components,
                                         const SkMatrix& matrix) {
    GrGLfloat coefficients[3 * 3];
    SkASSERT(components >= kS_PathTexGenComponents &&
             components <= kSTR_PathTexGenComponents);

    coefficients[0] = SkScalarToFloat(matrix[SkMatrix::kMScaleX]);
    coefficients[1] = SkScalarToFloat(matrix[SkMatrix::kMSkewX]);
    coefficients[2] = SkScalarToFloat(matrix[SkMatrix::kMTransX]);

    if (components >= kST_PathTexGenComponents) {
        coefficients[3] = SkScalarToFloat(matrix[SkMatrix::kMSkewY]);
        coefficients[4] = SkScalarToFloat(matrix[SkMatrix::kMScaleY]);
        coefficients[5] = SkScalarToFloat(matrix[SkMatrix::kMTransY]);
    }

    if (components >= kSTR_PathTexGenComponents) {
        coefficients[6] = SkScalarToFloat(matrix[SkMatrix::kMPersp0]);
        coefficients[7] = SkScalarToFloat(matrix[SkMatrix::kMPersp1]);
        coefficients[8] = SkScalarToFloat(matrix[SkMatrix::kMPersp2]);
    }

    this->enablePathTexGen(unitIdx, components, coefficients);
}

void GrGLPathRendering::flushPathTexGenSettings(int numUsedTexCoordSets) {
    SkASSERT(fGpu->glCaps().maxFixedFunctionTextureCoords() >= numUsedTexCoordSets);

    // Only write the inactive path tex gens, since active path tex gens were
    // written when they were enabled.

    SkDEBUGCODE(
        for (int i = 0; i < numUsedTexCoordSets; i++) {
            SkASSERT(0 != fHWPathTexGenSettings[i].fNumComponents);
        }
    );

    for (int i = numUsedTexCoordSets; i < fHWActivePathTexGenSets; i++) {
        SkASSERT(0 != fHWPathTexGenSettings[i].fNumComponents);

        fGpu->setTextureUnit(i);
        GL_CALL(PathTexGen(GR_GL_TEXTURE0 + i, GR_GL_NONE, 0, NULL));
        fHWPathTexGenSettings[i].fNumComponents = 0;
    }

    fHWActivePathTexGenSets = numUsedTexCoordSets;
}

void GrGLPathRendering::stencilPath(const GrPath* path, SkPath::FillType fill) {
    GrGLuint id = static_cast<const GrGLPath*>(path)->pathID();
    SkASSERT(NULL != fGpu->drawState()->getRenderTarget());
    SkASSERT(NULL != fGpu->drawState()->getRenderTarget()->getStencilBuffer());

    this->flushPathStencilSettings(fill);
    SkASSERT(!fHWPathStencilSettings.isTwoSided());

    GrGLenum fillMode =
        gr_stencil_op_to_gl_path_rendering_fill_mode(fHWPathStencilSettings.passOp(GrStencilSettings::kFront_Face));
    GrGLint writeMask = fHWPathStencilSettings.writeMask(GrStencilSettings::kFront_Face);
    this->stencilFillPath(id, fillMode, writeMask);
}

void GrGLPathRendering::drawPath(const GrPath* path, SkPath::FillType fill) {
    GrGLuint id = static_cast<const GrGLPath*>(path)->pathID();
    SkASSERT(NULL != fGpu->drawState()->getRenderTarget());
    SkASSERT(NULL != fGpu->drawState()->getRenderTarget()->getStencilBuffer());
    SkASSERT(!fGpu->fCurrentProgram->hasVertexShader());

    this->flushPathStencilSettings(fill);
    SkASSERT(!fHWPathStencilSettings.isTwoSided());

    const SkStrokeRec& stroke = path->getStroke();

    SkPath::FillType nonInvertedFill = SkPath::ConvertToNonInverseFillType(fill);

    GrGLenum fillMode =
        gr_stencil_op_to_gl_path_rendering_fill_mode(fHWPathStencilSettings.passOp(GrStencilSettings::kFront_Face));
    GrGLint writeMask = fHWPathStencilSettings.writeMask(GrStencilSettings::kFront_Face);

    if (nonInvertedFill == fill) {
        if (stroke.needToApply()) {
            if (SkStrokeRec::kStrokeAndFill_Style == stroke.getStyle()) {
                this->stencilFillPath(id, fillMode, writeMask);
            }
            this->stencilThenCoverStrokePath(id, 0xffff, writeMask, GR_GL_BOUNDING_BOX);
        } else {
            this->stencilThenCoverFillPath(id, fillMode, writeMask, GR_GL_BOUNDING_BOX);
        }
    } else {
        if (stroke.isFillStyle() || SkStrokeRec::kStrokeAndFill_Style == stroke.getStyle()) {
            this->stencilFillPath(id, fillMode, writeMask);
        }
        if (stroke.needToApply()) {
            this->stencilStrokePath(id, 0xffff, writeMask);
        }

        GrDrawState* drawState = fGpu->drawState();
        GrDrawState::AutoViewMatrixRestore avmr;
        SkRect bounds = SkRect::MakeLTRB(0, 0,
                                         SkIntToScalar(drawState->getRenderTarget()->width()),
                                         SkIntToScalar(drawState->getRenderTarget()->height()));
        SkMatrix vmi;
        // mapRect through persp matrix may not be correct
        if (!drawState->getViewMatrix().hasPerspective() && drawState->getViewInverse(&vmi)) {
            vmi.mapRect(&bounds);
            // theoretically could set bloat = 0, instead leave it because of matrix inversion
            // precision.
            SkScalar bloat = drawState->getViewMatrix().getMaxScale() * SK_ScalarHalf;
            bounds.outset(bloat, bloat);
        } else {
            avmr.setIdentity(drawState);
        }

        fGpu->drawSimpleRect(bounds);
    }
}

void GrGLPathRendering::drawPaths(const GrPathRange* pathRange, const uint32_t indices[], int count,
                                  const float transforms[], PathTransformType transformsType,
                                  SkPath::FillType fill) {
    SkASSERT(fGpu->caps()->pathRenderingSupport());
    SkASSERT(NULL != fGpu->drawState()->getRenderTarget());
    SkASSERT(NULL != fGpu->drawState()->getRenderTarget()->getStencilBuffer());
    SkASSERT(!fGpu->fCurrentProgram->hasVertexShader());

    GrGLuint baseID = static_cast<const GrGLPathRange*>(pathRange)->basePathID();

    this->flushPathStencilSettings(fill);
    SkASSERT(!fHWPathStencilSettings.isTwoSided());

    const SkStrokeRec& stroke = pathRange->getStroke();

    SkPath::FillType nonInvertedFill =
        SkPath::ConvertToNonInverseFillType(fill);

    GrGLenum fillMode =
        gr_stencil_op_to_gl_path_rendering_fill_mode(
            fHWPathStencilSettings.passOp(GrStencilSettings::kFront_Face));
    GrGLint writeMask =
        fHWPathStencilSettings.writeMask(GrStencilSettings::kFront_Face);

    if (nonInvertedFill == fill) {
        if (stroke.needToApply()) {
            if (SkStrokeRec::kStrokeAndFill_Style == stroke.getStyle()) {
                this->stencilFillPathInstanced(
                                    count, GR_GL_UNSIGNED_INT, indices, baseID, fillMode,
                                    writeMask, gXformType2GLType[transformsType],
                                    transforms);
            }
            this->stencilThenCoverStrokePathInstanced(
                                count, GR_GL_UNSIGNED_INT, indices, baseID, 0xffff, writeMask,
                                GR_GL_BOUNDING_BOX_OF_BOUNDING_BOXES,
                                gXformType2GLType[transformsType], transforms);
        } else {
            this->stencilThenCoverFillPathInstanced(
                                count, GR_GL_UNSIGNED_INT, indices, baseID, fillMode, writeMask,
                                GR_GL_BOUNDING_BOX_OF_BOUNDING_BOXES,
                                gXformType2GLType[transformsType], transforms);
        }
    } else {
        if (stroke.isFillStyle() || SkStrokeRec::kStrokeAndFill_Style == stroke.getStyle()) {
            this->stencilFillPathInstanced(
                                count, GR_GL_UNSIGNED_INT, indices, baseID, fillMode,
                                writeMask, gXformType2GLType[transformsType],
                                transforms);
        }
        if (stroke.needToApply()) {
            this->stencilStrokePathInstanced(
                                count, GR_GL_UNSIGNED_INT, indices, baseID, 0xffff,
                                writeMask, gXformType2GLType[transformsType],
                                transforms);
        }

        GrDrawState* drawState = fGpu->drawState();
        GrDrawState::AutoViewMatrixRestore avmr;
        SkRect bounds = SkRect::MakeLTRB(0, 0,
                                         SkIntToScalar(drawState->getRenderTarget()->width()),
                                         SkIntToScalar(drawState->getRenderTarget()->height()));
        SkMatrix vmi;
        // mapRect through persp matrix may not be correct
        if (!drawState->getViewMatrix().hasPerspective() && drawState->getViewInverse(&vmi)) {
            vmi.mapRect(&bounds);
            // theoretically could set bloat = 0, instead leave it because of matrix inversion
            // precision.
            SkScalar bloat = drawState->getViewMatrix().getMaxScale() * SK_ScalarHalf;
            bounds.outset(bloat, bloat);
        } else {
            avmr.setIdentity(drawState);
        }

        fGpu->drawSimpleRect(bounds);
    }
}

void GrGLPathRendering::flushPathStencilSettings(SkPath::FillType fill) {
    GrStencilSettings pathStencilSettings;
    fGpu->getPathStencilSettingsForFillType(fill, &pathStencilSettings);
    if (fHWPathStencilSettings != pathStencilSettings) {
        // Just the func, ref, and mask is set here. The op and write mask are params to the call
        // that draws the path to the SB (glStencilFillPath)
        GrGLenum func =
            GrToGLStencilFunc(pathStencilSettings.func(GrStencilSettings::kFront_Face));
        this->pathStencilFunc(func, pathStencilSettings.funcRef(GrStencilSettings::kFront_Face),
                              pathStencilSettings.funcMask(GrStencilSettings::kFront_Face));

        fHWPathStencilSettings = pathStencilSettings;
    }
}

void GrGLPathRendering::setProjectionMatrix(const SkMatrix& matrix,
                                  const SkISize& renderTargetSize,
                                  GrSurfaceOrigin renderTargetOrigin) {

    SkASSERT(fGpu->glCaps().pathRenderingSupport());

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
    GL_CALL(MatrixLoadf(GR_GL_PROJECTION, glMatrix));
}



// NV_path_rendering
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

GrGLvoid GrGLPathRendering::deletePaths(GrGLuint path, GrGLsizei range) {
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

GrGLvoid GrGLPathRendering::pathCommands(GrGLuint path, GrGLsizei numCommands,
                                         const GrGLubyte *commands, GrGLsizei numCoords,
                                         GrGLenum coordType, const GrGLvoid *coords) {
    GL_CALL(PathCommands(path, numCommands, commands, numCoords, coordType, coords));
}

GrGLvoid GrGLPathRendering::pathCoords(GrGLuint path, GrGLsizei numCoords,
                                       GrGLenum coordType, const GrGLvoid *coords) {
    GL_CALL(PathCoords(path, numCoords, coordType, coords));
}

GrGLvoid GrGLPathRendering::pathParameteri(GrGLuint path, GrGLenum pname, GrGLint value) {
    GL_CALL(PathParameteri(path, pname, value));
}

GrGLvoid GrGLPathRendering::pathParameterf(GrGLuint path, GrGLenum pname, GrGLfloat value) {
    GL_CALL(PathParameterf(path, pname, value));
}

GrGLboolean GrGLPathRendering::isPath(GrGLuint path) {
    GrGLboolean ret;
    GL_CALL_RET(ret, IsPath(path));
    return ret;
}

GrGLvoid GrGLPathRendering::pathStencilFunc(GrGLenum func, GrGLint ref, GrGLuint mask) {
    GL_CALL(PathStencilFunc(func, ref, mask));
}

GrGLvoid GrGLPathRendering::stencilFillPath(GrGLuint path, GrGLenum fillMode, GrGLuint mask) {
    // Decide how to manipulate the stencil buffer based on the fill rule.
    GL_CALL(StencilFillPath(path, fillMode, mask));
}

GrGLvoid GrGLPathRendering::stencilStrokePath(GrGLuint path, GrGLint reference, GrGLuint mask) {
    GL_CALL(StencilStrokePath(path, reference, mask));
}

GrGLvoid GrGLPathRendering::stencilFillPathInstanced(
             GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths,
             GrGLuint pathBase, GrGLenum fillMode, GrGLuint mask,
             GrGLenum transformType, const GrGLfloat *transformValues) {
    GL_CALL(StencilFillPathInstanced(numPaths, pathNameType, paths, pathBase,
                                     fillMode, mask, transformType, transformValues));
}

GrGLvoid GrGLPathRendering::stencilStrokePathInstanced(
             GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths,
             GrGLuint pathBase, GrGLint reference, GrGLuint mask,
             GrGLenum transformType, const GrGLfloat *transformValues) {
    GL_CALL(StencilStrokePathInstanced(numPaths, pathNameType, paths, pathBase,
                                       reference, mask, transformType, transformValues));
}

GrGLvoid GrGLPathRendering::pathTexGen(GrGLenum texCoordSet, GrGLenum genMode,
                                       GrGLint components, const GrGLfloat *coeffs) {
    GL_CALL(PathTexGen(texCoordSet, genMode, components, coeffs));
}

GrGLvoid GrGLPathRendering::coverFillPath(GrGLuint path, GrGLenum coverMode) {
    GL_CALL(CoverFillPath(path, coverMode));
}

GrGLvoid GrGLPathRendering::coverStrokePath(GrGLuint name, GrGLenum coverMode) {
    GL_CALL(CoverStrokePath(name, coverMode));
}

GrGLvoid GrGLPathRendering::coverFillPathInstanced(
             GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths, GrGLuint pathBase,
             GrGLenum coverMode, GrGLenum transformType, const GrGLfloat *transformValues) {
    GL_CALL(CoverFillPathInstanced(numPaths, pathNameType, paths, pathBase,
                                   coverMode, transformType, transformValues));
}

GrGLvoid GrGLPathRendering::coverStrokePathInstanced(
             GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths, GrGLuint pathBase,
             GrGLenum coverMode, GrGLenum transformType, const GrGLfloat* transformValues) {
    GL_CALL(CoverStrokePathInstanced(numPaths, pathNameType, paths, pathBase,
                                     coverMode, transformType, transformValues));
}

GrGLvoid GrGLPathRendering::stencilThenCoverFillPath(GrGLuint path, GrGLenum fillMode,
                                                     GrGLuint mask, GrGLenum coverMode) {
    GL_CALL(StencilFillPath(path, fillMode, mask));
    GL_CALL(CoverFillPath(path, coverMode));
}

GrGLvoid GrGLPathRendering::stencilThenCoverStrokePath(GrGLuint path, GrGLint reference,
                                                       GrGLuint mask, GrGLenum coverMode) {
    GL_CALL(StencilStrokePath(path, reference, mask));
    GL_CALL(CoverStrokePath(path, coverMode));
}

GrGLvoid GrGLPathRendering::stencilThenCoverFillPathInstanced(
             GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths,
             GrGLuint pathBase, GrGLenum fillMode, GrGLuint mask, GrGLenum coverMode,
             GrGLenum transformType, const GrGLfloat *transformValues) {
    GL_CALL(StencilFillPathInstanced(numPaths, pathNameType, paths, pathBase,
                                     fillMode, mask, transformType, transformValues));
    GL_CALL(CoverFillPathInstanced(numPaths, pathNameType, paths, pathBase,
                                   coverMode, transformType, transformValues));
}

GrGLvoid GrGLPathRendering::stencilThenCoverStrokePathInstanced(
             GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths,
             GrGLuint pathBase, GrGLint reference, GrGLuint mask, GrGLenum coverMode,
             GrGLenum transformType, const GrGLfloat *transformValues) {
    GL_CALL(StencilStrokePathInstanced(numPaths, pathNameType, paths, pathBase,
                                       reference, mask, transformType, transformValues));
    GL_CALL(CoverStrokePathInstanced(numPaths, pathNameType, paths, pathBase,
                                     coverMode, transformType, transformValues));
}

GrGLvoid GrGLPathRendering::programPathFragmentInputGen(
             GrGLuint program, GrGLint location, GrGLenum genMode,
             GrGLint components, const GrGLfloat *coeffs) {
    SkFAIL("ProgramPathFragmentInputGen not supported in this GL context.");
}


// NV_path_rendering v1.2
GrGLvoid GrGLPathRenderingV12::stencilThenCoverFillPath(GrGLuint path, GrGLenum fillMode,
                                                        GrGLuint mask, GrGLenum coverMode) {
    GL_CALL(StencilThenCoverFillPath(path, fillMode, mask, coverMode));
}

GrGLvoid GrGLPathRenderingV12::stencilThenCoverStrokePath(GrGLuint path, GrGLint reference,
                                                          GrGLuint mask, GrGLenum coverMode) {
    GL_CALL(StencilThenCoverStrokePath(path, reference, mask, coverMode));
}

GrGLvoid GrGLPathRenderingV12::stencilThenCoverFillPathInstanced(
             GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths,
             GrGLuint pathBase, GrGLenum fillMode, GrGLuint mask, GrGLenum coverMode,
             GrGLenum transformType, const GrGLfloat *transformValues) {
    GL_CALL(StencilThenCoverFillPathInstanced(numPaths, pathNameType, paths, pathBase, fillMode,
                                               mask, coverMode, transformType, transformValues));
}

GrGLvoid GrGLPathRenderingV12::stencilThenCoverStrokePathInstanced(
             GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths,
             GrGLuint pathBase, GrGLint reference, GrGLuint mask, GrGLenum coverMode,
             GrGLenum transformType, const GrGLfloat *transformValues) {
    GL_CALL(StencilThenCoverStrokePathInstanced(numPaths, pathNameType, paths, pathBase, reference,
                                                mask, coverMode, transformType, transformValues));
}


// NV_path_rendering v1.3
GrGLvoid GrGLPathRenderingV13::programPathFragmentInputGen(
             GrGLuint program, GrGLint location, GrGLenum genMode,
             GrGLint components, const GrGLfloat *coeffs) {
    GL_CALL(ProgramPathFragmentInputGen(program, location, genMode, components, coeffs));
}
