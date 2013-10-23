
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "SkTypes.h"

#if SK_SUPPORT_GPU && SK_ALLOW_STATIC_GLOBAL_INITIALIZERS

#include "gl/GrGpuGL.h"
#include "GrBackendEffectFactory.h"
#include "GrContextFactory.h"
#include "GrDrawEffect.h"
#include "effects/GrConfigConversionEffect.h"

#include "SkChecksum.h"
#include "SkRandom.h"
#include "Test.h"

void GrGLProgramDesc::setRandom(SkRandom* random,
                                const GrGpuGL* gpu,
                                const GrRenderTarget* dstRenderTarget,
                                const GrTexture* dstCopyTexture,
                                const GrEffectStage* stages[],
                                int numColorStages,
                                int numCoverageStages,
                                int currAttribIndex) {
    int numEffects = numColorStages + numCoverageStages;
    size_t keyLength = KeyLength(numEffects);
    fKey.reset(keyLength);
    *this->atOffset<uint32_t, kLengthOffset>() = static_cast<uint32_t>(keyLength);
    memset(this->header(), 0, kHeaderSize);

    KeyHeader* header = this->header();
    header->fEmitsPointSize = random->nextBool();

    header->fPositionAttributeIndex = 0;

    // if the effects have used up all off the available attributes,
    // don't try to use color or coverage attributes as input
    do {
        header->fColorInput = static_cast<GrGLProgramDesc::ColorInput>(
                                  random->nextULessThan(kColorInputCnt));
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

    header->fDiscardIfZeroCoverage = random->nextBool();

    bool useLocalCoords = random->nextBool() && currAttribIndex < GrDrawState::kMaxVertexAttribCnt;
    header->fLocalCoordAttributeIndex = useLocalCoords ? currAttribIndex++ : -1;

    header->fColorEffectCnt = numColorStages;
    header->fCoverageEffectCnt = numCoverageStages;

    bool dstRead = false;
    bool fragPos = false;
    bool vertexCode = false;
    int numStages = numColorStages + numCoverageStages;
    for (int s = 0; s < numStages; ++s) {
        const GrBackendEffectFactory& factory = (*stages[s]->getEffect())->getFactory();
        GrDrawEffect drawEffect(*stages[s], useLocalCoords);
        this->effectKeys()[s] = factory.glEffectKey(drawEffect, gpu->glCaps());
        if ((*stages[s]->getEffect())->willReadDstColor()) {
            dstRead = true;
        }
        if ((*stages[s]->getEffect())->willReadFragmentPosition()) {
            fragPos = true;
        }
        if ((*stages[s]->getEffect())->hasVertexCode()) {
            vertexCode = true;
        }
    }

    if (dstRead) {
        header->fDstReadKey = GrGLShaderBuilder::KeyForDstRead(dstCopyTexture, gpu->glCaps());
    } else {
        header->fDstReadKey = 0;
    }
    if (fragPos) {
        header->fFragPosKey = GrGLShaderBuilder::KeyForFragmentPosition(dstRenderTarget,
                                                                         gpu->glCaps());
    } else {
        header->fFragPosKey = 0;
    }

    header->fHasVertexCode = vertexCode ||
                             useLocalCoords ||
                             kAttribute_ColorInput == header->fColorInput ||
                             kAttribute_ColorInput == header->fCoverageInput;

    CoverageOutput coverageOutput;
    bool illegalCoverageOutput;
    do {
        coverageOutput = static_cast<CoverageOutput>(random->nextULessThan(kCoverageOutputCnt));
        illegalCoverageOutput = (!gpu->caps()->dualSourceBlendingSupport() &&
                                 CoverageOutputUsesSecondaryOutput(coverageOutput)) ||
                                (!dstRead && kCombineWithDst_CoverageOutput == coverageOutput);
    } while (illegalCoverageOutput);

    header->fCoverageOutput = coverageOutput;

    *this->checksum() = 0;
    *this->checksum() = SkChecksum::Compute(reinterpret_cast<uint32_t*>(fKey.get()), keyLength);
    fInitialized = true;
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
        int attribIndices[2];
        GrTexture* dummyTextures[] = {dummyTexture1.get(), dummyTexture2.get()};

        int numStages = random.nextULessThan(maxStages + 1);
        int numColorStages = random.nextULessThan(numStages + 1);
        int numCoverageStages = numStages - numColorStages;

        SkAutoSTMalloc<8, const GrEffectStage*> stages(numStages);

        bool useFixedFunctionTexturing = this->shouldUseFixedFunctionTexturing();

        for (int s = 0; s < numStages;) {
            SkAutoTUnref<const GrEffectRef> effect(GrEffectTestFactory::CreateStage(
                                                                            &random,
                                                                            this->getContext(),
                                                                            *this->caps(),
                                                                            dummyTextures));
            int numAttribs = (*effect)->numVertexAttribs();

            // If adding this effect would exceed the max attrib count then generate a
            // new random effect.
            if (currAttribIndex + numAttribs > GrDrawState::kMaxVertexAttribCnt) {
                continue;
            }


            // If adding this effect would exceed the max texture coord set count then generate a
            // new random effect.
            if (useFixedFunctionTexturing && !(*effect)->hasVertexCode()) {
                int numTransforms = (*effect)->numTransforms();
                if (currTextureCoordSet + numTransforms > this->glCaps().maxFixedFunctionTextureCoords()) {
                    continue;
                }
                currTextureCoordSet += numTransforms;
            }

            useFixedFunctionTexturing = useFixedFunctionTexturing && !(*effect)->hasVertexCode();

            for (int i = 0; i < numAttribs; ++i) {
                attribIndices[i] = currAttribIndex++;
            }
            GrEffectStage* stage = SkNEW_ARGS(GrEffectStage,
                                              (effect.get(), attribIndices[0], attribIndices[1]));
            stages[s] = stage;
            ++s;
        }
        const GrTexture* dstTexture = random.nextBool() ? dummyTextures[0] : dummyTextures[1];
        pdesc.setRandom(&random,
                        this,
                        dummyTextures[0]->asRenderTarget(),
                        dstTexture,
                        stages.get(),
                        numColorStages,
                        numCoverageStages,
                        currAttribIndex);

        SkAutoTUnref<GrGLProgram> program(GrGLProgram::Create(this,
                                                              pdesc,
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

static void GLProgramsTest(skiatest::Reporter* reporter, GrContextFactory* factory) {
    for (int type = 0; type < GrContextFactory::kLastGLContextType; ++type) {
        GrContext* context = factory->get(static_cast<GrContextFactory::GLContextType>(type));
        if (NULL != context) {
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

#include "TestClassDef.h"
DEFINE_GPUTESTCLASS("GLPrograms", GLProgramsTestClass, GLProgramsTest)

// This is evil evil evil. The linker may throw away whole translation units as dead code if it
// thinks none of the functions are called. It will do this even if there are static initializers
// in the unit that could pass pointers to functions from the unit out to other translation units!
// We force some of the effects that would otherwise be discarded to link here.

#include "SkLightingImageFilter.h"
#include "SkMagnifierImageFilter.h"
#include "SkColorMatrixFilter.h"
#include "SkBitmapAlphaThresholdShader.h"

void forceLinking();

void forceLinking() {
    SkLightingImageFilter::CreateDistantLitDiffuse(SkPoint3(0,0,0), 0, 0, 0);
    SkMagnifierImageFilter mag(SkRect::MakeWH(SK_Scalar1, SK_Scalar1), SK_Scalar1);
    GrConfigConversionEffect::Create(NULL,
                                     false,
                                     GrConfigConversionEffect::kNone_PMConversion,
                                     SkMatrix::I());
    SkScalar matrix[20];
    SkColorMatrixFilter cmf(matrix);
    SkBitmapAlphaThresholdShader::Create(SkBitmap(), SkRegion(), 0x80);
}

#endif
