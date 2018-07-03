/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef C_ARCORE_HELLO_AR_ANCHOR_WRAPPER_H_
#define C_ARCORE_HELLO_AR_ANCHOR_WRAPPER_H_
#include "arcore_c_api.h"
#include "util.h"
#include "SkArAnchor.h"
#include <memory>

namespace hello_ar {
    enum DrawableType {
        TEXT = 0, CIRCLE = 1, RECT = 2
    };

    class AnchorWrapper {
    /**
     * Util class for the purposes of the demo app. We use this class to add useful information
     * associated with an SkArAnchor, such as what type of object to draw, and some matrix
     * computation info (axes, model matrix, plane matrix...). This is NOT part of the SDK.
     */

    public:
        AnchorWrapper();
        AnchorWrapper(std::unique_ptr<SkArAnchor> anchor);
        ~AnchorWrapper();

        std::unique_ptr<SkArAnchor> getArAnchor() const;
        glm::vec4 getAnchorPos(ArSession* arSession);
        util::MatrixComputationInfo* getMatrixInfo();
        DrawableType getDrawableType();

        void setArAnchor(std::unique_ptr<SkArAnchor> a);
        void setMatrixInfo(std::unique_ptr<util::MatrixComputationInfo> info);
        void setDrawableType(DrawableType d);

    private:
        //Pointer to the ArAnchor this wraps around. AnchorWrapper is NOT responsible for freeing it
        std::unique_ptr<SkArAnchor> fAnchor;
        std::unique_ptr<util::MatrixComputationInfo> fMatrixInfo;
        DrawableType fDrawableType;
    };
}  // namespace hello_ar

#endif
