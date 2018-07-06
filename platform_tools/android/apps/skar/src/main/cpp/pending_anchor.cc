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

    PendingAnchor::~PendingAnchor() {}

    bool PendingAnchor::getEditMode() const {
        return editMode;
    }

    ArPlane* PendingAnchor::getContainingPlane() const {
        return containingPlane;
    }

    glm::vec4 PendingAnchor::getAnchorPos(ArSession* arSession) const {
        return anchorWrapper->getAnchorPos(arSession);
    }

    AnchorWrapper* PendingAnchor::getAnchorWrapper() const {
        return anchorWrapper;
    }

    SkArAnchor* PendingAnchor::getArAnchor() const {
        return anchorWrapper->getArAnchor();
    }

    void PendingAnchor::setAnchorWrapper(AnchorWrapper* anchorW) {
        anchorWrapper = anchorW;
    }

    void PendingAnchor::setEditMode(bool edit) {
        editMode = edit;
    }

    void PendingAnchor::setContainingPlane(ArPlane* plane) {
        containingPlane = plane;
    }



}  // namespace hello_ar
