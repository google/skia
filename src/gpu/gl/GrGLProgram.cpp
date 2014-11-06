/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLProgram.h"

#include "GrAllocator.h"
#include "GrProcessor.h"
#include "GrCoordTransform.h"
#include "GrGLGeometryProcessor.h"
#include "GrGLProcessor.h"
#include "GrGpuGL.h"
#include "GrGLPathRendering.h"
#include "GrGLShaderVar.h"
#include "GrGLSL.h"
#include "GrOptDrawState.h"
#include "SkXfermode.h"

#define GL_CALL(X) GR_GL_CALL(fGpu->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(fGpu->glInterface(), R, X)

/**
 * Retrieves the final matrix that a transform needs to apply to its source coords.
 */
static SkMatrix get_transform_matrix(const GrFragmentStage& processorStage,
                                     bool useExplicitLocalCoords,
                                     int transformIdx) {
    const GrCoordTransform& coordTransform =
            processorStage.getProcessor()->coordTransform(transformIdx);
    SkMatrix combined;

    if (kLocal_GrCoordSet == coordTransform.sourceCoords()) {
        // If we have explicit local coords then we shouldn't need a coord change.
        const SkMatrix& ccm =
                useExplicitLocalCoords ? SkMatrix::I() : processorStage.getCoordChangeMatrix();
        combined.setConcat(coordTransform.getMatrix(), ccm);
    } else {
        combined = coordTransform.getMatrix();
    }
    if (coordTransform.reverseY()) {
        // combined.postScale(1,-1);
        // combined.postTranslate(0,1);
        combined.set(SkMatrix::kMSkewY,
            combined[SkMatrix::kMPersp0] - combined[SkMatrix::kMSkewY]);
        combined.set(SkMatrix::kMScaleY,
            combined[SkMatrix::kMPersp1] - combined[SkMatrix::kMScaleY]);
        combined.set(SkMatrix::kMTransY,
            combined[SkMatrix::kMPersp2] - combined[SkMatrix::kMTransY]);
    }
    return combined;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

GrGLProgram::GrGLProgram(GrGpuGL* gpu,
                         const GrProgramDesc& desc,
                         const BuiltinUniformHandles& builtinUniforms,
                         GrGLuint programID,
                         const UniformInfoArray& uniforms,
                         GrGLInstalledGeoProc* geometryProcessor,
                         GrGLInstalledFragProcs* fragmentProcessors)
    : fColor(GrColor_ILLEGAL)
    , fCoverage(0)
    , fDstCopyTexUnit(-1)
    , fBuiltinUniformHandles(builtinUniforms)
    , fProgramID(programID)
    , fGeometryProcessor(geometryProcessor)
    , fFragmentProcessors(SkRef(fragmentProcessors))
    , fDesc(desc)
    , fGpu(gpu)
    , fProgramDataManager(gpu, uniforms) {
    this->initSamplerUniforms();
}

GrGLProgram::~GrGLProgram() {
    if (fProgramID) {
        GL_CALL(DeleteProgram(fProgramID));
    }
}

void GrGLProgram::abandon() {
    fProgramID = 0;
}

void GrGLProgram::initSamplerUniforms() {
    GL_CALL(UseProgram(fProgramID));
    GrGLint texUnitIdx = 0;
    if (fBuiltinUniformHandles.fDstCopySamplerUni.isValid()) {
        fProgramDataManager.setSampler(fBuiltinUniformHandles.fDstCopySamplerUni, texUnitIdx);
        fDstCopyTexUnit = texUnitIdx++;
    }
    if (fGeometryProcessor.get()) {
        this->initSamplers(fGeometryProcessor.get(), &texUnitIdx);
    }
    int numProcs = fFragmentProcessors->fProcs.count();
    for (int i = 0; i < numProcs; i++) {
        this->initSamplers(fFragmentProcessors->fProcs[i], &texUnitIdx);
    }
}

void GrGLProgram::initSamplers(GrGLInstalledProc* ip, int* texUnitIdx) {
    SkTArray<GrGLInstalledProc::Sampler, true>& samplers = ip->fSamplers;
    int numSamplers = samplers.count();
    for (int s = 0; s < numSamplers; ++s) {
        SkASSERT(samplers[s].fUniform.isValid());
        fProgramDataManager.setSampler(samplers[s].fUniform, *texUnitIdx);
        samplers[s].fTextureUnit = (*texUnitIdx)++;
    }
}

void GrGLProgram::bindTextures(const GrGLInstalledProc* ip, const GrProcessor& processor) {
    const SkTArray<GrGLInstalledProc::Sampler, true>& samplers = ip->fSamplers;
    int numSamplers = samplers.count();
    SkASSERT(numSamplers == processor.numTextures());
    for (int s = 0; s < numSamplers; ++s) {
        SkASSERT(samplers[s].fTextureUnit >= 0);
        const GrTextureAccess& textureAccess = processor.textureAccess(s);
        fGpu->bindTexture(samplers[s].fTextureUnit,
                          textureAccess.getParams(),
                          static_cast<GrGLTexture*>(textureAccess.getTexture()));
    }
}


///////////////////////////////////////////////////////////////////////////////

void GrGLProgram::setData(const GrOptDrawState& optState,
                          GrGpu::DrawType drawType,
                          const GrDeviceCoordTexture* dstCopy) {
    GrColor color = optState.getColor();
    uint8_t coverage = optState.getCoverage();

    this->setColor(optState, color);
    this->setCoverage(optState, coverage);
    this->setMatrixAndRenderTargetHeight(drawType, optState);

    if (dstCopy) {
        if (fBuiltinUniformHandles.fDstCopyTopLeftUni.isValid()) {
            fProgramDataManager.set2f(fBuiltinUniformHandles.fDstCopyTopLeftUni,
                                       static_cast<GrGLfloat>(dstCopy->offset().fX),
                                       static_cast<GrGLfloat>(dstCopy->offset().fY));
            fProgramDataManager.set2f(fBuiltinUniformHandles.fDstCopyScaleUni,
                                       1.f / dstCopy->texture()->width(),
                                       1.f / dstCopy->texture()->height());
            GrGLTexture* texture = static_cast<GrGLTexture*>(dstCopy->texture());
            static GrTextureParams kParams; // the default is clamp, nearest filtering.
            fGpu->bindTexture(fDstCopyTexUnit, kParams, texture);
        } else {
            SkASSERT(!fBuiltinUniformHandles.fDstCopyScaleUni.isValid());
            SkASSERT(!fBuiltinUniformHandles.fDstCopySamplerUni.isValid());
        }
    } else {
        SkASSERT(!fBuiltinUniformHandles.fDstCopyTopLeftUni.isValid());
        SkASSERT(!fBuiltinUniformHandles.fDstCopyScaleUni.isValid());
        SkASSERT(!fBuiltinUniformHandles.fDstCopySamplerUni.isValid());
    }

    // we set the textures, and uniforms for installed processors in a generic way, but subclasses
    // of GLProgram determine how to set coord transforms
    if (fGeometryProcessor.get()) {
        SkASSERT(optState.hasGeometryProcessor());
        const GrGeometryProcessor& gp = *optState.getGeometryProcessor();
        fGeometryProcessor->fGLProc->setData(fProgramDataManager, gp);
        this->bindTextures(fGeometryProcessor, gp);
    }
    this->setFragmentData(optState);

    // Some of GrGLProgram subclasses need to update state here
    this->didSetData(drawType);
}

void GrGLProgram::setFragmentData(const GrOptDrawState& optState) {
    int numProcessors = fFragmentProcessors->fProcs.count();
    for (int e = 0; e < numProcessors; ++e) {
        const GrFragmentStage& stage = optState.getFragmentStage(e);
        const GrProcessor& processor = *stage.getProcessor();
        fFragmentProcessors->fProcs[e]->fGLProc->setData(fProgramDataManager, processor);
        this->setTransformData(stage, fFragmentProcessors->fProcs[e]);
        this->bindTextures(fFragmentProcessors->fProcs[e], processor);
    }
}
void GrGLProgram::setTransformData(const GrFragmentStage& processor, GrGLInstalledFragProc* ip) {
    SkTArray<GrGLInstalledFragProc::Transform, true>& transforms = ip->fTransforms;
    int numTransforms = transforms.count();
    SkASSERT(numTransforms == processor.getProcessor()->numTransforms());
    for (int t = 0; t < numTransforms; ++t) {
        SkASSERT(transforms[t].fHandle.isValid());
        const SkMatrix& matrix = get_transform_matrix(processor, ip->fLocalCoordAttrib, t);
        if (!transforms[t].fCurrentValue.cheapEqualTo(matrix)) {
            fProgramDataManager.setSkMatrix(transforms[t].fHandle.convertToUniformHandle(), matrix);
            transforms[t].fCurrentValue = matrix;
        }
    }
}

void GrGLProgram::didSetData(GrGpu::DrawType drawType) {
    SkASSERT(!GrGpu::IsPathRenderingDrawType(drawType));
}

void GrGLProgram::setColor(const GrOptDrawState& optState, GrColor color) {
    const GrProgramDesc::KeyHeader& header = fDesc.header();
    switch (header.fColorInput) {
        case GrProgramDesc::kAttribute_ColorInput:
            // Attribute case is handled in GrGpuGL::setupGeometry
            break;
        case GrProgramDesc::kUniform_ColorInput:
            if (fColor != color && fBuiltinUniformHandles.fColorUni.isValid()) {
                // OpenGL ES doesn't support unsigned byte varieties of glUniform
                GrGLfloat c[4];
                GrColorToRGBAFloat(color, c);
                fProgramDataManager.set4fv(fBuiltinUniformHandles.fColorUni, 1, c);
                fColor = color;
            }
            break;
        case GrProgramDesc::kAllOnes_ColorInput:
            // Handled by shader creation
            break;
        default:
            SkFAIL("Unexpected color type.");
    }
}

void GrGLProgram::setCoverage(const GrOptDrawState& optState, uint8_t coverage) {
    const GrProgramDesc::KeyHeader& header = fDesc.header();
    switch (header.fCoverageInput) {
        case GrProgramDesc::kAttribute_ColorInput:
            // Attribute case is handled in GrGpuGL::setupGeometry
            break;
        case GrProgramDesc::kUniform_ColorInput:
            if (fCoverage != coverage) {
                // OpenGL ES doesn't support unsigned byte varieties of glUniform
                GrGLfloat c = GrNormalizeByteToFloat(coverage);
                fProgramDataManager.set1f(fBuiltinUniformHandles.fCoverageUni, c);
                fCoverage = coverage;
            }
            break;
        case GrProgramDesc::kAllOnes_ColorInput:
            // Handled by shader creation
            break;
        default:
            SkFAIL("Unexpected coverage type.");
    }
}

void GrGLProgram::setMatrixAndRenderTargetHeight(GrGpu::DrawType drawType,
                                                 const GrOptDrawState& optState) {
    // Load the RT height uniform if it is needed to y-flip gl_FragCoord.
    if (fBuiltinUniformHandles.fRTHeightUni.isValid() &&
        fMatrixState.fRenderTargetSize.fHeight != optState.getRenderTarget()->height()) {
        fProgramDataManager.set1f(fBuiltinUniformHandles.fRTHeightUni,
                                   SkIntToScalar(optState.getRenderTarget()->height()));
    }

    // call subclasses to set the actual view matrix
    this->onSetMatrixAndRenderTargetHeight(drawType, optState);
}

void GrGLProgram::onSetMatrixAndRenderTargetHeight(GrGpu::DrawType drawType,
                                                   const GrOptDrawState& optState) {
    const GrRenderTarget* rt = optState.getRenderTarget();
    SkISize size;
    size.set(rt->width(), rt->height());
    if (fMatrixState.fRenderTargetOrigin != rt->origin() ||
        fMatrixState.fRenderTargetSize != size ||
        !fMatrixState.fViewMatrix.cheapEqualTo(optState.getViewMatrix())) {
        SkASSERT(fBuiltinUniformHandles.fViewMatrixUni.isValid());

        fMatrixState.fViewMatrix = optState.getViewMatrix();
        fMatrixState.fRenderTargetSize = size;
        fMatrixState.fRenderTargetOrigin = rt->origin();

        GrGLfloat viewMatrix[3 * 3];
        fMatrixState.getGLMatrix<3>(viewMatrix);
        fProgramDataManager.setMatrix3f(fBuiltinUniformHandles.fViewMatrixUni, viewMatrix);

        GrGLfloat rtAdjustmentVec[4];
        fMatrixState.getRTAdjustmentVec(rtAdjustmentVec);
        fProgramDataManager.set4fv(fBuiltinUniformHandles.fRTAdjustmentUni, 1, rtAdjustmentVec);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////

GrGLNvprProgramBase::GrGLNvprProgramBase(GrGpuGL* gpu,
                                         const GrProgramDesc& desc,
                                         const BuiltinUniformHandles& builtinUniforms,
                                         GrGLuint programID,
                                         const UniformInfoArray& uniforms,
                                         GrGLInstalledFragProcs* fragmentProcessors)
    : INHERITED(gpu, desc, builtinUniforms, programID, uniforms, NULL, fragmentProcessors) {
}

void GrGLNvprProgramBase::onSetMatrixAndRenderTargetHeight(GrGpu::DrawType drawType,
                                                           const GrOptDrawState& optState) {
    SkASSERT(GrGpu::IsPathRenderingDrawType(drawType));
    const GrRenderTarget* rt = optState.getRenderTarget();
    SkISize size;
    size.set(rt->width(), rt->height());
    fGpu->glPathRendering()->setProjectionMatrix(optState.getViewMatrix(), size, rt->origin());
}

/////////////////////////////////////////////////////////////////////////////////////////

GrGLNvprProgram::GrGLNvprProgram(GrGpuGL* gpu,
                                 const GrProgramDesc& desc,
                                 const BuiltinUniformHandles& builtinUniforms,
                                 GrGLuint programID,
                                 const UniformInfoArray& uniforms,
                                 GrGLInstalledFragProcs* fragmentProcessors,
                                 const SeparableVaryingInfoArray& separableVaryings)
    : INHERITED(gpu, desc, builtinUniforms, programID, uniforms, fragmentProcessors) {
    int count = separableVaryings.count();
    fVaryings.push_back_n(count);
    for (int i = 0; i < count; i++) {
        Varying& varying = fVaryings[i];
        const SeparableVaryingInfo& builderVarying = separableVaryings[i];
        SkASSERT(GrGLShaderVar::kNonArray == builderVarying.fVariable.getArrayCount());
        SkDEBUGCODE(
            varying.fType = builderVarying.fVariable.getType();
        );
        varying.fLocation = builderVarying.fLocation;
    }
}

void GrGLNvprProgram::didSetData(GrGpu::DrawType drawType) {
    SkASSERT(GrGpu::IsPathRenderingDrawType(drawType));
}

void GrGLNvprProgram::setTransformData(const GrFragmentStage& proc, GrGLInstalledFragProc* ip) {
    SkTArray<GrGLInstalledFragProc::Transform, true>& transforms = ip->fTransforms;
    int numTransforms = transforms.count();
    SkASSERT(numTransforms == proc.getProcessor()->numTransforms());
    for (int t = 0; t < numTransforms; ++t) {
        SkASSERT(transforms[t].fHandle.isValid());
        const SkMatrix& transform = get_transform_matrix(proc, false, t);
        if (transforms[t].fCurrentValue.cheapEqualTo(transform)) {
            continue;
        }
        transforms[t].fCurrentValue = transform;
        const Varying& fragmentInput = fVaryings[transforms[t].fHandle.handle()];
        SkASSERT(transforms[t].fType == kVec2f_GrSLType || transforms[t].fType == kVec3f_GrSLType);
        unsigned components = transforms[t].fType == kVec2f_GrSLType ? 2 : 3;
        fGpu->glPathRendering()->setProgramPathFragmentInputTransform(fProgramID,
                                                                      fragmentInput.fLocation,
                                                                      GR_GL_OBJECT_LINEAR,
                                                                      components,
                                                                      transform);
    }
}

//////////////////////////////////////////////////////////////////////////////////////

GrGLLegacyNvprProgram::GrGLLegacyNvprProgram(GrGpuGL* gpu,
                                             const GrProgramDesc& desc,
                                             const BuiltinUniformHandles& builtinUniforms,
                                             GrGLuint programID,
                                             const UniformInfoArray& uniforms,
                                             GrGLInstalledFragProcs* fps,
                                             int texCoordSetCnt)
    : INHERITED(gpu, desc, builtinUniforms, programID, uniforms, fps)
    , fTexCoordSetCnt(texCoordSetCnt) {
}

void GrGLLegacyNvprProgram::didSetData(GrGpu::DrawType drawType) {
    SkASSERT(GrGpu::IsPathRenderingDrawType(drawType));
    fGpu->glPathRendering()->flushPathTexGenSettings(fTexCoordSetCnt);
}

void
GrGLLegacyNvprProgram::setTransformData(const GrFragmentStage& proc, GrGLInstalledFragProc* ip) {
    // We've hidden the texcoord index in the first entry of the transforms array for each effect
    int texCoordIndex = ip->fTransforms[0].fHandle.handle();
    int numTransforms = proc.getProcessor()->numTransforms();
    for (int t = 0; t < numTransforms; ++t) {
        const SkMatrix& transform = get_transform_matrix(proc, false, t);
        GrGLPathRendering::PathTexGenComponents components =
                GrGLPathRendering::kST_PathTexGenComponents;
        if (proc.isPerspectiveCoordTransform(t, false)) {
            components = GrGLPathRendering::kSTR_PathTexGenComponents;
        }
        fGpu->glPathRendering()->enablePathTexGen(texCoordIndex++, components, transform);
    }
}
