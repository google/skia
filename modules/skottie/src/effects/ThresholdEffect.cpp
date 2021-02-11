/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/effects/SkRuntimeEffect.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGColorFilter.h"

namespace skottie::internal {

namespace  {

// Convert to black & white, based on input luminance and a threshold uniform.
static constexpr char gThresholdSkSL[] = R"(
    uniform shader input;
    uniform half   t;

    half4 main() {
        half4 c = unpremul(sample(input));

        half lum = dot(c.rgb, half3(0.2126, 0.7152, 0.0722)),
              bw = step(t, lum);

        return bw.xxx1 * c.a;
    }
)";

static sk_sp<SkRuntimeEffect> threshold_effect() {
    static const SkRuntimeEffect* effect =
        SkRuntimeEffect::Make(SkString(gThresholdSkSL), {}).effect.release();
    SkASSERT(effect);

    return sk_ref_sp(effect);
}

class ThresholdAdapter final : public DiscardableAdapterBase<ThresholdAdapter,
                                                             sksg::ExternalColorFilter> {
public:
    ThresholdAdapter(const skjson::ArrayValue& jprops,
                     sk_sp<sksg::RenderNode> layer,
                     const AnimationBuilder& abuilder)
        : INHERITED(sksg::ExternalColorFilter::Make(std::move(layer)))
    {
        enum : size_t {
            kLevel_Index = 0,
        };

        EffectBinder(jprops, abuilder, this).bind(kLevel_Index, fLevel);
    }

private:
    void onSync() override {
        sk_sp<SkColorFilter> input; // nullptr -> input color
        auto cf = threshold_effect()
                      ->makeColorFilter(SkData::MakeWithCopy(&fLevel, sizeof(fLevel)), &input, 1);

        this->node()->setColorFilter(std::move(cf));
    }

    ScalarValue fLevel = 0;

    using INHERITED = DiscardableAdapterBase<ThresholdAdapter, sksg::ExternalColorFilter>;
};

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachThresholdEffect(const skjson::ArrayValue& jprops,
                                                             sk_sp<sksg::RenderNode> layer) const {
    return fBuilder->attachDiscardableAdapter<ThresholdAdapter>(jprops,
                                                                std::move(layer),
                                                                *fBuilder);
}

} // namespace skottie::internal
