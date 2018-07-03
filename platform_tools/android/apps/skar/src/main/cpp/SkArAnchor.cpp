/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkArAnchor.h"

std::unique_ptr<SkArAnchor> SkArAnchor::Make(sk_sp<SkArSession> session,
                                             std::unique_ptr<SkArPose> pose) {
    return std::unique_ptr<SkArAnchor>(new SkArAnchor(std::move(session), std::move(pose)));
}

SkArAnchor::~SkArAnchor() {
    ArAnchor_detach(fSkArSession.get()->getArSession(), fArAnchor);
}

SkArAnchor::SkArAnchor(sk_sp<SkArSession> arSession, std::unique_ptr<SkArPose> arPose)
        : fArAnchor(nullptr) {
    if (ArSession_acquireNewAnchor(arSession.get()->getArSession(), (*arPose).getArPose(),
                                   &fArAnchor) == AR_SUCCESS) {
        SKAR_LOGI("SkArAnchor: Success Acquiring Anchor");
        fSkArPose = std::move(arPose);
    } else {
        SKAR_LOGE("SkArAnchor: Failure Acquiring Anchor");
    }
}