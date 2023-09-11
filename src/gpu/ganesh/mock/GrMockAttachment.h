/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockAttachment_DEFINED
#define GrMockAttachment_DEFINED

#include "include/core/SkTextureCompressionType.h"
#include "src/gpu/ganesh/GrAttachment.h"
#include "src/gpu/ganesh/GrBackendUtils.h"
#include "src/gpu/ganesh/mock/GrMockGpu.h"

class GrMockAttachment : public GrAttachment {
public:
    GrMockAttachment(GrMockGpu* gpu,
                     SkISize dimensions,
                     UsageFlags supportedUsages,
                     int sampleCnt,
                     std::string_view label)
            : INHERITED(gpu,
                        dimensions,
                        supportedUsages,
                        sampleCnt,
                        skgpu::Mipmapped::kNo,
                        skgpu::Protected::kNo,
                        label) {
        SkASSERT(supportedUsages == UsageFlags::kStencilAttachment);
        this->registerWithCache(skgpu::Budgeted::kYes);
    }

    GrBackendFormat backendFormat() const override {
        return GrBackendFormat::MakeMock(GrColorType::kUnknown, SkTextureCompressionType::kNone,
                                         /*isStencilFormat*/ true);
    }

private:
    using INHERITED = GrAttachment;
};

#endif
