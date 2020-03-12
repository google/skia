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
#include "modules/sksg/include/SkSGGroup.h"
#include "modules/sksg/include/SkSGTransform.h"

#include <vector>

namespace skottie {
namespace internal {

namespace  {

class RepeaterAdapter final : public DiscardableAdapterBase<RepeaterAdapter, sksg::Group> {
public:
    RepeaterAdapter(const skjson::ObjectValue& jrepeater,
                    const skjson::ObjectValue& jtransform,
                    const AnimationBuilder& abuilder,
                    sk_sp<sksg::RenderNode> repeater_node)
        : fRepeaterNode(std::move(repeater_node))
        , fComposite((ParseDefault(jrepeater["m"], 1) == 1) ? Composite::kAbove
                                                            : Composite::kBelow) {
        this->bind(abuilder, jrepeater["c"], fCount);
        this->bind(abuilder, jrepeater["o"], fOffset);

        this->bind(abuilder, jtransform["a" ], fAnchorPoint);
        this->bind(abuilder, jtransform["p" ], fPosition);
        this->bind(abuilder, jtransform["s" ], fScale);
        this->bind(abuilder, jtransform["r" ], fRotation);
        this->bind(abuilder, jtransform["so"], fStartOpacity);
        this->bind(abuilder, jtransform["eo"], fEndOpacity);
    }

private:
    void onSync() override {
        static constexpr SkScalar kMaxCount = 512;
        const auto count = static_cast<size_t>(SkTPin(fCount, 0.0f, kMaxCount) + 0.5f);

        const auto& compute_transform = [&] (size_t index) {
            const auto t = fOffset + index;

            // Position, scale & rotation are "scaled" by index/offset.
            SkMatrix m = SkMatrix::MakeTrans(-fAnchorPoint.x,
                                             -fAnchorPoint.y);
            m.postScale(std::pow(fScale.x * .01f, fOffset),
                        std::pow(fScale.y * .01f, fOffset));
            m.postRotate(t * fRotation);
            m.postTranslate(t * fPosition.x + fAnchorPoint.x,
                            t * fPosition.y + fAnchorPoint.y);

            return m;
        };

        // TODO: start/end opacity support.

        // TODO: we can avoid rebuilding all the fragments in most cases.
        this->node()->clear();
        for (size_t i = 0; i < count; ++i) {
            const auto insert_index = (fComposite == Composite::kAbove) ? i : count - i - 1;
            this->node()->addChild(sksg::TransformEffect::Make(fRepeaterNode,
                                                               compute_transform(insert_index)));
        }
    }

    enum class Composite { kAbove, kBelow };

    const sk_sp<sksg::RenderNode> fRepeaterNode;
    const Composite               fComposite;

    // Repeater props
    ScalarValue fCount  = 0,
                fOffset = 0;

    // Transform props
    Vec2Value   fAnchorPoint  = {   0,   0 },
                fPosition     = {   0,   0 },
                fScale        = { 100, 100 };
    ScalarValue fRotation     = 0,
                fStartOpacity = 100,
                fEndOpacity   = 100;
};

} // namespace

std::vector<sk_sp<sksg::RenderNode>> ShapeBuilder::AttachRepeaterDrawEffect(
        const skjson::ObjectValue& jrepeater,
        const AnimationBuilder* abuilder,
        std::vector<sk_sp<sksg::RenderNode>>&& draws) {
    std::vector<sk_sp<sksg::RenderNode>> repeater_draws;

    if (const skjson::ObjectValue* jtransform = jrepeater["tr"]) {
        // We can skip the group if only one draw.
        auto repeater_node = (draws.size() > 1) ? sksg::Group::Make(std::move(draws))
                                                : std::move(draws[0]);

        auto repeater_root =
                abuilder->attachDiscardableAdapter<RepeaterAdapter>(jrepeater,
                                                                    *jtransform,
                                                                    *abuilder,
                                                                    std::move(repeater_node));
        repeater_draws.reserve(1);
        repeater_draws.push_back(std::move(repeater_root));
    } else {
        repeater_draws = std::move(draws);
    }

    return repeater_draws;
}

} // namespace internal
} // namespace skottie
