/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/private/SkTPin.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGColorFilter.h"
#include "modules/sksg/include/SkSGPaint.h"
#include "src/utils/SkJSON.h"

namespace skottie {
namespace internal {

namespace  {

class FillAdapter final : public AnimatablePropertyContainer {
public:
    static sk_sp<FillAdapter> Make(const skjson::ArrayValue& jprops,
                                   sk_sp<sksg::RenderNode> layer,
                                   const AnimationBuilder& abuilder) {
        return sk_sp<FillAdapter>(new FillAdapter(jprops, std::move(layer), abuilder));
    }

    const auto& node() const { return fFilterNode; }

private:
    FillAdapter(const skjson::ArrayValue& jprops,
                sk_sp<sksg::RenderNode> layer,
                const AnimationBuilder& abuilder)
        : fColorNode(sksg::Color::Make(SK_ColorBLACK))
        , fFilterNode(sksg::ModeColorFilter::Make(std::move(layer),
                                                  fColorNode,
                                                  SkBlendMode::kSrcIn)) {
        enum : size_t {
         // kFillMask_Index = 0,
         // kAllMasks_Index = 1,
               kColor_Index = 2,
         //   kInvert_Index = 3,
         // kHFeather_Index = 4,
         // kVFeather_Index = 5,
             kOpacity_Index = 6,
        };

        EffectBinder(jprops, abuilder, this)
            .bind(  kColor_Index, fColor  )
            .bind(kOpacity_Index, fOpacity);

        abuilder.dispatchColorProperty(fColorNode);
    }

    void onSync() override {
        const auto c = static_cast<SkColor>(fColor);
        const auto a =
                sk_float_round2int_no_saturate(SkColorGetA(c) * SkTPin(fOpacity, 0.0f, 1.0f));
        fColorNode->setColor(SkColorSetA(c, a));
    }

    const sk_sp<sksg::Color>           fColorNode;
    const sk_sp<sksg::ModeColorFilter> fFilterNode;

    VectorValue fColor;
    ScalarValue fOpacity = 1;
};

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachFillEffect(const skjson::ArrayValue& jprops,
                                                        sk_sp<sksg::RenderNode> layer) const {
    return fBuilder->attachDiscardableAdapter<FillAdapter>(jprops, std::move(layer), *fBuilder);
}

} // namespace internal
} // namespace skottie
