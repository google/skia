/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureProducer.h"
#include "GrClip.h"
#include "GrRenderTargetContext.h"
#include "GrResourceProvider.h"
#include "GrSurfaceProxy.h"
#include "GrSurfaceProxyPriv.h"
#include "GrTexture.h"
#include "effects/GrBicubicEffect.h"
#include "effects/GrSimpleTextureEffect.h"
#include "effects/GrTextureDomain.h"

GrTexture* GrTextureProducer::CopyOnGpu(GrTexture* inputTexture, const SkIRect* subset,
                                        const CopyParams& copyParams) {
    SkASSERT(!subset || !subset->isEmpty());
    GrContext* context = inputTexture->getContext();
    SkASSERT(context);

    GrPixelConfig config = GrMakePixelConfigUncompressed(inputTexture->config());

    sk_sp<GrRenderTargetContext> copyRTC = context->makeRenderTargetContextWithFallback(
        SkBackingFit::kExact, copyParams.fWidth, copyParams.fHeight, config, nullptr);
    if (!copyRTC) {
        return nullptr;
    }

    GrPaint paint;
    paint.setGammaCorrect(true);

    if (copyParams.fFilter != GrSamplerParams::kNone_FilterMode && subset &&
        (subset->width() != copyParams.fWidth || subset->height() != copyParams.fHeight)) {
        SkRect domain;
        domain.fLeft = subset->fLeft + 0.5f;
        domain.fTop = subset->fTop + 0.5f;
        domain.fRight = subset->fRight - 0.5f;
        domain.fBottom = subset->fBottom - 0.5f;
        // This would cause us to read values from outside the subset. Surely, the caller knows
        // better!
        SkASSERT(copyParams.fFilter != GrSamplerParams::kMipMap_FilterMode);
        paint.addColorFragmentProcessor(
            GrTextureDomainEffect::Make(inputTexture, nullptr, SkMatrix::I(), domain,
                                        GrTextureDomain::kClamp_Mode,
                                        copyParams.fFilter));
    } else {
        GrSamplerParams params(SkShader::kClamp_TileMode, copyParams.fFilter);
        paint.addColorTextureProcessor(inputTexture, nullptr, SkMatrix::I(), params);
    }
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

    SkRect localRect;
    if (subset) {
        localRect = SkRect::Make(*subset);
    } else {
        localRect = SkRect::MakeWH(inputTexture->width(), inputTexture->height());
    }

    SkRect dstRect = SkRect::MakeIWH(copyParams.fWidth, copyParams.fHeight);
    copyRTC->fillRectToRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), dstRect,
                            localRect);
    return copyRTC->asTexture().release();
}

sk_sp<GrTextureProxy> GrTextureProducer::CopyOnGpu(GrContext* context,
                                                   sk_sp<GrTextureProxy> inputProxy,
                                                   const SkIRect* subset,
                                                   const CopyParams& copyParams) {
    SkASSERT(!subset || !subset->isEmpty());
    SkASSERT(context);

    GrPixelConfig config = GrMakePixelConfigUncompressed(inputProxy->config());

    const SkRect dstRect = SkRect::MakeIWH(copyParams.fWidth, copyParams.fHeight);

    sk_sp<GrRenderTargetContext> copyRTC = context->makeRenderTargetContextWithFallback(
        SkBackingFit::kExact, dstRect.width(), dstRect.height(), config, nullptr);
    if (!copyRTC) {
        return nullptr;
    }

    GrPaint paint;
    paint.setGammaCorrect(true);

    SkRect localRect;
    if (subset) {
        localRect = SkRect::Make(*subset);
    } else {
        localRect = SkRect::MakeWH(inputProxy->width(), inputProxy->height());
    }

    bool needsDomain = false;
    if (copyParams.fFilter != GrSamplerParams::kNone_FilterMode) {
        bool resizing = localRect.width()  != dstRect.width() ||
                        localRect.height() != dstRect.height();

        if (GrResourceProvider::IsFunctionallyExact(inputProxy.get())) {
            needsDomain = subset && resizing;
        } else {
            needsDomain = resizing;
        }
    }

    if (needsDomain) {
        const SkRect domain = localRect.makeInset(0.5f, 0.5f);
        // This would cause us to read values from outside the subset. Surely, the caller knows
        // better!
        SkASSERT(copyParams.fFilter != GrSamplerParams::kMipMap_FilterMode);
        paint.addColorFragmentProcessor(
            GrTextureDomainEffect::Make(context->resourceProvider(), std::move(inputProxy),
                                        nullptr, SkMatrix::I(),
                                        domain, GrTextureDomain::kClamp_Mode, copyParams.fFilter));
    } else {
        GrSamplerParams params(SkShader::kClamp_TileMode, copyParams.fFilter);
        paint.addColorTextureProcessor(context->resourceProvider(), std::move(inputProxy),
                                       nullptr, SkMatrix::I(), params);
    }
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

    copyRTC->fillRectToRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), dstRect,
                            localRect);
    return copyRTC->asTextureProxyRef();
}

