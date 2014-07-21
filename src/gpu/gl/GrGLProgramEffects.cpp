/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLProgramEffects.h"
#include "GrDrawEffect.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLShaderBuilder.h"
#include "gl/GrGLVertexEffect.h"
#include "gl/GrGpuGL.h"

typedef GrGLProgramEffects::TransformedCoords TransformedCoords;
typedef GrGLProgramEffects::TransformedCoordsArray TransformedCoordsArray;
typedef GrGLProgramEffects::TextureSampler TextureSampler;
typedef GrGLProgramEffects::TextureSamplerArray TextureSamplerArray;

/**
 * We specialize the vertex code for each of these matrix types.
 */
enum MatrixType {
    kNoPersp_MatrixType  = 0,
    kGeneral_MatrixType  = 1,
};

/**
 * The key for an individual coord transform is made up of a matrix type and a bit that
 * indicates the source of the input coords.
 */
enum {
    kMatrixTypeKeyBits   = 1,
    kMatrixTypeKeyMask   = (1 << kMatrixTypeKeyBits) - 1,
    kPositionCoords_Flag = (1 << kMatrixTypeKeyBits),
    kTransformKeyBits    = kMatrixTypeKeyBits + 1,
};

namespace {

/**
 * Do we need to either map r,g,b->a or a->r. configComponentMask indicates which channels are
 * present in the texture's config. swizzleComponentMask indicates the channels present in the
 * shader swizzle.
 */
inline bool swizzle_requires_alpha_remapping(const GrGLCaps& caps,
                                             uint32_t configComponentMask,
                                             uint32_t swizzleComponentMask) {
    if (caps.textureSwizzleSupport()) {
        // Any remapping is handled using texture swizzling not shader modifications.
        return false;
    }
    // check if the texture is alpha-only
    if (kA_GrColorComponentFlag == configComponentMask) {
        if (caps.textureRedSupport() && (kA_GrColorComponentFlag & swizzleComponentMask)) {
            // we must map the swizzle 'a's to 'r'.
            return true;
        }
        if (kRGB_GrColorComponentFlags & swizzleComponentMask) {
            // The 'r', 'g', and/or 'b's must be mapped to 'a' according to our semantics that
            // alpha-only textures smear alpha across all four channels when read.
            return true;
        }
    }
    return false;
}

/**
 * Retrieves the matrix type from transformKey for the transform at transformIdx.
 */
MatrixType get_matrix_type(uint32_t transformKey, int transformIdx) {
    return static_cast<MatrixType>(
               (transformKey >> (kTransformKeyBits * transformIdx)) & kMatrixTypeKeyMask);
}

/**
 * Retrieves the source coords from transformKey for the transform at transformIdx. It may not be
 * the same coordinate set as the original GrCoordTransform if the position and local coords are
 * identical for this program.
 */
GrCoordSet get_source_coords(uint32_t transformKey, int transformIdx) {
    return (transformKey >> (kTransformKeyBits * transformIdx)) & kPositionCoords_Flag ?
               kPosition_GrCoordSet :
               kLocal_GrCoordSet;
}

/**
 * Retrieves the final matrix that a transform needs to apply to its source coords.
 */
SkMatrix get_transform_matrix(const GrDrawEffect& drawEffect, int transformIdx) {
    const GrCoordTransform& coordTransform = drawEffect.effect()->coordTransform(transformIdx);
    SkMatrix combined;
    if (kLocal_GrCoordSet == coordTransform.sourceCoords() &&
        !drawEffect.programHasExplicitLocalCoords()) {
        combined.setConcat(coordTransform.getMatrix(), drawEffect.getCoordChangeMatrix());
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

bool GrGLProgramEffects::GenEffectMetaKey(const GrDrawEffect& drawEffect, const GrGLCaps& caps,
                                          GrEffectKeyBuilder* b) {

    uint32_t textureKey = GrGLProgramEffects::GenTextureKey(drawEffect, caps);
    uint32_t transformKey = GrGLProgramEffects::GenTransformKey(drawEffect);
    uint32_t attribKey = GrGLProgramEffects::GenAttribKey(drawEffect);
    uint32_t classID = drawEffect.effect()->getFactory().effectClassID();

    // Currently we allow 16 bits for each of the above portions of the meta-key. Fail if they
    // don't fit.
    static const uint32_t kMetaKeyInvalidMask = ~((uint32_t) SK_MaxU16);
    if ((textureKey | transformKey | attribKey | classID) & kMetaKeyInvalidMask) {
        return false;
    }

    uint32_t* key = b->add32n(2);
    key[0] = (textureKey << 16 | transformKey);
    key[1] = (classID << 16 | attribKey);
    return true;
}

uint32_t GrGLProgramEffects::GenAttribKey(const GrDrawEffect& drawEffect) {
    uint32_t key = 0;
    int numAttributes = drawEffect.getVertexAttribIndexCount();
    SkASSERT(numAttributes <= 2);
    const int* attributeIndices = drawEffect.getVertexAttribIndices();
    for (int a = 0; a < numAttributes; ++a) {
        uint32_t value = attributeIndices[a] << 3 * a;
        SkASSERT(0 == (value & key)); // keys for each attribute ought not to overlap
        key |= value;
    }
    return key;
}

uint32_t GrGLProgramEffects::GenTransformKey(const GrDrawEffect& drawEffect) {
    uint32_t totalKey = 0;
    int numTransforms = drawEffect.effect()->numTransforms();
    for (int t = 0; t < numTransforms; ++t) {
        uint32_t key = 0;
        const GrCoordTransform& coordTransform = drawEffect.effect()->coordTransform(t);
        SkMatrix::TypeMask type0 = coordTransform.getMatrix().getType();
        SkMatrix::TypeMask type1;
        if (kLocal_GrCoordSet == coordTransform.sourceCoords()) {
            type1 = drawEffect.getCoordChangeMatrix().getType();
        } else {
            if (drawEffect.programHasExplicitLocalCoords()) {
                // We only make the key indicate that device coords are referenced when the local coords
                // are not actually determined by positions. Otherwise the local coords var and position
                // var are identical.
                key |= kPositionCoords_Flag;
            }
            type1 = SkMatrix::kIdentity_Mask;
        }

        int combinedTypes = type0 | type1;

        if (SkMatrix::kPerspective_Mask & combinedTypes) {
            key |= kGeneral_MatrixType;
        } else {
            key |= kNoPersp_MatrixType;
        }
        key <<= kTransformKeyBits * t;
        SkASSERT(0 == (totalKey & key)); // keys for each transform ought not to overlap
        totalKey |= key;
    }
    return totalKey;
}

uint32_t GrGLProgramEffects::GenTextureKey(const GrDrawEffect& drawEffect, const GrGLCaps& caps) {
    uint32_t key = 0;
    int numTextures = drawEffect.effect()->numTextures();
    for (int t = 0; t < numTextures; ++t) {
        const GrTextureAccess& access = drawEffect.effect()->textureAccess(t);
        uint32_t configComponentMask = GrPixelConfigComponentMask(access.getTexture()->config());
        if (swizzle_requires_alpha_remapping(caps, configComponentMask, access.swizzleMask())) {
            key |= 1 << t;
        }
    }
    return key;
}

GrGLProgramEffects::~GrGLProgramEffects() {
    int numEffects = fGLEffects.count();
    for (int e = 0; e < numEffects; ++e) {
        SkDELETE(fGLEffects[e]);
    }
}

void GrGLProgramEffects::emitSamplers(GrGLShaderBuilder* builder,
                                      const GrEffect* effect,
                                      TextureSamplerArray* outSamplers) {
    SkTArray<Sampler, true>& samplers = fSamplers.push_back();
    int numTextures = effect->numTextures();
    samplers.push_back_n(numTextures);
    SkString name;
    for (int t = 0; t < numTextures; ++t) {
        name.printf("Sampler%d", t);
        samplers[t].fUniform = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                                   kSampler2D_GrSLType,
                                                   name.c_str());
        SkNEW_APPEND_TO_TARRAY(outSamplers, TextureSampler,
                               (samplers[t].fUniform, effect->textureAccess(t)));
    }
}

void GrGLProgramEffects::initSamplers(const GrGLUniformManager& uniformManager, int* texUnitIdx) {
    int numEffects = fGLEffects.count();
    SkASSERT(numEffects == fSamplers.count());
    for (int e = 0; e < numEffects; ++e) {
        SkTArray<Sampler, true>& samplers = fSamplers[e];
        int numSamplers = samplers.count();
        for (int s = 0; s < numSamplers; ++s) {
            SkASSERT(samplers[s].fUniform.isValid());
            uniformManager.setSampler(samplers[s].fUniform, *texUnitIdx);
            samplers[s].fTextureUnit = (*texUnitIdx)++;
        }
    }
}

void GrGLProgramEffects::bindTextures(GrGpuGL* gpu, const GrEffect* effect, int effectIdx) {
    const SkTArray<Sampler, true>& samplers = fSamplers[effectIdx];
    int numSamplers = samplers.count();
    SkASSERT(numSamplers == effect->numTextures());
    for (int s = 0; s < numSamplers; ++s) {
        SkASSERT(samplers[s].fTextureUnit >= 0);
        const GrTextureAccess& textureAccess = effect->textureAccess(s);
        gpu->bindTexture(samplers[s].fTextureUnit,
                         textureAccess.getParams(),
                         static_cast<GrGLTexture*>(textureAccess.getTexture()));
    }
}

////////////////////////////////////////////////////////////////////////////////

void GrGLVertexProgramEffects::emitEffect(GrGLFullShaderBuilder* builder,
                                          const GrEffectStage& stage,
                                          const GrEffectKey& key,
                                          const char* outColor,
                                          const char* inColor,
                                          int stageIndex) {
    GrDrawEffect drawEffect(stage, fHasExplicitLocalCoords);
    const GrEffect* effect = stage.getEffect();
    SkSTArray<2, TransformedCoords> coords(effect->numTransforms());
    SkSTArray<4, TextureSampler> samplers(effect->numTextures());

    this->emitAttributes(builder, stage);
    this->emitTransforms(builder, drawEffect, &coords);
    this->emitSamplers(builder, effect, &samplers);

    GrGLEffect* glEffect = effect->getFactory().createGLInstance(drawEffect);
    fGLEffects.push_back(glEffect);

    // Enclose custom code in a block to avoid namespace conflicts
    SkString openBrace;
    openBrace.printf("\t{ // Stage %d: %s\n", stageIndex, glEffect->name());
    builder->vsCodeAppend(openBrace.c_str());
    builder->fsCodeAppend(openBrace.c_str());

    if (glEffect->isVertexEffect()) {
        GrGLVertexEffect* vertexEffect = static_cast<GrGLVertexEffect*>(glEffect);
        vertexEffect->emitCode(builder, drawEffect, key, outColor, inColor, coords, samplers);
    } else {
        glEffect->emitCode(builder, drawEffect, key, outColor, inColor, coords, samplers);
    }

    builder->vsCodeAppend("\t}\n");
    builder->fsCodeAppend("\t}\n");
}

void GrGLVertexProgramEffects::emitAttributes(GrGLFullShaderBuilder* builder,
                                              const GrEffectStage& stage) {
    int numAttributes = stage.getVertexAttribIndexCount();
    const int* attributeIndices = stage.getVertexAttribIndices();
    for (int a = 0; a < numAttributes; ++a) {
        // TODO: Make addAttribute mangle the name.
        SkString attributeName("aAttr");
        attributeName.appendS32(attributeIndices[a]);
        builder->addEffectAttribute(attributeIndices[a],
                                    stage.getEffect()->vertexAttribType(a),
                                    attributeName);
    }
}

void GrGLVertexProgramEffects::emitTransforms(GrGLFullShaderBuilder* builder,
                                              const GrDrawEffect& drawEffect,
                                              TransformedCoordsArray* outCoords) {
    SkTArray<Transform, true>& transforms = fTransforms.push_back();
    uint32_t totalKey = GenTransformKey(drawEffect);
    int numTransforms = drawEffect.effect()->numTransforms();
    transforms.push_back_n(numTransforms);
    for (int t = 0; t < numTransforms; t++) {
        GrSLType varyingType = kVoid_GrSLType;
        const char* uniName;
        switch (get_matrix_type(totalKey, t)) {
            case kNoPersp_MatrixType:
                uniName = "StageMatrix";
                varyingType = kVec2f_GrSLType;
                break;
            case kGeneral_MatrixType:
                uniName = "StageMatrix";
                varyingType = kVec3f_GrSLType;
                break;
            default:
                SkFAIL("Unexpected key.");
        }
        SkString suffixedUniName;
        if (0 != t) {
            suffixedUniName.append(uniName);
            suffixedUniName.appendf("_%i", t);
            uniName = suffixedUniName.c_str();
        }
        transforms[t].fHandle = builder->addUniform(GrGLShaderBuilder::kVertex_Visibility,
                                                    kMat33f_GrSLType,
                                                    uniName,
                                                    &uniName);

        const char* varyingName = "MatrixCoord";
        SkString suffixedVaryingName;
        if (0 != t) {
            suffixedVaryingName.append(varyingName);
            suffixedVaryingName.appendf("_%i", t);
            varyingName = suffixedVaryingName.c_str();
        }
        const char* vsVaryingName;
        const char* fsVaryingName;
        builder->addVarying(varyingType, varyingName, &vsVaryingName, &fsVaryingName);

        const GrGLShaderVar& coords = kPosition_GrCoordSet == get_source_coords(totalKey, t) ?
                                          builder->positionAttribute() :
                                          builder->localCoordsAttribute();
        // varying = matrix * coords (logically)
        SkASSERT(kVec2f_GrSLType == varyingType || kVec3f_GrSLType == varyingType);
        if (kVec2f_GrSLType == varyingType) {
            builder->vsCodeAppendf("\t%s = (%s * vec3(%s, 1)).xy;\n",
                                   vsVaryingName, uniName, coords.c_str());
        } else {
            builder->vsCodeAppendf("\t%s = %s * vec3(%s, 1);\n",
                                   vsVaryingName, uniName, coords.c_str());
        }
        SkNEW_APPEND_TO_TARRAY(outCoords, TransformedCoords,
                               (SkString(fsVaryingName), varyingType));
    }
}

void GrGLVertexProgramEffects::setData(GrGpuGL* gpu,
                                       const GrGLUniformManager& uniformManager,
                                       const GrEffectStage* effectStages[]) {
    int numEffects = fGLEffects.count();
    SkASSERT(numEffects == fTransforms.count());
    SkASSERT(numEffects == fSamplers.count());
    for (int e = 0; e < numEffects; ++e) {
        GrDrawEffect drawEffect(*effectStages[e], fHasExplicitLocalCoords);
        fGLEffects[e]->setData(uniformManager, drawEffect);
        this->setTransformData(uniformManager, drawEffect, e);
        this->bindTextures(gpu, drawEffect.effect(), e);
    }
}

void GrGLVertexProgramEffects::setTransformData(const GrGLUniformManager& uniformManager,
                                                const GrDrawEffect& drawEffect,
                                                int effectIdx) {
    SkTArray<Transform, true>& transforms = fTransforms[effectIdx];
    int numTransforms = transforms.count();
    SkASSERT(numTransforms == drawEffect.effect()->numTransforms());
    for (int t = 0; t < numTransforms; ++t) {
        SkASSERT(transforms[t].fHandle.isValid());
        const SkMatrix& matrix = get_transform_matrix(drawEffect, t);
        if (!transforms[t].fCurrentValue.cheapEqualTo(matrix)) {
            uniformManager.setSkMatrix(transforms[t].fHandle, matrix);
            transforms[t].fCurrentValue = matrix;
        }
    }
}

GrGLVertexProgramEffectsBuilder::GrGLVertexProgramEffectsBuilder(GrGLFullShaderBuilder* builder,
                                                                 int reserveCount)
    : fBuilder(builder)
    , fProgramEffects(SkNEW_ARGS(GrGLVertexProgramEffects,
                                 (reserveCount, fBuilder->hasExplicitLocalCoords()))) {
}

void GrGLVertexProgramEffectsBuilder::emitEffect(const GrEffectStage& stage,
                                                 const GrEffectKey& key,
                                                 const char* outColor,
                                                 const char* inColor,
                                                 int stageIndex) {
    SkASSERT(NULL != fProgramEffects.get());
    fProgramEffects->emitEffect(fBuilder, stage, key, outColor, inColor, stageIndex);
}

////////////////////////////////////////////////////////////////////////////////

void GrGLPathTexGenProgramEffects::emitEffect(GrGLFragmentOnlyShaderBuilder* builder,
                                          const GrEffectStage& stage,
                                          const GrEffectKey& key,
                                          const char* outColor,
                                          const char* inColor,
                                          int stageIndex) {
    GrDrawEffect drawEffect(stage, false);
    const GrEffect* effect = stage.getEffect();
    SkSTArray<2, TransformedCoords> coords(effect->numTransforms());
    SkSTArray<4, TextureSampler> samplers(effect->numTextures());

    SkASSERT(0 == stage.getVertexAttribIndexCount());
    this->setupPathTexGen(builder, drawEffect, &coords);
    this->emitSamplers(builder, effect, &samplers);

    GrGLEffect* glEffect = effect->getFactory().createGLInstance(drawEffect);
    fGLEffects.push_back(glEffect);

    // Enclose custom code in a block to avoid namespace conflicts
    SkString openBrace;
    openBrace.printf("\t{ // Stage %d: %s\n", stageIndex, glEffect->name());
    builder->fsCodeAppend(openBrace.c_str());

    SkASSERT(!glEffect->isVertexEffect());
    glEffect->emitCode(builder, drawEffect, key, outColor, inColor, coords, samplers);

    builder->fsCodeAppend("\t}\n");
}

void GrGLPathTexGenProgramEffects::setupPathTexGen(GrGLFragmentOnlyShaderBuilder* builder,
                                           const GrDrawEffect& drawEffect,
                                           TransformedCoordsArray* outCoords) {
    int numTransforms = drawEffect.effect()->numTransforms();
    uint32_t totalKey = GenTransformKey(drawEffect);
    int texCoordIndex = builder->addTexCoordSets(numTransforms);
    SkNEW_APPEND_TO_TARRAY(&fTransforms, Transforms, (totalKey, texCoordIndex));
    SkString name;
    for (int t = 0; t < numTransforms; ++t) {
        GrSLType type = kGeneral_MatrixType == get_matrix_type(totalKey, t) ?
                            kVec3f_GrSLType :
                            kVec2f_GrSLType;
        name.printf("%s(gl_TexCoord[%i])", GrGLSLTypeString(type), texCoordIndex++);
        SkNEW_APPEND_TO_TARRAY(outCoords, TransformedCoords, (name, type));
    }
}

void GrGLPathTexGenProgramEffects::setData(GrGpuGL* gpu,
                                       const GrGLUniformManager& uniformManager,
                                       const GrEffectStage* effectStages[]) {
    int numEffects = fGLEffects.count();
    SkASSERT(numEffects == fTransforms.count());
    SkASSERT(numEffects == fSamplers.count());
    for (int e = 0; e < numEffects; ++e) {
        GrDrawEffect drawEffect(*effectStages[e], false);
        fGLEffects[e]->setData(uniformManager, drawEffect);
        this->setPathTexGenState(gpu, drawEffect, e);
        this->bindTextures(gpu, drawEffect.effect(), e);
    }
}

void GrGLPathTexGenProgramEffects::setPathTexGenState(GrGpuGL* gpu,
                                              const GrDrawEffect& drawEffect,
                                              int effectIdx) {
    uint32_t totalKey = fTransforms[effectIdx].fTransformKey;
    int texCoordIndex = fTransforms[effectIdx].fTexCoordIndex;
    int numTransforms = drawEffect.effect()->numTransforms();
    for (int t = 0; t < numTransforms; ++t) {
        switch (get_matrix_type(totalKey, t)) {
            case kNoPersp_MatrixType: {
                const SkMatrix& transform = get_transform_matrix(drawEffect, t);
                gpu->enablePathTexGen(texCoordIndex++,
                                      GrGpuGL::kST_PathTexGenComponents,
                                      transform);
                break;
            }
            case kGeneral_MatrixType: {
                const SkMatrix& transform = get_transform_matrix(drawEffect, t);
                gpu->enablePathTexGen(texCoordIndex++,
                                      GrGpuGL::kSTR_PathTexGenComponents,
                                      transform);
                break;
            }
            default:
                SkFAIL("Unexpected matrixs type.");
        }
    }
}

GrGLPathTexGenProgramEffectsBuilder::GrGLPathTexGenProgramEffectsBuilder(
        GrGLFragmentOnlyShaderBuilder* builder,
        int reserveCount)
    : fBuilder(builder)
    , fProgramEffects(SkNEW_ARGS(GrGLPathTexGenProgramEffects, (reserveCount))) {
}

void GrGLPathTexGenProgramEffectsBuilder::emitEffect(const GrEffectStage& stage,
                                                     const GrEffectKey& key,
                                                     const char* outColor,
                                                     const char* inColor,
                                                     int stageIndex) {
    SkASSERT(NULL != fProgramEffects.get());
    fProgramEffects->emitEffect(fBuilder, stage, key, outColor, inColor, stageIndex);
}
