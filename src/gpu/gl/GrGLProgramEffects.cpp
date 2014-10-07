/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLProgramEffects.h"
#include "gl/GrGLProcessor.h"
#include "gl/GrGLPathRendering.h"
#include "gl/builders/GrGLFullProgramBuilder.h"
#include "gl/builders/GrGLFragmentOnlyProgramBuilder.h"
#include "gl/GrGLGeometryProcessor.h"
#include "gl/GrGpuGL.h"

typedef GrGLProcessor::TransformedCoords TransformedCoords;
typedef GrGLProcessor::TransformedCoordsArray TransformedCoordsArray;
typedef GrGLProcessor::TextureSampler TextureSampler;
typedef GrGLProcessor::TextureSamplerArray TextureSamplerArray;

namespace {
/**
 * Retrieves the final matrix that a transform needs to apply to its source coords.
 */
SkMatrix get_transform_matrix(const GrProcessorStage& effectStage,
                              bool useExplicitLocalCoords,
                              int transformIdx) {
    const GrCoordTransform& coordTransform = effectStage.getProcessor()->coordTransform(transformIdx);
    SkMatrix combined;

    if (kLocal_GrCoordSet == coordTransform.sourceCoords()) {
        // If we have explicit local coords then we shouldn't need a coord change.
        const SkMatrix& ccm =
                useExplicitLocalCoords ? SkMatrix::I() : effectStage.getCoordChangeMatrix();
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
}

////////////////////////////////////////////////////////////////////////////////

GrGLProgramEffects::~GrGLProgramEffects() {
    int numEffects = fGLProcessors.count();
    for (int e = 0; e < numEffects; ++e) {
        SkDELETE(fGLProcessors[e]);
    }
}

void GrGLProgramEffects::initSamplers(const GrGLProgramDataManager& programResourceManager, int* texUnitIdx) {
    int numEffects = fGLProcessors.count();
    SkASSERT(numEffects == fSamplers.count());
    for (int e = 0; e < numEffects; ++e) {
        SkTArray<Sampler, true>& samplers = fSamplers[e];
        int numSamplers = samplers.count();
        for (int s = 0; s < numSamplers; ++s) {
            SkASSERT(samplers[s].fUniform.isValid());
            programResourceManager.setSampler(samplers[s].fUniform, *texUnitIdx);
            samplers[s].fTextureUnit = (*texUnitIdx)++;
        }
    }
}

void GrGLProgramEffects::bindTextures(GrGpuGL* gpu, const GrProcessor& effect, int effectIdx) {
    const SkTArray<Sampler, true>& samplers = fSamplers[effectIdx];
    int numSamplers = samplers.count();
    SkASSERT(numSamplers == effect.numTextures());
    for (int s = 0; s < numSamplers; ++s) {
        SkASSERT(samplers[s].fTextureUnit >= 0);
        const GrTextureAccess& textureAccess = effect.textureAccess(s);
        gpu->bindTexture(samplers[s].fTextureUnit,
                         textureAccess.getParams(),
                         static_cast<GrGLTexture*>(textureAccess.getTexture()));
    }
}

////////////////////////////////////////////////////////////////////////////////

void GrGLVertexProgramEffects::setData(GrGpuGL* gpu,
                                       GrGpu::DrawType drawType,
                                       const GrGLProgramDataManager& programDataManager,
                                       const GrGeometryStage* effectStages) {
    SkASSERT(1 == fGLProcessors.count());
    SkASSERT(1 == fTransforms.count());
    SkASSERT(1 == fSamplers.count());
    this->setDataInternal(gpu, drawType, programDataManager, *effectStages, 0);
}

void GrGLVertexProgramEffects::setData(GrGpuGL* gpu,
                                       GrGpu::DrawType drawType,
                                       const GrGLProgramDataManager& programDataManager,
                                       const GrFragmentStage* effectStages[]) {
    int numEffects = fGLProcessors.count();
    SkASSERT(numEffects == fTransforms.count());
    SkASSERT(numEffects == fSamplers.count());
    for (int e = 0; e < numEffects; ++e) {
        this->setDataInternal(gpu, drawType, programDataManager, *effectStages[e], e);
    }
}

void GrGLVertexProgramEffects::setDataInternal(GrGpuGL* gpu,
                                               GrGpu::DrawType drawType,
                                               const GrGLProgramDataManager& programDataManager,
                                               const GrProcessorStage& effectStage,
                                               int index) {
    const GrProcessor& effect = *effectStage.getProcessor();
    fGLProcessors[index]->setData(programDataManager, effect);
    if (GrGpu::IsPathRenderingDrawType(drawType)) {
        this->setPathTransformData(gpu, programDataManager, effectStage, index);
    } else {
        this->setTransformData(gpu, programDataManager, effectStage, index);
    }
    this->bindTextures(gpu, effect, index);
}

void GrGLVertexProgramEffects::setTransformData(GrGpuGL* gpu,
                                                const GrGLProgramDataManager& pdman,
                                                const GrProcessorStage& effectStage,
                                                int effectIdx) {
    SkTArray<Transform, true>& transforms = fTransforms[effectIdx];
    int numTransforms = transforms.count();
    SkASSERT(numTransforms == effectStage.getProcessor()->numTransforms());
    for (int t = 0; t < numTransforms; ++t) {
        SkASSERT(transforms[t].fHandle.isValid());
        const SkMatrix& matrix = get_transform_matrix(effectStage, fHasExplicitLocalCoords, t);
        if (!transforms[t].fCurrentValue.cheapEqualTo(matrix)) {
            pdman.setSkMatrix(transforms[t].fHandle, matrix);
            transforms[t].fCurrentValue = matrix;
        }
    }
}

void GrGLVertexProgramEffects::setPathTransformData(GrGpuGL* gpu,
                                                    const GrGLProgramDataManager& pdman,
                                                    const GrProcessorStage& effectStage,
                                                    int effectIdx) {
    SkTArray<PathTransform, true>& transforms = fPathTransforms[effectIdx];
    int numTransforms = transforms.count();
    SkASSERT(numTransforms == effectStage.getProcessor()->numTransforms());
    for (int t = 0; t < numTransforms; ++t) {
        SkASSERT(transforms[t].fHandle.isValid());
        const SkMatrix& transform = get_transform_matrix(effectStage, fHasExplicitLocalCoords, t);
        if (transforms[t].fCurrentValue.cheapEqualTo(transform)) {
            continue;
        }
        transforms[t].fCurrentValue = transform;
        switch (transforms[t].fType) {
            case kVec2f_GrSLType:
                pdman.setProgramPathFragmentInputTransform(transforms[t].fHandle, 2, transform);
                break;
            case kVec3f_GrSLType:
                pdman.setProgramPathFragmentInputTransform(transforms[t].fHandle, 3, transform);
                break;
            default:
                SkFAIL("Unexpected matrix type.");
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void GrGLPathTexGenProgramEffects::setData(GrGpuGL* gpu,
                                           GrGpu::DrawType,
                                           const GrGLProgramDataManager& pdman,
                                           const GrFragmentStage* effectStages[]) {
    int numEffects = fGLProcessors.count();
    SkASSERT(numEffects == fTransforms.count());
    SkASSERT(numEffects == fSamplers.count());
    for (int e = 0; e < numEffects; ++e) {
        const GrProcessorStage& effectStage = *effectStages[e];
        const GrProcessor& effect = *effectStage.getProcessor();
        fGLProcessors[e]->setData(pdman, effect);
        this->setPathTexGenState(gpu, effectStage, e);
        this->bindTextures(gpu, effect, e);
    }
}

void GrGLPathTexGenProgramEffects::setPathTexGenState(GrGpuGL* gpu,
                                              const GrProcessorStage& effectStage,
                                              int effectIdx) {
    int texCoordIndex = fTransforms[effectIdx].fTexCoordIndex;
    int numTransforms = effectStage.getProcessor()->numTransforms();
    for (int t = 0; t < numTransforms; ++t) {
        const SkMatrix& transform = get_transform_matrix(effectStage, false, t);
        GrGLPathRendering::PathTexGenComponents components =
                GrGLPathRendering::kST_PathTexGenComponents;
        if (effectStage.isPerspectiveCoordTransform(t, false)) {
            components = GrGLPathRendering::kSTR_PathTexGenComponents;
        }
        gpu->glPathRendering()->enablePathTexGen(texCoordIndex++, components, transform);
    }
}