/** Determines whether a texture domain is necessary and if so what domain to use. There are two
 *  rectangles to consider:
 *  - The first is the content area specified by the texture adjuster. We can *never* allow
 *    filtering to cause bleed of pixels outside this rectangle.
 *  - The second rectangle is the constraint rectangle, which is known to be contained by the
 *    content area. The filterConstraint specifies whether we are allowed to bleed across this
 *    rect.
 *
 *  We want to avoid using a domain if possible. We consider the above rectangles, the filter type,
 *  and whether the coords generated by the draw would all fall within the constraint rect. If the
 *  latter is true we only need to consider whether the filter would extend beyond the rects.
 */
GrTextureProducer::DomainMode GrTextureProducer::DetermineDomainMode(
                                    const SkRect& constraintRect,
                                    FilterConstraint filterConstraint,
                                    bool coordsLimitedToConstraintRect,
                                    int texW, int texH,
                                    const SkIRect* textureContentArea,
                                    const GrSamplerParams::FilterMode* filterModeOrNullForBicubic,
                                    SkRect* domainRect) {

    SkASSERT(SkRect::MakeIWH(texW, texH).contains(constraintRect));
    // We only expect a content area rect if there is some non-content area.
    SkASSERT(!textureContentArea ||
             (!textureContentArea->contains(SkIRect::MakeWH(texW, texH)) &&
              SkRect::Make(*textureContentArea).contains(constraintRect)));

    SkRect textureBounds = SkRect::MakeIWH(texW, texH);
    // If the src rectangle contains the whole texture then no need for a domain.
    if (constraintRect.contains(textureBounds)) {
        return kNoDomain_DomainMode;
    }

    bool restrictFilterToRect = (filterConstraint == GrTextureProducer::kYes_FilterConstraint);

    // If we can filter outside the constraint rect, and there is no non-content area of the
    // texture, and we aren't going to generate sample coords outside the constraint rect then we
    // don't need a domain.
    if (!restrictFilterToRect && !textureContentArea && coordsLimitedToConstraintRect) {
        return kNoDomain_DomainMode;
    }

    // Get the domain inset based on sampling mode (or bail if mipped)
    SkScalar filterHalfWidth = 0.f;
    if (filterModeOrNullForBicubic) {
        switch (*filterModeOrNullForBicubic) {
            case GrSamplerParams::kNone_FilterMode:
                if (coordsLimitedToConstraintRect) {
                    return kNoDomain_DomainMode;
                } else {
                    filterHalfWidth = 0.f;
                }
                break;
            case GrSamplerParams::kBilerp_FilterMode:
                filterHalfWidth = .5f;
                break;
            case GrSamplerParams::kMipMap_FilterMode:
                if (restrictFilterToRect || textureContentArea) {
                    // No domain can save us here.
                    return kTightCopy_DomainMode;
                }
                return kNoDomain_DomainMode;
        }
    } else {
        // bicubic does nearest filtering internally.
        filterHalfWidth = 1.5f;
    }

    // Both bilerp and bicubic use bilinear filtering and so need to be clamped to the center
    // of the edge texel. Pinning to the texel center has no impact on nearest mode and MIP-maps

    static const SkScalar kDomainInset = 0.5f;
    // Figure out the limits of pixels we're allowed to sample from.
    // Unless we know the amount of outset and the texture matrix we have to conservatively enforce
    // the domain.
    if (restrictFilterToRect) {
        *domainRect = constraintRect.makeInset(kDomainInset, kDomainInset);
    } else if (textureContentArea) {
        // If we got here then: there is a textureContentArea, the coords are limited to the
        // constraint rect, and we're allowed to filter across the constraint rect boundary. So
        // we check whether the filter would reach across the edge of the content area.
        // We will only set the sides that are required.

        domainRect->setLargest();
        if (coordsLimitedToConstraintRect) {
            // We may be able to use the fact that the texture coords are limited to the constraint
            // rect in order to avoid having to add a domain.
            bool needContentAreaConstraint = false;
            if (textureContentArea->fLeft > 0 &&
                textureContentArea->fLeft + filterHalfWidth > constraintRect.fLeft) {
                domainRect->fLeft = textureContentArea->fLeft + kDomainInset;
                needContentAreaConstraint = true;
            }
            if (textureContentArea->fTop > 0 &&
                textureContentArea->fTop + filterHalfWidth > constraintRect.fTop) {
                domainRect->fTop = textureContentArea->fTop + kDomainInset;
                needContentAreaConstraint = true;
            }
            if (textureContentArea->fRight < texW &&
                textureContentArea->fRight - filterHalfWidth < constraintRect.fRight) {
                domainRect->fRight = textureContentArea->fRight - kDomainInset;
                needContentAreaConstraint = true;
            }
            if (textureContentArea->fBottom < texH &&
                textureContentArea->fBottom - filterHalfWidth < constraintRect.fBottom) {
                domainRect->fBottom = textureContentArea->fBottom - kDomainInset;
                needContentAreaConstraint = true;
            }
            if (!needContentAreaConstraint) {
                return kNoDomain_DomainMode;
            }
        } else {
            // Our sample coords for the texture are allowed to be outside the constraintRect so we
            // don't consider it when computing the domain.
            if (textureContentArea->fLeft != 0) {
                domainRect->fLeft = textureContentArea->fLeft + kDomainInset;
            }
            if (textureContentArea->fTop != 0) {
                domainRect->fTop = textureContentArea->fTop + kDomainInset;
            }
            if (textureContentArea->fRight != texW) {
                domainRect->fRight = textureContentArea->fRight - kDomainInset;
            }
            if (textureContentArea->fBottom != texH) {
                domainRect->fBottom = textureContentArea->fBottom - kDomainInset;
            }
        }
    } else {
        return kNoDomain_DomainMode;
    }

    if (domainRect->fLeft > domainRect->fRight) {
        domainRect->fLeft = domainRect->fRight = SkScalarAve(domainRect->fLeft, domainRect->fRight);
    }
    if (domainRect->fTop > domainRect->fBottom) {
        domainRect->fTop = domainRect->fBottom = SkScalarAve(domainRect->fTop, domainRect->fBottom);
    }
    return kDomain_DomainMode;
}

