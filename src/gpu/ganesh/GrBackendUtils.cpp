/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrBackendUtils.h"

#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/DataUtils.h"
#include "src/gpu/ganesh/GrBackendSurfacePriv.h"

SkTextureCompressionType GrBackendFormatToCompressionType(const GrBackendFormat& format) {
    if (!format.isValid()) {
        return SkTextureCompressionType::kNone;
    }
    SkASSERT(format.backend() != GrBackendApi::kUnsupported);
    return GrBackendSurfacePriv::GetBackendData(format)->compressionType();
}

size_t GrBackendFormatBytesPerBlock(const GrBackendFormat& format) {
    if (!format.isValid()) {
        return 0;
    }
    SkASSERT(format.backend() != GrBackendApi::kUnsupported);
    return GrBackendSurfacePriv::GetBackendData(format)->bytesPerBlock();
}

size_t GrBackendFormatBytesPerPixel(const GrBackendFormat& format) {
    if (GrBackendFormatToCompressionType(format) != SkTextureCompressionType::kNone) {
        return 0;
    }
    return GrBackendFormatBytesPerBlock(format);
}

int GrBackendFormatStencilBits(const GrBackendFormat& format) {
    if (!format.isValid()) {
        return 0;
    }
    SkASSERT(format.backend() != GrBackendApi::kUnsupported);
    return GrBackendSurfacePriv::GetBackendData(format)->stencilBits();
}
