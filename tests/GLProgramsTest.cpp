
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "SkTypes.h"

#if SK_SUPPORT_GPU && SK_ALLOW_STATIC_GLOBAL_INITIALIZERS

#include "GrBackendEffectFactory.h"
#include "GrContextFactory.h"
#include "GrDrawEffect.h"
#include "effects/GrConfigConversionEffect.h"
#include "gl/GrGLPathRendering.h"
#include "gl/GrGpuGL.h"
#include "SkChecksum.h"
#include "SkRandom.h"
#include "Test.h"

bool GrGLProgramDesc::setRandom(SkRandom* random,
                                const GrGpuGL* gpu,
                                const GrRenderTarget* dstRenderTarget,
                                const GrTexture* dstCopyTexture,
                                const GrEffectStage* geometryProcessor,
                                const GrEffectStage* stages[],
                                int numColorStages,
                                int numCoverageStages,
                                int currAttribIndex) {
    bool useLocalCoords = random->nextBool() && currAttribIndex < GrDrawState::kMaxVertexAttribCnt;

    int numStages = numColorStages + numCoverageStages;
    fKey.reset();

    GR_STATIC_ASSERT(0 == kEffectKeyOffsetsAndLengthOffset % sizeof(uint32_t));

    // Make room for everything up to and including the array of offsets to effect keys.
    fKey.push_back_n(kEffectKeyOffsetsAndLengthOffset + 2 * sizeof(uint16_t) * (numStages +
            (geometryProcessor ? 1 : 0)));

    bool dstRead = false;
    bool fragPos = false;
    bool vertexShader = SkToBool(geometryProcessor);
    int offset = 0;
    if (geometryProcessor) {
        const GrEffectStage* stage = geometryProcessor;
        uint16_t* offsetAndSize = reinterpret_cast<uint16_t*>(fKey.begin() +
                                                              kEffectKeyOffsetsAndLengthOffset +
                                                              offset * 2 * sizeof(uint16_t));
        uint32_t effectKeyOffset = fKey.count();
        if (effectKeyOffset > SK_MaxU16) {
            fKey.reset();
            return false;
        }
        GrDrawEffect drawEffect(*stage, useLocalCoords);
        GrEffectKeyBuilder b(&fKey);
        uint16_t effectKeySize;
        if (!GetEffectKeyAndUpdateStats(*stage, gpu->glCaps(), useLocalCoords, &b,
                                        &effectKeySize, &dstRead, &fragPos, &vertexShader)) {
            fKey.reset();
            return false;
        }
        offsetAndSize[0] = effectKeyOffset;
        offsetAndSize[1] = effectKeySize;
        offset++;
    }

    for (int s = 0; s < numStages; ++s, ++offset) {
        const GrEffectStage* stage = stages[s];
        uint16_t* offsetAndSize = reinterpret_cast<uint16_t*>(fKey.begin() +
                                                              kEffectKeyOffsetsAndLengthOffset +
                                                              offset * 2 * sizeof(uint16_t));
        uint32_t effectKeyOffset = fKey.count();
        if (effectKeyOffset > SK_MaxU16) {
            fKey.reset();
            return false;
        }
        GrDrawEffect drawEffect(*stage, useLocalCoords);
        GrEffectKeyBuilder b(&fKey);
        uint16_t effectKeySize;
        if (!GetEffectKeyAndUpdateStats(*stage, gpu->glCaps(), useLocalCoords, &b,
                                        &effectKeySize, &dstRead, &fragPos, &vertexShader)) {
            fKey.reset();
            return false;
        }
        offsetAndSize[0] = effectKeyOffset;
        offsetAndSize[1] = effectKeySize;
    }

    KeyHeader* header = this->header();
    memset(header, 0, kHeaderSize);
    header->fEmitsPointSize = random->nextBool();

    header->fPositionAttributeIndex = 0;

    // if the effects have used up all off the available attributes,
    // don't try to use color or coverage attributes as input
    do {
        uint32_t colorRand = random->nextULessThan(2);
        header->fColorInput = (0 == colorRand) ? GrGLProgramDesc::kAttribute_ColorInput :
                                                 GrGLProgramDesc::kUniform_ColorInput;
    } while (GrDrawState::kMaxVertexAttribCnt <= currAttribIndex &&
             kAttribute_ColorInput == header->fColorInput);

    header->fColorAttributeIndex = (header->fColorInput == kAttribute_ColorInput) ?
                                        currAttribIndex++ :
                                        -1;

    do {
        header->fCoverageInput = static_cast<GrGLProgramDesc::ColorInput>(
                                     random->nextULessThan(kColorInputCnt));
    } while (GrDrawState::kMaxVertexAttribCnt <= currAttribIndex  &&
             kAttribute_ColorInput == header->fCoverageInput);
    header->fCoverageAttributeIndex = (header->fCoverageInput == kAttribute_ColorInput) ?
                                        currAttribIndex++ :
                                        -1;

#if GR_GL_EXPERIMENTAL_GS
    header->fExperimentalGS = gpu->caps()->geometryShaderSupport() && random->nextBool();
#endif

    header->fLocalCoordAttributeIndex = useLocalCoords ? currAttribIndex++ : -1;

    header->fColorEffectCnt = numColorStages;
    header->fCoverageEffectCnt = numCoverageStages;

    if (dstRead) {
        header->fDstReadKey = SkToU8(GrGLFragmentShaderBuilder::KeyForDstRead(dstCopyTexture,
                                                                      gpu->glCaps()));
    } else {
        header->fDstReadKey = 0;
    }
    if (fragPos) {
        header->fFragPosKey = SkToU8(GrGLFragmentShaderBuilder::KeyForFragmentPosition(dstRenderTarget,
                                                                               gpu->glCaps()));
    } else {
        header->fFragPosKey = 0;
    }

    header->fRequiresVertexShader = vertexShader ||
                                    useLocalCoords ||
                                    kAttribute_ColorInput == header->fColorInput ||
                                    kAttribute_ColorInput == header->fCoverageInput;
    header->fHasGeometryProcessor = vertexShader;

    CoverageOutput coverageOutput;
    bool illegalCoverageOutput;
    do {
        coverageOutput = static_cast<CoverageOutput>(random->nextULessThan(kCoverageOutputCnt));
        illegalCoverageOutput = (!gpu->caps()->dualSourceBlendingSupport() &&
                                 CoverageOutputUsesSecondaryOutput(coverageOutput)) ||
                                (!dstRead && kCombineWithDst_CoverageOutput == coverageOutput);
    } while (illegalCoverageOutput);

    header->fCoverageOutput = coverageOutput;

    this->finalize();
    return true;
}

