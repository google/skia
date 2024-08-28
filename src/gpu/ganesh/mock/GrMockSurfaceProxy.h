/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockSurfaceProxy_DEFINED
#define GrMockSurfaceProxy_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"

#include <cstddef>
#include <string_view>
#include <utility>

class GrResourceProvider;
class GrSurface;

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
