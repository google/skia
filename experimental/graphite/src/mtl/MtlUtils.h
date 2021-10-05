/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlUtils_DEFINED
#define skgpu_MtlUtils_DEFINED

#import <Metal/Metal.h>

namespace skgpu::mtl {

bool FormatIsDepthOrStencil(MTLPixelFormat);

} // namespace skgpu::mtl

#endif // skgpu_MtlUtils_DEFINED
