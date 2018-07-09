/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "hello_ar_application.h"
#include "arcore_c_api.h"
#include "anchor_wrapper.h"
#include <memory>

namespace hello_ar {

    AnchorWrapper::AnchorWrapper() :
            fAnchor(nullptr), fMatrixInfo(nullptr) {}

    AnchorWrapper::AnchorWrapper(std::unique_ptr<SkArAnchor> anchor) :
            fAnchor(std::move(anchor)), fMatrixInfo(nullptr) {}

    AnchorWrapper::~AnchorWrapper() {
        //Matrix info will be deleted, as well as the SkArAnchor
    }

    SkArAnchor* AnchorWrapper::getArAnchor() const {
        return fAnchor.get();
    }

    util::MatrixComputationInfo* AnchorWrapper::getMatrixInfo() {
        return fMatrixInfo.get();
    }

    glm::vec4 AnchorWrapper::getAnchorPos(ArSession* arSession) {
        return glm::vec4(fAnchor->getAnchorPos(), 1);
    }

    DrawableType AnchorWrapper::getDrawableType() {
        return fDrawableType;
    }

    void AnchorWrapper::setArAnchor(std::unique_ptr<SkArAnchor> a) {
        fAnchor = std::move(a);
    }

    void AnchorWrapper::setMatrixInfo(std::unique_ptr<util::MatrixComputationInfo> info) {
        fMatrixInfo = std::move(info);
    }

    void AnchorWrapper::setDrawableType(DrawableType d) {
        fDrawableType = d;
    }

}  // namespace hello_ar
