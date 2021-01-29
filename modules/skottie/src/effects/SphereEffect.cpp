/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/core/SkM44.h"
#include "include/core/SkPictureRecorder.h"
#include "include/effects/SkRuntimeEffect.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGRenderNode.h"

namespace skottie::internal {

namespace  {

static constexpr char gSphereSkSL[] = R"(
    uniform shader child;
    uniform half2  scale;

    uniform half4x4 mat;

    half4 main(float2 xy) {
        // Position on sphere.
        // TODO: account for perspective
        half x = xy.x;
        half y = xy.y;
        half z = sqrt(max(0, 1 - x*x - y*y));

        // Rotated normal.
        half3 n = (mat*half4(x,y,z,1)).xyz;

        half PI = 3.1415927;

        // UV mapping (https://en.wikipedia.org/wiki/UV_mapping)
        half2 uv = half2(
            0.5 + atan(n.x, n.z) / (2*PI),
            0.5 - asin(n.y) / PI
        );

        return sample(child, uv*scale);
    }
)";

static sk_sp<SkRuntimeEffect> sphere_effect_singleton() {
    static const SkRuntimeEffect* effect =
            std::get<0>(SkRuntimeEffect::Make(SkString(gSphereSkSL))).release();
    if (1 && !effect) {
        auto err = std::get<1>(SkRuntimeEffect::Make(SkString(gSphereSkSL)));
        printf("!!! %s\n", err.c_str());
    }
    SkASSERT(effect);

    return sk_ref_sp(effect);
}

class SphereNode final : public sksg::CustomRenderNode {
public:
    SphereNode(sk_sp<RenderNode> child, const SkSize& child_size)
        : INHERITED({std::move(child)})
        , fChildSize(child_size) {}

    SG_ATTRIBUTE(Center      , SkPoint, fCenter      )
    SG_ATTRIBUTE(Radius      , float  , fRadius      )
    SG_ATTRIBUTE(RotateMatrix, SkM44  , fRotateMatrix)

private:
    sk_sp<SkShader> buildEffectShader() const {
        SkPictureRecorder recorder;
        this->children()[0]->render(recorder.beginRecording(SkRect::MakeSize(fChildSize)));
        auto content_shader = recorder.finishRecordingAsPicture()
                ->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, SkFilterMode::kLinear,
                             nullptr, nullptr);

        SkRuntimeShaderBuilder builder(sphere_effect_singleton());

        builder.child("child") = std::move(content_shader);

        builder.uniform("scale") = fChildSize;
        builder.uniform("mat") = fRotateMatrix;

        const auto lm = SkMatrix::Translate(fCenter.fX, fCenter.fY) *
                        SkMatrix::Scale(fRadius, fRadius);

        return builder.makeShader(&lm, false);
    }

    SkRect onRevalidate(sksg::InvalidationController* ic, const SkMatrix& ctm) override {
        // Child damage rect is ignored.
        this->children()[0]->revalidate(ic, ctm);

        fEffectShader = this->buildEffectShader();

        return SkRect::MakeLTRB(fCenter.fX - fRadius,
                                fCenter.fY - fRadius,
                                fCenter.fX + fRadius,
                                fCenter.fY + fRadius);
    }

    void onRender(SkCanvas* canvas, const RenderContext* ctx) const override {
        if (fRadius <= 0) {
            return;
        }

        SkPaint sphere_paint;
        sphere_paint.setAntiAlias(true);
        sphere_paint.setShader(fEffectShader);

        canvas->drawCircle(fCenter, fRadius, sphere_paint);
    }

    const RenderNode* onNodeAt(const SkPoint&) const override { return nullptr; } // no hit-testing

    const SkSize fChildSize;

    // Cached top-level shader
    sk_sp<SkShader> fEffectShader;

    // Effect controls.
    SkM44           fRotateMatrix;
    SkPoint         fCenter = {0,0};
    float           fRadius = 0;

    using INHERITED = sksg::CustomRenderNode;
};

class SphereAdapter final : public DiscardableAdapterBase<SphereAdapter, SphereNode> {
public:
    SphereAdapter(const skjson::ArrayValue& jprops,
                  const AnimationBuilder* abuilder,
                  sk_sp<SphereNode> node)
        : INHERITED(std::move(node))
    {
        enum : size_t {
            // kRotGrp_Index = 0,
                 kRotX_Index = 1,
                 kRotY_Index = 2,
                 kRotZ_Index = 3,
             kRotOrder_Index = 4,
            // ???           = 5,
               kRadius_Index = 6,
               kOffset_Index = 7,
               kRender_Index = 8,

            // TODO: Light params
        };

        EffectBinder(jprops, *abuilder, this)
            .bind(  kOffset_Index, fOffset  )
            .bind(  kRadius_Index, fRadius  )
            .bind(    kRotX_Index, fRotX    )
            .bind(    kRotY_Index, fRotY    )
            .bind(    kRotZ_Index, fRotZ    )
            .bind(kRotOrder_Index, fRotOrder)
            .bind(  kRender_Index, fRender  );
    }

private:
    void onSync() override {
        const auto& sph = this->node();

        sph->setCenter({fOffset.x, fOffset.y});
        sph->setRadius(fRadius);
        sph->setRotateMatrix(this->computeRotation());
    }

    SkM44 computeRotation() const {
        const SkM44 rx = SkM44::Rotate({1,0,0}, SkDegreesToRadians(-fRotX)),
                    ry = SkM44::Rotate({0,1,0}, SkDegreesToRadians(-fRotY)),
                    rz = SkM44::Rotate({0,0,1}, SkDegreesToRadians(-fRotZ));

        switch (SkScalarRoundToInt(fRotOrder)) {
            case 1:  return rx * ry * rz;
            case 2:  return rx * rz * ry;
            case 3:  return ry * rx * rz;
            case 4:  return ry * rz * rx;
            case 5:  return rz * rx * ry;
            case 6:
            default: return rz * ry * rx;
        }

        SkUNREACHABLE;
    }

    Vec2Value   fOffset   = {0,0};
    ScalarValue fRadius   = 0,
                fRotX     = 0,
                fRotY     = 0,
                fRotZ     = 0,
                fRotOrder = 1,
                fRender   = 1;

    using INHERITED = DiscardableAdapterBase<SphereAdapter, SphereNode>;
};

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachSphereEffect(
        const skjson::ArrayValue& jprops, sk_sp<sksg::RenderNode> layer) const {
    auto sphere = sk_make_sp<SphereNode>(std::move(layer), fLayerSize);

    return fBuilder->attachDiscardableAdapter<SphereAdapter>(jprops, fBuilder, std::move(sphere));
}

} // namespace skottie::internal
