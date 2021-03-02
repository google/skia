/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockAttachment_DEFINED
#define GrMockAttachment_DEFINED

#include "src/gpu/GrAttachment.h"
#include "src/gpu/GrBackendUtils.h"
#include "src/gpu/mock/GrMockGpu.h"

class GrMockAttachment : public GrAttachment {
public:
    GrMockAttachment(GrMockGpu* gpu, SkISize dimensions, UsageFlags supportedUsages, int sampleCnt)
            : INHERITED(gpu, dimensions, supportedUsages, sampleCnt, GrMipmapped::kNo,
                        GrProtected::kNo) {
        SkASSERT(supportedUsages == UsageFlags::kStencilAttachment);
        this->registerWithCache(SkBudgeted::kYes);
    }

    GrBackendFormat backendFormat() const override {
        return GrBackendFormat::MakeMock(GrColorType::kUnknown, SkImage::CompressionType::kNone,
                                         /*isStencilFormat*/ true);
    }

private:
    using INHERITED = GrAttachment;
};

#endif
