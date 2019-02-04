/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockBuffer_DEFINED
#define GrMockBuffer_DEFINED

#include "GrCaps.h"
#include "GrGpuBuffer.h"
#include "GrMockGpu.h"

class GrMockBuffer : public GrGpuBuffer {
public:
    GrMockBuffer(GrMockGpu* gpu, size_t sizeInBytes, GrGpuBufferType type,
                 GrAccessPattern accessPattern)
            : INHERITED(gpu, sizeInBytes, type, accessPattern) {
        this->registerWithCache(SkBudgeted::kYes);
    }

private:
    void onMap() override {
        if (GrCaps::kNone_MapFlags != this->getGpu()->caps()->mapBufferFlags()) {
            fMapPtr = sk_malloc_throw(this->size());
        }
    }
    void onUnmap() override { sk_free(fMapPtr); }
    bool onUpdateData(const void* src, size_t srcSizeInBytes) override { return true; }

    typedef GrGpuBuffer INHERITED;
};

#endif
