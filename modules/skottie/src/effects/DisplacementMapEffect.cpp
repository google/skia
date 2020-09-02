/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/core/SkPictureRecorder.h"
#include "include/effects/SkColorMatrix.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkColorData.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGRenderEffect.h"
#include "modules/sksg/include/SkSGRenderNode.h"

#include <tuple>

namespace skottie::internal {

namespace  {

static constexpr char gDisplacementSkSL[] = R"(
    in shader child;
    in shader displ;

    uniform float2 scale;

    // (per setup/upstream matrix)
    //
    // R,G -> x/y displacement input
    // B,A -> x/y displacement coverage/modulation

    half4 main(float2 xy) {
        half4 d = sample(displ);

        return sample(child, xy + (d.xy - 0.5) * scale * d.zw);
    }
)";

class DisplacementNode final : public sksg::CustomRenderNode {
public:
    ~DisplacementNode() override {
        this->unobserveInval(fDisplSource);
    }

    static sk_sp<DisplacementNode> Make(sk_sp<RenderNode> child,
                                        const SkSize& child_size,
                                        sk_sp<RenderNode> displ,
                                        const SkSize& displ_size) {
        if (!child || !displ) {
            return nullptr;
        }

        return sk_sp<DisplacementNode>(new DisplacementNode(std::move(child), child_size,
                                                            std::move(displ), displ_size));
    }

    enum class Pos : unsigned {
        kCenter,
        kStretch,
        kTile,

        kLast = kTile,
    };

    enum class Selector : unsigned {
        kR,
        kG,
        kB,
        kA,
        kLuminance,
        kHue,
        kLightness,
        kSaturation,
        kFull,
        kHalf,
        kOff,

        kLast = kOff,
    };

    SG_ATTRIBUTE(Scale        , SkV2      , fScale         )
    SG_ATTRIBUTE(ChildTileMode, SkTileMode, fChildTileMode )
    SG_ATTRIBUTE(Pos          , Pos       , fPos           )
    SG_ATTRIBUTE(XSelector    , Selector  , fXSelector     )
    SG_ATTRIBUTE(YSelector    , Selector  , fYSelector     )

private:
    DisplacementNode(sk_sp<RenderNode> child, const SkSize& child_size,
                     sk_sp<RenderNode> displ, const SkSize& displ_size)
        : INHERITED({std::move(child)})
        , fDisplSource(std::move(displ))
        , fDisplSize(displ_size)
        , fChildSize(child_size)
        , fRuntimeEffect(std::get<0>(SkRuntimeEffect::Make(SkString(gDisplacementSkSL))))
    {
        if (0 && !fRuntimeEffect) {
            auto err = std::get<1>(SkRuntimeEffect::Make(SkString(gDisplacementSkSL)));
            printf("!!! %s\n", err.c_str());
        }
        SkASSERT(fRuntimeEffect);
        this->observeInval(fDisplSource);
    }

    const RenderNode* onNodeAt(const SkPoint&) const override { return nullptr; } // no hit-testing

    SkRect onRevalidate(sksg::InvalidationController* ic, const SkMatrix& ctm) override {
        const auto child_dirty  = this->hasChildrenInval();
        const auto child_bounds = this->children()[0]->revalidate(ic, ctm);

        if (child_dirty) {
            SkPictureRecorder recorder;
            this->children()[0]->render(recorder.beginRecording(child_bounds));
            fChildContent = recorder.finishRecordingAsPicture();
        }

        if (fDisplSource->hasInval()) {
            const auto bounds = fDisplSource->revalidate(nullptr, SkMatrix::I());

            SkPictureRecorder recorder;
            fDisplSource->render(recorder.beginRecording(bounds));
            fDisplacementContent = recorder.finishRecordingAsPicture();
        }

        const auto child_tile = SkRect::MakeSize(fChildSize);
        auto child_shader = fChildContent->makeShader(fChildTileMode,
                                                      fChildTileMode,
                                                      nullptr, &child_tile);

        const auto displ_tile   = SkRect::MakeSize(fDisplSize);
        const auto displ_mode   = this->displacementTileMode();
        const auto displ_matrix = this->displacementMatrix();
        auto displ_shader = fDisplacementContent->makeShader(displ_mode,
                                                             displ_mode,
                                                             &displ_matrix,
                                                             &displ_tile);

        SkRuntimeShaderBuilder builder(fRuntimeEffect);
        builder.child("child") = child_shader;
        builder.child("displ") = this->selectChannels(displ_shader);

        builder.uniform("scale") = fScale * 2; // pre-scale 2x to simplify the shader

        fEffectShader = builder.makeShader(nullptr, false);

        return child_bounds;
    }

