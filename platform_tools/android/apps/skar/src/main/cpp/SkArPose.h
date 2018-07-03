/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKAR_SKARPOSE
#define SKAR_SKARPOSE

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/asset_manager.h>
#include <android/log.h>
#include <errno.h>
#include <jni.h>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <SkMatrix44.h>
#include <SkRefCnt.h>

#include "arcore_c_api.h"
#include "glm.h"
#include "SkArSession.h"
#include "SkArUtil.h"

class SkArPose {
public:
    static std::unique_ptr<SkArPose> Make(sk_sp<SkArSession> session, const float poseRaw[7]);

    ~SkArPose();

    ArPose* getArPose();

private:
    SkArPose(ArPose* arPose) : fArPose(arPose) {}

    // ArPose is managed by SkArPose
    ArPose* fArPose;
};
#endif  // SKAR_SKARPOSE
