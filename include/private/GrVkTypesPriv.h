/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkTypesPriv_DEFINED
#define GrVkTypesPriv_DEFINED

#include "vk/GrVkTypes.h"
#include "SkRefCnt.h"

class GrVkImageLayout;

// This struct is to used to store the the actual information about the vulkan backend image on the
// GrBackendTexture and GrBackendRenderTarget. When a client calls getVkImageInfo on a
// GrBackendTexture/RenderTarget, we use the GrVkBackendSurfaceInfo to create a snapshot
// GrVkImgeInfo object. Internally, this uses a ref count GrVkImageLayout object to track the
// current VkImageLayout which can be shared with an internal GrVkImage so that layout updates can
// be seen by all users of the image.
struct GrVkBackendSurfaceInfo {
    GrVkBackendSurfaceInfo(GrVkImageInfo info, GrVkImageLayout* layout)
            : fImageInfo(info), fLayout(layout) {}

    void cleanup();

    GrVkBackendSurfaceInfo& operator=(const GrVkBackendSurfaceInfo&) = delete;

    // Assigns the passed in GrVkBackendSurfaceInfo to this object. if isValid is true we will also
    // attempt to unref the old fLayout on this object.
    void assign(const GrVkBackendSurfaceInfo&, bool isValid);

    void setImageLayout(VkImageLayout layout);

    sk_sp<GrVkImageLayout> getGrVkImageLayout() const;

    GrVkImageInfo snapImageInfo() const;

#if GR_TEST_UTILS
    bool operator==(const GrVkBackendSurfaceInfo& that) const;
#endif

private:
    GrVkImageInfo    fImageInfo;
    GrVkImageLayout* fLayout;
};

#endif
