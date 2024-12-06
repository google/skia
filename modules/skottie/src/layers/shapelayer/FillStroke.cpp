/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/private/base/SkAssert.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/animator/Animator.h"
#include "modules/skottie/src/layers/shapelayer/ShapeLayer.h"
#include "modules/sksg/include/SkSGGeometryEffect.h"
#include "modules/sksg/include/SkSGGeometryNode.h"
#include "modules/sksg/include/SkSGPaint.h"
#include "src/utils/SkJSON.h"

#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

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
                                              std::size(gJoins) - 1)]);

            static constexpr SkPaint::Cap gCaps[] = {
                SkPaint::kButt_Cap,
                SkPaint::kRound_Cap,
                SkPaint::kSquare_Cap,
            };
            this->node()->setStrokeCap(
                        gCaps[std::min<size_t>(ParseDefault<size_t>(jpaint["lc"], 1) - 1,
                                             std::size(gCaps) - 1)]);
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
            color_node->setColor(fColor);
        }
    }

    enum class ShaderType { kColor, kGradient };

    const ShaderType fShaderType;

    ColorValue       fColor;
    ScalarValue      fOpacity     = 100,
                     fStrokeWidth = 1;

    using INHERITED = DiscardableAdapterBase<FillStrokeAdapter, sksg::PaintNode>;
};

class DashAdapter final : public DiscardableAdapterBase<DashAdapter, sksg::DashEffect> {
public:
    DashAdapter(const skjson::ArrayValue& jdash,
                const AnimationBuilder& abuilder,
                sk_sp<sksg::GeometryNode> geo)
        : INHERITED(sksg::DashEffect::Make(std::move(geo))) {
        SkASSERT(jdash.size() > 1);

        // The dash is encoded as an arbitrary number of intervals (alternating dash/gap),
        // plus a single trailing offset.  Each value can be animated independently.
        const auto interval_count = jdash.size() - 1;
        fIntervals.resize(interval_count, 0);

        for (size_t i = 0; i < jdash.size(); ++i) {
            if (const skjson::ObjectValue* jint = jdash[i]) {
                auto* target = i < interval_count
                        ? &fIntervals[i]
                        : &fOffset;
                this->bind(abuilder, (*jint)["v"], target);
            }
        }
    }

private:
    void onSync() override {
        this->node()->setPhase(fOffset);
        this->node()->setIntervals(fIntervals);
    }

    std::vector<ScalarValue> fIntervals;
    ScalarValue              fOffset = 0;

    using INHERITED = DiscardableAdapterBase<DashAdapter, sksg::DashEffect>;
};

} // namespace

sk_sp<sksg::PaintNode> ShapeBuilder::AttachFill(const skjson::ObjectValue& jpaint,
                                                const AnimationBuilder* abuilder,
                                                sk_sp<sksg::PaintNode> paint_node,
                                                sk_sp<AnimatablePropertyContainer> gradient) {
    return abuilder->attachDiscardableAdapter<FillStrokeAdapter>
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
    return abuilder->attachDiscardableAdapter<FillStrokeAdapter>
            (jpaint,
             *abuilder,
             std::move(paint_node),
             std::move(gradient),
             FillStrokeAdapter::Type::kStroke);
}

sk_sp<sksg::PaintNode> ShapeBuilder::AttachColorFill(const skjson::ObjectValue& jpaint,
                                                     const AnimationBuilder* abuilder) {
    auto color_node  = sksg::Color::Make(SK_ColorBLACK);
    auto color_paint = AttachFill(jpaint, abuilder, color_node);
    abuilder->dispatchColorProperty(color_node);
    return color_paint;
}

sk_sp<sksg::PaintNode> ShapeBuilder::AttachColorStroke(const skjson::ObjectValue& jpaint,
                                                       const AnimationBuilder* abuilder) {
    auto color_node  = sksg::Color::Make(SK_ColorBLACK);
    auto color_paint = AttachStroke(jpaint, abuilder, color_node);
    abuilder->dispatchColorProperty(color_node);
    return color_paint;
}

std::vector<sk_sp<sksg::GeometryNode>> ShapeBuilder::AdjustStrokeGeometry(
        const skjson::ObjectValue& jstroke,
        const AnimationBuilder* abuilder,
        std::vector<sk_sp<sksg::GeometryNode>>&& geos) {

    const skjson::ArrayValue* jdash = jstroke["d"];
    if (jdash && jdash->size() > 1) {
        for (size_t i = 0; i < geos.size(); ++i) {
            geos[i] = abuilder->attachDiscardableAdapter<DashAdapter>(
                          *jdash, *abuilder, std::move(geos[i]));
        }
    }

    return std::move(geos);
}

} // namespace internal
} // namespace skottie
