/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGColorFilter.h"
#include "modules/sksg/include/SkSGPaint.h"
#include "src/utils/SkJSON.h"

namespace skottie {
namespace internal {

namespace  {

class TritoneAdapter final : public AnimatablePropertyContainer {
public:
    static sk_sp<TritoneAdapter> Make(const skjson::ArrayValue& jprops,
                                      sk_sp<sksg::RenderNode> layer,
                                      const AnimationBuilder& abuilder) {
        return sk_sp<TritoneAdapter>(new TritoneAdapter(jprops, std::move(layer), abuilder));
    }

    const auto& node() const { return fCF; }

private:
    TritoneAdapter(const skjson::ArrayValue& jprops,
                   sk_sp<sksg::RenderNode> layer,
                   const AnimationBuilder& abuilder)
        : fLoColorNode(sksg::Color::Make(SK_ColorBLACK))
        , fMiColorNode(sksg::Color::Make(SK_ColorBLACK))
        , fHiColorNode(sksg::Color::Make(SK_ColorBLACK))
        , fCF(sksg::GradientColorFilter::Make(std::move(layer),
                                              { fLoColorNode, fMiColorNode, fHiColorNode })) {
        enum : size_t {
                kHiColor_Index = 0,
                kMiColor_Index = 1,
                kLoColor_Index = 2,
            kBlendAmount_Index = 3,
        };

        EffectBinder(jprops, abuilder, this)
            .bind(    kHiColor_Index, fHiColor)
            .bind(    kMiColor_Index, fMiColor)
            .bind(    kLoColor_Index, fLoColor)
            .bind(kBlendAmount_Index, fWeight );
    }

    void onSync() override {
        fLoColorNode->setColor(ValueTraits<VectorValue>::As<SkColor>(fLoColor));
        fMiColorNode->setColor(ValueTraits<VectorValue>::As<SkColor>(fMiColor));
        fHiColorNode->setColor(ValueTraits<VectorValue>::As<SkColor>(fHiColor));

        // 100-based, inverted
        fCF->setWeight((100 - fWeight) / 100);
    }

    const sk_sp<sksg::Color> fLoColorNode,
                             fMiColorNode,
                             fHiColorNode;
    const sk_sp<sksg::GradientColorFilter> fCF;

    VectorValue fLoColor,
                fMiColor,
                fHiColor;
    ScalarValue fWeight = 0;
};

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachTritoneEffect(const skjson::ArrayValue& jprops,
                                                           sk_sp<sksg::RenderNode> layer) const {
    return fBuilder->attachDiscardableAdapter<TritoneAdapter>(jprops, std::move(layer), *fBuilder);
}

} // namespace internal
} // namespace skottie