    void onRender(SkCanvas* canvas, const RenderContext* ctx) const override {
        auto local_ctx = ScopedRenderContext(canvas, ctx).setIsolation(this->bounds(),
                                                                       canvas->getTotalMatrix(),
                                                                       true);
        SkPaint shader_paint;
        shader_paint.setShader(fEffectShader);

        canvas->drawRect(this->bounds(), shader_paint);
    }

    SkTileMode displacementTileMode() const {
        return fPos == Pos::kTile
                ? SkTileMode::kRepeat
                : SkTileMode::kClamp;
    }

    SkMatrix displacementMatrix() const {
        switch (fPos) {
            case Pos::kCenter:  return SkMatrix::Translate(
                                    (fChildSize.fWidth  - fDisplSize.fWidth ) / 2,
                                    (fChildSize.fHeight - fDisplSize.fHeight) / 2);
                break;
            case Pos::kStretch: return SkMatrix::Scale(
                                    fChildSize.fWidth  / fDisplSize.fWidth,
                                    fChildSize.fHeight / fDisplSize.fHeight);
                break;
            case Pos::kTile:    return SkMatrix::I();
                break;
        }
        SkUNREACHABLE;
    }

    sk_sp<SkShader> selectChannels(sk_sp<SkShader> input) const {
        const auto xc = Coeffs(fXSelector),
                   yc = Coeffs(fYSelector);

        SkColorMatrix cm(
            xc.r, xc.g, xc.b, xc.a,       xc.bias,   // R channel used for horizontal displacement
            yc.r, yc.g, yc.b, yc.a,       yc.bias,   // G channel used for vertical displacement
               0,    0,    0,    1, xc.alpha_bias,   // B channel unused
               0,    0,    0,    1, yc.alpha_bias    // A channel used for modulation
        );

        // TODO: RGB->HSL stage

        return input->makeWithColorFilter(SkColorFilters::Matrix(cm));
    }

    struct SelectorCoeffs {
        float r, g, b, a, bias, alpha_bias;
    };

    static SelectorCoeffs Coeffs(Selector sel) {
        static constexpr SelectorCoeffs gCoeffs[] = {
            { 1, 0, 0, 0, 0, 0 },   // R
            { 0, 1, 0, 0, 0, 0 },   // G
            { 0, 0, 1, 0, 0, 0 },   // B
            { 0, 0, 0, 1, 0, 1 },   // A
            { SK_LUM_COEFF_R, SK_LUM_COEFF_G, SK_LUM_COEFF_B, 0, 0, 0 },  // Luminance
            { 1, 0, 0, 0,   0, 0 },   // H
            { 0, 1, 0, 0,   0, 0 },   // L
            { 0, 0, 1, 0,   0, 0 },   // S
            { 0, 0, 0, 0,   1, 1 },   // Full
            { 0, 0, 0, 0, .5f, 1 },   // Half
            { 0, 0, 0, 0,   0, 1 },   // Off
        };

        const auto i = static_cast<size_t>(sel);
        SkASSERT(i < SK_ARRAY_COUNT(gCoeffs));

        return gCoeffs[i];
    }

    const sk_sp<sksg::RenderNode> fDisplSource;
    const SkSize                  fDisplSize,
                                  fChildSize;
    const sk_sp<SkRuntimeEffect>  fRuntimeEffect;

    // Cached content
    sk_sp<SkPicture>       fChildContent,
                           fDisplacementContent;
    // Cached top-level shader
    sk_sp<SkShader>        fEffectShader;

