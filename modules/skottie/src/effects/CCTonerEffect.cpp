/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/private/SkNx.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGColorFilter.h"
#include "modules/sksg/include/SkSGPaint.h"
#include "src/utils/SkJSON.h"

namespace skottie::internal {

namespace {

class CCTonerAdapter final : public DiscardableAdapterBase<CCTonerAdapter,
                                                           sksg::GradientColorFilter> {
    public:
        CCTonerAdapter(const skjson::ArrayValue& jprops,
                       sk_sp<sksg::RenderNode> layer,
                       const AnimationBuilder& abuilder,
                       std::vector<sk_sp<sksg::Color>> colorNodes)
            : INHERITED(sksg::GradientColorFilter::Make(std::move(layer), colorNodes))
            , fColorNodes(std::move(colorNodes))
        {
            enum : size_t {
                kTone_Index        = 0,
                kHiColor_Index     = 1,
                kBrightColor_Index = 2,
                kMidColor_Index    = 3,
                kDarkColor_Index   = 4,
                kShadowColor_Index = 5,
                kBlendAmount_Index = 6,
            };


            EffectBinder(jprops, abuilder, this)
                .bind(       kTone_Index, fTone)
                .bind(    kHiColor_Index, fHighlights)
                .bind(kBrightColor_Index, fBrights)
                .bind(   kMidColor_Index, fMidtones)
                .bind(  kDarkColor_Index, fDarktones)
                .bind(kShadowColor_Index, fShadows);

        }
    private:
        static SkColor lerpColor(SkColor c0, SkColor c1, float t) {
            const auto c0_4f = SkNx_cast<float>(Sk4b::Load(&c0)),
                       c1_4f = SkNx_cast<float>(Sk4b::Load(&c1)),
                       c_4f = c0_4f + (c1_4f - c0_4f) * t;

            SkColor c;
            SkNx_cast<uint8_t>(Sk4f_round(c_4f)).store(&c);
            return c;
        }

        void onSync() override {
            switch (SkScalarRoundToInt(fTone)) {
                // duotone
                case 1: fColorNodes.at(0)->setColor(fShadows);
                        fColorNodes.at(1)->setColor(lerpColor(fShadows, fHighlights, 0.25));
                        fColorNodes.at(2)->setColor(lerpColor(fShadows, fHighlights, 0.5));
                        fColorNodes.at(3)->setColor(lerpColor(fShadows, fHighlights, 0.75));
                        fColorNodes.at(4)->setColor(fHighlights);
                        break;
                // tritone
                case 2: fColorNodes.at(0)->setColor(fShadows);
                        fColorNodes.at(1)->setColor(lerpColor(fShadows, fMidtones, 0.5));
                        fColorNodes.at(2)->setColor(fMidtones);
                        fColorNodes.at(3)->setColor(lerpColor(fMidtones, fHighlights, 0.5));
                        fColorNodes.at(4)->setColor(fHighlights);
                        break;
                // pentone
                case 3: fColorNodes.at(0)->setColor(fShadows);
                        fColorNodes.at(1)->setColor(fDarktones);
                        fColorNodes.at(2)->setColor(fMidtones);
                        fColorNodes.at(3)->setColor(fBrights);
                        fColorNodes.at(4)->setColor(fHighlights);
                        break;
                // solid
                default: fColorNodes.at(0)->setColor(fMidtones);
                        fColorNodes.at(1)->setColor(fMidtones);
                        fColorNodes.at(2)->setColor(fMidtones);
                        fColorNodes.at(3)->setColor(fMidtones);
                        fColorNodes.at(4)->setColor(fMidtones);
                        break;
            }

            this->node()->setWeight((100-fBlend)/100);
        }


        const std::vector<sk_sp<sksg::Color>> fColorNodes;

        ScalarValue fTone = 0;
        VectorValue fHighlights,
                    fBrights,
                    fMidtones,
                    fDarktones,
                    fShadows;
        ScalarValue fBlend = 0;

        using INHERITED = DiscardableAdapterBase<CCTonerAdapter, sksg::GradientColorFilter>;
};

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachCCTonerEffect(const skjson::ArrayValue& jprops,
                                                             sk_sp<sksg::RenderNode> layer) const {
    std::vector<sk_sp<sksg::Color>> colorNodes = {
        sksg::Color::Make(SK_ColorRED),
        sksg::Color::Make(SK_ColorRED),
        sksg::Color::Make(SK_ColorRED),
        sksg::Color::Make(SK_ColorRED),
        sksg::Color::Make(SK_ColorRED)
    };
    return fBuilder->attachDiscardableAdapter<CCTonerAdapter>(jprops,
                                                              std::move(layer),
                                                              *fBuilder,
                                                              std::move(colorNodes));
}

} // namespace skottie::internal
