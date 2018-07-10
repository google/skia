/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <vector>
#include "SkArAnchor.h"
#include "SkArPose.h"
#include "SkArTrackable.h"
#include "SkArUtil.h"

using namespace std;

SkArTrackable::SkArTrackable()
        : fArTrackable(nullptr), fSkArSession(nullptr) {}

SkArTrackable::SkArTrackable(sk_sp<SkArSession> session, ArTrackable* trackable)
        : fArTrackable(trackable), fSkArSession(std::move(session)) {}

SkArTrackable::SkArTrackable(SkArSession* session, ArTrackable* trackable)
        : fArTrackable(trackable), fSkArSession(std::move(session)) {}

SkArTrackable::~SkArTrackable() {
    ArTrackable_release(fArTrackable);
}

SkArAnchor* SkArTrackable::makeAnchor(std::unique_ptr<SkArPose> pose) {
    std::unique_ptr<SkArAnchor> anchor = SkArAnchor::Make(std::move(fSkArSession),
                                                          std::move(pose), this);
    if (!anchor) {
        return nullptr;
    }
    fSkArAnchorList.push_back(std::move(anchor));
    return anchor.get();
}

void SkArTrackable::getTrackingState(SkArTrackingState* trackingState) {
    ArTrackingState arTrackingState;
    ArTrackable_getTrackingState(fSkArSession->getArSession(), fArTrackable, &arTrackingState);
    *trackingState = SkArUtil::MakeSkArTrackingState(arTrackingState);
}

void SkArTrackable::getTrackableType(SkArTrackableType* trackableType) {
    ArTrackableType arTrackableType;
    ArTrackable_getType(fSkArSession->getArSession(), fArTrackable, &arTrackableType);
    *trackableType = SkArUtil::MakeSkArTrackableType(arTrackableType);
}

std::vector<SkArAnchor*> SkArTrackable::getAnchors() {
    std::vector<SkArAnchor*> out(fSkArAnchorList.size());
    for (auto&& a : fSkArAnchorList) {
        out.push_back(a.get());
    }
    return out;
}
