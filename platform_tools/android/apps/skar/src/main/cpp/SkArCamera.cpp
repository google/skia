/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArCamera.h"
#include "SkArUtil.h"

std::unique_ptr<SkArCamera> SkArCamera::Make(sk_sp<SkArSession> session,
                                                    sk_sp<SkArFrame> frame) {
    return std::unique_ptr<SkArCamera>(new SkArCamera(session, frame));
}

SkArCamera::~SkArCamera() {
    ArCamera_release(fArCamera);
}

SkArCamera::SkArCamera(sk_sp<SkArSession> session, sk_sp<SkArFrame> frame)
    : fArCamera(nullptr) {
    ArFrame_acquireCamera(session->getArSession(), frame->getArFrame(), &fArCamera);
}

void SkArCamera::getViewMatrix(const sk_sp<SkArSession> session, float outColMajor[16]) {
    ArCamera_getViewMatrix(session->getArSession(), fArCamera, outColMajor);
}

void SkArCamera::getProjectionMatrix(const sk_sp<SkArSession> session, float nearClip, float farClip,
                         float outColMajor[16]) {
    ArCamera_getProjectionMatrix(session->getArSession(), fArCamera, nearClip, farClip, outColMajor);
}

void SkArCamera::getTrackingState(const sk_sp<SkArSession> session, SkArTrackingState* trackingState) {
    ArTrackingState arTrackingState;
    ArCamera_getTrackingState(session->getArSession(), fArCamera, &arTrackingState);
    *trackingState = SkArUtil::MakeSkArTrackingState(&arTrackingState);
}