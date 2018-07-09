/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkArAnchor_DEFINED
#define SkArAnchor_DEFINED

#include <memory>
#include <SkRefCnt.h>
#include "arcore_c_api.h"
#include "glm.h"
#include "SkArSession.h"
#include "SkArPose.h"
#include "SkArTrackingState.h"

/**
 * SkArAnchor defines necessary utils associated with common APIs' AR Anchors.
 * AR Anchors are locations in global space associated with some type of Trackable.
 * Use them to define locations at which to draw.
 */

class SkArAnchor {

public:
    /**
     * Factory method for constructing SkArAnchors given an SkArPose.
     * @param session   shared pointer to the SkArSession currently used
     * @param pose      unique pointer to the SkArPose of the SkArAnchor to be constructed
     * @return          nullptr if failed to acquire new SkArAnchor. SkArAnchor otherwise
     */
    static std::unique_ptr<SkArAnchor> Make(sk_sp<SkArSession> session,
                                            std::unique_ptr<SkArPose> pose);

    /**
     * Factory method for constructing SkArAnchors given an SkArPose. The created SkArAnchor
     * is attached to the SkArTrackable.
     * @param session   shared pointer to the SkArSession currently used
     * @param pose      unique pointer to the SkArPose of the SkArAnchor to be constructed
     * @param trackable unique pointer to the SkArTrackable to which the SkArAnchor will be attached
     *                  to
     * @return          nullptr if failed to acquire new SkArAnchor. SkArAnchor otherwise
     */
    static std::unique_ptr<SkArAnchor> Make(sk_sp<SkArSession> session,
                                            std::unique_ptr<SkArPose> pose,
                                            SkArTrackable* trackable);
    ~SkArAnchor();

    /**
     * @return global AR Pose of this SkArAnchor
     */
    SkArPose* getPose() const;

    /**
     * @return global position of the SkArAnchor in (x, y, z) format
     */
    glm::vec3 getAnchorPos() const;

    /**
     * @return global rotation of the SkArAnchor in quaternion format
     */
    glm::vec4 getAnchorQuat() const;

    /**
     * @return global transformation matrix (4x4) of the SkArAnchor
     */
    glm::mat4 getAnchorModelMatrix() const;

    /**
     * Used to check the current SkArTrackingState of the SkArAnchor
     * @param session           current SkArSession
     * @param trackingState     out variable containing state
     */
    void getTrackingState(const sk_sp<SkArSession> session, SkArTrackingState& trackingState);
private:

    void setSkArTrackable(SkArTrackable* trackable);

    SkArAnchor(sk_sp<SkArSession> arSession, std::unique_ptr<SkArPose> arPose);

    // This is a raw pointer. Its lifetime matches that of this class (SkArAnchor)
    ArAnchor* fArAnchor;

    // This is a raw pointer to an SkArTrackable. Its lifetime is independent of that of this class
    SkArTrackable* fSkArTrackable;

    std::unique_ptr<SkArPose> fSkArPose;

    sk_sp<SkArSession> fSkArSession;
};
#endif
