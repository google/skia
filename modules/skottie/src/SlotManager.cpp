/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/include/SlotManager.h"
#include "modules/skottie/src/SkottiePriv.h"

skottie::SlotManager::SlotManager(sk_sp<skottie::internal::SceneGraphRevalidator> revalidator) :
fRevalidator(revalidator) {}

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
    // TODO
}

void skottie::SlotManager::setScalarSlot(SlotID slotID, SkScalar s) {
    const auto valueGroup = fScalarMap.find(slotID);
    if (valueGroup) {
        for (auto& sPair : *valueGroup) {
            *(sPair.value) = s;
            sPair.node->invalidate();
        }
        fRevalidator->revalidate();
    }
}

SkColor skottie::SlotManager::getColorSlot (SlotID slotID) const {
    const auto valueGroup = fColorMap.find(slotID);
    return valueGroup && !valueGroup->empty() ? *(valueGroup->at(0).value) : SK_ColorBLACK;
}

sk_sp<skresources::ImageAsset> skottie::SlotManager::getImageSlot (SlotID slotID) const {
    // TODO
    return nullptr;
}

SkScalar skottie::SlotManager::getScalarSlot (SlotID slotID) const {
    const auto valueGroup = fScalarMap.find(slotID);
    return valueGroup && !valueGroup->empty() ? *(valueGroup->at(0).value) : -1;

}

void skottie::SlotManager::trackColorValue(SlotID slotID, SkColor* colorValue,
                                           sk_sp<sksg::Node> node) {
    fColorMap[slotID].push_back({colorValue, std::move(node)});
}

void skottie::SlotManager::trackImageValue(SlotID slotID, sk_sp<skresources::ImageAsset> imageAsset,
                                           sk_sp<sksg::Node> node) {

}
void skottie::SlotManager::trackScalarValue(SlotID slotID, SkScalar* scalarValue,
                                            sk_sp<sksg::Node> node) {
    fScalarMap[slotID].push_back({scalarValue, std::move(node)});
}
