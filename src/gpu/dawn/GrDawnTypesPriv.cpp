/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/GrDawnTypesPriv.h"

GrDawnSurfaceInfo GrDawnTextureSpecToSurfaceInfo(const GrDawnTextureSpec& dawnSpec,
                                                 uint32_t sampleCount,
                                                 uint32_t levelCount,
                                                 GrProtected isProtected) {
    GrDawnSurfaceInfo info;
    // Shared info
    info.fSampleCount = sampleCount;
    info.fLevelCount = levelCount;
    info.fProtected = isProtected;

    // Dawn info
    info.fFormat = dawnSpec.fFormat;

    return info;
}
