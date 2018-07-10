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
#include <SkPoint3.h>
#include "arcore_c_api.h"

class SkArSession;
class SkArPose;
class SkArTrackable;
class SkArTrackingState;

/**
 * SkArAnchor defines necessary utils associated with common API's AR Anchors.
 * AR Anchors are locations in global space. Their location (Pose) may change between updates, but
 * will never change spontaneously.
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
     * @param trackable raw pointer to the SkArTrackable to which the SkArAnchor will be attached
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
    SkVector3 getAnchorPos() const;

    /**
     * @return global rotation of the SkArAnchor in quaternion format
     */
    SkVector4 getAnchorQuat() const;

    /**
     * @return global transformation matrix (4x4) of the SkArAnchor
     */
    SkMatrix44 getAnchorModelMatrix() const;

    /**
     * Used to check the current SkArTrackingState of the SkArAnchor
     * @param session           current SkArSession
     * @param trackingState     out variable containing state
     */
    void getTrackingState(const sk_sp<SkArSession> session, SkArTrackingState& trackingState);
private:

    /**
     * Changes the raw pointer to the trackable this anchor belongs to
     * @param trackable
     */
    void setSkArTrackable(SkArTrackable* trackable);

    SkArAnchor(sk_sp<SkArSession> session, std::unique_ptr<SkArPose> pose, ArAnchor* arAnchor);

    // This is a raw pointer. Its lifetime matches that of this class (SkArAnchor)
    ArAnchor* fArAnchor;

    // This is a raw pointer to an SkArTrackable. Its lifetime is independent of that of this class
    SkArTrackable* fSkArTrackable;

    std::unique_ptr<SkArPose> fSkArPose;

    sk_sp<SkArSession> fSkArSession;
};
#endif
