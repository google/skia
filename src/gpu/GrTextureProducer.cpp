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

std::unique_ptr<GrFragmentProcessor> GrTextureProducer::createFragmentProcessorForView(
        GrSurfaceProxyView view,
        const SkMatrix& textureMatrix,
        const SkRect* subset,
        const SkRect* domain,
        GrSamplerState samplerState) {
    if (!view) {
        return nullptr;
    }
    SkRect tempSubset;

    if (!subset && !view.proxy()->isFullyLazy() && !view.proxy()->isFunctionallyExact()) {
        tempSubset = view.proxy()->getBoundsRect();
        subset = &tempSubset;
    }

    const auto& caps = *fContext->priv().caps();
    if (subset) {
        if (domain) {
            return GrTextureEffect::MakeSubset(std::move(view), this->alphaType(), textureMatrix,
                                               samplerState, *subset, *domain, caps);
        } else {
            return GrTextureEffect::MakeSubset(std::move(view), this->alphaType(), textureMatrix,
                                               samplerState, *subset, caps);
        }
    } else {
        return GrTextureEffect::Make(std::move(view), this->alphaType(), textureMatrix,
                                     samplerState, caps);
    }
}

std::unique_ptr<GrFragmentProcessor> GrTextureProducer::createBicubicFragmentProcessorForView(
        GrSurfaceProxyView view,
        const SkMatrix& textureMatrix,
        const SkRect* subset,
        const SkRect* domain,
        GrSamplerState::WrapMode wrapX,
        GrSamplerState::WrapMode wrapY,
        SkImage::CubicResampler kernel) {
    if (!view) {
        return nullptr;
    }
    SkRect tempSubset;

    if (!subset && !view.proxy()->isFullyLazy() && !view.proxy()->isFunctionallyExact()) {
        tempSubset = view.proxy()->getBoundsRect();
        subset = &tempSubset;
    }

    const auto& caps = *fContext->priv().caps();
    static constexpr auto kDir = GrBicubicEffect::Direction::kXY;
    if (subset) {
        if (domain) {
            return GrBicubicEffect::MakeSubset(std::move(view), this->alphaType(), textureMatrix,
                                               wrapX, wrapY, *subset, *domain, kernel, kDir, caps);
        } else {
            return GrBicubicEffect::MakeSubset(std::move(view), this->alphaType(), textureMatrix,
                                               wrapX, wrapY, *subset, kernel, kDir, caps);
        }
    } else {
        return GrBicubicEffect::Make(std::move(view), this->alphaType(), textureMatrix, wrapX,
                                     wrapY, kernel, kDir, caps);
    }
}

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
