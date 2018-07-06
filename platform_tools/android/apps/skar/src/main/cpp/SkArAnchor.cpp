/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkArAnchor.h"
#include <memory>

std::unique_ptr<SkArAnchor> SkArAnchor::Make(sk_sp<SkArSession> session,
                                             std::unique_ptr<SkArPose> pose) {
    std::unique_ptr<SkArAnchor> ptr (new SkArAnchor(std::move(session), std::move(pose)));

    if (ptr->fArAnchor == nullptr) {
        SKAR_LOGI("SkArAnchor: Failure Acquiring Anchor");
        return nullptr;
    }

    SKAR_LOGI("SkArAnchor: Success Acquiring Anchor");
    return ptr;
}

SkArAnchor::~SkArAnchor() {
    ArAnchor_detach(fSkArSession.get()->getArSession(), fArAnchor);
}

SkArAnchor::SkArAnchor(sk_sp<SkArSession> arSession, std::unique_ptr<SkArPose> arPose)
        : fArAnchor(nullptr) {
    if (ArSession_acquireNewAnchor(arSession.get()->getArSession(), arPose->getArPose(),
                                   &fArAnchor) != AR_SUCCESS) {
        fArAnchor = nullptr;
    } else {
        fSkArPose = std::move(arPose);
        fSkArSession = std::move(arSession);
    }
}

SkArPose* SkArAnchor::getPose() const {
    return fSkArPose.get();
}

glm::vec3 SkArAnchor::getAnchorPos() const {
    float poseRaw[] = {0, 0, 0, 0, 0, 0, 0};
    ArPose* anchorPose = nullptr;
    ArPose_create(fSkArSession->getArSession(), poseRaw, &anchorPose);
    ArAnchor_getPose(fSkArSession->getArSession(), fArAnchor, anchorPose);
    ArPose_getPoseRaw(fSkArSession->getArSession(), anchorPose, poseRaw);
    ArPose_destroy(anchorPose);
    glm::vec3 anchorPos = glm::vec3(poseRaw[4], poseRaw[5], poseRaw[6]);
    return anchorPos;
}

void SkArAnchor::getTrackingState(const sk_sp<SkArSession> session,
                                  SkArTrackingState& trackingState) {
    ArTrackingState arTrackingState;
    ArAnchor_getTrackingState(session->getArSession(), fArAnchor, &arTrackingState);
    trackingState = SkArUtil::MakeSkArTrackingState(arTrackingState);
}
