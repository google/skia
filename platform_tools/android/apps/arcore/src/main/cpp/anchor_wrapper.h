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

#ifndef C_ARCORE_HELLO_AR_ANCHOR_WRAPPER_H_
#define C_ARCORE_HELLO_AR_ANCHOR_WRAPPER_H_
#include "arcore_c_api.h"
#include "util.h"

namespace hello_ar {
    enum DrawableType {
        TEXT = 0, CIRCLE = 1, RECT = 2
    };

    class AnchorWrapper {
    public:
        AnchorWrapper(ArAnchor* anchor);
        ~AnchorWrapper();

        ArAnchor* GetArAnchor() const;
        glm::vec4 GetAnchorPos(ArSession* arSession);
        util::MatrixComputationInfo* GetMatrixInfo();
        DrawableType GetDrawableType();

        void SetArAnchor(ArAnchor* anchor);
        void SetMatrixInfo(util::MatrixComputationInfo* info);
        void SetDrawableType(DrawableType drawableType);

    private:
        ArAnchor* anchor;
        util::MatrixComputationInfo* matrixInfo;
        DrawableType drawableType;
        bool inEditMode = false;
    };
}  // namespace hello_ar

#endif
