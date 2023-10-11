/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/mock/GrMockTypes.h"

#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/GrBackendSurface.h"
#include "src/gpu/ganesh/mock/GrMockTypesPriv.h"

GrBackendFormat GrMockRenderTargetInfo::getBackendFormat() const {
    return GrBackendFormat::MakeMock(fColorType, SkTextureCompressionType::kNone);
}

GrBackendFormat GrMockTextureInfo::getBackendFormat() const {
    return GrBackendFormat::MakeMock(fColorType, fCompressionType);
}

GrMockSurfaceInfo GrMockTextureSpecToSurfaceInfo(const GrMockTextureSpec& mockSpec,
                                                 uint32_t sampleCount,
                                                 uint32_t levelCount,
                                                 GrProtected isProtected) {
    GrMockSurfaceInfo info;
    // Shared info
    info.fSampleCount = sampleCount;
    info.fLevelCount = levelCount;
    info.fProtected = isProtected;

    // Mock info
    info.fColorType = mockSpec.fColorType;
    info.fCompressionType = mockSpec.fCompressionType;

    return info;
}