bool GrGpuGL::programUnitTest(int maxStages) {

    GrTextureDesc dummyDesc;
    dummyDesc.fFlags = kRenderTarget_GrTextureFlagBit;
    dummyDesc.fConfig = kSkia8888_GrPixelConfig;
    dummyDesc.fWidth = 34;
    dummyDesc.fHeight = 18;
    SkAutoTUnref<GrTexture> dummyTexture1(this->createTexture(dummyDesc, NULL, 0));
    dummyDesc.fFlags = kNone_GrTextureFlags;
    dummyDesc.fConfig = kAlpha_8_GrPixelConfig;
    dummyDesc.fWidth = 16;
    dummyDesc.fHeight = 22;
    SkAutoTUnref<GrTexture> dummyTexture2(this->createTexture(dummyDesc, NULL, 0));

    if (!dummyTexture1 || ! dummyTexture2) {
        return false;
    }

    static const int NUM_TESTS = 512;

    SkRandom random;
    for (int t = 0; t < NUM_TESTS; ++t) {

#if 0
        GrPrintf("\nTest Program %d\n-------------\n", t);
        static const int stop = -1;
        if (t == stop) {
            int breakpointhere = 9;
        }
#endif

        GrGLProgramDesc pdesc;

        int currAttribIndex = 1;  // we need to always leave room for position
        int currTextureCoordSet = 0;
        int attribIndices[2] = { 0, 0 };
        GrTexture* dummyTextures[] = {dummyTexture1.get(), dummyTexture2.get()};

        int numStages = random.nextULessThan(maxStages + 1);
        int numColorStages = random.nextULessThan(numStages + 1);
        int numCoverageStages = numStages - numColorStages;

        SkAutoSTMalloc<8, const GrEffectStage*> stages(numStages);

        bool useFixedFunctionPathRendering = this->glCaps().pathRenderingSupport() &&
            this->glPathRendering()->texturingMode() == GrGLPathRendering::FixedFunction_TexturingMode &&
            random.nextBool();

        SkAutoTDelete<GrEffectStage> geometryProcessor;
        bool hasGeometryProcessor = useFixedFunctionPathRendering ? false : random.nextBool();
        if (hasGeometryProcessor) {
            while (true) {
                SkAutoTUnref<const GrEffect> effect(GrEffectTestFactory::CreateStage(
                                                                                &random,
                                                                                this->getContext(),
                                                                                *this->caps(),
                                                                                dummyTextures));
                SkASSERT(effect);

                // Only geometryProcessor can use vertex shader
                if (!effect->requiresVertexShader()) {
                    continue;
                }

                int numAttribs = effect->numVertexAttribs();
                for (int i = 0; i < numAttribs; ++i) {
                    attribIndices[i] = currAttribIndex++;
                }
                GrEffectStage* stage = SkNEW_ARGS(GrEffectStage,
                                                  (effect.get(), attribIndices[0], attribIndices[1]));
                geometryProcessor.reset(stage);
                break;
            }
        }
        for (int s = 0; s < numStages;) {
            SkAutoTUnref<const GrEffect> effect(GrEffectTestFactory::CreateStage(
                                                                            &random,
                                                                            this->getContext(),
                                                                            *this->caps(),
                                                                            dummyTextures));
            SkASSERT(effect);

            // Only geometryProcessor can use vertex shader
            if (effect->requiresVertexShader()) {
                continue;
            }

            // If adding this effect would exceed the max texture coord set count then generate a
            // new random effect.
            if (useFixedFunctionPathRendering) {
                int numTransforms = effect->numTransforms();
                if (currTextureCoordSet + numTransforms > this->glCaps().maxFixedFunctionTextureCoords()) {
                    continue;
                }
                currTextureCoordSet += numTransforms;
            }
            GrEffectStage* stage = SkNEW_ARGS(GrEffectStage,
                                              (effect.get(), attribIndices[0], attribIndices[1]));

            stages[s] = stage;
            ++s;
        }
        const GrTexture* dstTexture = random.nextBool() ? dummyTextures[0] : dummyTextures[1];
        if (!pdesc.setRandom(&random,
                             this,
                             dummyTextures[0]->asRenderTarget(),
                             dstTexture,
                             geometryProcessor.get(),
                             stages.get(),
                             numColorStages,
                             numCoverageStages,
                             currAttribIndex)) {
            return false;
        }

        SkAutoTUnref<GrGLProgram> program(GrGLProgram::Create(this,
                                                              pdesc,
                                                              geometryProcessor.get(),
                                                              stages,
                                                              stages + numColorStages));
        for (int s = 0; s < numStages; ++s) {
            SkDELETE(stages[s]);
        }
        if (NULL == program.get()) {
            return false;
        }
    }
    return true;
}

