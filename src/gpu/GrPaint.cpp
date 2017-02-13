/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPaint.h"

#include "GrProcOptInfo.h"
#include "effects/GrCoverageSetOpXP.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "effects/GrSimpleTextureEffect.h"

void GrPaint::setCoverageSetOpXPFactory(SkRegion::Op regionOp, bool invertCoverage) {
    fXPFactory = GrCoverageSetOpXPFactory::Get(regionOp, invertCoverage);
}

void GrPaint::addColorTextureProcessor(GrTexture* texture,
                                       sk_sp<GrColorSpaceXform> colorSpaceXform,
                                       const SkMatrix& matrix) {
    this->addColorFragmentProcessor(GrSimpleTextureEffect::Make(texture,
                                                                std::move(colorSpaceXform),
                                                                matrix));
}

void GrPaint::addCoverageTextureProcessor(GrTexture* texture, const SkMatrix& matrix) {
    this->addCoverageFragmentProcessor(GrSimpleTextureEffect::Make(texture, nullptr, matrix));
}

void GrPaint::addColorTextureProcessor(GrTexture* texture,
                                       sk_sp<GrColorSpaceXform> colorSpaceXform,
                                       const SkMatrix& matrix,
                                       const GrSamplerParams& params) {
    this->addColorFragmentProcessor(GrSimpleTextureEffect::Make(texture,
                                                                std::move(colorSpaceXform),
                                                                matrix, params));
}

void GrPaint::addCoverageTextureProcessor(GrTexture* texture,
                                          const SkMatrix& matrix,
                                          const GrSamplerParams& params) {
    this->addCoverageFragmentProcessor(GrSimpleTextureEffect::Make(texture, nullptr, matrix,
                                                                   params));
}

void GrPaint::addColorTextureProcessor(GrContext* ctx, sk_sp<GrTextureProxy> proxy,
                                       sk_sp<GrColorSpaceXform> colorSpaceXform,
                                       const SkMatrix& matrix) {
    this->addColorFragmentProcessor(GrSimpleTextureEffect::Make(ctx, std::move(proxy),
                                                                std::move(colorSpaceXform),
                                                                matrix));
}

void GrPaint::addColorTextureProcessor(GrContext* ctx, sk_sp<GrTextureProxy> proxy,
                                       sk_sp<GrColorSpaceXform> colorSpaceXform,
                                       const SkMatrix& matrix,
                                       const GrSamplerParams& params) {
    this->addColorFragmentProcessor(GrSimpleTextureEffect::Make(ctx,
                                                                std::move(proxy),
                                                                std::move(colorSpaceXform),
                                                                matrix, params));
}

void GrPaint::addCoverageTextureProcessor(GrContext* ctx, sk_sp<GrTextureProxy> proxy,
                                          const SkMatrix& matrix) {
    this->addCoverageFragmentProcessor(GrSimpleTextureEffect::Make(ctx, std::move(proxy),
                                                                   nullptr, matrix));
}

void GrPaint::addCoverageTextureProcessor(GrContext* ctx, sk_sp<GrTextureProxy> proxy,
                                          const SkMatrix& matrix,
                                          const GrSamplerParams& params) {
    this->addCoverageFragmentProcessor(GrSimpleTextureEffect::Make(ctx, std::move(proxy),
                                                                   nullptr, matrix, params));
}

bool GrPaint::internalIsConstantBlendedColor(GrColor paintColor, GrColor* color) const {
    GrProcOptInfo colorProcInfo((GrPipelineInput(paintColor)));
    colorProcInfo.analyzeProcessors(
            sk_sp_address_as_pointer_address(fColorFragmentProcessors.begin()),
            this->numColorFragmentProcessors());

    return GrXPFactory::IsPreCoverageBlendedColorConstant(fXPFactory, colorProcInfo, color);
}
