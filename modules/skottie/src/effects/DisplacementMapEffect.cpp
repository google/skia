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

#include <cmath>
#include <tuple>

namespace skottie::internal {

#ifdef SK_ENABLE_SKSL

namespace  {

// AE's displacement map effect [1] is somewhat similar to SVG's feDisplacementMap [2].  Main
// differences:
//
//   - more selector options: full/half/off, luminance, hue/saturation/lightness
//   - the scale factor is anisotropic (independent x/y values)
//   - displacement coverage is restricted to non-transparent source for some selectors
//     (specifically: r, g, b, h, s, l).
//
// [1] https://helpx.adobe.com/after-effects/using/distort-effects.html#displacement_map_effect
// [2] https://www.w3.org/TR/SVG11/filters.html#feDisplacementMapElement

// |selector_matrix| and |selector_offset| are set up to select and scale the x/y displacement
// in R/G, and the x/y coverage modulation in B/A.
static constexpr char gDisplacementSkSL[] = R"(
    uniform shader child;
    uniform shader displ;

    uniform half4x4 selector_matrix;
    uniform half4   selector_offset;

    half4 main(float2 xy) {
        half4 d = displ.eval(xy);

        d = selector_matrix*unpremul(d) + selector_offset;

        return child.eval(xy + d.xy*d.zw);
    }
)";

static sk_sp<SkRuntimeEffect> displacement_effect_singleton() {
    static const SkRuntimeEffect* effect =
            SkRuntimeEffect::MakeForShader(SkString(gDisplacementSkSL)).effect.release();
    if (0 && !effect) {
        auto err = SkRuntimeEffect::MakeForShader(SkString(gDisplacementSkSL)).errorText;
        printf("!!! %s\n", err.c_str());
    }
    SkASSERT(effect);

    return sk_ref_sp(effect);
}

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
    SG_ATTRIBUTE(ExpandBounds , bool      , fExpandBounds  )

private:
    DisplacementNode(sk_sp<RenderNode> child, const SkSize& child_size,
                     sk_sp<RenderNode> displ, const SkSize& displ_size)
        : INHERITED({std::move(child)})
        , fDisplSource(std::move(displ))
        , fDisplSize(displ_size)
        , fChildSize(child_size)
    {
        this->observeInval(fDisplSource);
    }

    struct SelectorCoeffs {
        float dr, dg, db, da, d_offset,  // displacement contribution
              c_scale, c_offset;         // coverage as a function of alpha
    };

    static SelectorCoeffs Coeffs(Selector sel) {
        // D = displacement input
        // C = displacement coverage
        static constexpr SelectorCoeffs gCoeffs[] = {
            { 1,0,0,0,0,   1,0 },   // kR: D = r, C = a
            { 0,1,0,0,0,   1,0 },   // kG: D = g, C = a
            { 0,0,1,0,0,   1,0 },   // kB: D = b, C = a
            { 0,0,0,1,0,   0,1 },   // kA: D = a, C = 1.0
            { SK_LUM_COEFF_R,SK_LUM_COEFF_G, SK_LUM_COEFF_B,0,0,   1,0},
                                    // kLuminance: D = lum(rgb), C = a
            { 1,0,0,0,0,   0,1 },   // kH: D = h, C = 1.0   (HSLA)
            { 0,1,0,0,0,   0,1 },   // kL: D = l, C = 1.0   (HSLA)
            { 0,0,1,0,0,   0,1 },   // kS: D = s, C = 1.0   (HSLA)
            { 0,0,0,0,1,   0,1 },   // kFull: D = 1.0, C = 1.0
            { 0,0,0,0,.5f, 0,1 },   // kHalf: D = 0.5, C = 1.0
            { 0,0,0,0,0,   0,1 },   // kOff:  D = 0.0, C = 1.0
        };

        const auto i = static_cast<size_t>(sel);
        SkASSERT(i < SK_ARRAY_COUNT(gCoeffs));

        return gCoeffs[i];
    }

    static bool IsConst(Selector s) {
        return s == Selector::kFull
            || s == Selector::kHalf
            || s == Selector::kOff;
    }

