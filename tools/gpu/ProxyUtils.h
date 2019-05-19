/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ProxyUtils_DEFINED
#define ProxyUtils_DEFINED

#include "include/private/GrTextureProxy.h"
#include "include/private/GrTypesPriv.h"

namespace sk_gpu_test {

/** Makes a texture proxy containing the passed in color data. */
sk_sp<GrTextureProxy> MakeTextureProxyFromData(GrContext*, GrRenderable, int width, int height,
                                               GrColorType, GrSRGBEncoded, GrSurfaceOrigin,
                                               const void* data, size_t rowBytes);

/** Version that takes GrColorType rather than SkColorType and assumes GrSRGBEncoded::kNo. */
inline sk_sp<GrTextureProxy> MakeTextureProxyFromData(GrContext* context,
                                                      GrRenderable renderable, int width,
                                                      int height, GrColorType grCT,
                                                      GrSurfaceOrigin origin, const void* data,
                                                      size_t rowBytes) {
    return MakeTextureProxyFromData(context, renderable, width, height,
                                    grCT, GrSRGBEncoded::kNo, origin, data, rowBytes);
}

/** Version that takes SkColorType rather than GrColorType and assumes GrSRGBEncoded::kNo. */
inline sk_sp<GrTextureProxy> MakeTextureProxyFromData(GrContext* context,
                                                      GrRenderable renderable, int width,
                                                      int height, SkColorType ct,
                                                      GrSurfaceOrigin origin, const void* data,
                                                      size_t rowBytes) {
    GrColorType grCT = SkColorTypeToGrColorType(ct);
    if (GrColorType::kUnknown == grCT) {
        return nullptr;
    }

    return MakeTextureProxyFromData(context, renderable, width, height,
                                    grCT, GrSRGBEncoded::kNo, origin, data, rowBytes);
}

}  // namespace sk_gpu_test

#endif
