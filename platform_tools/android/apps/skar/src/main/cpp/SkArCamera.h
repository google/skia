/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKAR_SKARCAMERA
#define SKAR_SKARCAMERA

#include <memory>
#include "SkArFrame.h"
#include "SkArSession.h"
#include "SkArTrackingState.h"

class SkArSession;

class SkArCamera {
public:
    static std::unique_ptr<SkArCamera> Make(sk_sp<SkArSession> session, sk_sp<SkArFrame> frame);

    ~SkArCamera();

    void getViewMatrix(const sk_sp<SkArSession> session, float outColMajor[16]);

    void getProjectionMatrix(const sk_sp<SkArSession> session, float nearClip, float farClip,
                             float outColMajor[16]);

    void getTrackingState(const sk_sp<SkArSession> session, SkArTrackingState* trackingState);

private:
    SkArCamera(sk_sp<SkArSession> session, sk_sp<SkArFrame> frame);

    // ArCamera is managed by SkArCamera
    ArCamera* fArCamera;
};
#endif  // SKAR_SKARCAMERA
