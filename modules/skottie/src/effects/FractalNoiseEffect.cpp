/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/effects/SkRuntimeEffect.h"
#include "include/utils/SkRandom.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGRenderNode.h"

namespace skottie::internal {
namespace {

// An implementation of the ADBE Fractal Noise effect:
//
//  - multiple noise sublayers (octaves) are combined using a weighted average
//  - each layer is subject to a (cumulative) transform, filter and post-sampling options
//
// Parameters:
//
//   * Noise Type    -- controls noise layer post-sampling filtering
//                      (Block, Linear, Soft Linear, Spline)
//   * Fractal Type  -- determines a noise layer post-filtering transformation
//                      (Basic, Turbulent Smooth, Turbulent Basic, etc)
//   * Transform     -- offset/scale/rotate the noise effect (local matrix)
//
//   * Complexity    -- number of sublayers;
//                      can be fractional, where the fractional part modulates the last layer
//   * Evolution     -- controls noise topology in a gradual manner (can be animated for smooth
//                      noise transitions)
//   * Sub Influence -- relative amplitude weight for sublayers (cumulative)
//
//   * Sub Scaling/Rotation/Offset -- relative scale for sublayers (cumulative)
//
//   * Invert        -- invert noise values
//
//   * Contrast      -- apply a contrast to the noise result
//
//   * Brightness    -- apply a brightness effect to the noise result
//
//
// TODO:
//   - Invert
//   - Contrast/Brightness

static constexpr char gNoiseEffectSkSL[] =
    "uniform half3x3 u_submatrix;" // sublayer transform

    "uniform half u_octaves,"      // number of octaves (can be fractional)
                 "u_persistence,"  // relative octave weight
                 "u_evolution;"    // evolution/seed

    // Hash based on hash13 (https://www.shadertoy.com/view/4djSRW).
    "half hash(half3 v) {"
        "v  = fract(v*0.1031);"
        "v += dot(v, v.zxy + 31.32);"
        "return fract((v.x + v.y)*v.z);"
    "}"

    // The general idea is to compute a coherent hash for two planes in discretized (x,y,e) space,
    // and interpolate between them.  This yields gradual changes when animating |e| - which is the
    // desired outcome.
    "half sample_noise(vec2 xy) {"
        "xy = floor(xy);"

        "half e_ = floor(u_evolution),"
             "t  = u_evolution - e_,"
             "n0 = hash(half3(xy, e_ + 0)),"
             "n1 = hash(half3(xy, e_ + 1));"

        // Note: Ideally we would use 4 samples (-1, 0, 1, 2) and cubic interpolation for
        //       better results -- but that's significantly more expensive than lerp.

        "return mix(n0, n1, t);"
    "}"

    // filter() placeholder
    "%s"

    // fractal() placeholder
    "%s"

    // Generate ceil(u_octaves) noise layers and combine based on persistentce and sublayer xform.
    "half4 main(vec2 xy) {"
        "half oct = u_octaves," // initial octave count (this is the effective loop counter)
             "amp = 1,"         // initial layer amplitude
            "wacc = 0,"         // weight accumulator
               "n = 0;"         // noise accumulator

        // Constant loop counter chosen to be >= ceil(u_octaves).
        // The logical counter is actually 'oct'.
        "for (half i = 0; i < %u; ++i) {"
            // effective layer weight computed to accommodate fixed loop counters
            //
            //   -- for full octaves:              layer amplitude
            //   -- for fractional octave:         layer amplitude modulated by fractional part
            //   -- for octaves > ceil(u_octaves): 0
            //
            // e.g. for 6 loops and u_octaves = 2.3, this generates the sequence [1,1,.3,0,0]
            "half w = amp*saturate(oct);"

            "n += w*fractal(filter(xy));"

            "wacc += w;"
            "amp  *= u_persistence;"
            "oct  -= 1;"

            "xy = (u_submatrix*half3(xy,1)).xy;"
        "}"

        "n /= wacc;"

        // TODO: fractal functions

        "return half4(n,n,n,1);"
    "}";

static constexpr char gFilterNearestSkSL[] =
    "half filter(half2 xy) {"
        "return sample_noise(xy);"
    "}";

static constexpr char gFilterLinearSkSL[] =
    "half filter(half2 xy) {"
        "xy -= 0.5;"

        "half n00 = sample_noise(xy + half2(0,0)),"
             "n10 = sample_noise(xy + half2(1,0)),"
             "n01 = sample_noise(xy + half2(0,1)),"
             "n11 = sample_noise(xy + half2(1,1));"

