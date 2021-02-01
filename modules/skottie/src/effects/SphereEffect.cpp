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

#include <array>

namespace skottie::internal {

namespace  {

static constexpr char gSphereSkSL[] = R"(
    uniform shader  child;

    uniform half3x3 rot;
    uniform half2   child_scale;
    uniform half    side_select;

    half3 to_sphere(half2 xy) {
        // Ray-cast to find sphere intersection.
        // Camera distance = 10 visually tuned to match AE.
        half cam_z = 10, cam_z2 = cam_z*cam_z;
        half3  POI = half3(xy, -cam_z);

        half a = dot(POI,POI),
             b = -2*cam_z2,
             c = cam_z2 - 1,
             t = (-b + side_select*sqrt(b*b - 4*a*c))/(2*a);

        return half3(0, 0, cam_z) + POI*t;
    }

    half4 main(float2 xy) {
        // Rotated normal.
        half3 n = rot*to_sphere(xy);

        half RPI = 1/3.1415927;

        // Sphere UV mapping
        half2 uv = half2(
            0.5 + RPI * 0.5 * atan(n.x, n.z),
            0.5 + RPI * asin(n.y)
        );

        return sample(child, uv*child_scale);
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

    enum class RenderSide {
        kFull,
        kOutside,
        kInside,
    };

    using RotMatrix = std::array<float,9>;

    SG_ATTRIBUTE(Center      , SkPoint   , fCenter      )
    SG_ATTRIBUTE(Radius      , float     , fRadius      )
    SG_ATTRIBUTE(RotateMatrix, RotMatrix , fRotateMatrix)
    SG_ATTRIBUTE(Side        , RenderSide, fSide        )

private:
    sk_sp<SkShader> contentShader() const {
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

    sk_sp<SkShader> buildEffectShader(float selector) const {
        SkRuntimeShaderBuilder builder(sphere_effect_singleton());

        builder.child  ("child")       = this->contentShader();
        builder.uniform("child_scale") = fChildSize;
        builder.uniform("side_select") = selector;
        builder.uniform("rot")         = fRotateMatrix;

        const auto lm = SkMatrix::Translate(fCenter.fX, fCenter.fY) *
                        SkMatrix::Scale(fRadius, fRadius);

        return builder.makeShader(&lm, false);
    }

    SkRect onRevalidate(sksg::InvalidationController* ic, const SkMatrix& ctm) override {
        fEffectShader.reset();
        if (fSide != RenderSide::kOutside) {
            fEffectShader = this->buildEffectShader(1);
        }
        if (fSide != RenderSide::kInside) {
            auto outside = this->buildEffectShader(-1);
            fEffectShader = fEffectShader
                    ? SkShaders::Blend(SkBlendMode::kSrcOver,
                                       std::move(fEffectShader),
                                       std::move(outside))
                    : std::move(outside);
        }
        SkASSERT(fEffectShader);

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

    // Cached efect shader
    mutable sk_sp<SkShader> fEffectShader;
    // Cached content shader
    mutable sk_sp<SkShader> fContentShader;

    // Effect controls.
    std::array<float,9> fRotateMatrix;
    SkPoint             fCenter = {0,0};
    float               fRadius = 0;
    RenderSide          fSide = RenderSide::kFull;

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
        const auto side = [](ScalarValue s) {
            switch (SkScalarRoundToInt(s)) {
                case 1: return SphereNode::RenderSide::kFull;
                case 2: return SphereNode::RenderSide::kOutside;
                case 3:
               default: return SphereNode::RenderSide::kInside;
            }
            SkUNREACHABLE;
        };

        const auto& sph = this->node();

        sph->setCenter({fOffset.x, fOffset.y});
        sph->setRadius(fRadius);
        sph->setSide(side(fRender));

        const auto r = this->computeRotation();
        sph->setRotateMatrix({
            r.rc(0,0), r.rc(0,1), r.rc(0,2),
            r.rc(1,0), r.rc(1,1), r.rc(1,2),
            r.rc(2,0), r.rc(2,1), r.rc(2,2),
        });
    }

    SkM44 computeRotation() const {
        const SkM44 rx = SkM44::Rotate({1,0,0}, SkDegreesToRadians( fRotX)),
                    ry = SkM44::Rotate({0,1,0}, SkDegreesToRadians( fRotY)),
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
