/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkArTrackable_DEFINED
#define SkArTrackable_DEFINED

#include <memory>
#include "SkArTrackingState.h"
#include "SkArAnchor.h"
#include <SkRefCnt.h>

class SkArSession;
class SkArPose;

/**
 * Represents trackable & approximated real-world geometry such as Planes and Points.
 * Can attach SkArAnchors to an SkArTrackable.
 */

class SkArTrackable {
public:
    SkArTrackable(SkArSession* session, ArTrackable* trackable);

    ~SkArTrackable();

    /**
     * Create an return a raw pointer to an SkArAnchor residing at the given SkArPose, managed by
     * this SkArTrackable.
     * @param pose  pose at which to create this SkArAnchor
     * @return      raw pointer to SkArAnchor
     */
    SkArAnchor* makeAnchor(std::unique_ptr<SkArPose> pose);

    /**
    * Used to check the current SkArTrackingState of the trackable
    * @param session           current SkArSession
    * @param trackingState     out variable containing state
    */
    void getTrackingState(SkArTrackingState* trackingState);

private:

    // This is a raw pointer. Its lifetime matches that of this class (SkArAnchor)
    ArTrackable* fArTrackable;

    sk_sp<SkArSession> fSkArSession;

    // List of SkArAnchors this SkArTrackable is keeping track of. The SkArTrackable uniquely owns
    // these SkArAnchors. These are NOT free-floating SkArAnchors
    std::vector<std::unique_ptr<SkArAnchor>> fSkArAnchorList;
};


#endif  // SkArTrackable_DEFINED
