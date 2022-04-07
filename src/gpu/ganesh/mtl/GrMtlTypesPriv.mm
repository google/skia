/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/GrMtlTypesPriv.h"

GrMtlSurfaceInfo GrMtlTextureSpecToSurfaceInfo(const GrMtlTextureSpec& mtlSpec,
                                               uint32_t sampleCount,
                                               uint32_t levelCount,
                                               GrProtected isProtected) {
    GrMtlSurfaceInfo info;
    // Shared info
    info.fSampleCount = sampleCount;
    info.fLevelCount = levelCount;
    info.fProtected = isProtected;

    // Mtl info
    info.fFormat = mtlSpec.fFormat;
    info.fUsage = mtlSpec.fUsage;
    info.fStorageMode = mtlSpec.fStorageMode;

    return info;
}
