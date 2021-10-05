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

} // namespace skgpu::mtl

