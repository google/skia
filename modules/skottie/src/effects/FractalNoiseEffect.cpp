/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkTPin.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/effects/Effects.h"
#include "modules/sksg/include/SkSGNode.h"
#include "modules/sksg/include/SkSGRenderNode.h"
#include "src/base/SkRandom.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <utility>
#include <vector>

struct SkPoint;

namespace skjson {
class ArrayValue;
}
namespace sksg {
class InvalidationController;
}

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
    "uniform float3x3 u_submatrix;" // sublayer transform

    "uniform float2 u_noise_planes;" // noise planes computed based on evolution params
    "uniform float  u_noise_weight," // noise planes lerp weight
                   "u_octaves,"      // number of octaves (can be fractional)
                   "u_persistence;"  // relative octave weight

    // Hash based on hash13 (https://www.shadertoy.com/view/4djSRW).
    "float hash(float3 v) {"
        "v  = fract(v*0.1031);"
        "v += dot(v, v.zxy + 31.32);"
        "return fract((v.x + v.y)*v.z);"
    "}"

    // The general idea is to compute a coherent hash for two planes in discretized (x,y,e) space,
    // and interpolate between them.  This yields gradual changes when animating |e| - which is the
    // desired outcome.
    "float sample_noise(float2 xy) {"
        "xy = floor(xy);"

        "float n0  = hash(float3(xy, u_noise_planes.x)),"
              "n1  = hash(float3(xy, u_noise_planes.y));"

        // Note: Ideally we would use 4 samples (-1, 0, 1, 2) and cubic interpolation for
        //       better results -- but that's significantly more expensive than lerp.

        "return mix(n0, n1, u_noise_weight);"
    "}"

    // filter() placeholder
    "%s"

    // fractal() placeholder
    "%s"

    // Generate ceil(u_octaves) noise layers and combine based on persistentce and sublayer xform.
    "float4 main(vec2 xy) {"
        "float oct = u_octaves," // initial octave count (this is the effective loop counter)
              "amp = 1,"         // initial layer amplitude
             "wacc = 0,"         // weight accumulator
                "n = 0;"         // noise accumulator

        // Constant loop counter chosen to be >= ceil(u_octaves).
        // The logical counter is actually 'oct'.
        "for (float i = 0; i < %u; ++i) {"
            // effective layer weight
            //   -- for full octaves:              layer amplitude
            //   -- for fractional octave:         layer amplitude modulated by fractional part
            "float w = amp*min(oct,1.0);"

            "n    += w*fractal(filter(xy));"
            "wacc += w;"

            "if (oct <= 1.0) { break; }"

            "oct -= 1.0;"
            "amp *= u_persistence;"
            "xy   = (u_submatrix*float3(xy,1)).xy;"
        "}"

        "n /= wacc;"

        // TODO: fractal functions

        "return float4(n,n,n,1);"
    "}";

static constexpr char gFilterNearestSkSL[] =
    "float filter(float2 xy) {"
        "return sample_noise(xy);"
    "}";

static constexpr char gFilterLinearSkSL[] =
    "float filter(float2 xy) {"
        "xy -= 0.5;"

        "float n00 = sample_noise(xy + float2(0,0)),"
              "n10 = sample_noise(xy + float2(1,0)),"
              "n01 = sample_noise(xy + float2(0,1)),"
              "n11 = sample_noise(xy + float2(1,1));"

        "float2 t = fract(xy);"

        "return mix(mix(n00, n10, t.x), mix(n01, n11, t.x), t.y);"
    "}";

static constexpr char gFilterSoftLinearSkSL[] =
    "float filter(float2 xy) {"
        "xy -= 0.5;"

        "float n00 = sample_noise(xy + float2(0,0)),"
              "n10 = sample_noise(xy + float2(1,0)),"
              "n01 = sample_noise(xy + float2(0,1)),"
              "n11 = sample_noise(xy + float2(1,1));"

        "float2 t = smoothstep(0, 1, fract(xy));"

        "return mix(mix(n00, n10, t.x), mix(n01, n11, t.x), t.y);"
    "}";

static constexpr char gFractalBasicSkSL[] =
    "float fractal(float n) {"
        "return n;"
    "}";

static constexpr char gFractalTurbulentBasicSkSL[] =
    "float fractal(float n) {"
        "return 2*abs(0.5 - n);"
    "}";

static constexpr char gFractalTurbulentSmoothSkSL[] =
    "float fractal(float n) {"
        "n = 2*abs(0.5 - n);"
        "return n*n;"
    "}";

static constexpr char gFractalTurbulentSharpSkSL[] =
    "float fractal(float n) {"
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
    auto result = SkRuntimeEffect::MakeForShader(
            SkStringPrintf(gNoiseEffectSkSL, filter, fractal, loops), {});

    return std::move(result.effect);
}

sk_sp<SkRuntimeEffect> noise_effect(float octaves, NoiseFilter filter, NoiseFractal fractal) {
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

    SkASSERT(static_cast<size_t>(filter)  < std::size(gFilters));
    SkASSERT(static_cast<size_t>(fractal) < std::size(gFractals));

    // Bin the loop counter based on the number of octaves (range: [1..20]).
    // Low complexities are common, so we maximize resolution for the low end.
    struct BinInfo {
        float    threshold;
        unsigned loops;
    };
    static constexpr BinInfo kLoopBins[] = {
        { 8, 20 },
        { 4,  8 },
        { 3,  4 },
        { 2,  3 },
        { 1,  2 },
        { 0,  1 }
    };

    auto bin_index = [](float octaves) {
        SkASSERT(octaves > kLoopBins[std::size(kLoopBins) - 1].threshold);

        for (size_t i = 0; i < std::size(kLoopBins); ++i) {
            if (octaves > kLoopBins[i].threshold) {
                return i;
            }
        }
        SkUNREACHABLE;
    };

    static SkRuntimeEffect* kEffectCache[std::size(kLoopBins)]
                                        [std::size(gFilters)]
                                        [std::size(gFractals)];

    const size_t bin = bin_index(octaves);

    auto& effect = kEffectCache[bin]
                               [static_cast<size_t>(filter)]
                               [static_cast<size_t>(fractal)];
    if (!effect) {
        effect = make_noise_effect(kLoopBins[bin].loops,
                                   gFilters[static_cast<size_t>(filter)],
                                   gFractals[static_cast<size_t>(fractal)])
                 .release();
    }

    SkASSERT(effect);
    return sk_ref_sp(effect);
}

class FractalNoiseNode final : public sksg::CustomRenderNode {
public:
    explicit FractalNoiseNode(sk_sp<RenderNode> child) : INHERITED({std::move(child)}) {}

    SG_ATTRIBUTE(Matrix         , SkMatrix    , fMatrix         )
    SG_ATTRIBUTE(SubMatrix      , SkMatrix    , fSubMatrix      )

    SG_ATTRIBUTE(NoiseFilter    , NoiseFilter , fFilter         )
    SG_ATTRIBUTE(NoiseFractal   , NoiseFractal, fFractal        )
    SG_ATTRIBUTE(NoisePlanes    , SkV2        , fNoisePlanes    )
    SG_ATTRIBUTE(NoiseWeight    , float       , fNoiseWeight    )
    SG_ATTRIBUTE(Octaves        , float       , fOctaves        )
    SG_ATTRIBUTE(Persistence    , float       , fPersistence    )

private:
    sk_sp<SkRuntimeEffect> getEffect(NoiseFilter filter) const {
        switch (fFractal) {
            case NoiseFractal::kBasic:
                return noise_effect(fOctaves, filter, NoiseFractal::kBasic);
            case NoiseFractal::kTurbulentBasic:
                return noise_effect(fOctaves, filter, NoiseFractal::kTurbulentBasic);
            case NoiseFractal::kTurbulentSmooth:
                return noise_effect(fOctaves, filter, NoiseFractal::kTurbulentSmooth);
            case NoiseFractal::kTurbulentSharp:
                return noise_effect(fOctaves, filter, NoiseFractal::kTurbulentSharp);
        }
        SkUNREACHABLE;
    }

    sk_sp<SkRuntimeEffect> getEffect() const {
        switch (fFilter) {
            case NoiseFilter::kNearest   : return this->getEffect(NoiseFilter::kNearest);
            case NoiseFilter::kLinear    : return this->getEffect(NoiseFilter::kLinear);
            case NoiseFilter::kSoftLinear: return this->getEffect(NoiseFilter::kSoftLinear);
        }
        SkUNREACHABLE;
    }

    sk_sp<SkShader> buildEffectShader() const {
        SkRuntimeShaderBuilder builder(this->getEffect());

        builder.uniform("u_noise_planes") = fNoisePlanes;
        builder.uniform("u_noise_weight") = fNoiseWeight;
        builder.uniform("u_octaves"     ) = fOctaves;
        builder.uniform("u_persistence" ) = fPersistence;
        builder.uniform("u_submatrix"   ) = std::array<float,9>{
            fSubMatrix.rc(0,0), fSubMatrix.rc(1,0), fSubMatrix.rc(2,0),
            fSubMatrix.rc(0,1), fSubMatrix.rc(1,1), fSubMatrix.rc(2,1),
            fSubMatrix.rc(0,2), fSubMatrix.rc(1,2), fSubMatrix.rc(2,2),
        };

        return builder.makeShader(&fMatrix);
    }

    SkRect onRevalidate(sksg::InvalidationController* ic, const SkMatrix& ctm) override {
        const auto& child = this->children()[0];
        const auto bounds = child->revalidate(ic, ctm);

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
    NoiseFilter  fFilter          = NoiseFilter::kNearest;
    NoiseFractal fFractal         = NoiseFractal::kBasic;
    SkV2         fNoisePlanes     = {0,0};
    float        fNoiseWeight     = 0,
                 fOctaves         = 1,
                 fPersistence     = 1;

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
            .bind(25, fCycleEvolution  )
            .bind(26, fCycleRevolutions)
            .bind(27, fRandomSeed      )
             // 28 -- evolution options end-group
            .bind(29, fOpacity         );
            // 30 -- TODO: blending mode
    }

private:
    std::tuple<SkV2, float> noise() const {
        // Constant chosen to visually match AE's evolution rate.
        static constexpr auto kEvolutionScale = 0.25f;

        // Evolution inputs:
        //
        //   * evolution         - main evolution control (degrees)
        //   * cycle evolution   - flag controlling whether evolution cycles
        //   * cycle revolutions - number of revolutions after which evolution cycles (period)
        //   * random seed       - determines an arbitrary starting plane (evolution offset)
        //
        // The shader uses evolution floor/ceil to select two noise planes, and the fractional part
        // to interpolate between the two -> in order to wrap around smoothly, the cycle/period
        // must be integral.
        const float
            evo_rad = SkDegreesToRadians(fEvolution),
            rev_rad = std::max(fCycleRevolutions, 1.0f)*SK_FloatPI*2,
            cycle   = fCycleEvolution
                          ? SkScalarRoundToScalar(rev_rad*kEvolutionScale)
                          : SK_ScalarMax,
            // Adjust scale when cycling to ensure an integral period (post scaling).
            scale   = fCycleEvolution
                          ? cycle/rev_rad
                          : kEvolutionScale,
            offset  = SkRandom(static_cast<uint32_t>(fRandomSeed)).nextRangeU(0, 100),
            evo     = evo_rad*scale,
            evo_    = std::floor(evo),
            weight  = evo - evo_;

        // We want the GLSL mod() flavor.
        auto glsl_mod = [](float x, float y) {
            return x - y*std::floor(x/y);
        };

        const SkV2 noise_planes = {
            glsl_mod(evo_ + 0, cycle) + offset,
            glsl_mod(evo_ + 1, cycle) + offset,
        };

        return std::make_tuple(noise_planes, weight);
    }

    SkMatrix shaderMatrix() const {
        static constexpr float kGridSize = 64;

        const auto scale = (SkScalarRoundToInt(fUniformScaling) == 1)
                ? SkV2{fScale, fScale}
                : SkV2{fScaleWidth, fScaleHeight};

        return SkMatrix::Translate(fOffset.x, fOffset.y)
             * SkMatrix::Scale(SkTPin(scale.x, 1.0f, 10000.0f) * 0.01f,
                               SkTPin(scale.y, 1.0f, 10000.0f) * 0.01f)
             * SkMatrix::RotateDeg(fRotation)
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

        const auto [noise_planes, noise_weight] = this->noise();

        n->setOctaves(SkTPin(fComplexity, 1.0f, 20.0f));
        n->setPersistence(SkTPin(fSubInfluence * 0.01f, 0.0f, 100.0f));
        n->setNoisePlanes(noise_planes);
        n->setNoiseWeight(noise_weight);
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
                fCycleEvolution   =     0,
                fCycleRevolutions =     0,
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
