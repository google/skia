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
                                             const SkMatrix& m)
    : fCoordTransform(m, texture, GrSamplerParams::kNone_FilterMode)
    , fTextureSampler(texture)
    , fColorSpaceXform(std::move(colorSpaceXform)) {
    this->addCoordTransform(&fCoordTransform);
    this->addTextureSampler(&fTextureSampler);
}

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture,
                                             sk_sp<GrColorSpaceXform> colorSpaceXform,
                                             const SkMatrix& m,
                                             GrSamplerParams::FilterMode filterMode)
    : fCoordTransform(m, texture, filterMode)
    , fTextureSampler(texture, filterMode)
    , fColorSpaceXform(std::move(colorSpaceXform)) {
    this->addCoordTransform(&fCoordTransform);
    this->addTextureSampler(&fTextureSampler);
}

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture,
                                             sk_sp<GrColorSpaceXform> colorSpaceXform,
                                             const SkMatrix& m,
                                             const GrSamplerParams& params)
    : fCoordTransform(m, texture, params.filterMode())
    , fTextureSampler(texture, params)
    , fColorSpaceXform(std::move(colorSpaceXform)) {
    this->addCoordTransform(&fCoordTransform);
    this->addTextureSampler(&fTextureSampler);
}

GrSingleTextureEffect::GrSingleTextureEffect(GrContext* ctx, sk_sp<GrTextureProxy> proxy,
                                             sk_sp<GrColorSpaceXform> colorSpaceXform,
                                             const SkMatrix& m)
    : fCoordTransform(ctx, m, proxy.get(), GrSamplerParams::kNone_FilterMode)
    , fTextureSampler(ctx->textureProvider(), std::move(proxy))
    , fColorSpaceXform(std::move(colorSpaceXform)) {
    this->addCoordTransform(&fCoordTransform);
    this->addTextureSampler(&fTextureSampler);
}

GrSingleTextureEffect::GrSingleTextureEffect(GrContext* ctx, sk_sp<GrTextureProxy> proxy,
                                             sk_sp<GrColorSpaceXform> colorSpaceXform,
                                             const SkMatrix& m,
                                             GrSamplerParams::FilterMode filterMode)
    : fCoordTransform(ctx, m, proxy.get(), filterMode)
    , fTextureSampler(ctx->textureProvider(), std::move(proxy), filterMode)
    , fColorSpaceXform(std::move(colorSpaceXform)) {
    this->addCoordTransform(&fCoordTransform);
    this->addTextureSampler(&fTextureSampler);
}

GrSingleTextureEffect::GrSingleTextureEffect(GrContext* ctx, sk_sp<GrTextureProxy> proxy,
                                             sk_sp<GrColorSpaceXform> colorSpaceXform,
                                             const SkMatrix& m,
                                             const GrSamplerParams& params)
    : fCoordTransform(ctx, m, proxy.get(), params.filterMode())
    , fTextureSampler(ctx->textureProvider(), std::move(proxy), params)
    , fColorSpaceXform(std::move(colorSpaceXform)) {
    this->addCoordTransform(&fCoordTransform);
    this->addTextureSampler(&fTextureSampler);
}

GrSingleTextureEffect::~GrSingleTextureEffect() {
}
