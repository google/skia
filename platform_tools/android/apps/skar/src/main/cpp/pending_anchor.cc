/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "hello_ar_application.h"
#include "plane_renderer.h"
#include "util.h"
#include "SkCanvas.h"

namespace hello_ar {
    PendingAnchor::PendingAnchor() {}

    PendingAnchor::~PendingAnchor() {
        if (!editMode && anchorWrapper) {
            ArAnchor_release(anchorWrapper->GetArAnchor());
        }
    }

    bool PendingAnchor::GetEditMode() const {
        return editMode;
    }

    ArPlane* PendingAnchor::GetContainingPlane() const {
        return containingPlane;
    }

    glm::vec4 PendingAnchor::GetAnchorPos(ArSession* arSession) const {
        float poseRaw[] = {0, 0, 0, 0, 0, 0, 0};
        ArPose* anchorPose = nullptr;
        ArPose_create(arSession, poseRaw, &anchorPose);
        ArAnchor_getPose(arSession, anchorWrapper->GetArAnchor(), anchorPose);
        ArPose_getPoseRaw(arSession, anchorPose, poseRaw);
        ArPose_destroy(anchorPose);
        glm::vec4 anchorPos = glm::vec4(poseRaw[4], poseRaw[5], poseRaw[6], 1);
        return anchorPos;
    }

    AnchorWrapper* PendingAnchor::GetAnchorWrapper() const {
        return anchorWrapper;
    }

    ArAnchor* PendingAnchor::GetArAnchor() const {
        return anchorWrapper->GetArAnchor();
    }

    void PendingAnchor::SetAnchorWrapper(AnchorWrapper* anchorW) {
        anchorWrapper = anchorW;
    }

    void PendingAnchor::SetEditMode(bool edit) {
        editMode = edit;
    }

    void PendingAnchor::SetContainingPlane(ArPlane* plane) {
        containingPlane = plane;
    }



}  // namespace hello_ar
