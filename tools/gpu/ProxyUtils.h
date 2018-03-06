/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ProxyUtils_DEFINED
#define ProxyUtils_DEFINED

#include "GrTextureProxy.h"
#include "GrTypesPriv.h"

namespace sk_gpu_test {

sk_sp<GrTextureProxy> MakeTextureProxyFromData(GrContext* context, bool isRT, int width, int height, GrColorType, GrSRGBEncoded, GrSurfaceOrigin, const void* data, size_t rowBytes);

sk_sp<GrTextureProxy> MakeTextureProxyFromData(GrContext* context, bool isRT, int width, int height, GrColorType ct, GrSurfaceOrigin origin, const void* data, size_t rowBytes) {
    return MakeTextureProxyFromData(context, isRT, width, height, ct, GrSRGBEncoded::kNo, origin, data, rowBytes);
}

sk_sp<GrTextureProxy> MakeTextureProxyFromData(GrContext* context, bool isRT, int width, int height, SkColorType ct, GrSurfaceOrigin origin, const void* data, size_t rowBytes) {
    return MakeTextureProxyFromData(context, isRT, width, height, SkColorTypeToGrColorType(ct), origin, data, rowBytes);
}

}  // namespace sk_gpu_test

#endif
