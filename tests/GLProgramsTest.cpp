
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
#include "effects/GrConfigConversionEffect.h"

#include "SkRandom.h"
#include "Test.h"

namespace {

// SkRandoms nextU() values have patterns in the low bits
// So using nextU() % array_count might never take some values.
int random_int(SkRandom* r, int count) {
    return (int)(r->nextF() * count);
}

bool random_bool(SkRandom* r) {
    return r->nextF() > .5f;
}

const GrEffectRef* create_random_effect(SkRandom* random,
                                        GrContext* context,
                                        GrTexture* dummyTextures[]) {

    SkRandom sk_random;
    sk_random.setSeed(random->nextU());
    GrEffectRef* effect = GrEffectTestFactory::CreateStage(&sk_random, context, dummyTextures);
    GrAssert(effect);
    return effect;
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

        ProgramDesc pdesc;
        pdesc.fVertexLayout = 0;
        pdesc.fEmitsPointSize = random.nextF() > .5f;
        pdesc.fColorInput = random_int(&random, ProgramDesc::kColorInputCnt);
        pdesc.fCoverageInput = random_int(&random, ProgramDesc::kColorInputCnt);

        pdesc.fColorFilterXfermode = random_int(&random, SkXfermode::kCoeffModesCnt);

        pdesc.fFirstCoverageStage = random_int(&random, GrDrawState::kNumStages);

        pdesc.fVertexLayout |= random_bool(&random) ?
                                    GrDrawState::kCoverage_VertexLayoutBit :
                                    0;

#if GR_GL_EXPERIMENTAL_GS
        pdesc.fExperimentalGS = this->getCaps().geometryShaderSupport() &&
                                random_bool(&random);
#endif

        bool edgeAA = random_bool(&random);
        if (edgeAA) {
            pdesc.fVertexLayout |= GrDrawState::kEdge_VertexLayoutBit;
            if (this->getCaps().shaderDerivativeSupport()) {
                pdesc.fVertexEdgeType = (GrDrawState::VertexEdgeType) random_int(&random, GrDrawState::kVertexEdgeTypeCnt);
                pdesc.fDiscardIfOutsideEdge = random.nextBool();
            } else {
                pdesc.fVertexEdgeType = GrDrawState::kHairLine_EdgeType;
                pdesc.fDiscardIfOutsideEdge = false;
            }
        } else {
        }

        if (this->getCaps().dualSourceBlendingSupport()) {
            pdesc.fDualSrcOutput = random_int(&random, ProgramDesc::kDualSrcOutputCnt);
        } else {
            pdesc.fDualSrcOutput = ProgramDesc::kNone_DualSrcOutput;
        }

        GrEffectStage stages[GrDrawState::kNumStages];

        for (int s = 0; s < GrDrawState::kNumStages; ++s) {
            // enable the stage?
            if (random_bool(&random)) {
                // use separate tex coords?
                if (random_bool(&random)) {
                    int t = random_int(&random, GrDrawState::kMaxTexCoords);
                    pdesc.fVertexLayout |= GrDrawState::StageTexCoordVertexLayoutBit(s, t);
                }
                // use text-formatted verts?
                if (random_bool(&random)) {
                    pdesc.fVertexLayout |= GrDrawState::kTextFormat_VertexLayoutBit;
                }

                GrTexture* dummyTextures[] = {dummyTexture1.get(), dummyTexture2.get()};
                SkAutoTUnref<const GrEffectRef> effect(create_random_effect(&random,
                                                                            getContext(),
                                                                            dummyTextures));
                stages[s].setEffect(effect.get());
                if (NULL != stages[s].getEffect()) {
                    pdesc.fEffectKeys[s] =
                        (*stages[s].getEffect())->getFactory().glEffectKey(stages[s],
                                                                           this->glCaps());
                }
            }
        }
        const GrEffectStage* stagePtrs[GrDrawState::kNumStages];
        for (int s = 0; s < GrDrawState::kNumStages; ++s) {
            stagePtrs[s] = &stages[s];
        }
        SkAutoTUnref<GrGLProgram> program(GrGLProgram::Create(this->glContextInfo(),
                                                              pdesc,
                                                              stagePtrs));
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
// thinks none of the functions are called. It will do this even if there are static initializers
// in the unit that could pass pointers to functions from the unit out to other translation units!
// We force some of the effects that would otherwise be discarded to link here.

#include "SkLightingImageFilter.h"
#include "SkMagnifierImageFilter.h"
#include "SkColorMatrixFilter.h"

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
}

#endif
