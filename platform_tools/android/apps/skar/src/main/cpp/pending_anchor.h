/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
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
