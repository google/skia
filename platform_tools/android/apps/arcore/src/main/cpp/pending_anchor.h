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

#ifndef C_ARCORE_HELLO_AR_PENDING_ANCHOR_H_
#define C_ARCORE_HELLO_AR_PENDING_ANCHOR_H_

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/gl/GrGLTypes.h"

#include "arcore_c_api.h"
#include "platform_tools/android/apps/arcore/src/main/cpp/glm.h"

namespace hello_ar {
    class PendingAnchor {
    public:
        PendingAnchor(SkPoint touchLocation);
        ~PendingAnchor();

        SkPoint GetTouchLocation();
        bool GetEditMode();
        ArPlane* GetContainingPlane();
        glm::vec4 GetAnchorPos(ArSession* arSession);
        ArAnchor* GetArAnchor();

        void SetArAnchor(ArAnchor* anchor);
        void SetEditMode(bool editMode);
        void SetContainingPlane(ArPlane* plane);

    private:
        SkPoint touchLocation;
        bool editMode = false;
        ArAnchor* anchor;
        ArPlane* containingPlane;
    };
}  // namespace hello_ar

#endif