    sk_sp<SkShader> buildEffectShader(sksg::InvalidationController* ic, const SkMatrix& ctm) {
        // AE quirk: combining two const/generated modes does not displace - we need at
        // least one non-const selector to trigger the effect.  *shrug*
        if ((IsConst(fXSelector) && IsConst(fYSelector)) ||
            (SkScalarNearlyZero(fScale.x) && SkScalarNearlyZero(fScale.y))) {
            return nullptr;
        }

        auto get_content_picture = [](const sk_sp<sksg::RenderNode>& node,
                                      sksg::InvalidationController* ic, const SkMatrix& ctm) {
            if (!node) {
                return sk_sp<SkPicture>(nullptr);
            }

            const auto bounds = node->revalidate(ic, ctm);

            SkPictureRecorder recorder;
            node->render(recorder.beginRecording(bounds));
            return recorder.finishRecordingAsPicture();
        };

        const auto child_content = get_content_picture(this->children()[0], ic, ctm),
                   displ_content = get_content_picture(fDisplSource, ic, ctm);
        if (!child_content || !displ_content) {
            return nullptr;
        }

        const auto child_tile = SkRect::MakeSize(fChildSize);
        auto child_shader = child_content->makeShader(fChildTileMode,
                                                      fChildTileMode,
                                                      SkFilterMode::kLinear,
                                                      nullptr,
                                                      &child_tile);

        const auto displ_tile   = SkRect::MakeSize(fDisplSize);
        const auto displ_mode   = this->displacementTileMode();
        const auto displ_matrix = this->displacementMatrix();
        auto displ_shader = displ_content->makeShader(displ_mode,
                                                      displ_mode,
                                                      SkFilterMode::kLinear,
                                                      &displ_matrix,
                                                      &displ_tile);

        SkRuntimeShaderBuilder builder(displacement_effect_singleton());
        builder.child("child") = std::move(child_shader);
        builder.child("displ") = std::move(displ_shader);

        const auto xc = Coeffs(fXSelector),
                   yc = Coeffs(fYSelector);

        const auto s = fScale * 2;

        const float selector_m[] = {
            xc.dr*s.x, yc.dr*s.y,          0,          0,
            xc.dg*s.x, yc.dg*s.y,          0,          0,
            xc.db*s.x, yc.db*s.y,          0,          0,
            xc.da*s.x, yc.da*s.y, xc.c_scale, yc.c_scale,

            //  │          │               │           └────  A -> vertical modulation
            //  │          │               └────────────────  B -> horizontal modulation
            //  │          └────────────────────────────────  G -> vertical displacement
            //  └───────────────────────────────────────────  R -> horizontal displacement
        };
        const float selector_o[] = {
            (xc.d_offset - .5f) * s.x,
            (yc.d_offset - .5f) * s.y,
                          xc.c_offset,
                          yc.c_offset,
        };

        builder.uniform("selector_matrix") = selector_m;
        builder.uniform("selector_offset") = selector_o;

        // TODO: RGB->HSL stage
        return builder.makeShader(nullptr, false);
    }

    SkRect onRevalidate(sksg::InvalidationController* ic, const SkMatrix& ctm) override {
        fEffectShader = this->buildEffectShader(ic, ctm);

        auto bounds = this->children()[0]->revalidate(ic, ctm);
        if (fExpandBounds) {
            // Expand the bounds to accommodate max displacement (which is |fScale|).
            bounds.outset(std::abs(fScale.x), std::abs(fScale.y));
        }

        return bounds;
    }

    void onRender(SkCanvas* canvas, const RenderContext* ctx) const override {
        if (!fEffectShader) {
            // no displacement effect - just render the content
            this->children()[0]->render(canvas, ctx);
            return;
        }

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

    const RenderNode* onNodeAt(const SkPoint&) const override { return nullptr; } // no hit-testing

    const sk_sp<sksg::RenderNode> fDisplSource;
    const SkSize                  fDisplSize,
                                  fChildSize;

    // Cached top-level shader
    sk_sp<SkShader>        fEffectShader;

    SkV2                   fScale          = { 0, 0 };
    SkTileMode             fChildTileMode  = SkTileMode::kDecal;
    Pos                    fPos            = Pos::kCenter;
    Selector               fXSelector      = Selector::kR,
                           fYSelector      = Selector::kR;
    bool                   fExpandBounds   = false;

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
                .bind(kEdgeBehavior_Index    , fEdgeBehavior      )
                .bind(kExpandOutput_Index    , fExpandOutput      );
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
        kExpandOutput_Index     = 7,
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
        this->node()->setExpandBounds(fExpandOutput != 0);
    }

    ScalarValue  fHorizontalSelector = 0,
                 fVerticalSelector   = 0,
                 fMaxHorizontal      = 0,
                 fMaxVertical        = 0,
                 fMapBehavior        = 0,
                 fEdgeBehavior       = 0,
                 fExpandOutput       = 0;

    using INHERITED = DiscardableAdapterBase<DisplacementMapAdapter, DisplacementNode>;
};

} // namespace

#endif  // SK_ENABLE_SKSL

sk_sp<sksg::RenderNode> EffectBuilder::attachDisplacementMapEffect(
        const skjson::ArrayValue& jprops, sk_sp<sksg::RenderNode> layer) const {
#ifdef SK_ENABLE_SKSL
    auto [ displ, displ_size ] = DisplacementMapAdapter::GetDisplacementSource(jprops, this);

    auto displ_node = DisplacementNode::Make(layer, fLayerSize, std::move(displ), displ_size);

    if (!displ_node) {
        return layer;
    }

    return fBuilder->attachDiscardableAdapter<DisplacementMapAdapter>(jprops,
                                                                      fBuilder,
                                                                      std::move(displ_node));
#else
    // TODO(skia:12197)
    return layer;
#endif
}

} // namespace skottie::internal
