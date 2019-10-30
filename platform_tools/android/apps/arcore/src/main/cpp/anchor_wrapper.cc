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

#include "platform_tools/android/apps/arcore/src/main/cpp/hello_ar_application.h"
#include "arcore_c_api.h"
#include "platform_tools/android/apps/arcore/src/main/cpp/anchor_wrapper.h"

namespace hello_ar {

    AnchorWrapper::AnchorWrapper(ArAnchor *anchor) : anchor(anchor) {}

    const ArAnchor* AnchorWrapper::GetArAnchor() {
        return anchor;
    }
    DrawableType AnchorWrapper::GetDrawableType() {
        return drawableType;
    }

    void AnchorWrapper::SetArAnchor(ArAnchor* anchor) {
        this->anchor = anchor;
    }
    void AnchorWrapper::SetDrawableType(DrawableType drawableType) {
        this->drawableType = drawableType;
    }



}  // namespace hello_ar