        "half2 t = fract(xy);"

        "return mix(mix(n00, n10, t.x), mix(n01, n11, t.x), t.y);"
    "}";

static constexpr char gFilterSoftLinearSkSL[] =
    "half filter(half2 xy) {"
        "xy -= 0.5;"

        "half n00 = sample_noise(xy + half2(0,0)),"
             "n10 = sample_noise(xy + half2(1,0)),"
             "n01 = sample_noise(xy + half2(0,1)),"
             "n11 = sample_noise(xy + half2(1,1));"

        "half2 t = smoothstep(0, 1, fract(xy));"

        "return mix(mix(n00, n10, t.x), mix(n01, n11, t.x), t.y);"
    "}";

static constexpr char gFractalBasicSkSL[] =
    "half fractal(half n) {"
        "return n;"
    "}";

static constexpr char gFractalTurbulentBasicSkSL[] =
    "half fractal(half n) {"
        "return 2*abs(0.5 - n);"
    "}";

static constexpr char gFractalTurbulentSmoothSkSL[] =
    "half fractal(half n) {"
        "n = 2*abs(0.5 - n);"
        "return n*n;"
    "}";

static constexpr char gFractalTurbulentSharpSkSL[] =
    "half fractal(half n) {"
        "return sqrt(2*abs(0.5 - n));"
    "}";

enum class NoiseFilter {
    kNearest,
    kLinear,
    kSoftLinear,
    // TODO: kSpline?
};

enum class NoiseFractal {
    kBasic,
    kTurbulentBasic,
    kTurbulentSmooth,
    kTurbulentSharp,
};

sk_sp<SkRuntimeEffect> make_noise_effect(unsigned loops, const char* filter, const char* fractal) {
    auto result =
            SkRuntimeEffect::Make(SkStringPrintf(gNoiseEffectSkSL, filter, fractal, loops), {});

    if (0 && !result.effect) {
        printf("!!! %s\n", result.errorText.c_str());
    }

    return std::move(result.effect);
}

template <unsigned LOOPS, NoiseFilter FILTER, NoiseFractal FRACTAL>
sk_sp<SkRuntimeEffect> noise_effect() {
    static constexpr char const* gFilters[] = {
        gFilterNearestSkSL,
        gFilterLinearSkSL,
        gFilterSoftLinearSkSL
    };

    static constexpr char const* gFractals[] = {
        gFractalBasicSkSL,
        gFractalTurbulentBasicSkSL,
        gFractalTurbulentSmoothSkSL,
        gFractalTurbulentSharpSkSL
    };

    static_assert(static_cast<size_t>(FILTER)  < SK_ARRAY_COUNT(gFilters));
    static_assert(static_cast<size_t>(FRACTAL) < SK_ARRAY_COUNT(gFractals));

    static const SkRuntimeEffect* effect =
            make_noise_effect(LOOPS,
                              gFilters[static_cast<size_t>(FILTER)],
                              gFractals[static_cast<size_t>(FRACTAL)])
            .release();

    SkASSERT(effect);
    return sk_ref_sp(effect);
}

class FractalNoiseNode final : public sksg::CustomRenderNode {
public:
    explicit FractalNoiseNode(sk_sp<RenderNode> child) : INHERITED({std::move(child)}) {}

    SG_ATTRIBUTE(Matrix      , SkMatrix    , fMatrix     )
    SG_ATTRIBUTE(SubMatrix   , SkMatrix    , fSubMatrix  )

    SG_ATTRIBUTE(NoiseFilter , NoiseFilter , fFilter     )
    SG_ATTRIBUTE(NoiseFractal, NoiseFractal, fFractal    )
    SG_ATTRIBUTE(Octaves     , float       , fOctaves    )
    SG_ATTRIBUTE(Persistence , float       , fPersistence)
    SG_ATTRIBUTE(Evolution   , float       , fEvolution  )

private:
    template <NoiseFilter FI, NoiseFractal FR>
    sk_sp<SkRuntimeEffect> getEffect() const {
        // Bin the loop counter based on the number of octaves (range: [1..20]).
        // Low complexities are common, so we maximize resolution for the low end.
        if (fOctaves > 8) return noise_effect<20, FI, FR>();
        if (fOctaves > 4) return noise_effect< 8, FI, FR>();
        if (fOctaves > 3) return noise_effect< 4, FI, FR>();
        if (fOctaves > 2) return noise_effect< 3, FI, FR>();
        if (fOctaves > 1) return noise_effect< 2, FI, FR>();

        return noise_effect<1, FI, FR>();
    }

