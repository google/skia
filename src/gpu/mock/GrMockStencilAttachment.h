/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockStencilAttachment_DEFINED
#define GrMockStencilAttachment_DEFINED

#include "src/gpu/GrStencilAttachment.h"
#include "src/gpu/mock/GrMockGpu.h"

class GrMockStencilAttachment : public GrStencilAttachment {
public:
    GrMockStencilAttachment(GrMockGpu* gpu, SkISize dimensions, int bits, int sampleCnt)
            : INHERITED(gpu, dimensions, bits, sampleCnt, GrProtected::kNo) {
        this->registerWithCache(SkBudgeted::kYes);
    }

    GrBackendFormat backendFormat() const override { return GrBackendFormat(); }

private:
    size_t onGpuMemorySize() const override {
        return std::max(1, (int)(this->bits() / sizeof(char))) * this->width() * this->height();
    }

    using INHERITED = GrStencilAttachment;
};

#endif
