/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockMtlAttachment_DEFINED
#define GrMockMtlAttachment_DEFINED

#include "src/gpu/GrAttachment.h"
#include "src/gpu/GrBackendUtils.h"
#include "src/gpu/mock/GrMockGpu.h"

class GrMockAttachment : public GrAttachment {
public:
    GrMockAttachment(GrMockGpu* gpu, SkISize dimensions, int sampleCnt)
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

    using INHERITED = GrAttachment;
};

#endif
