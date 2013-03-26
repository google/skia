
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

#include "SkRandom.h"
#include "Test.h"

void GrGLProgram::Desc::setRandom(SkMWCRandom* random,
                                  const GrGpuGL* gpu,
                                  const GrEffectStage stages[GrDrawState::kNumStages]) {
    fAttribBindings = 0;
    fEmitsPointSize = random->nextBool();
    fColorInput = random->nextULessThan(kColorInputCnt);
    fCoverageInput = random->nextULessThan(kColorInputCnt);

    fColorFilterXfermode = random->nextULessThan(SkXfermode::kCoeffModesCnt);

    fFirstCoverageStage = random->nextULessThan(GrDrawState::kNumStages);

    fAttribBindings |= random->nextBool() ? GrDrawState::kCoverage_AttribBindingsBit : 0;

#if GR_GL_EXPERIMENTAL_GS
    fExperimentalGS = gpu->caps()->geometryShaderSupport() && random->nextBool();
#endif

    if (gpu->caps()->shaderDerivativeSupport()) {
        fDiscardIfOutsideEdge = random->nextBool();
    } else {
        fDiscardIfOutsideEdge = false;
    }

    if (gpu->caps()->dualSourceBlendingSupport()) {
        fDualSrcOutput = random->nextULessThan(kDualSrcOutputCnt);
    } else {
        fDualSrcOutput = kNone_DualSrcOutput;
    }

    // use separate tex coords?
    if (random->nextBool()) {
        fAttribBindings |= GrDrawState::kLocalCoords_AttribBindingsBit;
    }

    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        if (NULL != stages[s].getEffect()) {
            const GrBackendEffectFactory& factory = (*stages[s].getEffect())->getFactory();
            bool explicitLocalCoords = (fAttribBindings &
                                        GrDrawState::kLocalCoords_AttribBindingsBit);
            GrDrawEffect drawEffect(stages[s], explicitLocalCoords);
            fEffectKeys[s] = factory.glEffectKey(drawEffect, gpu->glCaps());
        }
    }

    int attributeIndex = 0;
    fPositionAttributeIndex = attributeIndex;
    ++attributeIndex;
    if (fColorInput || (fAttribBindings & GrDrawState::kColor_AttribBindingsBit)) {
        fColorAttributeIndex = attributeIndex;
        ++attributeIndex;
    }
    if (fCoverageInput || (fAttribBindings & GrDrawState::kCoverage_AttribBindingsBit)) {
        fCoverageAttributeIndex = attributeIndex;
        ++attributeIndex;
    }
    if (fAttribBindings & GrDrawState::kLocalCoords_AttribBindingsBit) {
        fLocalCoordsAttributeIndex = attributeIndex;
        ++attributeIndex;
    }
}

bool GrGpuGL::programUnitTest(int maxStages) {

    maxStages = GrMin(maxStages, (int)GrDrawState::kNumStages);

    GrTextureDesc dummyDesc;
    dummyDesc.fConfig = kSkia8888_GrPixelConfig;
    dummyDesc.fWidth = 34;
    dummyDesc.fHeight = 18;
    SkAutoTUnref<GrTexture> dummyTexture1(this->createTexture(dummyDesc, NULL, 0));
    dummyDesc.fConfig = kAlpha_8_GrPixelConfig;
    dummyDesc.fWidth = 16;
    dummyDesc.fHeight = 22;
    SkAutoTUnref<GrTexture> dummyTexture2(this->createTexture(dummyDesc, NULL, 0));

    static const int NUM_TESTS = 512;

    SkMWCRandom random;
    for (int t = 0; t < NUM_TESTS; ++t) {

#if 0
        GrPrintf("\nTest Program %d\n-------------\n", t);
        static const int stop = -1;
        if (t == stop) {
            int breakpointhere = 9;
        }
#endif

        GrGLProgram::Desc pdesc;
        GrEffectStage stages[GrDrawState::kNumStages];

        int currAttribIndex = GrDrawState::kAttribIndexCount;
        int attribIndices[2];
        for (int s = 0; s < maxStages; ++s) {
            // enable the stage?
            if (random.nextBool()) {
                GrTexture* dummyTextures[] = {dummyTexture1.get(), dummyTexture2.get()};
                SkAutoTUnref<const GrEffectRef> effect(GrEffectTestFactory::CreateStage(
                                                                                &random,
                                                                                this->getContext(),
                                                                                *this->caps(),
                                                                                dummyTextures));
                int numAttribs = (*effect)->numVertexAttribs();

                // If adding this effect would cause to exceed the max attrib count then generate a
                // new random effect. The explanation for why this check is correct is a bit
                // convoluted and this code will be removed soon.
                if (currAttribIndex + numAttribs > GrDrawState::kCoverageOverrideAttribIndexValue) {
                    --s;
                    continue;
                }
                for (int i = 0; i < numAttribs; ++i) {
                    attribIndices[i] = currAttribIndex++;
                }
                stages[s].setEffect(effect.get(), attribIndices[0], attribIndices[1]);
            }
        }
        pdesc.setRandom(&random, this, stages);

        const GrEffectStage* stagePtrs[GrDrawState::kNumStages];
        for (int s = 0; s < GrDrawState::kNumStages; ++s) {
            stagePtrs[s] = &stages[s];
        }
        SkAutoTUnref<GrGLProgram> program(GrGLProgram::Create(this->glContext(),
                                                              pdesc,
                                                              stagePtrs));
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
            int maxStages = GrDrawState::kNumStages;
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
