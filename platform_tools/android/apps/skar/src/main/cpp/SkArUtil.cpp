/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArUtil.h"

namespace SkAR {
    void SkArUtil::CopyArPose(const ArSession* arSession, const ArPose* arPose, ArPose* outArPose) {
        float rawPose[] = {0, 0, 0, 0, 0, 0, 0};
        ArPose_getPoseRaw(arSession, arPose, rawPose);
        ArPose_create(arSession, rawPose, &outArPose);
    }
}  // namespace SkAr
