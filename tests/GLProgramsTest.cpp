
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "gl/GrGpuGL.h"
#include "effects/GrColorTableEffect.h"
#include "effects/GrConvolutionEffect.h"
#include "effects/GrMorphologyEffect.h"
#include "SkLightingImageFilter.h"
#include "GrProgramStageFactory.h"
#include "GrRandom.h"
#include "Test.h"

#if SK_SUPPORT_GPU && SK_ALLOW_STATIC_GLOBAL_INITIALIZERS

namespace {

// GrRandoms nextU() values have patterns in the low bits
// So using nextU() % array_count might never take some values.
int random_int(GrRandom* r, int count) {
    return (int)(r->nextF() * count);
}

// min is inclusive, max is exclusive
int random_int(GrRandom* r, int min, int max) {
    return (int)(r->nextF() * (max-min)) + min;
}

bool random_bool(GrRandom* r) {
    return r->nextF() > .5f;
}

SkPoint3 random_point3(GrRandom* r) {
    return SkPoint3(r->nextF(), r->nextF(), r->nextF());
}

typedef GrGLProgram::StageDesc StageDesc;
// TODO: Effects should be able to register themselves for inclusion in the
// randomly generated shaders. They should be able to configure themselves
// randomly.
const GrCustomStage* create_random_effect(StageDesc* stageDesc,
                                          GrRandom* random,
                                          GrContext* context,
                                          GrTexture* dummyTextures[]) {
    enum EffectType {
        kConvolution_EffectType,
        kErode_EffectType,
        kDilate_EffectType,
        /**
         * Lighting effects don't work in unit test because they assume they insert functions and
         * assume the names are unique. This breaks when there are two light effects in the same
         * shader.
         */
        /*
        kDiffuseDistant_EffectType,
        kDiffusePoint_EffectType,
        kDiffuseSpot_EffectType,
        kSpecularDistant_EffectType,
        kSpecularPoint_EffectType,
        kSpecularSpot_EffectType,
        */

        kColorTable_EffectType,

        kEffectCount
    };

    // TODO: Remove this when generator doesn't apply this non-custom-stage
    // notion to custom stages automatically.
    static const uint32_t kMulByAlphaMask =
        StageDesc::kMulRGBByAlpha_RoundUp_InConfigFlag |
        StageDesc::kMulRGBByAlpha_RoundDown_InConfigFlag;

    static const Gr1DKernelEffect::Direction gKernelDirections[] = {
        Gr1DKernelEffect::kX_Direction,
        Gr1DKernelEffect::kY_Direction
    };

    // The new code uses SkRandom not GrRandom.
    // TODO: Remove GrRandom.
    SkRandom sk_random;
    sk_random.setSeed(random->nextU());

    bool useFactory = random_bool(random);
    if (useFactory) {
        GrCustomStage* stage = GrCustomStageTestFactory::CreateStage(&sk_random,
                                                                     context,
                                                                     dummyTextures);
        GrAssert(stage);
        return stage;
    }


    // TODO: When matrices are property of the custom-stage then remove the
    // no-persp flag code below.
    int effect = random_int(random, kEffectCount);
    switch (effect) {
        case kConvolution_EffectType: {
            int direction = random_int(random, 2);
            int kernelRadius = random_int(random, 1, 4);
            float kernel[GrConvolutionEffect::kMaxKernelWidth];
            for (int i = 0; i < GrConvolutionEffect::kMaxKernelWidth; i++) {
                kernel[i] = random->nextF();
            }
            // does not work with perspective or mul-by-alpha-mask
            stageDesc->fOptFlags |= StageDesc::kNoPerspective_OptFlagBit;
            stageDesc->fInConfigFlags &= ~kMulByAlphaMask;
            return SkNEW_ARGS(GrConvolutionEffect,
                              (NULL,
                               gKernelDirections[direction],
                               kernelRadius,
                               kernel));
            }
        case kErode_EffectType: {
            int direction = random_int(random, 2);
            int kernelRadius = random_int(random, 1, 4);
            // does not work with perspective or mul-by-alpha-mask
            stageDesc->fOptFlags |= StageDesc::kNoPerspective_OptFlagBit;
            stageDesc->fInConfigFlags &= ~kMulByAlphaMask;
            return SkNEW_ARGS(GrMorphologyEffect,
                              (NULL,
                               gKernelDirections[direction],
                               kernelRadius,
                               GrContext::kErode_MorphologyType));
            }
        case kDilate_EffectType: {
            int direction = random_int(random, 2);
            int kernelRadius = random_int(random, 1, 4);
            // does not work with perspective or mul-by-alpha-mask
            stageDesc->fOptFlags |= StageDesc::kNoPerspective_OptFlagBit;
            stageDesc->fInConfigFlags &= ~kMulByAlphaMask;
            return SkNEW_ARGS(GrMorphologyEffect,
                              (NULL,
                               gKernelDirections[direction],
                               kernelRadius,
                               GrContext::kDilate_MorphologyType));
            }
        /*
        case kDiffuseDistant_EffectType: {
            SkPoint3 direction = random_point3(random);
            direction.normalize();
            SkColor lightColor = random->nextU();
            SkScalar surfaceScale = SkFloatToScalar(random->nextF());
            SkScalar kd = SkFloatToScalar(random->nextF());
            SkAutoTUnref<SkImageFilter> filter(SkLightingImageFilter::CreateDistantLitDiffuse(direction, lightColor, surfaceScale, kd));
            // does not work with perspective or mul-by-alpha-mask
            GrCustomStage* stage;
            bool ok = filter->asNewCustomStage(&stage, NULL);
            SkASSERT(ok);
            return stage;
        }
        case kDiffusePoint_EffectType: {
            SkPoint3 location = random_point3(random);
            SkColor lightColor = random->nextU();
            SkScalar surfaceScale = SkFloatToScalar(random->nextF());
            SkScalar kd = SkFloatToScalar(random->nextF());
            SkAutoTUnref<SkImageFilter> filter(SkLightingImageFilter::CreatePointLitDiffuse(location, lightColor, surfaceScale, kd));
            // does not work with perspective or mul-by-alpha-mask
            GrCustomStage* stage;
            bool ok = filter->asNewCustomStage(&stage, NULL);
            SkASSERT(ok);
            return stage;
        }
        case kDiffuseSpot_EffectType: {
            SkPoint3 location = random_point3(random);
            SkPoint3 target = random_point3(random);
            SkScalar cutoffAngle = SkFloatToScalar(random->nextF());
            SkScalar specularExponent = SkFloatToScalar(random->nextF());
            SkColor lightColor = random->nextU();
            SkScalar surfaceScale = SkFloatToScalar(random->nextF());
            SkScalar ks = SkFloatToScalar(random->nextF());
            SkScalar shininess = SkFloatToScalar(random->nextF());
            SkAutoTUnref<SkImageFilter> filter(SkLightingImageFilter::CreateSpotLitSpecular(
                location, target, specularExponent, cutoffAngle, lightColor, surfaceScale, ks, shininess));
            // does not work with perspective or mul-by-alpha-mask
            GrCustomStage* stage;
            bool ok = filter->asNewCustomStage(&stage, NULL);
            SkASSERT(ok);
            return stage;
        }
        case kSpecularDistant_EffectType: {
            SkPoint3 direction = random_point3(random);
            direction.normalize();
            SkColor lightColor = random->nextU();
            SkScalar surfaceScale = SkFloatToScalar(random->nextF());
            SkScalar ks = SkFloatToScalar(random->nextF());
            SkScalar shininess = SkFloatToScalar(random->nextF());
            SkAutoTUnref<SkImageFilter> filter(SkLightingImageFilter::CreateDistantLitSpecular(direction, lightColor, surfaceScale, ks, shininess));
            // does not work with perspective or mul-by-alpha-mask
            GrCustomStage* stage;
            bool ok = filter->asNewCustomStage(&stage, NULL);
            SkASSERT(ok);
            return stage;
        }
        case kSpecularPoint_EffectType: {
            SkPoint3 location = random_point3(random);
            SkColor lightColor = random->nextU();
            SkScalar surfaceScale = SkFloatToScalar(random->nextF());
            SkScalar ks = SkFloatToScalar(random->nextF());
            SkScalar shininess = SkFloatToScalar(random->nextF());
            SkAutoTUnref<SkImageFilter> filter(SkLightingImageFilter::CreatePointLitSpecular(location, lightColor, surfaceScale, ks, shininess));
            // does not work with perspective or mul-by-alpha-mask
            GrCustomStage* stage;
            bool ok = filter->asNewCustomStage(&stage, NULL);
            SkASSERT(ok);
            return stage;
        }
        case kSpecularSpot_EffectType: {
            SkPoint3 location = random_point3(random);
            SkPoint3 target = random_point3(random);
            SkScalar cutoffAngle = SkFloatToScalar(random->nextF());
            SkScalar specularExponent = SkFloatToScalar(random->nextF());
            SkColor lightColor = random->nextU();
            SkScalar surfaceScale = SkFloatToScalar(random->nextF());
            SkScalar ks = SkFloatToScalar(random->nextF());
            SkScalar shininess = SkFloatToScalar(random->nextF());
            SkAutoTUnref<SkImageFilter> filter(SkLightingImageFilter::CreateSpotLitSpecular(
                location, target, specularExponent, cutoffAngle, lightColor, surfaceScale, ks, shininess));
            // does not work with perspective or mul-by-alpha-mask
            GrCustomStage* stage;
            bool ok = filter->asNewCustomStage(&stage, NULL);
            SkASSERT(ok);
            return stage;
        }
        */
        case kColorTable_EffectType: {
            GrTexture* alphaTexture = dummyTextures[GrCustomStageTestFactory::kAlphaTextureIdx];
            return SkNEW_ARGS(GrColorTableEffect, (alphaTexture));
        }
        default:
            GrCrash("Unexpected custom effect type");
    }
    return NULL;
}
}

