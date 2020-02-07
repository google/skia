/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/layers/shapelayer/ShapeLayer.h"
#include "modules/sksg/include/SkSGPaint.h"

namespace skottie {
namespace internal {

namespace  {

class FillStrokeAdapter final : public DiscardableAdapterBase<FillStrokeAdapter, sksg::PaintNode> {
public:
    enum class Type { kFill, kStroke };

    FillStrokeAdapter(const skjson::ObjectValue& jpaint,
                      const AnimationBuilder& abuilder,
                      sk_sp<sksg::PaintNode> paint_node,
                      sk_sp<AnimatablePropertyContainer> gradient_adapter,
                      Type type)
        : INHERITED(std::move(paint_node))
        , fShaderType(gradient_adapter ? ShaderType::kGradient : ShaderType::kColor) {

        this->attachDiscardableAdapter(std::move(gradient_adapter));

        this->bind(abuilder, jpaint["o"], fOpacity);

        this->node()->setAntiAlias(true);

        if (type == Type::kStroke) {
            this->bind(abuilder, jpaint["w"], fStrokeWidth);

            this->node()->setStyle(SkPaint::kStroke_Style);
            this->node()->setStrokeMiter(ParseDefault<SkScalar>(jpaint["ml"], 4.0f));

            static constexpr SkPaint::Join gJoins[] = {
                SkPaint::kMiter_Join,
                SkPaint::kRound_Join,
                SkPaint::kBevel_Join,
            };
            this->node()->setStrokeJoin(
                        gJoins[std::min<size_t>(ParseDefault<size_t>(jpaint["lj"], 1) - 1,
                                              SK_ARRAY_COUNT(gJoins) - 1)]);

            static constexpr SkPaint::Cap gCaps[] = {
                SkPaint::kButt_Cap,
                SkPaint::kRound_Cap,
                SkPaint::kSquare_Cap,
            };
            this->node()->setStrokeCap(
                        gCaps[std::min<size_t>(ParseDefault<size_t>(jpaint["lc"], 1) - 1,
                                             SK_ARRAY_COUNT(gCaps) - 1)]);
        }

        if (fShaderType == ShaderType::kColor) {
            this->bind(abuilder, jpaint["c"], fColor);
        }
    }

private:
    void onSync() override {
        this->node()->setOpacity(fOpacity * 0.01f);
        this->node()->setStrokeWidth(fStrokeWidth);

        if (fShaderType == ShaderType::kColor) {
            auto* color_node = static_cast<sksg::Color*>(this->node().get());
            color_node->setColor(ValueTraits<VectorValue>::As<SkColor>(fColor));
        }
    }

    enum class ShaderType { kColor, kGradient };

    const ShaderType fShaderType;

    VectorValue      fColor;
    ScalarValue      fOpacity     = 100,
                     fStrokeWidth = 1;

    using INHERITED = DiscardableAdapterBase<FillStrokeAdapter, sksg::PaintNode>;
};

} // namespace

sk_sp<sksg::PaintNode> ShapeBuilder::AttachFill(const skjson::ObjectValue& jpaint,
                                                const AnimationBuilder* abuilder,
                                                sk_sp<sksg::PaintNode> paint_node,
                                                sk_sp<AnimatablePropertyContainer> gradient) {
    return abuilder->attachDiscardableAdapter<FillStrokeAdapter, sk_sp<sksg::PaintNode>>
            (jpaint,
             *abuilder,
             std::move(paint_node),
             std::move(gradient),
             FillStrokeAdapter::Type::kFill);
}

sk_sp<sksg::PaintNode> ShapeBuilder::AttachStroke(const skjson::ObjectValue& jpaint,
                                                  const AnimationBuilder* abuilder,
                                                  sk_sp<sksg::PaintNode> paint_node,
                                                  sk_sp<AnimatablePropertyContainer> gradient) {
    return abuilder->attachDiscardableAdapter<FillStrokeAdapter, sk_sp<sksg::PaintNode>>
            (jpaint,
             *abuilder,
             std::move(paint_node),
             std::move(gradient),
             FillStrokeAdapter::Type::kStroke);
}

sk_sp<sksg::PaintNode> ShapeBuilder::AttachColorFill(const skjson::ObjectValue& jpaint,
                                                     const AnimationBuilder* abuilder) {
    auto color_node = sksg::Color::Make(SK_ColorBLACK);
    abuilder->dispatchColorProperty(color_node);

    return AttachFill(jpaint, abuilder, std::move(color_node));
}

sk_sp<sksg::PaintNode> ShapeBuilder::AttachColorStroke(const skjson::ObjectValue& jpaint,
                                                       const AnimationBuilder* abuilder) {
    auto color_node = sksg::Color::Make(SK_ColorBLACK);
    abuilder->dispatchColorProperty(color_node);

    return AttachStroke(jpaint, abuilder, std::move(color_node));
}

} // namespace internal
} // namespace skottie
