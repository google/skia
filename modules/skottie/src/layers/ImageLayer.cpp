/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/SkottiePriv.h"

#include "include/core/SkImage.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/sksg/include/SkSGImage.h"
#include "modules/sksg/include/SkSGTransform.h"

#include "src/core/SkMakeUnique.h"

namespace skottie {
namespace internal {

const AnimationBuilder::ImageAssetInfo*
AnimationBuilder::loadImageAsset(const skjson::ObjectValue& jimage) const {
    const skjson::StringValue* name = jimage["p"];
    const skjson::StringValue* path = jimage["u"];
    const skjson::StringValue* id   = jimage["id"];
    if (!name || !path || !id) {
        return nullptr;
    }

    const SkString res_id(id->begin());
    if (auto* cached_info = fImageAssetCache.find(res_id)) {
        return cached_info;
    }

    auto asset = fResourceProvider->loadImageAsset(path->begin(), name->begin(), id->begin());
    if (!asset) {
        this->log(Logger::Level::kError, nullptr, "Could not load image asset: %s/%s (id: '%s').",
                  path->begin(), name->begin(), id->begin());
        return nullptr;
    }

    const auto size = SkISize::Make(ParseDefault<int>(jimage["w"], 0),
                                    ParseDefault<int>(jimage["h"], 0));
    return fImageAssetCache.set(res_id, { std::move(asset), size });
}

sk_sp<sksg::RenderNode> AnimationBuilder::attachImageAsset(const skjson::ObjectValue& jimage,
                                                           LayerInfo* layer_info,
                                                           AnimatorScope* ascope) const {
    const auto* asset_info = this->loadImageAsset(jimage);
    if (!asset_info) {
        return nullptr;
    }
    SkASSERT(asset_info->fAsset);

    auto image = asset_info->fAsset->getFrame(0);
    if (!image) {
        this->log(Logger::Level::kError, nullptr, "Could not load first image asset frame.");
        return nullptr;
    }

    auto image_node = sksg::Image::Make(image);
    image_node->setQuality(kMedium_SkFilterQuality);

    if (asset_info->fAsset->isMultiFrame()) {
        class MultiFrameAnimator final : public sksg::Animator {
        public:
            MultiFrameAnimator(sk_sp<ImageAsset> asset, sk_sp<sksg::Image> image_node,
                               float time_bias, float time_scale)
                : fAsset(std::move(asset))
                , fImageNode(std::move(image_node))
                , fTimeBias(time_bias)
                , fTimeScale(time_scale) {}

            void onTick(float t) override {
                fImageNode->setImage(fAsset->getFrame((t + fTimeBias) * fTimeScale));
            }

        private:
            sk_sp<ImageAsset>     fAsset;
            sk_sp<sksg::Image>    fImageNode;
            float                 fTimeBias,
                                  fTimeScale;
        };

        ascope->push_back(skstd::make_unique<MultiFrameAnimator>(asset_info->fAsset,
                                                                 image_node,
                                                                 -layer_info->fInPoint,
                                                                 1 / fFrameRate));
    }

    const auto asset_size = SkISize::Make(
            asset_info->fSize.width()  > 0 ? asset_info->fSize.width()  : image->width(),
            asset_info->fSize.height() > 0 ? asset_info->fSize.height() : image->height());

    // Image layers are sized explicitly.
    layer_info->fSize = asset_size;

    if (asset_size == image->bounds().size()) {
        // No resize needed.
        return std::move(image_node);
    }

    return sksg::TransformEffect::Make(std::move(image_node),
        SkMatrix::MakeRectToRect(SkRect::Make(image->bounds()),
                                 SkRect::Make(asset_size),
                                 SkMatrix::kCenter_ScaleToFit));
}

sk_sp<sksg::RenderNode> AnimationBuilder::attachImageLayer(const skjson::ObjectValue& jlayer,
                                                           LayerInfo* layer_info,
                                                           AnimatorScope* ascope) const {
    return this->attachAssetRef(jlayer, ascope,
        [this, &layer_info] (const skjson::ObjectValue& jimage, AnimatorScope* ascope) {
            return this->attachImageAsset(jimage, layer_info, ascope);
        });
}

} // namespace internal
} // namespace skottie
