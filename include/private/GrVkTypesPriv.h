/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkTypesPriv_DEFINED
#define GrVkTypesPriv_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/vk/GrVkTypes.h"

class GrBackendSurfaceMutableStateImpl;

// This struct is to used to store the the actual information about the vulkan backend image on the
// GrBackendTexture and GrBackendRenderTarget. When a client calls getVkImageInfo on a
// GrBackendTexture/RenderTarget, we use the GrVkBackendSurfaceInfo to create a snapshot
// GrVkImgeInfo object. Internally, this uses a ref count GrVkImageLayout object to track the
// current VkImageLayout which can be shared with an internal GrVkImage so that layout updates can
// be seen by all users of the image.
struct GrVkBackendSurfaceInfo {
    GrVkBackendSurfaceInfo(GrVkImageInfo info) : fImageInfo(info) {}

    void cleanup();

    GrVkBackendSurfaceInfo& operator=(const GrVkBackendSurfaceInfo&) = delete;

    // Assigns the passed in GrVkBackendSurfaceInfo to this object. if isValid is true we will also
    // attempt to unref the old fLayout on this object.
    void assign(const GrVkBackendSurfaceInfo&, bool isValid);

    GrVkImageInfo snapImageInfo(const GrBackendSurfaceMutableStateImpl*) const;

    bool isProtected() const { return fImageInfo.fProtected == GrProtected::kYes; }
#if GR_TEST_UTILS
    bool operator==(const GrVkBackendSurfaceInfo& that) const;
#endif

private:
    GrVkImageInfo    fImageInfo;
};

class GrVkSharedImageInfo {
public:
    GrVkSharedImageInfo(VkImageLayout layout, uint32_t queueFamilyIndex)
            : fLayout(layout)
            , fQueueFamilyIndex(queueFamilyIndex) {}

    GrVkSharedImageInfo& operator=(const GrVkSharedImageInfo& that) {
        fLayout = that.getImageLayout();
        fQueueFamilyIndex = that.getQueueFamilyIndex();
        return *this;
    }


     void setImageLayout(VkImageLayout layout) {
        // Defaulting to use std::memory_order_seq_cst
        fLayout.store(layout);
    }

    VkImageLayout getImageLayout() const {
        // Defaulting to use std::memory_order_seq_cst
        return fLayout.load();
    }

    void setQueueFamilyIndex(uint32_t queueFamilyIndex) {
        // Defaulting to use std::memory_order_seq_cst
        fQueueFamilyIndex.store(queueFamilyIndex);
    }

    uint32_t getQueueFamilyIndex() const {
        // Defaulting to use std::memory_order_seq_cst
        return fQueueFamilyIndex.load();
    }

private:
    std::atomic<VkImageLayout> fLayout;
    std::atomic<uint32_t> fQueueFamilyIndex;
};

#endif
