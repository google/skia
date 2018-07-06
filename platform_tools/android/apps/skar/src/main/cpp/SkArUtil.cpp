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

SkArTrackingState SkArUtil::MakeSkArTrackingState(ArTrackingState& trackingState) {
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
