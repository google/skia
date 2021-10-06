/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlUtils_DEFINED
#define skgpu_MtlUtils_DEFINED

#include "experimental/graphite/include/private/GraphiteTypesPriv.h"
#include "include/core/SkImageInfo.h"
#include "include/ports/SkCFObject.h"

#import <Metal/Metal.h>

namespace SkSL {
class String;
}

namespace skgpu::mtl {

class Gpu;

bool FormatIsDepthOrStencil(MTLPixelFormat);

MTLPixelFormat SkColorTypeToFormat(SkColorType);

MTLPixelFormat DepthStencilTypeToFormat(DepthStencilType);

sk_cfp<id<MTLLibrary>> CompileShaderLibrary(const Gpu* gpu,
                                            const SkSL::String& msl);

} // namespace skgpu::mtl

#endif // skgpu_MtlUtils_DEFINED
