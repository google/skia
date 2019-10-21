/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ProxyUtils_DEFINED
#define ProxyUtils_DEFINED

#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrTextureProxy.h"

namespace sk_gpu_test {

/** Makes a texture proxy containing the passed in color data. */
sk_sp<GrTextureProxy> MakeTextureProxyFromData(GrContext*,
                                               GrRenderable,
                                               GrSurfaceOrigin,
                                               const GrImageInfo&,
                                               const void* data,
                                               size_t rowBytes);

}  // namespace sk_gpu_test

#endif
