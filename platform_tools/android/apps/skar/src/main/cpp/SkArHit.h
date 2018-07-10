/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkArHit_DEFINED
#define SkArHit_DEFINED

#include <memory>
#include "SkArFrame.h"
#include "SkArPose.h"

/**
 *  Encapsulates the results of a hit test. A hit test is meant to approximate intersections of
 *  UI touch locations with real world geometry.
 */

class SkArHit {

public:
    /**
     * Performs a ray cast from the user's device in the direction of the given (x,y) location
     * in the camera view. Returns a vector populated by SkArHit objects, all of which were created
     * during the intersection of the ray with approximated real-world geometry.
     * @param session   shared pointer to the current SkArSession
     * @param frame     shared pointer to the current SkArFrame
     * @param x         logical X position within the view
     * @param y         logical Y position within the view
     * @return          std::vector of unique pointers to SkArHit objects created during
     *                  the hit test
     */
    static std::vector<std::unique_ptr<SkArHit>> HitTest(const sk_sp<SkArSession> session,
                                                         const SkArFrame* frame, float x, float y);

    std::unique_ptr<SkArTrackable> makeTrackable();

    ~SkArHit();

    SkArPose* getPose() const;

    float getDistanceFromCamera() const;

private:
    SkArHit(sk_sp<SkArSession> session, ArHitResult* hitResult);

    // This is a raw pointer. Its lifetime matches that of this class (SkArHit)
    ArHitResult* fArHitResult;

    std::unique_ptr<SkArPose> fSkArPose;

    sk_sp<SkArSession> fSkArSession;
};


#endif  // SkArHit_DEFINED
