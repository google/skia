/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/core/SkPictureRecorder.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkNx.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGPaint.h"
#include "modules/sksg/include/SkSGRenderNode.h"
#include "src/utils/SkJSON.h"

namespace skottie::internal {

#ifdef SK_ENABLE_SKSL

namespace {

static constexpr char gBulgeDisplacementSkSL[] =
    "uniform shader u_layer;"

    "uniform float2 u_center;"
    "uniform float2 u_radius;"
    "uniform float  u_h;"
    "uniform float  u_r;"
    "uniform float  u_asinInverseR;"
    "uniform float  u_rcpR;"
    "uniform float  u_rcpAsinInvR;"
    "uniform float  u_selector;"

    // AE's bulge effect appears to be a combination of spherical displacement and
    // exponential displacement along the radius.
    // To simplify the math, we pre scale/translate such that the ellipse becomes a
    // circle with radius == 1, centered on origin.
    "float2 displace_sph(float2 v) {"
        "float arc_ratio = asin(length(v)*u_rcpR)*u_rcpAsinInvR;"
        "return normalize(v)*arc_ratio - v;"
    "}"

    "float2 displace_exp(float2 v) {"
        "return v*pow(dot(v,v),u_h) - v;"
    "}"

    "half2 displace(float2 v) {"
        "float t = dot(v, v);"
        "if (t >= 1) {"
            "return v;"
        "}"
        "float2 d = displace_sph(v) + displace_exp(v);"
        "return v + (d * u_selector);"
    "}"

    "half4 main(float2 xy) {"
        "xy = displace(xy);"
        "xy = xy*u_radius + u_center;"
        "return u_layer.eval(xy);"
    "}";

static sk_sp<SkRuntimeEffect> bulge_effect() {
    static const SkRuntimeEffect* effect =
        SkRuntimeEffect::MakeForShader(SkString(gBulgeDisplacementSkSL), {}).effect.release();
    SkASSERT(effect);

    return sk_ref_sp(effect);
}

class BulgeNode final : public sksg::CustomRenderNode {
public:
    explicit BulgeNode(sk_sp<RenderNode> child, const SkSize& child_size)
        : INHERITED({std::move(child)})
        , fChildSize(child_size) {}

    SG_ATTRIBUTE(Center  , SkPoint   , fCenter)
    SG_ATTRIBUTE(Radius  , SkVector  , fRadius)
    SG_ATTRIBUTE(Height  , float     , fHeight)

private:
    sk_sp<SkShader> contentShader() {
        if (!fContentShader || this->hasChildrenInval()) {
            const auto& child = this->children()[0];
            child->revalidate(nullptr, SkMatrix::I());

            SkPictureRecorder recorder;
            child->render(recorder.beginRecording(SkRect::MakeSize(fChildSize)));

            fContentShader = recorder.finishRecordingAsPicture()
                    ->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, SkFilterMode::kLinear,
                                 nullptr, nullptr);
        }

        return fContentShader;
    }

    sk_sp<SkShader> buildEffectShader() {
        if (fHeight == 0) {
            return nullptr;
        }

        SkRuntimeShaderBuilder builder(bulge_effect());
        float adjHeight = std::abs(fHeight)/4;
        float r = (1 + adjHeight)/2/sqrt(adjHeight);
        float h = std::pow(adjHeight, 3)*1.3;
        builder.uniform("u_center")       = fCenter;
        builder.uniform("u_radius")       = fRadius;
        builder.uniform("u_h")            = h;
        builder.uniform("u_r")            = r;
        builder.uniform("u_asinInverseR") = std::asin(1/r);
        builder.uniform("u_rcpR")            = 1.0f/r;
        builder.uniform("u_rcpAsinInvR") = 1.0f/std::asin(1/r);
        builder.uniform("u_selector")     = (fHeight > 0 ? 1.0f : -1.0f);

        builder.child("u_layer") = this->contentShader();

        const auto lm = SkMatrix::Translate(fCenter.x(), fCenter.y())
                      * SkMatrix::Scale(fRadius.x(), fRadius.y());
        return builder.makeShader(&lm, false);
    }

    SkRect onRevalidate(sksg::InvalidationController* ic, const SkMatrix& ctm) override {
        const auto& child = this->children()[0];
        fEffectShader = buildEffectShader();
        return child->revalidate(ic, ctm);
    }

    void onRender(SkCanvas* canvas, const RenderContext* ctx) const override {
        if (fHeight == 0) {
            this->children()[0]->render(canvas, ctx);
            return;
        }
        const auto& bounds = this->bounds();
        const auto local_ctx = ScopedRenderContext(canvas, ctx)
                .setIsolation(bounds, canvas->getTotalMatrix(), true);

        canvas->saveLayer(&bounds, nullptr);

        SkPaint effect_paint;
        effect_paint.setShader(fEffectShader);
        effect_paint.setBlendMode(SkBlendMode::kSrcOver);

        canvas->drawPaint(effect_paint);
    }

    const RenderNode* onNodeAt(const SkPoint&) const override { return nullptr; } // no hit-testing

    sk_sp<SkShader> fEffectShader;
    sk_sp<SkShader> fContentShader;
    const SkSize fChildSize;

    SkPoint  fCenter = {0,0};
    SkVector fRadius = {0,0};
    float   fHeight = 0;

    using INHERITED = sksg::CustomRenderNode;
};

class BulgeEffectAdapter final : public DiscardableAdapterBase<BulgeEffectAdapter,
                                                             BulgeNode> {
public:
    BulgeEffectAdapter(const skjson::ArrayValue& jprops,
                      const AnimationBuilder& abuilder,
                      sk_sp<BulgeNode> node)
        : INHERITED(std::move(node))
    {
        enum : size_t {
            kHorizontalRadius_Index = 0,
            kVerticalRadius_Index = 1,
            kBulgeCenter_Index = 2,
            kBulgeHeight_Index = 3,
            // kTaper_Index = 4,
            // kAA_Index = 5,
            // kPinning_Index = 6,
        };
        EffectBinder(jprops, abuilder, this).bind(kHorizontalRadius_Index, fHorizontalRadius)
                                            .bind(kVerticalRadius_Index, fVerticalRadius)
                                            .bind(kBulgeCenter_Index, fCenter)
                                            .bind(kBulgeHeight_Index, fBulgeHeight);
    }

private:
    void onSync() override {
        // pre-shader math
        auto n = this->node();
        n->setCenter({fCenter.x, fCenter.y});
        n->setRadius({fHorizontalRadius, fVerticalRadius});
        n->setHeight(fBulgeHeight);
    }

    Vec2Value fCenter;
    ScalarValue fHorizontalRadius,
                fVerticalRadius,
                fBulgeHeight;
    using INHERITED = DiscardableAdapterBase<BulgeEffectAdapter, BulgeNode>;
};

} // namespace

#endif  // SK_ENABLE_SKSL

sk_sp<sksg::RenderNode> EffectBuilder::attachBulgeEffect(const skjson::ArrayValue& jprops,
                                                             sk_sp<sksg::RenderNode> layer) const {
#ifdef SK_ENABLE_SKSL
    auto shaderNode = sk_make_sp<BulgeNode>(std::move(layer), fLayerSize);
    return fBuilder->attachDiscardableAdapter<BulgeEffectAdapter>(jprops, *fBuilder, std::move(shaderNode));
#else
    return layer;
#endif
}

} // namespace skottie::internal
