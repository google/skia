/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrSingleTextureEffect.h"

#include "GrContext.h"
#include "GrTextureProxy.h"

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture,
                                             sk_sp<GrColorSpaceXform> colorSpaceXform,
                                             const SkMatrix& m, OptimizationFlags optFlags)
        : INHERITED(optFlags)
        , fCoordTransform(m, texture, GrSamplerParams::kNone_FilterMode)
        , fTextureSampler(texture)
        , fColorSpaceXform(std::move(colorSpaceXform)) {
    this->addCoordTransform(&fCoordTransform);
    this->addTextureSampler(&fTextureSampler);
}

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture,
                                             sk_sp<GrColorSpaceXform> colorSpaceXform,
                                             const SkMatrix& m,
                                             GrSamplerParams::FilterMode filterMode,
                                             OptimizationFlags optFlags)
        : INHERITED(optFlags)
        , fCoordTransform(m, texture, filterMode)
        , fTextureSampler(texture, filterMode)
        , fColorSpaceXform(std::move(colorSpaceXform)) {
    this->addCoordTransform(&fCoordTransform);
    this->addTextureSampler(&fTextureSampler);
}

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture,
                                             sk_sp<GrColorSpaceXform> colorSpaceXform,
                                             const SkMatrix& m, const GrSamplerParams& params,
                                             OptimizationFlags optFlags)
        : INHERITED(optFlags)
        , fCoordTransform(m, texture, params.filterMode())
        , fTextureSampler(texture, params)
        , fColorSpaceXform(std::move(colorSpaceXform)) {
    this->addCoordTransform(&fCoordTransform);
    this->addTextureSampler(&fTextureSampler);
}

GrSingleTextureEffect::GrSingleTextureEffect(GrContext* ctx, OptimizationFlags optFlags,
                                             sk_sp<GrTextureProxy> proxy,
                                             sk_sp<GrColorSpaceXform> colorSpaceXform,
                                             const SkMatrix& m)
        : INHERITED(optFlags)
        , fCoordTransform(ctx, m, proxy.get(), GrSamplerParams::kNone_FilterMode)
        , fTextureSampler(ctx->resourceProvider(), std::move(proxy))
        , fColorSpaceXform(std::move(colorSpaceXform)) {
    this->addCoordTransform(&fCoordTransform);
    this->addTextureSampler(&fTextureSampler);
}

GrSingleTextureEffect::GrSingleTextureEffect(GrContext* ctx, OptimizationFlags optFlags,
                                             sk_sp<GrTextureProxy> proxy,
                                             sk_sp<GrColorSpaceXform> colorSpaceXform,
                                             const SkMatrix& m,
                                             GrSamplerParams::FilterMode filterMode)
        : INHERITED(optFlags)
        , fCoordTransform(ctx, m, proxy.get(), filterMode)
        , fTextureSampler(ctx->resourceProvider(), std::move(proxy), filterMode)
        , fColorSpaceXform(std::move(colorSpaceXform)) {
    this->addCoordTransform(&fCoordTransform);
    this->addTextureSampler(&fTextureSampler);
}

GrSingleTextureEffect::GrSingleTextureEffect(GrContext* ctx, OptimizationFlags optFlags,
                                             sk_sp<GrTextureProxy> proxy,
                                             sk_sp<GrColorSpaceXform> colorSpaceXform,
                                             const SkMatrix& m, const GrSamplerParams& params)
        : INHERITED(optFlags)
        , fCoordTransform(ctx, m, proxy.get(), params.filterMode())
        , fTextureSampler(ctx->resourceProvider(), std::move(proxy), params)
        , fColorSpaceXform(std::move(colorSpaceXform)) {
    this->addCoordTransform(&fCoordTransform);
    this->addTextureSampler(&fTextureSampler);
}
