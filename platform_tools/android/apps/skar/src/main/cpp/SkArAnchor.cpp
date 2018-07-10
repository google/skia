/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkArAnchor.h"
#include "SkArUtil.h"
#include "glm.h"
#include "SkArSession.h"
#include "SkArPose.h"
#include "SkArTrackingState.h"
#include <memory>

std::unique_ptr<SkArAnchor> SkArAnchor::Make(sk_sp<SkArSession> session,
                                             std::unique_ptr<SkArPose> pose) {
    ArAnchor* anchor = nullptr;
    if (ArSession_acquireNewAnchor(session.get()->getArSession(), pose->getArPose(),
                                   &anchor) != AR_SUCCESS) {
        SKAR_LOGI("SkArAnchor: Failure Acquiring Anchor");
        return nullptr;
    }
    SKAR_LOGI("SkArAnchor: Success Acquiring Anchor");
    return std::unique_ptr<SkArAnchor>(new SkArAnchor(session, std::move(pose), anchor));
}

std::unique_ptr<SkArAnchor> SkArAnchor::Make(sk_sp<SkArSession> session,
                                             std::unique_ptr<SkArPose> pose,
                                             SkArTrackable* trackable) {
    std::unique_ptr<SkArAnchor> ptr (SkArAnchor::Make(std::move(session), std::move(pose)));
    if (ptr != nullptr) {
        ptr->setSkArTrackable(trackable);
    }
    return ptr;
}

SkArAnchor::SkArAnchor(sk_sp<SkArSession> session, std::unique_ptr<SkArPose> pose,
                       ArAnchor* arAnchor)
            : fSkArSession(std::move(session)), fSkArPose(std::move(pose)), fArAnchor(nullptr) {}

SkArAnchor::~SkArAnchor() {
    ArAnchor_detach(fSkArSession.get()->getArSession(), fArAnchor);
}

SkArPose* SkArAnchor::getPose() const {
    return fSkArPose.get();
}

SkVector3 SkArAnchor::getAnchorPos() const {
    float poseRaw[] = {0, 0, 0, 0, 0, 0, 0};
    ArPose* anchorPose = nullptr;
    ArPose_create(fSkArSession->getArSession(), poseRaw, &anchorPose);
    ArAnchor_getPose(fSkArSession->getArSession(), fArAnchor, anchorPose);
    ArPose_getPoseRaw(fSkArSession->getArSession(), anchorPose, poseRaw);
    ArPose_destroy(anchorPose);
    SkVector3 anchorPos = SkVector3::Make(poseRaw[4], poseRaw[5], poseRaw[6]);
    return anchorPos;
}

void SkArAnchor::getTrackingState(SkArTrackingState* trackingState) {
    ArTrackingState arTrackingState;
    ArAnchor_getTrackingState(fSkArSession->getArSession(), fArAnchor, &arTrackingState);
    *trackingState = SkArUtil::MakeSkArTrackingState(arTrackingState);
}
