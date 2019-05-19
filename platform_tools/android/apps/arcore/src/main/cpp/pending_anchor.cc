/*
 * Copyright 2018 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "include/core/SkCanvas.h"
#include "platform_tools/android/apps/arcore/src/main/cpp/hello_ar_application.h"
#include "platform_tools/android/apps/arcore/src/main/cpp/plane_renderer.h"
#include "platform_tools/android/apps/arcore/src/main/cpp/util.h"

namespace hello_ar {
    PendingAnchor::PendingAnchor(SkPoint touchLocation) : touchLocation(touchLocation) {}

    PendingAnchor::~PendingAnchor() {}

    SkPoint PendingAnchor::GetTouchLocation() {
        return touchLocation;
    }

    bool PendingAnchor::GetEditMode() {
        return editMode;
    }

    ArPlane* PendingAnchor::GetContainingPlane() {
        return containingPlane;
    }

    glm::vec4 PendingAnchor::GetAnchorPos(ArSession* arSession) {
        float poseRaw[] = {0, 0, 0, 0, 0, 0, 0};
        ArPose* anchorPose = nullptr;
        ArPose_create(arSession, poseRaw, &anchorPose);
        ArAnchor_getPose(arSession, this->anchor, anchorPose);
        ArPose_getPoseRaw(arSession, anchorPose, poseRaw);
        ArPose_destroy(anchorPose);
        glm::vec4 anchorPos = glm::vec4(poseRaw[4], poseRaw[5], poseRaw[6], 1);
        return anchorPos;
    }

    ArAnchor* PendingAnchor::GetArAnchor() {
        return anchor;
    }

    void PendingAnchor::SetArAnchor(ArAnchor* anchor) {
        this->anchor = anchor;
    }

    void PendingAnchor::SetEditMode(bool editMode) {
        this->editMode = editMode;
    }

    void PendingAnchor::SetContainingPlane(ArPlane* plane) {
        this->containingPlane = plane;
    }



}  // namespace hello_ar
