/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkArPose_DEFINED
#define SkArPose_DEFINED

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
    /**
     * Immutable rigid transformation from one coordinate space to another
     */

public:
    /**
     * Factory method used to construct a unique pointer to an SkArPose. This only allocates and
     * initializes a new SkArPose object using the values in poseRaw
     * @param session   shared pointer to the current SkArSession
     * @param poseRaw   7 floats describing the transformation: (4 values in quaternion format
     *                  and the (x, y, z) position)
     * @return          unique pointer to an SkArPose object
     */
    static std::unique_ptr<SkArPose> Make(sk_sp<SkArSession> session, const float poseRaw[7]);

    ~SkArPose();

    /**
     * @param session         shared pointer to the current SkArSession
     * @param outPoseRaw[7]   float array of length 7 to be filled with 7 floats
     *                        describing the transformation: (4 values in quaternion format
     *                        and the (x, y, z) position)
     */
    void getPoseRaw(float outPoseRaw[7]) const;


private:
    SkArPose(sk_sp<SkArSession> session, ArPose* arPose);

    /**
     * @return the ARCore-backed AR Pose object of this SkArPose
     */
    ArPose* getArPose();

    // ArPose is managed by SkArPose
    ArPose* fArPose;

    sk_sp<SkArSession> fSkArSession;

    friend class SkArAnchor;
};
#endif  // SkArPose_DEFINED
