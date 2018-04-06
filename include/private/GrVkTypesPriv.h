/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkTypesPriv_DEFINED
#define GrVkTypesPriv_DEFINED

#include "SkRefCnt.h"

#ifdef SK_VULKAN

#include "vk/GrVkTypes.h"

class GrVkImageLayout : public SkRefCnt {
public:
    GrVkImageLayout(VkImageLayout layout) : fLayout(layout) {}

    void setImageLayout(VkImageLayout layout) {
        // Defaulting ot use std::memory_order_seq_cst
        fLayout.store(layout);
    }

    VkImageLayout getImageLayout() const {
        // Defaulting ot use std::memory_order_seq_cst
        return fLayout.load();
    }

private:
    std::atomic<VkImageLayout> fLayout;
};

#endif

#endif
