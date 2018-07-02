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

#include <gl/GrGLTypes.h>
#include <GrBackendSurface.h>

#include "arcore_c_api.h"
#include "glm.h"
#include "anchor_wrapper.h"

namespace hello_ar {
    class PendingAnchor {
    public:
        PendingAnchor();
        ~PendingAnchor();

        bool GetEditMode() const;
        ArPlane* GetContainingPlane() const;
        glm::vec4 GetAnchorPos(ArSession* arSession) const;
        AnchorWrapper* GetAnchorWrapper() const;
        ArAnchor* GetArAnchor() const;

        void SetAnchorWrapper(AnchorWrapper* anchorW);
        void SetEditMode(bool editMode);
        void SetContainingPlane(ArPlane* plane);

    private:
        //Determines if the PendingAnchor is associated with a new ArAnchor (false), or an old ArAnchor
        //to be edited (true)
        bool editMode = false;

        //Pointer to the AnchorWrapper holding the ArAnchor info. PendingAnchor is responsible for
        //deleting it ONLY if the PendingAnchor is deleted while in editMode.
        //e.g: PendingAnchor is in editMode because it is not yet decided if its ArAnchor is decided/finalized.
        //If PendingAnchor is deleted in that state, it will also free the AnchorWrapper it holds
        AnchorWrapper* anchorWrapper;

        //Pointer to the plane this anchor is on. PendingAnchor is not responsible for deleting it
        ArPlane* containingPlane;
    };
}  // namespace hello_ar

#endif
