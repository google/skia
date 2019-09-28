/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkBufferView_DEFINED
#define GrVkBufferView_DEFINED

#include "include/gpu/GrTypes.h"
#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/vk/GrVkResource.h"

class GrVkBufferView : public GrVkResource {
public:
    static const GrVkBufferView* Create(const GrVkGpu* gpu, VkBuffer buffer, VkFormat format,
                                        VkDeviceSize offset, VkDeviceSize range);

    VkBufferView bufferView() const { return fBufferView; }

#ifdef SK_TRACE_VK_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkBufferView: %d (%d refs)\n", fBufferView, this->getRefCnt());
    }
#endif

private:
    GrVkBufferView(VkBufferView bufferView) : INHERITED(), fBufferView(bufferView) {}

    void freeGPUData(GrVkGpu* gpu) const override;

    VkBufferView  fBufferView;

    typedef GrVkResource INHERITED;
};

#endif
