
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
#include "GrProgramStageFactory.h"
#include "effects/GrConfigConversionEffect.h"

#include "GrRandom.h"
#include "Test.h"

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

typedef GrGLProgram::StageDesc StageDesc;
// TODO: Effects should be able to register themselves for inclusion in the
// randomly generated shaders. They should be able to configure themselves
// randomly.
const GrCustomStage* create_random_effect(StageDesc* stageDesc,
                                          GrRandom* random,
                                          GrContext* context,
                                          GrTexture* dummyTextures[]) {

    // The new code uses SkRandom not GrRandom.
    // TODO: Remove GrRandom.
    SkRandom sk_random;
    sk_random.setSeed(random->nextU());
    GrCustomStage* stage = GrCustomStageTestFactory::CreateStage(&sk_random,
                                                                    context,
                                                                    dummyTextures);
    GrAssert(stage);
    return stage;
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

// This is evil evil evil. The linker may throw away whole translation units as dead code if it
// thinks none of the functions are called. It will do this even if there are static initilializers
// in the unit that could pass pointers to functions from the unit out to other translation units!
// We force some of the effects that would otherwise be discarded to link here.

#include "SkLightingImageFilter.h"
#include "SkMagnifierImageFilter.h"

void forceLinking();

void forceLinking() {
    SkLightingImageFilter::CreateDistantLitDiffuse(SkPoint3(0,0,0), 0, 0, 0);
    SkMagnifierImageFilter mag(SkRect::MakeWH(SK_Scalar1, SK_Scalar1), SK_Scalar1);
    GrConfigConversionEffect::Create(NULL, false);
}

#endif
