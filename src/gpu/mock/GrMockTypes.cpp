/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/mock/GrMockTypes.h"

#include "include/gpu/GrBackendSurface.h"

GrBackendFormat GrMockRenderTargetInfo::getBackendFormat() const {
    return GrBackendFormat::MakeMock(fColorType);
}

GrBackendFormat GrMockTextureInfo::getBackendFormat() const {
    return GrBackendFormat::MakeMock(fColorType);
}
