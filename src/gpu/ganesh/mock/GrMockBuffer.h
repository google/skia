/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockBuffer_DEFINED
#define GrMockBuffer_DEFINED

#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"
#include "src/gpu/ganesh/mock/GrMockGpu.h"

class GrMockBuffer : public GrGpuBuffer {
public:
    GrMockBuffer(GrMockGpu* gpu, size_t sizeInBytes, GrGpuBufferType type,
                 GrAccessPattern accessPattern,
                 std::string_view label)
            : INHERITED(gpu, sizeInBytes, type, accessPattern, label) {
        this->registerWithCache(skgpu::Budgeted::kYes);
    }

private:
    void onMap(MapType) override {
        if (GrCaps::kNone_MapFlags != this->getGpu()->caps()->mapBufferFlags()) {
            fMapPtr = sk_malloc_throw(this->size());
        }
    }
    void onUnmap(MapType) override { sk_free(fMapPtr); }
    bool onClearToZero() override { return true; }
    bool onUpdateData(const void* src, size_t offset, size_t size, bool preserve) override {
        return true;
    }

    using INHERITED = GrGpuBuffer;
};

#endif
