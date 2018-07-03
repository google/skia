/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKAR_SKARUTIL
#define SKAR_SKARUTIL

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

#include "arcore_c_api.h"
#include "glm.h"
#include "SkArTrackingState.h"

#ifndef SKAR_LOGI
#define SKAR_LOGI(...) \
  __android_log_print(ANDROID_LOG_INFO, "SkAR", __VA_ARGS__)
#endif  // SKAR_LOGI

#ifndef SKAR_LOGE
#define SKAR_LOGE(...) \
  __android_log_print(ANDROID_LOG_ERROR, "SkAR", __VA_ARGS__)
#endif  // SKAR_LOGE

#ifndef SKAR_CHECK
#define SKAR_CHECK(condition)                                                        \
  if (!(condition)) {                                                                \
    SKAR_LOGE("*** AR ERROR AT %s: ABORTING!", __FILE__);                            \
    abort();                                                                         \
  }
#endif  // SKAR_CHECK

class SkArUtil {
public:
    static void CopyArPose(const ArSession* arSession, const ArPose* arPose, ArPose* outArPose);

    static SkArTrackingState MakeSkArTrackingState(ArTrackingState& trackingState);
};


#endif  // SKAR_SKARUTIL
