/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlUtils.h"

namespace skgpu::mtl {

bool FormatIsDepthOrStencil(MTLPixelFormat format) {
    switch (format) {
        case MTLPixelFormatStencil8: // fallthrough
        case MTLPixelFormatDepth32Float_Stencil8:
            return true;
        default:
            return false;
    }
}

MTLPixelFormat SkColorTypeToFormat(SkColorType colorType) {
    switch (colorType) {
        case (kRGBA_8888_SkColorType):
            return MTLPixelFormatRGBA8Unorm;
        default:
            // TODO: fill in the rest of the formats
            SkUNREACHABLE;
    }
}

MTLPixelFormat DepthStencilTypeToFormat(DepthStencilType type) {
    // TODO: Decide if we want to change this to always return a combined depth and stencil format
    // to allow more sharing of depth stencil allocations.
    switch (type) {
        case DepthStencilType::kDepthOnly:
            // MTLPixelFormatDepth16Unorm is also a universally supported option here
            return MTLPixelFormatDepth32Float;
        case DepthStencilType::kStencilOnly:
            return MTLPixelFormatStencil8;
        case DepthStencilType::kDepthStencil:
            // MTLPixelFormatDepth24Unorm_Stencil8 is supported on Mac family GPUs.
            return MTLPixelFormatDepth32Float_Stencil8;
    }
}

} // namespace skgpu::mtl

