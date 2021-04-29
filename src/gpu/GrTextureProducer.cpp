/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrTextureProducer.h"

#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkRectPriv.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrBicubicEffect.h"
#include "src/gpu/effects/GrTextureEffect.h"

GrSurfaceProxyView GrTextureProducer::view(GrMipmapped mipMapped) {
    const GrCaps* caps = this->context()->priv().caps();
    // Sanitize the MIP map request.
    if (mipMapped == GrMipmapped::kYes) {
        if ((this->width() == 1 && this->height() == 1) || !caps->mipmapSupport()) {
            mipMapped = GrMipmapped::kNo;
        }
    }
    auto result = this->onView(mipMapped);
    // Check to make sure if we requested MIPs that the returned texture has MIP maps or the format
    // is not copyable.
    SkASSERT(!result || mipMapped == GrMipmapped::kNo ||
             result.asTextureProxy()->mipmapped() == GrMipmapped::kYes ||
             !caps->isFormatCopyable(result.proxy()->backendFormat()));
    return result;
}