DEF_GPUTEST(GLPrograms, reporter, factory) {
    for (int type = 0; type < GrContextFactory::kLastGLContextType; ++type) {
        GrContext* context = factory->get(static_cast<GrContextFactory::GLContextType>(type));
        if (context) {
            GrGpuGL* gpu = static_cast<GrGpuGL*>(context->getGpu());
            int maxStages = 6;
#if SK_ANGLE
            // Some long shaders run out of temporary registers in the D3D compiler on ANGLE.
            if (type == GrContextFactory::kANGLE_GLContextType) {
                maxStages = 3;
            }
#endif
            REPORTER_ASSERT(reporter, gpu->programUnitTest(maxStages));
        }
    }
}

// This is evil evil evil. The linker may throw away whole translation units as dead code if it
// thinks none of the functions are called. It will do this even if there are static initializers
// in the unit that could pass pointers to functions from the unit out to other translation units!
// We force some of the effects that would otherwise be discarded to link here.

#include "SkAlphaThresholdFilter.h"
#include "SkColorMatrixFilter.h"
#include "SkLightingImageFilter.h"
#include "SkMagnifierImageFilter.h"

void forceLinking();

void forceLinking() {
    SkLightingImageFilter::CreateDistantLitDiffuse(SkPoint3(0,0,0), 0, 0, 0);
    SkAlphaThresholdFilter::Create(SkRegion(), .5f, .5f);
    SkAutoTUnref<SkImageFilter> mag(SkMagnifierImageFilter::Create(
        SkRect::MakeWH(SK_Scalar1, SK_Scalar1), SK_Scalar1));
    GrConfigConversionEffect::Create(NULL,
                                     false,
                                     GrConfigConversionEffect::kNone_PMConversion,
                                     SkMatrix::I());
    SkScalar matrix[20];
    SkAutoTUnref<SkColorMatrixFilter> cmf(SkColorMatrixFilter::Create(matrix));
}

#endif
