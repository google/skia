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
static SkMatrix get_transform_matrix(const GrProcessorStage& processorStage,
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
                         const GrGLProgramDesc& desc,
                         const BuiltinUniformHandles& builtinUniforms,
                         GrGLuint programID,
                         const UniformInfoArray& uniforms,
                         GrGLInstalledProcessors* geometryProcessor,
                         GrGLInstalledProcessors* colorProcessors,
                         GrGLInstalledProcessors* coverageProcessors)
    : fColor(GrColor_ILLEGAL)
    , fCoverage(GrColor_ILLEGAL)
    , fDstCopyTexUnit(-1)
    , fBuiltinUniformHandles(builtinUniforms)
    , fProgramID(programID)
    , fGeometryProcessor(SkSafeRef(geometryProcessor))
    , fColorEffects(SkRef(colorProcessors))
    , fCoverageEffects(SkRef(coverageProcessors))
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
    this->initSamplers(fColorEffects.get(), &texUnitIdx);
    this->initSamplers(fCoverageEffects.get(), &texUnitIdx);
}

void GrGLProgram::initSamplers(GrGLInstalledProcessors* ip, int* texUnitIdx) {
    int numEffects = ip->fGLProcessors.count();
    SkASSERT(numEffects == ip->fSamplers.count());
    for (int e = 0; e < numEffects; ++e) {
        SkTArray<GrGLInstalledProcessors::Sampler, true>& samplers = ip->fSamplers[e];
        int numSamplers = samplers.count();
        for (int s = 0; s < numSamplers; ++s) {
            SkASSERT(samplers[s].fUniform.isValid());
            fProgramDataManager.setSampler(samplers[s].fUniform, *texUnitIdx);
            samplers[s].fTextureUnit = (*texUnitIdx)++;
        }
    }
}

void GrGLProgram::bindTextures(const GrGLInstalledProcessors* ip,
                               const GrProcessor& processor,
                               int effectIdx) {
    const SkTArray<GrGLInstalledProcessors::Sampler, true>& samplers = ip->fSamplers[effectIdx];
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
                          const GrGeometryStage* geometryProcessor,
                          const GrFragmentStage* colorStages[],
                          const GrFragmentStage* coverageStages[],
                          const GrDeviceCoordTexture* dstCopy,
                          SharedGLState* sharedState) {
    GrColor color = optState.getColor();
    GrColor coverage = optState.getCoverageColor();

    this->setColor(optState, color, sharedState);
    this->setCoverage(optState, coverage, sharedState);
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
        SkASSERT(geometryProcessor);
        this->setData<GrGeometryStage>(&geometryProcessor, fGeometryProcessor.get());
    }
    this->setData<GrFragmentStage>(colorStages, fColorEffects.get());
    this->setData<GrFragmentStage>(coverageStages, fCoverageEffects.get());

    // Some of GrGLProgram subclasses need to update state here
    this->didSetData(drawType);
}

void GrGLProgram::setTransformData(const GrProcessorStage& processor,
                                   int effectIdx,
                                   GrGLInstalledProcessors* ip) {
    SkTArray<GrGLInstalledProcessors::Transform, true>& transforms = ip->fTransforms[effectIdx];
    int numTransforms = transforms.count();
    SkASSERT(numTransforms == processor.getProcessor()->numTransforms());
    for (int t = 0; t < numTransforms; ++t) {
        SkASSERT(transforms[t].fHandle.isValid());
        const SkMatrix& matrix = get_transform_matrix(processor, ip->fHasExplicitLocalCoords, t);
        if (!transforms[t].fCurrentValue.cheapEqualTo(matrix)) {
            fProgramDataManager.setSkMatrix(transforms[t].fHandle.convertToUniformHandle(), matrix);
            transforms[t].fCurrentValue = matrix;
        }
    }
}

void GrGLProgram::didSetData(GrGpu::DrawType drawType) {
    SkASSERT(!GrGpu::IsPathRenderingDrawType(drawType));
}

void GrGLProgram::setColor(const GrOptDrawState& optState,
                           GrColor color,
                           SharedGLState* sharedState) {
    const GrGLProgramDesc::KeyHeader& header = fDesc.getHeader();
    if (!optState.hasColorVertexAttribute()) {
        switch (header.fColorInput) {
            case GrGLProgramDesc::kAttribute_ColorInput:
                SkASSERT(-1 != header.fColorAttributeIndex);
                if (sharedState->fConstAttribColor != color ||
                    sharedState->fConstAttribColorIndex != header.fColorAttributeIndex) {
                    // OpenGL ES only supports the float varieties of glVertexAttrib
                    GrGLfloat c[4];
                    GrColorToRGBAFloat(color, c);
                    GL_CALL(VertexAttrib4fv(header.fColorAttributeIndex, c));
                    sharedState->fConstAttribColor = color;
                    sharedState->fConstAttribColorIndex = header.fColorAttributeIndex;
                }
                break;
            case GrGLProgramDesc::kUniform_ColorInput:
                if (fColor != color && fBuiltinUniformHandles.fColorUni.isValid()) {
                    // OpenGL ES doesn't support unsigned byte varieties of glUniform
                    GrGLfloat c[4];
                    GrColorToRGBAFloat(color, c);
                    fProgramDataManager.set4fv(fBuiltinUniformHandles.fColorUni, 1, c);
                    fColor = color;
                }
                sharedState->fConstAttribColorIndex = -1;
                break;
            case GrGLProgramDesc::kAllOnes_ColorInput:
                sharedState->fConstAttribColorIndex = -1;
                break;
            default:
                SkFAIL("Unexpected color type.");
        }
    } else {
        sharedState->fConstAttribColorIndex = -1;
    }
}

