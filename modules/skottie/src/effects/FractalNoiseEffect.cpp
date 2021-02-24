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

// Implementation:
//
//   - SkSL noise generator produces a modulated (weighted) noise layer
//   - the layer transform (effect transform * sublayer matrix) is pushed as a local matrix
//   - layers are added together using Blend(kPlus) for the final result
//   - we instantiate one noise runtime effect per noise type (filter)
//
// TODO:
//   - Fractal Type
//   - Invert
//   - Contrast/Brightness

// Generate a noise layer for the given evolution (depth) and amplitude.
static constexpr char gNoiseLayerSkSL[] =
    "uniform half u_evo,"
                 "u_amp;"

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

        "half e_ = floor(u_evo),"
             "t  = u_evo - e_,"
             "n0 = hash(half3(xy, e_ + 0)),"
             "n1 = hash(half3(xy, e_ + 1));"

        // Note: Ideally we would use 4 samples (-1, 0, 1, 2) and cubic interpolation for
        //       better results -- but that's significantly more expensive than lerp.

        "return mix(n0, n1, t);"
    "}"

    // filter() placeholder
    "%s"

    "half4 main(vec2 xy) {"
        "half n = filter(xy);"

        // TODO: fractal functions

        "return u_amp*half4(n,n,n,1);"
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

enum class NoiseFilter {
    kNearest,
    kLinear,
    kSoftLinear,
    // TODO: kSpline?
};

sk_sp<SkRuntimeEffect> make_noise_layer_effect(const char* filter) {
    auto result = SkRuntimeEffect::Make(SkStringPrintf(gNoiseLayerSkSL, filter), {});

    if (1 && !result.effect) {
        printf("!!! %s\n", result.errorText.c_str());
    }

    return std::move(result.effect);
}

template <NoiseFilter F>
sk_sp<SkRuntimeEffect> noise_layer() {
    static constexpr char const* gFilters[] = {
        gFilterNearestSkSL,
        gFilterLinearSkSL,
        gFilterSoftLinearSkSL
    };

    static const SkRuntimeEffect* effect =
            make_noise_layer_effect(gFilters[static_cast<size_t>(F)]).release();

    SkASSERT(effect);
    return sk_ref_sp(effect);
}

class FractalNoiseNode final : public sksg::CustomRenderNode {
public:
    explicit FractalNoiseNode(sk_sp<RenderNode> child) : INHERITED({std::move(child)}) {}

    SG_ATTRIBUTE(Matrix      , SkMatrix   , fMatrix     )
    SG_ATTRIBUTE(SubMatrix   , SkMatrix   , fSubMatrix  )

    SG_ATTRIBUTE(NoiseFilter , NoiseFilter, fFilter     )
    SG_ATTRIBUTE(Octaves     , float      , fOctaves    )
    SG_ATTRIBUTE(Persistence , float      , fPersistence)
    SG_ATTRIBUTE(Evolution   , float      , fEvolution  )

private:
    sk_sp<SkRuntimeEffect> getEffect() const {
        switch (fFilter) {
            case NoiseFilter::kNearest   : return noise_layer<NoiseFilter::kNearest>();
            case NoiseFilter::kLinear    : return noise_layer<NoiseFilter::kLinear>();
            case NoiseFilter::kSoftLinear: return noise_layer<NoiseFilter::kSoftLinear>();
        }
        SkUNREACHABLE;
    }

    sk_sp<SkShader> buildEffectShader() const {
        SkASSERT(fOctaves >= 1);
        const auto octaves = std::ceil(fOctaves);

        // For p == persistence and a given octave K, the layer amplitude is
        //
        //   amp = p^(K-1)
        //
        // For N octaves, we compute the total amplitude summation
        //
        //   1 + p^1 + p^2 + ... p^(N-1) == (p^N - 1)/(p - 1)
        //
        auto total_amp = SkScalarNearlyEqual(fPersistence, 1)
                ? octaves
                : (std::pow(fPersistence, octaves) - 1) / (fPersistence - 1);

        // The last octave can be fractional -- adjust for the fractional part.
        total_amp -= (octaves - fOctaves)*std::pow(fPersistence, octaves - 1);

        sk_sp<SkShader> noise_shader;
        float amp = 1;
        SkMatrix layer_matrix = fMatrix;

        for (auto o = fOctaves; o > 0; o -= 1) {
            const auto layer_amp = amp * std::min(o, 1.0f) / total_amp;

            SkRuntimeShaderBuilder builder(this->getEffect());
            builder.uniform("u_evo") = fEvolution;
            builder.uniform("u_amp") = layer_amp;
            auto layer = builder.makeShader(&layer_matrix, false);

            noise_shader = noise_shader
                ? SkShaders::Blend(SkBlendMode::kPlus, std::move(noise_shader), std::move(layer))
                : std::move(layer);

            amp *= fPersistence;
            layer_matrix = layer_matrix * fSubMatrix;
        }

        return noise_shader;
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

    SkMatrix    fMatrix,
                fSubMatrix;
    NoiseFilter fFilter      = NoiseFilter::kNearest;
    float       fOctaves     = 1,
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
            .bind(21, fCenterSubscale  )
             // 22 -- sub settings end-group
            .bind(23, fEvolution       )
             // 24 -- evolution options begin-group
            .bind(25, fCycleEvolution  )
            .bind(26, fCycleRevolutions)
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
        const auto scale = SkTPin(fSubScale, 10.0f, 10000.0f) * 0.01f;

        return SkMatrix::Translate(fSubOffset.x * 0.01f, fSubOffset.y * 0.01f)
             * SkMatrix::RotateDeg(fSubRotation)
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

    void onSync() override {
        const auto& n = this->node();

        n->setOctaves(SkTPin(fComplexity, 1.0f, 20.0f));
        n->setPersistence(SkTPin(fSubInfluence * 0.01f, 0.0f, 100.0f));
        n->setEvolution(this->evolution());
        n->setNoiseFilter(this->noiseFilter());
        n->setMatrix(this->shaderMatrix());
        n->setSubMatrix(this->subMatrix());
    }

    ScalarValue fFractalType      =     0,
                fNoiseType        =     0,
                fInvert           =     0,
                fContrast         =   100,
                fBrightness       =     0;

    Vec2Value   fOffset           = {0,0};
    ScalarValue fRotation         =     0,
                fUniformScaling   =     0,
                fScale            =   100,  // - used when uniform scaling is selected
                fScaleWidth       =   100,  // - used when uniform scaling is not selected
                fScaleHeight      =   100,  // /

                fComplexity       =     1,
                fSubInfluence     =   100,
                fSubScale         =    50,
                fSubRotation      =     0;
    Vec2Value   fSubOffset        = {0,0};
    ScalarValue fCenterSubscale   =     0,

                fEvolution        =     0,
                fCycleEvolution   =     1,
                fCycleRevolutions =     0,
                fRandomSeed       =     0,
                fOpacity          =   100;

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
