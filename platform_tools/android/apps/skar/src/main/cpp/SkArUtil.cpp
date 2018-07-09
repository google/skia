/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArUtil.h"

void SkArUtil::CopyArPose(const ArSession* arSession, const ArPose* arPose, ArPose* outArPose) {
    float rawPose[] = {0, 0, 0, 0, 0, 0, 0};
    ArPose_getPoseRaw(arSession, arPose, rawPose);
    ArPose_create(arSession, rawPose, &outArPose);
}

SkArTrackingState SkArUtil::MakeSkArTrackingState(const ArTrackingState& trackingState) {
    switch (trackingState) {
        case AR_TRACKING_STATE_STOPPED:
            return SkArTrackingState::kStopped;
        case AR_TRACKING_STATE_TRACKING:
            return SkArTrackingState::kTracking;
        case AR_TRACKING_STATE_PAUSED:
            return SkArTrackingState::kPaused;
        default:
            return SkArTrackingState::kStopped;
    }
}

ArTrackableType SkArUtil::MakeArTrackableType(const SkArTrackableType& skArTrackableType) {
    switch (skArTrackableType) {
        case SkArTrackableType::kBaseTrackable:
            return AR_TRACKABLE_BASE_TRACKABLE;
        case SkArTrackableType::kPlane:
            return AR_TRACKABLE_PLANE;
        case SkArTrackableType::kPoint:
            return AR_TRACKABLE_POINT;
        case SkArTrackableType::kAugmentedImage:
            return AR_TRACKABLE_AUGMENTED_IMAGE;
        case SkArTrackableType::kInvalid:
            return AR_TRACKABLE_NOT_VALID;
        default:
            return AR_TRACKABLE_NOT_VALID;
    }
}