void GrGLProgram::setCoverage(const GrOptDrawState& optState,
                              GrColor coverage,
                              SharedGLState* sharedState) {
    const GrGLProgramDesc::KeyHeader& header = fDesc.getHeader();
    if (!optState.hasCoverageVertexAttribute()) {
        switch (header.fCoverageInput) {
            case GrGLProgramDesc::kAttribute_ColorInput:
                if (sharedState->fConstAttribCoverage != coverage ||
                    sharedState->fConstAttribCoverageIndex != header.fCoverageAttributeIndex) {
                    // OpenGL ES only supports the float varieties of  glVertexAttrib
                    GrGLfloat c[4];
                    GrColorToRGBAFloat(coverage, c);
                    GL_CALL(VertexAttrib4fv(header.fCoverageAttributeIndex, c));
                    sharedState->fConstAttribCoverage = coverage;
                    sharedState->fConstAttribCoverageIndex = header.fCoverageAttributeIndex;
                }
                break;
            case GrGLProgramDesc::kUniform_ColorInput:
                if (fCoverage != coverage) {
                    // OpenGL ES doesn't support unsigned byte varieties of glUniform
                    GrGLfloat c[4];
                    GrColorToRGBAFloat(coverage, c);
                    fProgramDataManager.set4fv(fBuiltinUniformHandles.fCoverageUni, 1, c);
                    fCoverage = coverage;
                }
                sharedState->fConstAttribCoverageIndex = -1;
                break;
            case GrGLProgramDesc::kAllOnes_ColorInput:
                sharedState->fConstAttribCoverageIndex = -1;
                break;
            default:
                SkFAIL("Unexpected coverage type.");
        }
    } else {
        sharedState->fConstAttribCoverageIndex = -1;
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
                                         const GrGLProgramDesc& desc,
                                         const BuiltinUniformHandles& builtinUniforms,
                                         GrGLuint programID,
                                         const UniformInfoArray& uniforms,
                                         GrGLInstalledProcessors* colorProcessors,
                                         GrGLInstalledProcessors* coverageProcessors)
    : INHERITED(gpu, desc, builtinUniforms, programID, uniforms, NULL, colorProcessors,
                coverageProcessors) {
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
                                 const GrGLProgramDesc& desc,
                                 const BuiltinUniformHandles& builtinUniforms,
                                 GrGLuint programID,
                                 const UniformInfoArray& uniforms,
                                 GrGLInstalledProcessors* colorProcessors,
                                 GrGLInstalledProcessors* coverageProcessors,
                                 const SeparableVaryingInfoArray& separableVaryings)
    : INHERITED(gpu, desc, builtinUniforms, programID, uniforms, colorProcessors,
                coverageProcessors) {
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

void GrGLNvprProgram::setTransformData(const GrProcessorStage& processor,
                                       int effectIdx,
                                       GrGLInstalledProcessors* ip) {
    SkTArray<GrGLInstalledProcessors::Transform, true>& transforms = ip->fTransforms[effectIdx];
    int numTransforms = transforms.count();
    SkASSERT(numTransforms == processor.getProcessor()->numTransforms());
    for (int t = 0; t < numTransforms; ++t) {
        SkASSERT(transforms[t].fHandle.isValid());
        const SkMatrix& transform = get_transform_matrix(processor, ip->fHasExplicitLocalCoords, t);
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
                                 const GrGLProgramDesc& desc,
                                 const BuiltinUniformHandles& builtinUniforms,
                                 GrGLuint programID,
                                 const UniformInfoArray& uniforms,
                                 GrGLInstalledProcessors* colorProcessors,
                                 GrGLInstalledProcessors* coverageProcessors,
                                 int texCoordSetCnt)
    : INHERITED(gpu, desc, builtinUniforms, programID, uniforms, colorProcessors,
                coverageProcessors)
    , fTexCoordSetCnt(texCoordSetCnt) {
}

void GrGLLegacyNvprProgram::didSetData(GrGpu::DrawType drawType) {
    SkASSERT(GrGpu::IsPathRenderingDrawType(drawType));
    fGpu->glPathRendering()->flushPathTexGenSettings(fTexCoordSetCnt);
}

void GrGLLegacyNvprProgram::setTransformData(const GrProcessorStage& processorStage,
                                       int effectIdx,
                                       GrGLInstalledProcessors* ip) {
    // We've hidden the texcoord index in the first entry of the transforms array for each effect
    int texCoordIndex = ip->fTransforms[effectIdx][0].fHandle.handle();
    int numTransforms = processorStage.getProcessor()->numTransforms();
    for (int t = 0; t < numTransforms; ++t) {
        const SkMatrix& transform = get_transform_matrix(processorStage, false, t);
        GrGLPathRendering::PathTexGenComponents components =
                GrGLPathRendering::kST_PathTexGenComponents;
        if (processorStage.isPerspectiveCoordTransform(t, false)) {
            components = GrGLPathRendering::kSTR_PathTexGenComponents;
        }
        fGpu->glPathRendering()->enablePathTexGen(texCoordIndex++, components, transform);
    }
}
