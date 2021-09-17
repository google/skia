/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DTypesPriv_DEFINED
#define GrD3DTypesPriv_DEFINED

#include "include/gpu/d3d/GrD3DTypes.h"

struct GrD3DTextureResourceSpec {
    GrD3DTextureResourceSpec()
            : fFormat(DXGI_FORMAT_UNKNOWN)
            , fSampleQualityPattern(DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN) {}

    GrD3DTextureResourceSpec(const GrD3DSurfaceInfo& info)
            : fFormat(info.fFormat), fSampleQualityPattern(info.fSampleQualityPattern) {}

    DXGI_FORMAT fFormat;
    unsigned int fSampleQualityPattern;
};

GrD3DSurfaceInfo GrD3DTextureResourceSpecToSurfaceInfo(const GrD3DTextureResourceSpec& d3dSpec,
                                                       uint32_t sampleCount,
                                                       uint32_t levelCount,
                                                       GrProtected isProtected);

#endif
