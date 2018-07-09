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

SkArTrackable::SkArTrackable(SkArSession* session, ArTrackable* trackable)
        : fSkArSession(std::move(session)), fArTrackable(trackable) {}

SkArTrackable::~SkArTrackable() {
    ArTrackable_release(fArTrackable);
}

SkArAnchor* SkArTrackable::makeAnchor(std::unique_ptr<SkArPose> pose) {
    std::unique_ptr<SkArAnchor> anchor = SkArAnchor::Make(std::move(fSkArSession),
                                                          std::move(pose), this);
    if (!anchor) {
        return nullptr;
    }
    fSkArAnchorList.push_back(anchor);
    return anchor.get();
}

/**
* Used to check the current SkArTrackingState of the trackable
* @param session           current SkArSession
* @param trackingState     out variable containing state
*/
void SkArTrackable::getTrackingState(SkArTrackingState* trackingState) {
    ArTrackingState arTrackingState;
    ArTrackable_getTrackingState(fSkArSession->getArSession(), fArTrackable, &arTrackingState);
    *trackingState = SkArUtil::MakeSkArTrackingState(arTrackingState);
}
