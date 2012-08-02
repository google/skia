#include "GrGpuGL.h"

#include "effects/GrColorTableEffect.h"
#include "effects/GrConvolutionEffect.h"
#include "../effects/gradients/SkLinearGradient.h"
#include "../effects/gradients/SkRadialGradient.h"
#include "../effects/gradients/SkTwoPointRadialGradient.h"
#include "../effects/gradients/SkTwoPointConicalGradient.h"
#include "../effects/gradients/SkSweepGradient.h"
#include "effects/GrMorphologyEffect.h"
#include "SkLightingImageFilter.h"
#include "GrProgramStageFactory.h"
#include "GrRandom.h"

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

// populate a pair of arrays with colors and stop info, colorCount indicates
// the max number of colors, and is set to the actual number on return
void random_gradient(GrRandom* r, int* colorCount, SkColor* colors, 
                           SkScalar** stops) {
    int outColors = random_int(r, 1, *colorCount);

    // if one color, omit stops, if two colors, randomly decide whether or not to
    if (outColors == 1 || (outColors == 2 && random_bool(r))) *stops = NULL;

    GrScalar stop = 0.f;
    for (int i = 0; i < outColors; ++i) {
        colors[i] = static_cast<SkColor>(r->nextF() * 0xffffffff);
        if (*stops) {
            (*stops)[i] = stop;
            stop = i < outColors - 1 ? stop + r->nextF() * (1.f - stop) : 1.f;
        }
    }

    *colorCount = outColors;
}

typedef GrGLProgram::StageDesc StageDesc;
// TODO: Effects should be able to register themselves for inclusion in the
// randomly generated shaders. They should be able to configure themselves
// randomly.
const GrCustomStage* create_random_effect(StageDesc* stageDesc, GrRandom* random,
                                          GrContext* context) {
    enum EffectType {
        kConvolution_EffectType,
        kErode_EffectType,
        kDilate_EffectType,
        kRadialGradient_EffectType,
        kRadial2Gradient_EffectType,
        kConical2Gradient_EffectType,
        kDiffuseDistant_EffectType,
        kDiffusePoint_EffectType,
        kDiffuseSpot_EffectType,
        kSpecularDistant_EffectType,
        kSpecularPoint_EffectType,
        kSpecularSpot_EffectType,
        kSweepGradient_EffectType,
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

    static const int kMaxGradientStops = 4;

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
        case kRadialGradient_EffectType: {
            SkPoint center = {random->nextF(), random->nextF()};
            SkScalar radius = random->nextF();
            int colorCount = kMaxGradientStops;
            SkColor colors[kMaxGradientStops];
            SkScalar stops[kMaxGradientStops];
            SkScalar* stopsPtr = stops;
            random_gradient(random, &colorCount, colors, &stopsPtr);
            SkShader::TileMode tileMode = static_cast<SkShader::TileMode>(
                random_int(random, SkShader::kTileModeCount));
            SkAutoTUnref<SkGradientShaderBase> gradient(
                static_cast<SkGradientShaderBase*>(SkGradientShader::CreateRadial(
                center, radius, colors, stopsPtr, colorCount, tileMode, NULL)));
            GrSamplerState sampler;
            GrCustomStage* stage = gradient->asNewCustomStage(context, &sampler);
            GrAssert(NULL != stage);
            return stage;
            }
        case kRadial2Gradient_EffectType: {
            SkPoint center1 = {random->nextF(), random->nextF()};
            SkPoint center2 = {random->nextF(), random->nextF()};
            SkScalar radius1 = random->nextF();
            SkScalar radius2;
            do {
                radius2 = random->nextF();
            } while (radius1 == radius2);
            int colorCount = kMaxGradientStops;
            SkColor colors[kMaxGradientStops];
            SkScalar stops[kMaxGradientStops];
            SkScalar* stopsPtr = stops;
            random_gradient(random, &colorCount, colors, &stopsPtr);
            SkShader::TileMode tileMode = static_cast<SkShader::TileMode>(
                random_int(random, SkShader::kTileModeCount));
            SkAutoTUnref<SkGradientShaderBase> gradient(
                static_cast<SkGradientShaderBase*>(SkGradientShader::
                CreateTwoPointRadial(center1, radius1, center2, radius2,
                colors, stopsPtr, colorCount, tileMode, NULL)));
            GrSamplerState sampler;
            GrCustomStage* stage = gradient->asNewCustomStage(context, &sampler);
            GrAssert(NULL != stage);
            return stage;
            }
        case kConical2Gradient_EffectType: {
            SkPoint center1 = {random->nextF(), random->nextF()};
            SkScalar radius1 = random->nextF();
            SkPoint center2;
            SkScalar radius2;
            do {
                center1.set(random->nextF(), random->nextF());
                radius2 = random->nextF();
            } while (radius1 == radius2 && center1 == center2);
            int colorCount = kMaxGradientStops;
            SkColor colors[kMaxGradientStops];
            SkScalar stops[kMaxGradientStops];
            SkScalar* stopsPtr = stops;
            random_gradient(random, &colorCount, colors, &stopsPtr);
            SkShader::TileMode tileMode = static_cast<SkShader::TileMode>(
                random_int(random, SkShader::kTileModeCount));
            SkAutoTUnref<SkGradientShaderBase> gradient(
                static_cast<SkGradientShaderBase*>(SkGradientShader::
                CreateTwoPointConical(center1, radius1, center2, radius2,
                colors, stopsPtr, colorCount, tileMode, NULL)));
            GrSamplerState sampler;
            GrCustomStage* stage = gradient->asNewCustomStage(context, &sampler);
            GrAssert(NULL != stage);
            return stage;
            }
        case kSweepGradient_EffectType: {
            SkPoint center = {random->nextF(), random->nextF()};
            SkScalar radius = random->nextF();
            int colorCount = kMaxGradientStops;
            SkColor colors[kMaxGradientStops];
            SkScalar stops[kMaxGradientStops];
            SkScalar* stopsPtr = stops;
            random_gradient(random, &colorCount, colors, &stopsPtr);
            SkAutoTUnref<SkGradientShaderBase> gradient(
                static_cast<SkGradientShaderBase*>(SkGradientShader::CreateSweep(
                center.fX, center.fY, colors, stopsPtr, colorCount, NULL)));
            GrSamplerState sampler;
            GrCustomStage* stage = gradient->asNewCustomStage(context, &sampler);
            GrAssert(NULL != stage);
            return stage;
            }
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
        case kColorTable_EffectType: {
            return SkNEW_ARGS(GrColorTableEffect, (NULL));
        }
        default:
            GrCrash("Unexpected custom effect type");
    }
    return NULL;
}
}

bool GrGpuGL::programUnitTest() {

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

            stage.fOptFlags = STAGE_OPTS[random_int(&random, GR_ARRAY_COUNT(STAGE_OPTS))];
            stage.fInConfigFlags = IN_CONFIG_FLAGS[random_int(&random, GR_ARRAY_COUNT(IN_CONFIG_FLAGS))];

            bool useCustomEffect = random_bool(&random);
            if (useCustomEffect) {
                customStages[s].reset(create_random_effect(&stage, &random, getContext()));
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
