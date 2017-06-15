/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrSingleTextureEffect.h"

#include "GrTextureProxy.h"

GrSingleTextureEffect::GrSingleTextureEffect(OptimizationFlags optFlags,
                                             sk_sp<GrTextureProxy> proxy,
                                             sk_sp<GrColorSpaceXform> colorSpaceXform,
                                             const SkMatrix& m)
        : INHERITED(optFlags)
        , fCoordTransform(m, proxy.get())
        , fTextureSampler(std::move(proxy))
        , fColorSpaceXform(std::move(colorSpaceXform)) {
    this->addCoordTransform(&fCoordTransform);
    this->addTextureSampler(&fTextureSampler);
}

GrSingleTextureEffect::GrSingleTextureEffect(OptimizationFlags optFlags,
                                             sk_sp<GrTextureProxy> proxy,
                                             sk_sp<GrColorSpaceXform> colorSpaceXform,
                                             const SkMatrix& m,
                                             GrSamplerParams::FilterMode filterMode)
        : INHERITED(optFlags)
        , fCoordTransform(m, proxy.get())
        , fTextureSampler(std::move(proxy), filterMode)
        , fColorSpaceXform(std::move(colorSpaceXform)) {
    this->addCoordTransform(&fCoordTransform);
    this->addTextureSampler(&fTextureSampler);
}

GrSingleTextureEffect::GrSingleTextureEffect(OptimizationFlags optFlags,
                                             sk_sp<GrTextureProxy> proxy,
                                             sk_sp<GrColorSpaceXform> colorSpaceXform,
                                             const SkMatrix& m, const GrSamplerParams& params)
        : INHERITED(optFlags)
        , fCoordTransform(m, proxy.get())
        , fTextureSampler(std::move(proxy), params)
        , fColorSpaceXform(std::move(colorSpaceXform)) {
    this->addCoordTransform(&fCoordTransform);
    this->addTextureSampler(&fTextureSampler);
}