    SkV2                   fScale          = { 0, 0 };
    SkTileMode             fChildTileMode  = SkTileMode::kDecal;
    Pos                    fPos            = Pos::kCenter;
    Selector               fXSelector      = Selector::kR,
                           fYSelector      = Selector::kR;

    using INHERITED = sksg::CustomRenderNode;
};

class DisplacementMapAdapter final : public DiscardableAdapterBase<DisplacementMapAdapter,
                                                                   DisplacementNode> {
public:
    DisplacementMapAdapter(const skjson::ArrayValue& jprops,
                           const AnimationBuilder* abuilder,
                           sk_sp<DisplacementNode> node)
        : INHERITED(std::move(node)) {
        EffectBinder(jprops, *abuilder, this)
                .bind(kUseForHorizontal_Index, fHorizontalSelector)
                .bind(kMaxHorizontal_Index   , fMaxHorizontal     )
                .bind(kUseForVertical_Index  , fVerticalSelector  )
                .bind(kMaxVertical_Index     , fMaxVertical       )
                .bind(kMapBehavior_Index     , fMapBehavior       )
                .bind(kEdgeBehavior_Index    , fEdgeBehavior      );
    }

    static std::tuple<sk_sp<sksg::RenderNode>, SkSize> GetDisplacementSource(
            const skjson::ArrayValue& jprops,
            const EffectBuilder* ebuilder) {

        if (const skjson::ObjectValue* jv = EffectBuilder::GetPropValue(jprops, kMapLayer_Index)) {
            auto* map_builder = ebuilder->getLayerBuilder(ParseDefault((*jv)["k"], -1));
            if (map_builder) {
                return std::make_tuple(map_builder->contentTree(), map_builder->size());
            }
        }

        return std::make_tuple<sk_sp<sksg::RenderNode>, SkSize>(nullptr, {0,0});
    }

private:
    enum : size_t {
        kMapLayer_Index         = 0,
        kUseForHorizontal_Index = 1,
        kMaxHorizontal_Index    = 2,
        kUseForVertical_Index   = 3,
        kMaxVertical_Index      = 4,
        kMapBehavior_Index      = 5,
        kEdgeBehavior_Index     = 6,
        // kExpandOutput_Index     = 7,
    };

    template <typename E>
    E ToEnum(float v) {
        // map one-based float "enums" to real enum types
        const auto uv = std::min(static_cast<unsigned>(v) - 1,
                                 static_cast<unsigned>(E::kLast));

        return static_cast<E>(uv);
    }

    void onSync() override {
        if (!this->node()) {
            return;
        }

        this->node()->setScale({fMaxHorizontal, fMaxVertical});
        this->node()->setChildTileMode(fEdgeBehavior != 0 ? SkTileMode::kRepeat
                                                          : SkTileMode::kDecal);

        this->node()->setPos(ToEnum<DisplacementNode::Pos>(fMapBehavior));
        this->node()->setXSelector(ToEnum<DisplacementNode::Selector>(fHorizontalSelector));
        this->node()->setYSelector(ToEnum<DisplacementNode::Selector>(fVerticalSelector));
    }

    ScalarValue  fHorizontalSelector = 0,
                 fVerticalSelector   = 0,
                 fMaxHorizontal      = 0,
                 fMaxVertical        = 0,
                 fMapBehavior        = 0,
                 fEdgeBehavior       = 0;

    using INHERITED = DiscardableAdapterBase<DisplacementMapAdapter, DisplacementNode>;
};

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachDisplacementMapEffect(
        const skjson::ArrayValue& jprops, sk_sp<sksg::RenderNode> layer) const {
    auto [ displ, displ_size ] = DisplacementMapAdapter::GetDisplacementSource(jprops, this);

    auto displ_node = DisplacementNode::Make(layer, fLayerSize, std::move(displ), displ_size);

    if (!displ_node) {
        return layer;
    }

    return fBuilder->attachDiscardableAdapter<DisplacementMapAdapter>(jprops,
                                                                      fBuilder,
                                                                      std::move(displ_node));
}

} // namespace skottie::internal
