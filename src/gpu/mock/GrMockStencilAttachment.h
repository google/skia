/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockStencilAttachment_DEFINED
#define GrMockStencilAttachment_DEFINED

#include "src/gpu/GrBackendUtils.h"
#include "src/gpu/GrStencilAttachment.h"
#include "src/gpu/mock/GrMockGpu.h"

class GrMockStencilAttachment : public GrStencilAttachment {
public:
    GrMockStencilAttachment(GrMockGpu* gpu, SkISize dimensions, int sampleCnt)
            : INHERITED(gpu, dimensions, sampleCnt, GrProtected::kNo) {
        this->registerWithCache(SkBudgeted::kYes);
    }

    GrBackendFormat backendFormat() const override {
        return GrBackendFormat::MakeMock(GrColorType::kUnknown, SkImage::CompressionType::kNone,
                                         /*isStencilFormat*/ true);
    }

private:
    size_t onGpuMemorySize() const override {
        int bpp = GrBackendFormatBytesPerBlock(this->backendFormat());
        return std::max(1, (int)(bpp)) * this->width() * this->height();
    }

    using INHERITED = GrStencilAttachment;
};

#endif
