/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImage.h"
#include "modules/skottie/include/SlotManager.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skresources/include/SkResources.h"

class skottie::SlotManager::ImageAssetProxy final : public skresources::ImageAsset {
public:
    explicit ImageAssetProxy(sk_sp<skresources::ImageAsset> asset)
    : fImageAsset(std::move(asset)) {}

    // always returns true to force the FootageLayer to always redraw in case asset is swapped
    bool isMultiFrame() override { return true; }

    FrameData getFrameData(float t) override {
        if (fImageAsset) {
            return fImageAsset->getFrameData(t);
        }
        return {nullptr , SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNearest),
            SkMatrix::I(), SizeFit::kCenter};
    }

    void setImageAsset (sk_sp<skresources::ImageAsset> asset) {
        fImageAsset = std::move(asset);
    }

    sk_sp<const skresources::ImageAsset> getImageAsset() const {
        return fImageAsset;
    }
private:
    sk_sp<skresources::ImageAsset> fImageAsset;
};

skottie::SlotManager::SlotManager(sk_sp<skottie::internal::SceneGraphRevalidator> revalidator) :
fRevalidator(revalidator) {}

skottie::SlotManager::~SlotManager() = default;

void skottie::SlotManager::setColorSlot(SlotID slotID, SkColor c) {
    const auto valueGroup = fColorMap.find(slotID);
    if (valueGroup) {
        for (auto& cPair : *valueGroup) {
            *(cPair.value) = c;
            cPair.node->invalidate();
        }
        fRevalidator->revalidate();
    }
}

void skottie::SlotManager::setImageSlot(SlotID slotID, sk_sp<skresources::ImageAsset> i) {
    const auto valueGroup = fImageMap.find(slotID);
    if (valueGroup) {
        for (auto& iPair : *valueGroup) {
            iPair.value->setImageAsset(i);
            iPair.node->invalidate();
        }
        fRevalidator->revalidate();
    }
}

void skottie::SlotManager::setScalarSlot(SlotID slotID, SkScalar s) {
    const auto valueGroup = fScalarMap.find(slotID);
    if (valueGroup) {
        for (auto& sPair : *valueGroup) {
            *(sPair.value) = s;
            if (sPair.node) {
                sPair.node->invalidate();
            } else if (sPair.adapter) {
                sPair.adapter->onSync();
            }
        }
        fRevalidator->revalidate();
    }
}

SkColor skottie::SlotManager::getColorSlot (SlotID slotID) const {
    const auto valueGroup = fColorMap.find(slotID);
    return valueGroup && !valueGroup->empty() ? *(valueGroup->at(0).value) : SK_ColorBLACK;
}

sk_sp<const skresources::ImageAsset> skottie::SlotManager::getImageSlot (SlotID slotID) const {
    const auto valueGroup = fImageMap.find(slotID);
    return valueGroup && !valueGroup->empty() ? valueGroup->at(0).value->getImageAsset() : nullptr;
}

SkScalar skottie::SlotManager::getScalarSlot (SlotID slotID) const {
    const auto valueGroup = fScalarMap.find(slotID);
    return valueGroup && !valueGroup->empty() ? *(valueGroup->at(0).value) : -1;

}

void skottie::SlotManager::trackColorValue(SlotID slotID, SkColor* colorValue,
                                           sk_sp<sksg::Node> node) {
    fColorMap[slotID].push_back({colorValue, std::move(node), nullptr});
}

sk_sp<skresources::ImageAsset> skottie::SlotManager::trackImageValue(SlotID slotID,
                                                                     sk_sp<skresources::ImageAsset>
                                                                        imageAsset,
                                                                     sk_sp<sksg::Node> node) {
    auto proxy = sk_make_sp<ImageAssetProxy>(std::move(imageAsset));
    fImageMap[slotID].push_back({proxy, std::move(node), nullptr});
    return std::move(proxy);
}

void skottie::SlotManager::trackScalarValue(SlotID slotID, SkScalar* scalarValue,
                                            sk_sp<sksg::Node> node) {
    fScalarMap[slotID].push_back({scalarValue, std::move(node), nullptr});
}

void skottie::SlotManager::trackScalarValue(SlotID slotID, SkScalar* scalarValue,
                                            sk_sp<internal::AnimatablePropertyContainer> adapter) {
    fScalarMap[slotID].push_back({scalarValue, nullptr, adapter});
}
