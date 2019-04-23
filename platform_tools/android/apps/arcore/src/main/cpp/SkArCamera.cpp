/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <arcore_c_api.h>
#include "platform_tools/android/apps/arcore/src/main/cpp/SkArCamera.h"
#include "SkArUtil.h"

std::unique_ptr<SkArCamera> SkArCamera::Make(SkArSession* session, SkArFrame* frame) {
    return std::unique_ptr<SkArCamera>(new SkArCamera(session, frame));
}

SkArCamera::~SkArCamera() {
    ArCamera_release(fArCamera);
}

SkArCamera::SkArCamera(SkArSession* session, SkArFrame* frame) : fArCamera(nullptr) {
    ArFrame_acquireCamera(session->getArSession(), frame->getArFrame(), &fArCamera);
}

void SkArCamera::getViewMatrix(const SkArSession* session, float outColMajor[16]) {
    ArCamera_getViewMatrix(session->getArSession(), fArCamera, outColMajor);
}

void SkArCamera::getProjectionMatrix(const SkArSession* session, float nearClip,
                                     float farClip, float outColMajor[16]) {
    ArCamera_getProjectionMatrix(session->getArSession(), fArCamera, nearClip, farClip,
                                 outColMajor);
}

SkArTrackingState SkArCamera::getTrackingState(const SkArSession* session) {
    ArTrackingState arTrackingState;
    ArCamera_getTrackingState(session->getArSession(), fArCamera, &arTrackingState);
    return SkArUtil::MakeSkArTrackingState(arTrackingState);
}
