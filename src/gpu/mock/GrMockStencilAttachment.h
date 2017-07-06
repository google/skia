/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockStencilAttachment_DEFINED
#define GrMockStencilAttachment_DEFINED

#include "GrMockGpu.h"
#include "GrStencilAttachment.h"

class GrMockStencilAttachment : public GrStencilAttachment {
public:
    GrMockStencilAttachment(GrMockGpu* gpu, int width, int height, int bits, int sampleCnt)
            : INHERITED(gpu, width, height, bits, sampleCnt) {
        this->registerWithCache(SkBudgeted::kYes);
    }

private:
    size_t onGpuMemorySize() const override {
        return SkTMax(1, (int)(this->bits() / sizeof(char))) * this->width() * this->height();
    }

    typedef GrStencilAttachment INHERITED;
};

#endif
