/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlGraphiteUtilsPriv_DEFINED
#define skgpu_graphite_MtlGraphiteUtilsPriv_DEFINED

#include "include/ports/SkCFObject.h"
#include "src/gpu/graphite/ResourceTypes.h"

#import <Metal/Metal.h>

namespace skgpu {
class ShaderErrorHandler;
}

namespace skgpu::graphite {
class MtlSharedContext;

MTLPixelFormat MtlDepthStencilFlagsToFormat(SkEnumBitMask<DepthStencilFlags>);

sk_cfp<id<MTLLibrary>> MtlCompileShaderLibrary(const MtlSharedContext* sharedContext,
                                               const std::string& msl,
                                               ShaderErrorHandler* errorHandler);
} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlGraphiteUtilsPriv_DEFINED
