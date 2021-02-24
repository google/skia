/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/effects/SkRuntimeEffect.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGRenderNode.h"

namespace skottie::internal {
namespace {

static constexpr char gNoiseSkSL[] = R"(
    uniform half3x3 u_submatrix;
    uniform half    u_randseed,
                    u_evolution,
                    u_complexity,
                    u_subweight;

    half hash(half3 v) {
        v  = fract(v*0.1031);
        v += dot(v, v.zxy + 31.32);
        return fract((v.x + v.y)*v.z);
    }

    half sample_noise(half2 xy) {
        xy = floor(xy);

        half e = floor(u_evolution) + u_randseed,
             a = hash(half3(xy, e + 0)),
             b = hash(half3(xy, e + 1));

        return mix(a, b, fract(u_evolution));
    }

    // filter_noise()
    %s

    half compute_noise(half2 xy) {
        half n = filter_noise(xy);

        // TODO: fractal function
        return n;
    }

    half4 main(float2 xy) {
        half c = u_complexity,
             w = 1,
         w_acc = 0,
             n = 0;

        // ${OCTAVES}
        for (half z = 0; z < %u; z += 1) {
            half ww = w*saturate(c);
            n += compute_noise(xy) * ww;

            c -= 1;
            w *= u_subweight;
            w_acc += ww;

            xy = (u_submatrix*half3(xy,1)).xy;
        }

        n /= w_acc;

        return half4(n,n,n,1);
    }
)";

static constexpr char gFilterNoneSkSL[] = R"(
    half filter_noise(half2 xy) {
        return sample_noise(xy);
    }
)";

static constexpr char gFilterLinearSkSL[] = R"(
    half filter_noise(half2 xy) {
        xy -= 0.5;

        half tl = sample_noise(xy + half2(0,0)),
             tr = sample_noise(xy + half2(1,0)),
             bl = sample_noise(xy + half2(0,1)),
             br = sample_noise(xy + half2(1,1));

        half2 t = fract(xy);

        return mix(mix(tl, tr, t.x), mix(bl, br, t.x), t.y);
    }
)";

static constexpr char gFilterSoftLinearSkSL[] = R"(
    half filter_noise(half2 xy) {
        xy -= 0.5;

        half tl = sample_noise(xy + half2(0,0)),
             tr = sample_noise(xy + half2(1,0)),
             bl = sample_noise(xy + half2(0,1)),
             br = sample_noise(xy + half2(1,1));

        half2 t = smoothstep(0, 1, fract(xy));

        return mix(mix(tl, tr, t.x), mix(bl, br, t.x), t.y);
    }
)";

enum class NoiseInterpolation {
    kNone,
    kLinear,
    kSoftLinear,
    // TODO: kSpline
};

template <NoiseInterpolation NI, unsigned OCTAVES>
sk_sp<SkRuntimeEffect> make_effect() {
    static constexpr char const* gFilters[] = {
        gFilterNoneSkSL,
        gFilterLinearSkSL,
        gFilterSoftLinearSkSL
    };
    const auto sksl_prog = SkStringPrintf(gNoiseSkSL, gFilters[static_cast<size_t>(NI)], OCTAVES);

    auto result = SkRuntimeEffect::Make(sksl_prog, {});

    if (1 && !result.effect) {
        printf("!!! %s\n", result.errorText.c_str());
    }

    return std::move(result.effect);
}

template <NoiseInterpolation NI, unsigned OCTAVES>
sk_sp<SkRuntimeEffect> effect() {
    static const SkRuntimeEffect* effect = make_effect<NI, OCTAVES>().release();
    SkASSERT(effect);
    return sk_ref_sp(effect);
}

class FractalNoiseNode final : public sksg::CustomRenderNode {
public:
    explicit FractalNoiseNode(sk_sp<RenderNode> child) : INHERITED({std::move(child)}) {}

    SG_ATTRIBUTE(Matrix            , SkMatrix          , fMatrix            )
    SG_ATTRIBUTE(SubMatrix         , SkMatrix          , fSubMatrix         )

    SG_ATTRIBUTE(NoiseInterpolation, NoiseInterpolation, fNoiseInterpolation)
    SG_ATTRIBUTE(Complexity        , float             , fComplexity        )
    SG_ATTRIBUTE(SubWeight         , float             , fSubWeight         )
    SG_ATTRIBUTE(Evolution         , float             , fEvolution         )
    SG_ATTRIBUTE(RandomSeed        , float             , fRandomSeed        )

private:
    template <NoiseInterpolation NI>
    sk_sp<SkRuntimeEffect> getEffectOctaves() const {
        if (fComplexity <= 1) {
            return effect<NI, 1>();
        } else if (fComplexity <= 4) {
            return effect<NI, 4>();
        } else if (fComplexity <= 8) {
            return effect<NI, 8>();
        } else {
            return effect<NI, 20>();
        }
    }

    sk_sp<SkRuntimeEffect> getEffect() const {
        switch (fNoiseInterpolation) {
            case NoiseInterpolation::kNone:
                return this->getEffectOctaves<NoiseInterpolation::kNone>();
            case NoiseInterpolation::kLinear:
                return this->getEffectOctaves<NoiseInterpolation::kLinear>();
            case NoiseInterpolation::kSoftLinear:
                return this->getEffectOctaves<NoiseInterpolation::kSoftLinear>();
        }
        SkUNREACHABLE;
    }

    sk_sp<SkShader> buildEffectShader() const {
        SkRuntimeShaderBuilder builder(this->getEffect());

        builder.uniform("u_complexity") = fComplexity;
        builder.uniform("u_subweight")  = fSubWeight;
        builder.uniform("u_evolution")  = fEvolution;
        builder.uniform("u_randseed")   = fRandomSeed;
        builder.uniform("u_submatrix")  = std::array<float,9>{
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

    SkMatrix           fMatrix,
                       fSubMatrix;
    NoiseInterpolation fNoiseInterpolation = NoiseInterpolation::kNone;
    float              fComplexity         = 1,
                       fSubWeight          = 1,
                       fEvolution          = 0,
                       fRandomSeed         = 0;

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
             // 5 -- TODO: overflow
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

    NoiseInterpolation noiseInterpolation() const {
        switch (SkScalarRoundToInt(fNoiseType)) {
            case 1:  return NoiseInterpolation::kNone;
            case 2:  return NoiseInterpolation::kLinear;
            default: return NoiseInterpolation::kSoftLinear;
        }
        SkUNREACHABLE;
    }

    void onSync() override {
        const auto& n = this->node();

        n->setNoiseInterpolation(this->noiseInterpolation());
        n->setComplexity(SkTPin(fComplexity, 1.0f, 20.0f));
        n->setSubWeight(SkTPin(fSubInfluence * 0.01f, 0.0f, 100.0f));
        n->setEvolution(SkDegreesToRadians(fEvolution)/4);
        n->setRandomSeed(SkTPin(fRandomSeed, 0.0f, 100000.0f));

        n->setMatrix(this->shaderMatrix());
        n->setSubMatrix(
                        SkMatrix::RotateDeg(-fSubRotation) *
                        SkMatrix::Translate(-fSubOffset.x * 0.01f, -fSubOffset.y * 0.01f) *
                        SkMatrix::Scale(100 / fSubScale, 100 / fSubScale));
    }

    ScalarValue fFractalType      =     0,
                fNoiseType        =     0,
                fInvert           =     0,
                fContrast         =   100,
                fBrightness       =     0;

    Vec2Value   fOffset           = {0,0};
    ScalarValue fRotation         =     0,
                fUniformScaling   =     0,
                fScale            =   100,
                fScaleWidth       =   100,
                fScaleHeight      =   100,

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
