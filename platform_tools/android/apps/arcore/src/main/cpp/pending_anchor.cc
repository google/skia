/*
 * Copyright 2017 Google Inc. All Rights Reserved.
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
#include <gtx/string_cast.hpp>

#include "plane_renderer.h"
#include "util.h"
#include "SkCanvas.h"
#include "GrContext.h"
#include "gl/GrGLTypes.h"
#include "SkSurface.h"
#include "SkTypeface.h"
#include "SkFontStyle.h"
#include "GrBackendSurface.h"
#include "SkMatrix44.h"
#include "SkMatrix.h"
#include "SkTextBlob.h"
#include "glm.h"
#include "SkPoint3.h"
#include "Sk3D.h"
#include <math.h>       /* acos */
#include "SkShaper.h"
#include "Skottie.h"
#include "SkAnimTimer.h"
#include "Resources.h"
#include "SkStream.h"

namespace hello_ar {
    PendingAnchor::PendingAnchor(SkPoint touchLocation) : touchLocation(touchLocation) {}

    SkPoint PendingAnchor::GetTouchLocation() {
        return touchLocation;
    }

    bool PendingAnchor::GetEditMode() {
        return editMode;
    }

    ArPlane* PendingAnchor::GetContainingPlane() {
        return containingPlane;
    }

    glm::vec4 PendingAnchor::GetAnchorPos() {
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

    void PendingAnchor::SetAnchorPos(glm::vec4 anchorPos) {
        this->anchorPos = anchorPos;
    }

    void PendingAnchor::SetContainingPlane(ArPlane* plane) {
        this->containingPlane = plane;
    }



}  // namespace hello_ar
