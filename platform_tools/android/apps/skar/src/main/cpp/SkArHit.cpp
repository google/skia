/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <vector>
#include "SkArHit.h"
using namespace std;

SkArHit::~SkArHit() {
    ArHitResult_destroy(fArHitResult);
}

vector<unique_ptr<SkArHit>> SkArHit::HitTest(const sk_sp<SkArSession> session,
                                             const SkArFrame* frame, float x, float y) {
    // Allocate an ArHitResultList
    ArHitResultList* hitList = nullptr;
    int32_t hitSize = 0;
    ArHitResultList_create(session->getArSession(), &hitList);
    if (!hitList) {
        return vector<unique_ptr<SkArHit>>(0);
    }

    // Perform hit test
    ArFrame_hitTest(session->getArSession(), frame->getArFrame(), x, y, hitList);
    ArHitResultList_getSize(session->getArSession(), hitList, &hitSize);

    // Populate the vector with SkArHit objects
    vector<unique_ptr<SkArHit>> outVector(hitSize);
    for (int i = 0; i < hitSize; i++) {
        ArHitResult* currHitResult = nullptr;
        ArHitResult_create(session->getArSession(), &currHitResult);

        ArHitResultList_getItem(session->getArSession(), hitList, i, currHitResult);
        unique_ptr<SkArHit> currSkArHit(new SkArHit(session, currHitResult));
        outVector.push_back(std::move(currSkArHit));
    }

    ArHitResultList_destroy(hitList); // NOTE: THIS MIGHT DE-ALLOCATE HITRESULTS, CHECK IT
    return outVector;
}

SkArHit::SkArHit(sk_sp<SkArSession> session, ArHitResult* hitResult)
        : fArHitResult(hitResult), fSkArSession(std::move(session)) {
    // Allocate an ArPose & fill in hit pose information in it
    ArPose* arPose = nullptr;
    ArPose_create(session->getArSession(), nullptr, &arPose);
    ArHitResult_getHitPose(session->getArSession(), hitResult, arPose);

    // Create an SkArPose object out of the ArPose
    float arPoseRaw[7] = {0, 0, 0, 0, 0, 0, 0};
    ArPose_getPoseRaw(session->getArSession(), arPose, arPoseRaw);
    fSkArPose = std::unique_ptr<SkArPose> (SkArPose::Make(session, arPoseRaw));
}

SkArPose* SkArHit::getPose() const {
    return fSkArPose.get();
}

float SkArHit::getDistanceFromCamera() const {
    float outDistance = 0;
    ArHitResult_getDistance(fSkArSession->getArSession(), fArHitResult, &outDistance);
    return outDistance;
}
