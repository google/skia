/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureAdjuster.h"

#include "GrContext.h"
#include "GrGpu.h"
#include "GrGpuResourcePriv.h"
#include "GrResourceProvider.h"
#include "GrTexture.h"
#include "SkGrPriv.h"

GrTextureAdjuster::GrTextureAdjuster(GrTexture* original, SkAlphaType alphaType,
                                     const SkIRect& contentArea, uint32_t uniqueID,
                                     SkColorSpace* cs)
    : INHERITED(contentArea.width(), contentArea.height(),
                GrPixelConfigIsAlphaOnly(original->config()))
    , fOriginal(original)
    , fAlphaType(alphaType)
    , fColorSpace(cs)
    , fUniqueID(uniqueID) {
    SkASSERT(SkIRect::MakeWH(original->width(), original->height()).contains(contentArea));
    if (contentArea.fLeft > 0 || contentArea.fTop > 0 ||
        contentArea.fRight < original->width() || contentArea.fBottom < original->height()) {
        fContentArea.set(contentArea);
    }
}

void GrTextureAdjuster::makeCopyKey(const CopyParams& params, GrUniqueKey* copyKey,
                                    SkColorSpace* dstColorSpace) {
    // Destination color space is irrelevant - we already have a texture so we're just sub-setting
    GrUniqueKey baseKey;
    GrMakeKeyFromImageID(&baseKey, fUniqueID, SkIRect::MakeWH(this->width(), this->height()));
    MakeCopyKeyFromOrigKey(baseKey, params, copyKey);
}

void GrTextureAdjuster::didCacheCopy(const GrUniqueKey& copyKey) {
    // We don't currently have a mechanism for notifications on Images!
}

GrTexture* GrTextureAdjuster::refCopy(const CopyParams& copyParams) {
    GrTexture* texture = this->originalTexture();
    GrContext* context = texture->getContext();
    const SkIRect* contentArea = this->contentAreaOrNull();
    GrUniqueKey key;
    this->makeCopyKey(copyParams, &key, nullptr);
    if (key.isValid()) {
        GrTexture* cachedCopy = context->resourceProvider()->findAndRefTextureByUniqueKey(key);
        if (cachedCopy) {
            return cachedCopy;
        }
    }
    GrTexture* copy = CopyOnGpu(texture, contentArea, copyParams);
    if (copy) {
        if (key.isValid()) {
            context->resourceProvider()->assignUniqueKeyToTexture(key, copy);
            this->didCacheCopy(key);
        }
    }
    return copy;
}

GrTexture* GrTextureAdjuster::refTextureSafeForParams(const GrSamplerParams& params,
                                                      SkIPoint* outOffset,
                                                      SkScalar scaleAdjust[2]) {
    GrTexture* texture = this->originalTexture();
    GrContext* context = texture->getContext();
    CopyParams copyParams;
    const SkIRect* contentArea = this->contentAreaOrNull();

    if (!context) {
        // The texture was abandoned.
        return nullptr;
    }

    if (contentArea && GrSamplerParams::kMipMap_FilterMode == params.filterMode()) {
        // If we generate a MIP chain for texture it will read pixel values from outside the content
        // area.
        copyParams.fWidth = contentArea->width();
        copyParams.fHeight = contentArea->height();
        copyParams.fFilter = GrSamplerParams::kBilerp_FilterMode;
    } else if (!context->getGpu()->makeCopyForTextureParams(texture, params, &copyParams,
                                                            scaleAdjust)) {
        if (outOffset) {
            if (contentArea) {
                outOffset->set(contentArea->fLeft, contentArea->fRight);
            } else {
                outOffset->set(0, 0);
            }
        }
        return SkRef(texture);
    }

    GrTexture* copy = this->refCopy(copyParams);
    if (copy && outOffset) {
        outOffset->set(0, 0);
    }
    return copy;
}

sk_sp<GrFragmentProcessor> GrTextureAdjuster::createFragmentProcessor(
                                        const SkMatrix& origTextureMatrix,
                                        const SkRect& origConstraintRect,
                                        FilterConstraint filterConstraint,
                                        bool coordsLimitedToConstraintRect,
                                        const GrSamplerParams::FilterMode* filterOrNullForBicubic,
                                        SkColorSpace* dstColorSpace) {

    SkMatrix textureMatrix = origTextureMatrix;
    const SkIRect* contentArea = this->contentAreaOrNull();
    // Convert the constraintRect to be relative to the texture rather than the content area so
    // that both rects are in the same coordinate system.
    SkTCopyOnFirstWrite<SkRect> constraintRect(origConstraintRect);
    if (contentArea) {
        SkScalar l = SkIntToScalar(contentArea->fLeft);
        SkScalar t = SkIntToScalar(contentArea->fTop);
        constraintRect.writable()->offset(l, t);
        textureMatrix.postTranslate(l, t);
    }

    SkRect domain;
    GrSamplerParams params;
    if (filterOrNullForBicubic) {
        params.setFilterMode(*filterOrNullForBicubic);
    }
    SkScalar scaleAdjust[2] = { 1.0f, 1.0f };
    sk_sp<GrTexture> texture(this->refTextureSafeForParams(params, nullptr, scaleAdjust));
    if (!texture) {
        return nullptr;
    }
    // If we made a copy then we only copied the contentArea, in which case the new texture is all
    // content.
    if (texture.get() != this->originalTexture()) {
        contentArea = nullptr;
        textureMatrix.postScale(scaleAdjust[0], scaleAdjust[1]);
    }

    DomainMode domainMode =
        DetermineDomainMode(*constraintRect, filterConstraint, coordsLimitedToConstraintRect,
                            texture->width(), texture->height(),
                            contentArea, filterOrNullForBicubic,
                            &domain);
    if (kTightCopy_DomainMode == domainMode) {
        // TODO: Copy the texture and adjust the texture matrix (both parts need to consider
        // non-int constraint rect)
        // For now: treat as bilerp and ignore what goes on above level 0.

        // We only expect MIP maps to require a tight copy.
        SkASSERT(filterOrNullForBicubic &&
                 GrSamplerParams::kMipMap_FilterMode == *filterOrNullForBicubic);
        static const GrSamplerParams::FilterMode kBilerp = GrSamplerParams::kBilerp_FilterMode;
        domainMode =
            DetermineDomainMode(*constraintRect, filterConstraint, coordsLimitedToConstraintRect,
                                texture->width(), texture->height(),
                                contentArea, &kBilerp, &domain);
        SkASSERT(kTightCopy_DomainMode != domainMode);
    }
    SkASSERT(kNoDomain_DomainMode == domainMode ||
             (domain.fLeft <= domain.fRight && domain.fTop <= domain.fBottom));
    sk_sp<GrColorSpaceXform> colorSpaceXform = GrColorSpaceXform::Make(fColorSpace,
                                                                       dstColorSpace);
    return CreateFragmentProcessorForDomainAndFilter(texture.get(), std::move(colorSpaceXform),
                                                     textureMatrix, domainMode, domain,
                                                     filterOrNullForBicubic);
}
