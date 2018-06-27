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

#include "hello_ar_application.h"
#include "arcore_c_api.h"
#include "anchor_wrapper.h"
#include <memory>

namespace hello_ar {

    AnchorWrapper::AnchorWrapper() :
            anchor(nullptr), matrixInfo(nullptr) {}

    AnchorWrapper::AnchorWrapper(ArAnchor* anchor) :
            anchor(anchor), matrixInfo(nullptr) {}

    AnchorWrapper::~AnchorWrapper() {
        //Matrix info will be deleted
    }

    ArAnchor* AnchorWrapper::GetArAnchor() const {
        return anchor;
    }

    glm::vec4 AnchorWrapper::GetAnchorPos(ArSession* arSession) {
        float poseRaw[] = {0, 0, 0, 0, 0, 0, 0};
        ArPose* anchorPose = nullptr;
        ArPose_create(arSession, poseRaw, &anchorPose);
        ArAnchor_getPose(arSession, GetArAnchor(), anchorPose);
        ArPose_getPoseRaw(arSession, anchorPose, poseRaw);
        ArPose_destroy(anchorPose);
        glm::vec4 anchorPos = glm::vec4(poseRaw[4], poseRaw[5], poseRaw[6], 1);
        return anchorPos;
    }

    util::MatrixComputationInfo* AnchorWrapper::GetMatrixInfo() {
        return matrixInfo.get();
    }

    DrawableType AnchorWrapper::GetDrawableType() {
        return drawableType;
    }

    void AnchorWrapper::SetArAnchor(ArAnchor* anchor) {
        this->anchor = anchor;
    }

    void AnchorWrapper::SetMatrixInfo(std::unique_ptr<util::MatrixComputationInfo> info) {
        matrixInfo = std::move(info);
    }

    void AnchorWrapper::SetDrawableType(DrawableType drawableType) {
        this->drawableType = drawableType;
    }



}  // namespace hello_ar