    template <NoiseFilter FI>
    sk_sp<SkRuntimeEffect> getEffect() const {
        switch (fFractal) {
            case NoiseFractal::kBasic:
                return this->getEffect<FI, NoiseFractal::kBasic>();
            case NoiseFractal::kTurbulentBasic:
                return this->getEffect<FI, NoiseFractal::kTurbulentBasic>();
            case NoiseFractal::kTurbulentSmooth:
                return this->getEffect<FI, NoiseFractal::kTurbulentSmooth>();
            case NoiseFractal::kTurbulentSharp:
                return this->getEffect<FI, NoiseFractal::kTurbulentSharp>();
        }
        SkUNREACHABLE;
    }

    sk_sp<SkRuntimeEffect> getEffect() const {
        switch (fFilter) {
            case NoiseFilter::kNearest   : return this->getEffect<NoiseFilter::kNearest>();
            case NoiseFilter::kLinear    : return this->getEffect<NoiseFilter::kLinear>();
            case NoiseFilter::kSoftLinear: return this->getEffect<NoiseFilter::kSoftLinear>();
        }
        SkUNREACHABLE;
    }

    sk_sp<SkShader> buildEffectShader() const {
        SkRuntimeShaderBuilder builder(this->getEffect());

        builder.uniform("u_octaves")     = fOctaves;
        builder.uniform("u_persistence") = fPersistence;
        builder.uniform("u_evolution")   = fEvolution;
        builder.uniform("u_submatrix")   = std::array<float,9>{
            fSubMatrix.rc(0,0), fSubMatrix.rc(1,0), fSubMatrix.rc(2,0),
            fSubMatrix.rc(0,1), fSubMatrix.rc(1,1), fSubMatrix.rc(2,1),
            fSubMatrix.rc(0,2), fSubMatrix.rc(1,2), fSubMatrix.rc(2,2),
        };

        return builder.makeShader(&fMatrix, false);
    }

    SkRect onRevalidate(sksg::InvalidationController* ic, const SkMatrix& ctm) override {
        const auto& child = this->children()[0];
        const auto bounds = child->revalidate(nullptr, SkMatrix::I());

        fEffectShader = this->buildEffectShader();

        return bounds;
    }

    void onRender(SkCanvas* canvas, const RenderContext* ctx) const override {
        const auto& bounds = this->bounds();
        const auto local_ctx = ScopedRenderContext(canvas, ctx)
                .setIsolation(bounds, canvas->getTotalMatrix(), true);

        canvas->saveLayer(&bounds, nullptr);
        this->children()[0]->render(canvas, local_ctx);

        SkPaint effect_paint;
        effect_paint.setShader(fEffectShader);
        effect_paint.setBlendMode(SkBlendMode::kSrcIn);

        canvas->drawPaint(effect_paint);
    }

    const RenderNode* onNodeAt(const SkPoint&) const override { return nullptr; } // no hit-testing

    sk_sp<SkShader> fEffectShader;

    SkMatrix     fMatrix,
                 fSubMatrix;
    NoiseFilter  fFilter      = NoiseFilter::kNearest;
    NoiseFractal fFractal     = NoiseFractal::kBasic;
    float        fOctaves     = 1,
                 fPersistence = 1,
                 fEvolution   = 0;

