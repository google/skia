/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImageFilter.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/base/SkPoint_impl.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/effects/Effects.h"
#include "modules/sksg/include/SkSGRenderEffect.h"
#include "modules/sksg/include/SkSGRenderNode.h"

#include <cstddef>
#include <utility>

namespace skjson {
class ArrayValue;
}

namespace skottie::internal {

namespace {

class SharpenAdapter final : public DiscardableAdapterBase<SharpenAdapter,
                                                           sksg::ExternalImageFilter> {
    public:
        SharpenAdapter(const skjson::ArrayValue& jprops,
                       const AnimationBuilder& abuilder)
            : INHERITED(sksg::ExternalImageFilter::Make())
        {
            enum : size_t {
                kSharpenAmount_Index  = 0,
            };

            EffectBinder(jprops, abuilder, this).bind(kSharpenAmount_Index, fAmount);
        }
    private:
        void onSync() override {
            SkScalar intensity = 1 + (fAmount * 0.01);
            SkScalar discount = (1 - intensity) / 8.0;
            SkScalar kernel[9] = {
                discount, discount, discount,
                discount, intensity, discount,
                discount, discount, discount,
            };
            auto filter = SkImageFilters::MatrixConvolution(SkISize::Make(3,3), kernel, 1, 0,
                                                            SkIPoint::Make(1,1), SkTileMode::kRepeat,
                                                            true, nullptr);
            this->node()->setImageFilter(std::move(filter));
        }

        ScalarValue fAmount = 0;

        using INHERITED = DiscardableAdapterBase<SharpenAdapter, sksg::ExternalImageFilter>;
};

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachSharpenEffect(const skjson::ArrayValue& jprops,
                                                             sk_sp<sksg::RenderNode> layer) const {
    auto imageFilterNode = fBuilder->attachDiscardableAdapter<SharpenAdapter>(jprops,
                                                                              *fBuilder);
    return sksg::ImageFilterEffect::Make(std::move(layer), std::move(imageFilterNode));
}

} // namespace skottie::internal
