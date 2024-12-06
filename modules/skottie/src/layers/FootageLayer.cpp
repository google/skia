/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImage.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/private/base/SkAssert.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/include/SlotManager.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/animator/Animator.h"
#include "modules/skresources/include/SkResources.h"
#include "modules/sksg/include/SkSGImage.h"
#include "modules/sksg/include/SkSGRenderNode.h"
#include "modules/sksg/include/SkSGTransform.h"
#include "src/core/SkTHash.h"
#include "src/utils/SkJSON.h"

#include <utility>

namespace skottie {
namespace internal {

namespace  {

SkMatrix image_matrix(const ImageAsset::FrameData& frame_data, const SkISize& dest_size) {
    if (!frame_data.image) {
        return SkMatrix::I();
    }

    const auto size_fit_matrix = frame_data.scaling == ImageAsset::SizeFit::kNone
            ? SkMatrix::I()
            : SkMatrix::RectToRect(SkRect::Make(frame_data.image->bounds()),
                                   SkRect::Make(dest_size),
                                   static_cast<SkMatrix::ScaleToFit>(frame_data.scaling));

    return frame_data.matrix * size_fit_matrix;
}

class FootageAnimator final : public Animator {
public:
    FootageAnimator(sk_sp<ImageAsset> asset,
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

        auto frame_data = fAsset->getFrameData((t + fTimeBias) * fTimeScale);
        const auto m = image_matrix(frame_data, fAssetSize);
        if (frame_data.image    != fImageNode->getImage() ||
            frame_data.sampling != fImageNode->getSamplingOptions() ||
            m                   != fImageTransformNode->getMatrix()) {

            fImageNode->setImage(std::move(frame_data.image));
            fImageNode->setSamplingOptions(frame_data.sampling);
            fImageTransformNode->setMatrix(m);
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

const AnimationBuilder::FootageAssetInfo*
AnimationBuilder::loadFootageAsset(const skjson::ObjectValue& defaultJImage) const {
    const skjson::ObjectValue* jimage = &defaultJImage;
    const skjson::StringValue* slotID = defaultJImage["sid"];
    if (slotID) {
        if (!(fSlotsRoot)) {
            this->log(Logger::Level::kWarning, nullptr,
                         "Slotid found but no slots were found in the json. Using default asset.");
        } else {
            const skjson::ObjectValue* slot = (*(fSlotsRoot))[slotID->begin()];
            if (!slot) {
                this->log(Logger::Level::kWarning, nullptr,
                             "Specified slotID not found in 'slots'. Using default asset.");
            } else {
                jimage = (*slot)["p"];
            }
        }
    }

    const skjson::StringValue* name = (*jimage)["p"];
    const skjson::StringValue* path = (*jimage)["u"];
    const skjson::StringValue* id   = (*jimage)["id"];
    if (!name || !path || !id) {
        return nullptr;
    }

    const SkString res_id(id->begin());
    if (auto* cached_info = fImageAssetCache.find(res_id)) {
        return cached_info;
    }

    auto asset = fResourceProvider->loadImageAsset(path->begin(), name->begin(), id->begin());
    if (!asset && !slotID) {
        this->log(Logger::Level::kError, nullptr, "Could not load image asset: %s/%s (id: '%s').",
                  path->begin(), name->begin(), id->begin());
        return nullptr;
    }

    if (slotID) {
        asset = fSlotManager->trackImageValue(SkString(slotID->begin()), std::move(asset));
    }
    const auto size = SkISize::Make(ParseDefault<int>((*jimage)["w"], 0),
                                    ParseDefault<int>((*jimage)["h"], 0));
    return fImageAssetCache.set(res_id, { std::move(asset), size });
}

sk_sp<sksg::RenderNode> AnimationBuilder::attachFootageAsset(const skjson::ObjectValue& jimage,
                                                             LayerInfo* layer_info) const {
    const auto* asset_info = this->loadFootageAsset(jimage);
    if (!asset_info) {
        return nullptr;
    }
    SkASSERT(asset_info->fAsset);

    auto image_node = sksg::Image::Make(nullptr);

    // Optional image transform (mapping the intrinsic image size to declared asset size).
    sk_sp<sksg::Matrix<SkMatrix>> image_transform;

    const auto requires_animator = (fFlags & Animation::Builder::kDeferImageLoading)
                                    || asset_info->fAsset->isMultiFrame();
    if (requires_animator) {
        // We don't know the intrinsic image size yet (plus, in the general case,
        // the size may change from frame to frame) -> we always prepare a scaling transform.
        image_transform = sksg::Matrix<SkMatrix>::Make(SkMatrix::I());
        fCurrentAnimatorScope->push_back(sk_make_sp<FootageAnimator>(asset_info->fAsset,
                                                                     image_node,
                                                                     image_transform,
                                                                     asset_info->fSize,
                                                                     -layer_info->fInPoint,
                                                                     1 / fFrameRate));
    } else {
        // No animator needed, resolve the (only) frame upfront.
        auto frame_data = asset_info->fAsset->getFrameData(0);
        if (!frame_data.image) {
            this->log(Logger::Level::kError, nullptr, "Could not load single-frame image asset.");
            return nullptr;
        }

        const auto m = image_matrix(frame_data, asset_info->fSize);
        if (!m.isIdentity()) {
            image_transform = sksg::Matrix<SkMatrix>::Make(m);
        }

        image_node->setImage(std::move(frame_data.image));
        image_node->setSamplingOptions(frame_data.sampling);
    }

    // Image layers are sized explicitly.
    layer_info->fSize = SkSize::Make(asset_info->fSize);

    if (!image_transform) {
        // No resize needed.
        return image_node;
    }

    return sksg::TransformEffect::Make(std::move(image_node), std::move(image_transform));
}

sk_sp<sksg::RenderNode> AnimationBuilder::attachFootageLayer(const skjson::ObjectValue& jlayer,
                                                             LayerInfo* layer_info) const {
    const ScopedAssetRef footage_asset(this, jlayer);

    return footage_asset
        ? this->attachFootageAsset(*footage_asset, layer_info)
        : nullptr;
}

} // namespace internal
} // namespace skottie
