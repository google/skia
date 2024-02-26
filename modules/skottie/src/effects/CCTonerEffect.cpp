/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/effects/Effects.h"
#include "modules/sksg/include/SkSGColorFilter.h"
#include "modules/sksg/include/SkSGPaint.h"
#include "modules/sksg/include/SkSGRenderNode.h"
#include "src/base/SkVx.h"
#include "src/core/SkSwizzlePriv.h"

#include <cstddef>
#include <utility>
#include <vector>

namespace skjson {
class ArrayValue;
}

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
            const auto c0_4f = Sk4f_fromL32(c0),
                       c1_4f = Sk4f_fromL32(c1),
                       c_4f = c0_4f + (c1_4f - c0_4f) * t;

            return Sk4f_toL32(c_4f);
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
        ColorValue  fHighlights,
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