bool GrGpuGL::programUnitTest() {

    GrTextureDesc dummyDesc;
    dummyDesc.fConfig = kSkia8888_PM_GrPixelConfig;
    dummyDesc.fWidth = 34;
    dummyDesc.fHeight = 18;
    SkAutoTUnref<GrTexture> dummyTexture1(this->createTexture(dummyDesc, NULL, 0));
    dummyDesc.fConfig = kAlpha_8_GrPixelConfig;
    dummyDesc.fWidth = 16;
    dummyDesc.fHeight = 22;
    SkAutoTUnref<GrTexture> dummyTexture2(this->createTexture(dummyDesc, NULL, 0));

    // GrGLSLGeneration glslGeneration = 
            GrGetGLSLGeneration(this->glBinding(), this->glInterface());
    static const int STAGE_OPTS[] = {
        0,
        StageDesc::kNoPerspective_OptFlagBit,
    };
    static const int IN_CONFIG_FLAGS[] = {
        StageDesc::kNone_InConfigFlag,
        StageDesc::kSwapRAndB_InConfigFlag,
        StageDesc::kSwapRAndB_InConfigFlag |
        StageDesc::kMulRGBByAlpha_RoundUp_InConfigFlag,
        StageDesc::kMulRGBByAlpha_RoundDown_InConfigFlag,
        StageDesc::kSmearAlpha_InConfigFlag,
        StageDesc::kSmearRed_InConfigFlag,
    };

    static const int NUM_TESTS = 512;

    GrRandom random;
    for (int t = 0; t < NUM_TESTS; ++t) {

#if 0
        GrPrintf("\nTest Program %d\n-------------\n", t);
        static const int stop = -1;
        if (t == stop) {
            int breakpointhere = 9;
        }
#endif

        ProgramDesc pdesc;
        pdesc.fVertexLayout = 0;
        pdesc.fEmitsPointSize = random.nextF() > .5f;
        pdesc.fColorInput = random_int(&random, ProgramDesc::kColorInputCnt);
        pdesc.fCoverageInput = random_int(&random, ProgramDesc::kColorInputCnt);

        pdesc.fColorFilterXfermode = random_int(&random, SkXfermode::kCoeffModesCnt);

        pdesc.fFirstCoverageStage = random_int(&random, GrDrawState::kNumStages);

        pdesc.fVertexLayout |= random_bool(&random) ?
                                    GrDrawTarget::kCoverage_VertexLayoutBit :
                                    0;

#if GR_GL_EXPERIMENTAL_GS
        pdesc.fExperimentalGS = this->getCaps().fGeometryShaderSupport &&
                                random_bool(&random);
#endif
        pdesc.fOutputConfig =  random_int(&random, ProgramDesc::kOutputConfigCnt);

        bool edgeAA = random_bool(&random);
        if (edgeAA) {
            pdesc.fVertexLayout |= GrDrawTarget::kEdge_VertexLayoutBit;
            if (this->getCaps().fShaderDerivativeSupport) {
                pdesc.fVertexEdgeType = (GrDrawState::VertexEdgeType) random_int(&random, GrDrawState::kVertexEdgeTypeCnt);
            } else {
                pdesc.fVertexEdgeType = GrDrawState::kHairLine_EdgeType;
            }
        } else {
        }

        pdesc.fColorMatrixEnabled = random_bool(&random);

        if (this->getCaps().fDualSourceBlendingSupport) {
            pdesc.fDualSrcOutput = random_int(&random, ProgramDesc::kDualSrcOutputCnt);
        } else {
            pdesc.fDualSrcOutput = ProgramDesc::kNone_DualSrcOutput;
        }

        SkAutoTUnref<const GrCustomStage> customStages[GrDrawState::kNumStages];

        for (int s = 0; s < GrDrawState::kNumStages; ++s) {
            StageDesc& stage = pdesc.fStages[s];
            // enable the stage?
            if (random_bool(&random)) {
                // use separate tex coords?
                if (random_bool(&random)) {
                    int t = random_int(&random, GrDrawState::kMaxTexCoords);
                    pdesc.fVertexLayout |= StageTexCoordVertexLayoutBit(s, t);
                }
                stage.setEnabled(true);
            }
            // use text-formatted verts?
            if (random_bool(&random)) {
                pdesc.fVertexLayout |= kTextFormat_VertexLayoutBit;
            }

            stage.fCustomStageKey = 0;

            stage.fOptFlags |= STAGE_OPTS[random_int(&random, GR_ARRAY_COUNT(STAGE_OPTS))];
            stage.fInConfigFlags = IN_CONFIG_FLAGS[random_int(&random, GR_ARRAY_COUNT(IN_CONFIG_FLAGS))];

            if (stage.isEnabled()) {
                GrTexture* dummyTextures[] = {dummyTexture1.get(), dummyTexture2.get()};
                customStages[s].reset(create_random_effect(&stage,
                                                           &random,
                                                           getContext(),
                                                           dummyTextures));
                if (NULL != customStages[s]) {
                    stage.fCustomStageKey =
                        customStages[s]->getFactory().glStageKey(*customStages[s], this->glCaps());
                }
            }
        }
        GR_STATIC_ASSERT(sizeof(customStages) ==
                         GrDrawState::kNumStages * sizeof(GrCustomStage*));
        const GrCustomStage** stages = reinterpret_cast<const GrCustomStage**>(&customStages);
        SkAutoTUnref<GrGLProgram> program(GrGLProgram::Create(this->glContextInfo(),
                                                              pdesc,
                                                              stages));
        if (NULL == program.get()) {
            return false;
        }
    }
    return true;
}

static void GLProgramsTest(skiatest::Reporter* reporter, GrContext* context) {
    GrGpuGL* shadersGpu = static_cast<GrGpuGL*>(context->getGpu());
    REPORTER_ASSERT(reporter, shadersGpu->programUnitTest());
}


#include "TestClassDef.h"
DEFINE_GPUTESTCLASS("GLPrograms", GLProgramsTestClass, GLProgramsTest)

#endif
