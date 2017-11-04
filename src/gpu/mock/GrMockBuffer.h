/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockBuffer_DEFINED
#define GrMockBuffer_DEFINED

#include "GrBuffer.h"
#include "GrCaps.h"
#include "GrMockGpu.h"

class GrMockBuffer : public GrBuffer {
public:
    GrMockBuffer(GrMockGpu* gpu, size_t sizeInBytes, GrBufferType type,
                 GrAccessPattern accessPattern)
            : INHERITED(gpu, sizeInBytes, type, accessPattern) {
        this->registerWithCache(SkBudgeted::kYes);
    }

private:
    void onMap() override {
        if (GrCaps::kNone_MapFlags != this->getGpu()->caps()->mapBufferFlags()) {
            fMapPtr = sk_malloc_throw(this->sizeInBytes());
        }
    }
    void onUnmap() override { sk_free(fMapPtr); }
    bool onUpdateData(const void* src, size_t srcSizeInBytes) override { return true; }

    typedef GrBuffer INHERITED;
};

#endif
