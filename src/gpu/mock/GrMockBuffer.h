/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockBuffer_DEFINED
#define GrMockBuffer_DEFINED

#include "GrBuffer.h"
#include "GrMockGpu.h"

class GrMockBuffer : public GrBuffer {
public:
    GrMockBuffer(GrMockGpu* gpu, size_t sizeInBytes, GrBufferType type,
                 GrAccessPattern accessPattern)
            : INHERITED(gpu, sizeInBytes, type, accessPattern) {
        this->registerWithCache(SkBudgeted::kYes);
    }

private:
    void onMap() override {}
    void onUnmap() override {}
    bool onUpdateData(const void* src, size_t srcSizeInBytes) override { return true; }

    typedef GrBuffer INHERITED;
};

#endif
