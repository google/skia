/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrImageTextureMaker.h"

#include "src/gpu/GrColorSpaceXform.h"
#include "src/gpu/GrImageContextPriv.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrBicubicEffect.h"
#include "src/gpu/effects/GrYUVtoRGBEffect.h"
#include "src/image/SkImage_GpuYUVA.h"
#include "src/image/SkImage_Lazy.h"

static GrImageInfo get_image_info(GrRecordingContext* context, const SkImage* client) {
    SkASSERT(client->isLazyGenerated());
    const SkImage_Lazy* lazyImage = static_cast<const SkImage_Lazy*>(client);

    GrColorType ct = lazyImage->colorTypeOfLockTextureProxy(context->priv().caps());

    return {ct, client->alphaType(), client->refColorSpace(), client->dimensions()};
}

GrImageTextureMaker::GrImageTextureMaker(GrRecordingContext* context,
                                         const SkImage* client,
                                         GrImageTexGenPolicy texGenPolicy)
        : INHERITED(context, get_image_info(context, client))
        , fImage(static_cast<const SkImage_Lazy*>(client))
        , fTexGenPolicy(texGenPolicy) {
    SkASSERT(client->isLazyGenerated());
}

GrSurfaceProxyView GrImageTextureMaker::refOriginalTextureProxyView(GrMipmapped mipMapped) {
    return fImage->lockTextureProxyView(this->context(), fTexGenPolicy, mipMapped);
}
