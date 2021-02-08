/*
 * Copyright 2021 Google Inc.
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

// This shader maps its child shader onto a sphere.  To simplify things, we set it up such that:
//
//   - the sphere is centered at origin and has r == 1
//   - the eye is positioned at (0,0,cam_z), where cam_z is chosen to visually match AE
//   - the POI for a given pixel is on the z = 0 plane (x,y,0)
//   - we're only rendering inside the projected circle, which guarantees a quadratic solution
//
// Effect stages:
//
//   1) ray-cast to find the sphere intersection (selectable front/back solution);
//      given the sphere geometry, this is also the normal
//   2) rotate the normal
//   3) UV-map the sphere
//   4) scale uv to source size and sample
//
// Note: the current implementation uses two passes for two-side ("full") rendering, on the
//       assumption that in practice most textures are opaque and two-side mode is infrequent;
//       if this proves to be problematic, we could expand the implementation to blend both sides
//       in one pass.
//
// TODO: lighting
static constexpr char gSphereSkSL[] = R"(
    uniform shader  child;

    uniform half3x3 rot_matrix;
    uniform half2   child_scale;
    uniform half    side_select;

    half3 to_sphere(half2 xy) {
        half cam_z  = 5.5,
             cam_z2 = cam_z*cam_z;
        half3   RAY = half3(xy, -cam_z);

        half a = dot(RAY, RAY),
             b = -2*cam_z2,
             c = cam_z2 - 1,
             t = (-b + side_select*sqrt(b*b - 4*a*c))/(2*a);

        return half3(0, 0, cam_z) + RAY*t;
    }

    half4 main(float2 xy) {
        half3 N = rot_matrix*to_sphere(xy);

        half kRPI = 1/3.1415927;

        half2 UV = half2(
            0.5 + kRPI * 0.5 * atan(N.x, N.z),
            0.5 + kRPI * asin(N.y)
        );

        return sample(child, UV*child_scale);
    }
)";

static sk_sp<SkRuntimeEffect> sphere_effect_singleton() {
    static const SkRuntimeEffect* effect =
            SkRuntimeEffect::Make(SkString(gSphereSkSL), /*options=*/{}).effect.release();
    if (0 && !effect) {
        printf("!!! %s\n",
               SkRuntimeEffect::Make(SkString(gSphereSkSL), /*options=*/{}).errorText.c_str());
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

    SG_ATTRIBUTE(Center  , SkPoint   , fCenter)
    SG_ATTRIBUTE(Radius  , float     , fRadius)
    SG_ATTRIBUTE(Rotation, SkM44     , fRot   )
    SG_ATTRIBUTE(Side    , RenderSide, fSide  )

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

    sk_sp<SkShader> buildEffectShader(float selector) {
        SkRuntimeShaderBuilder builder(sphere_effect_singleton());

        builder.child  ("child")       = this->contentShader();
        builder.uniform("child_scale") = fChildSize;
        builder.uniform("side_select") = selector;
        builder.uniform("rot_matrix")  = std::array<float,9>{
            fRot.rc(0,0), fRot.rc(0,1), fRot.rc(0,2),
            fRot.rc(1,0), fRot.rc(1,1), fRot.rc(1,2),
            fRot.rc(2,0), fRot.rc(2,1), fRot.rc(2,2),
        };

        const auto lm = SkMatrix::Translate(fCenter.fX, fCenter.fY) *
                        SkMatrix::Scale(fRadius, fRadius);

        return builder.makeShader(&lm, false);
    }

    SkRect onRevalidate(sksg::InvalidationController* ic, const SkMatrix& ctm) override {
        fSphereShader.reset();
        if (fSide != RenderSide::kOutside) {
            fSphereShader = this->buildEffectShader(1);
        }
        if (fSide != RenderSide::kInside) {
            auto outside = this->buildEffectShader(-1);
            fSphereShader = fSphereShader
                    ? SkShaders::Blend(SkBlendMode::kSrcOver,
                                       std::move(fSphereShader),
                                       std::move(outside))
                    : std::move(outside);
        }
        SkASSERT(fSphereShader);

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
        sphere_paint.setShader(fSphereShader);

        canvas->drawCircle(fCenter, fRadius, sphere_paint);
    }

    const RenderNode* onNodeAt(const SkPoint&) const override { return nullptr; } // no hit-testing

    const SkSize fChildSize;

    // Cached shaders
    sk_sp<SkShader> fSphereShader;
    sk_sp<SkShader> fContentShader;

    // Effect controls.
    SkM44      fRot;
    SkPoint    fCenter = {0,0};
    float      fRadius = 0;
    RenderSide fSide   = RenderSide::kFull;

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
                case 1:  return SphereNode::RenderSide::kFull;
                case 2:  return SphereNode::RenderSide::kOutside;
                case 3:
                default: return SphereNode::RenderSide::kInside;
            }
            SkUNREACHABLE;
        };

        const auto rotation = [](ScalarValue order,
                                 ScalarValue x, ScalarValue y, ScalarValue z) {
            const SkM44 rx = SkM44::Rotate({1,0,0}, SkDegreesToRadians( x)),
                        ry = SkM44::Rotate({0,1,0}, SkDegreesToRadians( y)),
                        rz = SkM44::Rotate({0,0,1}, SkDegreesToRadians(-z));

            switch (SkScalarRoundToInt(order)) {
                case 1: return rx * ry * rz;
                case 2: return rx * rz * ry;
                case 3: return ry * rx * rz;
                case 4: return ry * rz * rx;
                case 5: return rz * rx * ry;
                case 6:
               default: return rz * ry * rx;
            }
            SkUNREACHABLE;
        };

        const auto& sph = this->node();

        sph->setCenter({fOffset.x, fOffset.y});
        sph->setRadius(fRadius);
        sph->setSide(side(fRender));
        sph->setRotation(rotation(fRotOrder, fRotX, fRotY, fRotZ));
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
