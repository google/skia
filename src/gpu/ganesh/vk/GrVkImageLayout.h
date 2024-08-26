/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkImageLayout_DEFINED
#define GrVkImageLayout_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/ganesh/vk/GrVkTypes.h"

class GrVkImageLayout : public SkRefCnt {
public:
    GrVkImageLayout(VkImageLayout layout) : fLayout(layout) {}

    void setImageLayout(VkImageLayout layout) {
        // Defaulting to use std::memory_order_seq_cst
        fLayout.store(layout);
    }

    VkImageLayout getImageLayout() const {
        // Defaulting to use std::memory_order_seq_cst
        return fLayout.load();
    }

private:
    std::atomic<VkImageLayout> fLayout;
};

#endif
