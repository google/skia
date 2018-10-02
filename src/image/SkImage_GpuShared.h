/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_GpuShared_DEFINED
#define SkImage_GpuShared_DEFINED

#include "GrContext.h"
#include "GrTextureAdjuster.h"
#include "SkColorSpace.h"
#include "SkImage_Base.h"

namespace SkImage_GpuShared {

bool ValidateBackendTexture(GrContext* ctx, const GrBackendTexture& tex,
                            GrPixelConfig* config, SkColorType ct, SkAlphaType at,
                            sk_sp<SkColorSpace> cs);

sk_sp<GrTextureProxy> AsTextureProxyRef(GrContext* context,
                                        const GrSamplerState& params,
                                        SkColorSpace* dstColorSpace,
                                        sk_sp<SkColorSpace>* texColorSpace,
                                        SkScalar scaleAdjust[2],
                                        GrContext* imageContext,
                                        const SkImage_Base* image,
                                        const SkAlphaType imageAlphaType,
                                        SkColorSpace* imageColorSpace);

sk_sp<SkImage> OnMakeSubset(const SkIRect& subset, sk_sp<GrContext> imageContext,
                            const SkImage_Base* image, const SkAlphaType imageAlphaType,
                            sk_sp<SkColorSpace> imageColorSpace, const SkBudgeted budgeted);
}
#endif
