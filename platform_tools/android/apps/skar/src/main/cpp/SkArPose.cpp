/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArPose.h"

std::unique_ptr<SkArPose> SkArPose::Make(sk_sp<SkArSession> session,
                                         const float poseRaw[7]) {
    ArPose* pose = nullptr;
    ArPose_create(session.get()->getArSession(), poseRaw, &pose);
    return std::unique_ptr<SkArPose>(new SkArPose(session, pose));
}

SkArPose::~SkArPose() {
    ArPose_destroy(fArPose);
}

SkArPose::SkArPose(sk_sp<SkArSession> session, ArPose* arPose)
        : fArPose(arPose), fSkArSession(std::move(session)) {}

ArPose* SkArPose::getArPose() {
    return fArPose;
}

void SkArPose::getPoseRaw(float outPoseRaw[7]) const {
    ArPose_getPoseRaw(fSkArSession->getArSession(), fArPose, outPoseRaw);
}
