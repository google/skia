/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/GrVkTypesPriv.h"

#include "src/gpu/vk/GrVkImageLayout.h"

void GrVkBackendSurfaceInfo::cleanup() {
    SkSafeUnref(fLayout);
    fLayout = nullptr;
};

void GrVkBackendSurfaceInfo::assign(const GrVkBackendSurfaceInfo& that, bool isThisValid) {
    fImageInfo = that.fImageInfo;
    GrVkImageLayout* oldLayout = fLayout;
    fLayout = SkSafeRef(that.fLayout);
    if (isThisValid) {
        SkSafeUnref(oldLayout);
    }
}

void GrVkBackendSurfaceInfo::setImageLayout(VkImageLayout layout) {
    SkASSERT(fLayout);
    fLayout->setImageLayout(layout);
}

sk_sp<GrVkImageLayout> GrVkBackendSurfaceInfo::getGrVkImageLayout() const {
    SkASSERT(fLayout);
    return sk_ref_sp(fLayout);
}

GrVkImageInfo GrVkBackendSurfaceInfo::snapImageInfo() const {
    return GrVkImageInfo(fImageInfo, fLayout->getImageLayout());
}

#if GR_TEST_UTILS
bool GrVkBackendSurfaceInfo::operator==(const GrVkBackendSurfaceInfo& that) const {
    GrVkImageInfo cpyInfoThis = fImageInfo;
    GrVkImageInfo cpyInfoThat = that.fImageInfo;
    // We don't care about the fImageLayout here since we require they use the same
    // GrVkImageLayout.
    cpyInfoThis.fImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    cpyInfoThat.fImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    return cpyInfoThis == cpyInfoThat && fLayout == that.fLayout;
}
#endif
