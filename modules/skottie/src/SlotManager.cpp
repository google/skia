/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/include/SlotManager.h"

#include "include/core/SkImage.h"
#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkSamplingOptions.h"
#include "modules/skottie/include/SkottieProperty.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/animator/Animator.h"
#include "modules/skottie/src/text/TextAdapter.h"
#include "modules/skresources/include/SkResources.h"

#include <utility>

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

skottie::SlotManager::SlotManager(sk_sp<skottie::internal::SceneGraphRevalidator> revalidator)
        : fRevalidator(std::move(revalidator)) {}

skottie::SlotManager::~SlotManager() = default;

bool skottie::SlotManager::setColorSlot(const SlotID& slotID, SkColor c) {
    auto c4f = SkColor4f::FromColor(c);
    ColorValue v{c4f.fR, c4f.fG, c4f.fB, c4f.fA};
    const auto valueGroup = fColorMap.find(slotID);
    if (valueGroup) {
        for (auto& cPair : *valueGroup) {
            *(cPair.value) = v;
            cPair.adapter->onSync();
        }
        fRevalidator->revalidate();
        return true;
    }
    return false;
}

bool skottie::SlotManager::setImageSlot(const SlotID& slotID,
                                        const sk_sp<skresources::ImageAsset>& i) {
    const auto imageGroup = fImageMap.find(slotID);
    if (imageGroup) {
        for (auto& imageAsset : *imageGroup) {
            imageAsset->setImageAsset(i);
        }
        fRevalidator->revalidate();
        return true;
    }
    return false;
}

bool skottie::SlotManager::setScalarSlot(const SlotID& slotID, float s) {
    const auto valueGroup = fScalarMap.find(slotID);
    if (valueGroup) {
        for (auto& sPair : *valueGroup) {
            *(sPair.value) = s;
            sPair.adapter->onSync();
        }
        fRevalidator->revalidate();
        return true;
    }
    return false;
}

bool skottie::SlotManager::setVec2Slot(const SlotID& slotID, SkV2 v) {
    const auto valueGroup = fVec2Map.find(slotID);
    if (valueGroup) {
        for (auto& vPair : *valueGroup) {
            *(vPair.value) = v;
            vPair.adapter->onSync();
        }
        fRevalidator->revalidate();
        return true;
    }
    return false;
}

bool skottie::SlotManager::setTextSlot(const SlotID& slotID, const TextPropertyValue& t) {
    const auto adapterGroup = fTextMap.find(slotID);
    if (adapterGroup) {
        for (auto& textAdapter : *adapterGroup) {
            textAdapter->setText(t);
        }
        fRevalidator->revalidate();
        return true;
    }
    return false;
}

std::optional<SkColor> skottie::SlotManager::getColorSlot(const SlotID& slotID) const {
    const auto valueGroup = fColorMap.find(slotID);
    return valueGroup && !valueGroup->empty() ? std::optional<SkColor>(*(valueGroup->at(0).value))
                                              : std::nullopt;
}

sk_sp<const skresources::ImageAsset> skottie::SlotManager::getImageSlot(
        const SlotID& slotID) const {
    const auto imageGroup = fImageMap.find(slotID);
    return imageGroup && !imageGroup->empty() ? imageGroup->at(0)->getImageAsset() : nullptr;
}

std::optional<float> skottie::SlotManager::getScalarSlot(const SlotID& slotID) const {
    const auto valueGroup = fScalarMap.find(slotID);
    return valueGroup && !valueGroup->empty() ? std::optional<float>(*(valueGroup->at(0).value))
                                              : std::nullopt;
}

std::optional<SkV2> skottie::SlotManager::getVec2Slot(const SlotID& slotID) const {
    const auto valueGroup = fVec2Map.find(slotID);
    return valueGroup && !valueGroup->empty() ? std::optional<SkV2>(*(valueGroup->at(0).value))
                                              : std::nullopt;
}

std::optional<skottie::TextPropertyValue> skottie::SlotManager::getTextSlot(
        const SlotID& slotID) const {
    const auto adapterGroup = fTextMap.find(slotID);
    return adapterGroup && !adapterGroup->empty() ?
           std::optional<TextPropertyValue>(adapterGroup->at(0)->getText()) :
           std::nullopt;
}

void skottie::SlotManager::trackColorValue(const SlotID& slotID, ColorValue* colorValue,
                                           sk_sp<internal::AnimatablePropertyContainer> adapter) {
    fColorMap[slotID].push_back({colorValue, std::move(adapter)});
}

sk_sp<skresources::ImageAsset> skottie::SlotManager::trackImageValue(const SlotID& slotID,
                                                                     sk_sp<skresources::ImageAsset>
                                                                        imageAsset) {
    auto proxy = sk_make_sp<ImageAssetProxy>(std::move(imageAsset));
    fImageMap[slotID].push_back(proxy);
    return proxy;
}

void skottie::SlotManager::trackScalarValue(const SlotID& slotID, ScalarValue* scalarValue,
                                            sk_sp<internal::AnimatablePropertyContainer> adapter) {
    fScalarMap[slotID].push_back({scalarValue, std::move(adapter)});
}

void skottie::SlotManager::trackVec2Value(const SlotID& slotID, Vec2Value* vec2Value,
                                          sk_sp<internal::AnimatablePropertyContainer> adapter) {
    fVec2Map[slotID].push_back({vec2Value, std::move(adapter)});
}

void skottie::SlotManager::trackTextValue(const SlotID& slotID,
                                          sk_sp<internal::TextAdapter> adapter) {
    fTextMap[slotID].push_back(std::move(adapter));
}

skottie::SlotManager::SlotInfo skottie::SlotManager::getSlotInfo() const {
    SlotInfo sInfo;
    for (const auto& c : fColorMap) {
        sInfo.fColorSlotIDs.push_back(c.first);
    }
    for (const auto& s : fScalarMap) {
        sInfo.fScalarSlotIDs.push_back(s.first);
    }
    for (const auto& v : fVec2Map) {
        sInfo.fVec2SlotIDs.push_back(v.first);
    }
    for (const auto& i : fImageMap) {
        sInfo.fImageSlotIDs.push_back(i.first);
    }
    for (const auto& t : fTextMap) {
        sInfo.fTextSlotIDs.push_back(t.first);
    }
    return sInfo;
}