sk_sp<GrFragmentProcessor> GrTextureProducer::CreateFragmentProcessorForDomainAndFilter(
                                        GrTexture* texture,
                                        sk_sp<GrColorSpaceXform> colorSpaceXform,
                                        const SkMatrix& textureMatrix,
                                        DomainMode domainMode,
                                        const SkRect& domain,
                                        const GrSamplerParams::FilterMode* filterOrNullForBicubic) {
    SkASSERT(kTightCopy_DomainMode != domainMode);
    if (filterOrNullForBicubic) {
        if (kDomain_DomainMode == domainMode) {
            return GrTextureDomainEffect::Make(texture, std::move(colorSpaceXform), textureMatrix,
                                               domain, GrTextureDomain::kClamp_Mode,
                                               *filterOrNullForBicubic);
        } else {
            GrSamplerParams params(SkShader::kClamp_TileMode, *filterOrNullForBicubic);
            return GrSimpleTextureEffect::Make(texture, std::move(colorSpaceXform), textureMatrix,
                                               params);
        }
    } else {
        if (kDomain_DomainMode == domainMode) {
            return GrBicubicEffect::Make(texture, std::move(colorSpaceXform), textureMatrix,
                                         domain);
        } else {
            static const SkShader::TileMode kClampClamp[] =
                { SkShader::kClamp_TileMode, SkShader::kClamp_TileMode };
            return GrBicubicEffect::Make(texture, std::move(colorSpaceXform), textureMatrix,
                                         kClampClamp);
        }
    }
}

sk_sp<GrFragmentProcessor> GrTextureProducer::CreateFragmentProcessorForDomainAndFilter(
                                        GrResourceProvider* resourceProvider,
                                        sk_sp<GrTextureProxy> proxy,
                                        sk_sp<GrColorSpaceXform> colorSpaceXform,
                                        const SkMatrix& textureMatrix,
                                        DomainMode domainMode,
                                        const SkRect& domain,
                                        const GrSamplerParams::FilterMode* filterOrNullForBicubic) {
    SkASSERT(kTightCopy_DomainMode != domainMode);
    if (filterOrNullForBicubic) {
        if (kDomain_DomainMode == domainMode) {
            return GrTextureDomainEffect::Make(resourceProvider, std::move(proxy),
                                               std::move(colorSpaceXform), textureMatrix,
                                               domain, GrTextureDomain::kClamp_Mode,
                                               *filterOrNullForBicubic);
        } else {
            GrSamplerParams params(SkShader::kClamp_TileMode, *filterOrNullForBicubic);
            return GrSimpleTextureEffect::Make(resourceProvider, std::move(proxy),
                                               std::move(colorSpaceXform), textureMatrix,
                                               params);
        }
    } else {
        if (kDomain_DomainMode == domainMode) {
            return GrBicubicEffect::Make(resourceProvider, std::move(proxy),
                                         std::move(colorSpaceXform),
                                         textureMatrix, domain);
        } else {
            static const SkShader::TileMode kClampClamp[] =
                { SkShader::kClamp_TileMode, SkShader::kClamp_TileMode };
            return GrBicubicEffect::Make(resourceProvider, std::move(proxy),
                                         std::move(colorSpaceXform),
                                         textureMatrix, kClampClamp);
        }
    }
}
