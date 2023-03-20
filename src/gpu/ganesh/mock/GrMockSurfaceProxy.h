/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockSurfaceProxy_DEFINED
#define GrMockSurfaceProxy_DEFINED

#include "include/core/SkTextureCompressionType.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"

class GrMockSurfaceProxy : public GrSurfaceProxy {
public:
    GrMockSurfaceProxy(SkString name, std::string_view label)
            : GrSurfaceProxy(GrBackendFormat::MakeMock(GrColorType::kRGBA_8888,
                                                       SkTextureCompressionType::kNone),
                             SkISize::Make(1, 1),
                             SkBackingFit::kExact,
                             skgpu::Budgeted::kNo,
                             skgpu::Protected::kNo,
                             GrInternalSurfaceFlags::kNone,
                             UseAllocator::kNo,
                             label) {
        SkDEBUGCODE(this->setDebugName(std::move(name)));
    }

    bool instantiate(GrResourceProvider*) override { return false; }
    SkDEBUGCODE(void onValidateSurface(const GrSurface*) override {} )
    size_t onUninstantiatedGpuMemorySize() const override { return 0; }

protected:
    sk_sp<GrSurface> createSurface(GrResourceProvider*) const override { return nullptr; }

private:
    LazySurfaceDesc callbackDesc() const override { SkUNREACHABLE; }
};

#endif
