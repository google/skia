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

namespace skottie {
namespace internal {

namespace  {

SkMatrix image_matrix(const sk_sp<SkImage>& image, const SkISize& dest_size) {
    return image ? SkMatrix::MakeRectToRect(SkRect::Make(image->bounds()),
                                            SkRect::Make(dest_size),
                                            SkMatrix::kCenter_ScaleToFit)
                 : SkMatrix::I();
}

class ImageAnimator final : public Animator {
public:
    ImageAnimator(sk_sp<ImageAsset> asset,
                  sk_sp<sksg::Image> image_node,
                  sk_sp<sksg::Matrix<SkMatrix>> image_transform_node,
                  const SkISize& asset_size,
                  float time_bias, float time_scale)
        : fAsset(std::move(asset))
        , fImageNode(std::move(image_node))
        , fImageTransformNode(std::move(image_transform_node))
        , fAssetSize(asset_size)
        , fTimeBias(time_bias)
        , fTimeScale(time_scale)
        , fIsMultiframe(fAsset->isMultiFrame()) {}

    StateChanged onSeek(float t) override {
        if (!fIsMultiframe && fImageNode->getImage()) {
            // Single frame already resolved.
            return false;
        }

        auto frame = fAsset->getFrame((t + fTimeBias) * fTimeScale);
        if (frame != fImageNode->getImage()) {
            fImageTransformNode->setMatrix(image_matrix(frame, fAssetSize));
            fImageNode->setImage(std::move(frame));
            return true;
        }

        return false;
    }

private:
    const sk_sp<ImageAsset>             fAsset;
    const sk_sp<sksg::Image>            fImageNode;
    const sk_sp<sksg::Matrix<SkMatrix>> fImageTransformNode;
    const SkISize                       fAssetSize;
    const float                         fTimeBias,
                                        fTimeScale;
    const bool                          fIsMultiframe;
};

} // namespace

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
                                                           LayerInfo* layer_info) const {
    const auto* asset_info = this->loadImageAsset(jimage);
    if (!asset_info) {
        return nullptr;
    }
    SkASSERT(asset_info->fAsset);

    auto image_node = sksg::Image::Make(nullptr);
    image_node->setQuality(kMedium_SkFilterQuality);

    // Optional image transform (mapping the intrinsic image size to declared asset size).
    sk_sp<sksg::Matrix<SkMatrix>> image_transform;

    const auto requires_animator = (fFlags & Animation::Builder::kDeferImageLoading)
                                    || asset_info->fAsset->isMultiFrame();
    if (requires_animator) {
        // We don't know the intrinsic image size yet (plus, in the general case,
        // the size may change from frame to frame) -> we always prepare a scaling transform.
        image_transform = sksg::Matrix<SkMatrix>::Make(SkMatrix::I());
        fCurrentAnimatorScope->push_back(sk_make_sp<ImageAnimator>(asset_info->fAsset,
                                                                   image_node,
                                                                   image_transform,
                                                                   asset_info->fSize,
                                                                   -layer_info->fInPoint,
                                                                   1 / fFrameRate));
    } else {
        // No animator needed, resolve the (only) frame upfront.
        auto frame = asset_info->fAsset->getFrame(0);
        if (!frame) {
            this->log(Logger::Level::kError, nullptr, "Could not load single-frame image asset.");
            return nullptr;
        }

        if (frame->bounds().size() != asset_info->fSize) {
            image_transform = sksg::Matrix<SkMatrix>::Make(image_matrix(frame, asset_info->fSize));
        }

        image_node->setImage(std::move(frame));
    }

    // Image layers are sized explicitly.
    layer_info->fSize = SkSize::Make(asset_info->fSize);

    if (!image_transform) {
        // No resize needed.
        return std::move(image_node);
    }

    return sksg::TransformEffect::Make(std::move(image_node), std::move(image_transform));
}

sk_sp<sksg::RenderNode> AnimationBuilder::attachImageLayer(const skjson::ObjectValue& jlayer,
                                                           LayerInfo* layer_info) const {
    return this->attachAssetRef(jlayer,
        [this, &layer_info] (const skjson::ObjectValue& jimage) {
            return this->attachImageAsset(jimage, layer_info);
        });
}

} // namespace internal
} // namespace skottie