    using INHERITED = sksg::CustomRenderNode;
};

class FractalNoiseAdapter final : public DiscardableAdapterBase<FractalNoiseAdapter,
                                                                FractalNoiseNode> {
public:
    FractalNoiseAdapter(const skjson::ArrayValue& jprops,
                        const AnimationBuilder* abuilder,
                        sk_sp<FractalNoiseNode> node)
        : INHERITED(std::move(node))
    {
        EffectBinder(jprops, *abuilder, this)
            .bind( 0, fFractalType     )
            .bind( 1, fNoiseType       )
            .bind( 2, fInvert          )
            .bind( 3, fContrast        )
            .bind( 4, fBrightness      )
             // 5 -- overflow
             // 6 -- transform begin-group
            .bind( 7, fRotation        )
            .bind( 8, fUniformScaling  )
            .bind( 9, fScale           )
            .bind(10, fScaleWidth      )
            .bind(11, fScaleHeight     )
            .bind(12, fOffset          )
             // 13 -- TODO: perspective offset
             // 14 -- transform end-group
            .bind(15, fComplexity      )
             // 16 -- sub settings begin-group
            .bind(17, fSubInfluence    )
            .bind(18, fSubScale        )
            .bind(19, fSubRotation     )
            .bind(20, fSubOffset       )
             // 21 -- center subscale
             // 22 -- sub settings end-group
            .bind(23, fEvolution       )
             // 24 -- evolution options begin-group
             // 25 -- cycle evolution
             // 26 -- cycle revolution
            .bind(27, fRandomSeed      )
             // 28 -- evolution options end-group
            .bind(29, fOpacity         );
            // 30 -- TODO: blending mode
    }

private:
    float evolution() const {
        // Constant chosen to visually match AE's evolution rate.
        const auto evo = SkDegreesToRadians(fEvolution) * 0.25f;

        // The random seed determines an arbitrary start plane.
        const auto base = SkRandom(static_cast<uint32_t>(fRandomSeed)).nextRangeU(0, 100);

        return evo + base;
    }

    SkMatrix shaderMatrix() const {
        static constexpr float kGridSize = 64;

        const auto scale = (SkScalarRoundToInt(fUniformScaling) == 1)
                ? SkV2{fScale, fScale}
                : SkV2{fScaleWidth, fScaleHeight};

        return SkMatrix::Translate(fOffset.x, fOffset.y)
             * SkMatrix::RotateDeg(fRotation)
             * SkMatrix::Scale(SkTPin(scale.x, 1.0f, 10000.0f) * 0.01f,
                               SkTPin(scale.y, 1.0f, 10000.0f) * 0.01f)
             * SkMatrix::Scale(kGridSize, kGridSize);
    }

    SkMatrix subMatrix() const {
        const auto scale = 100 / SkTPin(fSubScale, 10.0f, 10000.0f);

        return SkMatrix::Translate(-fSubOffset.x * 0.01f, -fSubOffset.y * 0.01f)
             * SkMatrix::RotateDeg(-fSubRotation)
             * SkMatrix::Scale(scale, scale);
    }

    NoiseFilter noiseFilter() const {
        switch (SkScalarRoundToInt(fNoiseType)) {
            case 1:  return NoiseFilter::kNearest;
            case 2:  return NoiseFilter::kLinear;
            default: return NoiseFilter::kSoftLinear;
        }
        SkUNREACHABLE;
    }

    NoiseFractal noiseFractal() const {
        switch (SkScalarRoundToInt(fFractalType)) {
            case 1:  return NoiseFractal::kBasic;
            case 3:  return NoiseFractal::kTurbulentSmooth;
            case 4:  return NoiseFractal::kTurbulentBasic;
            default: return NoiseFractal::kTurbulentSharp;
        }
        SkUNREACHABLE;
    }

    void onSync() override {
        const auto& n = this->node();

        n->setOctaves(SkTPin(fComplexity, 1.0f, 20.0f));
        n->setPersistence(SkTPin(fSubInfluence * 0.01f, 0.0f, 100.0f));
        n->setEvolution(this->evolution());
        n->setNoiseFilter(this->noiseFilter());
        n->setNoiseFractal(this->noiseFractal());
        n->setMatrix(this->shaderMatrix());
        n->setSubMatrix(this->subMatrix());
    }

    Vec2Value   fOffset           = {0,0},
                fSubOffset        = {0,0};

    ScalarValue fFractalType      =     0,
                fNoiseType        =     0,

                fRotation         =     0,
                fUniformScaling   =     0,
                fScale            =   100,  // used when uniform scaling is selected
                fScaleWidth       =   100,  // used when uniform scaling is not selected
                fScaleHeight      =   100,  // ^

                fComplexity       =     1,
                fSubInfluence     =   100,
                fSubScale         =    50,
                fSubRotation      =     0,

                fEvolution        =     0,
                fRandomSeed       =     0,

                fOpacity          =   100, // TODO
                fInvert           =     0, // TODO
                fContrast         =   100, // TODO
                fBrightness       =     0; // TODO

    using INHERITED = DiscardableAdapterBase<FractalNoiseAdapter, FractalNoiseNode>;
};

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachFractalNoiseEffect(
        const skjson::ArrayValue& jprops, sk_sp<sksg::RenderNode> layer) const {
    auto fractal_noise = sk_make_sp<FractalNoiseNode>(std::move(layer));

    return fBuilder->attachDiscardableAdapter<FractalNoiseAdapter>(jprops, fBuilder,
                                                                   std::move(fractal_noise));
}

} // namespace skottie::internal
