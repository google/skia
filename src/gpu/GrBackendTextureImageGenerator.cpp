/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBackendTextureImageGenerator.h"

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrResourceProvider.h"

#include "SkGr.h"

void GrBackendTextureImageGenerator::RefHelper::ref() {
    SkASSERT(fRefCnt > 0);
    ++fRefCnt;
}

void GrBackendTextureImageGenerator::RefHelper::unref() {
    SkASSERT(fRefCnt > 0);
    --fRefCnt;
    if (0 == fRefCnt) {
        fReleaseProc(fReleaseCtx);
        delete this;
    }
}

std::unique_ptr<SkImageGenerator>
GrBackendTextureImageGenerator::Make(const GrBackendTexture& backendTex, GrSurfaceOrigin origin,
                                     SkAlphaType alphaType, sk_sp<SkColorSpace> colorSpace,
                                     ReleaseProc releaseProc, ReleaseCtx releaseCtx) {
    if (colorSpace && (!colorSpace->gammaCloseToSRGB() && !colorSpace->gammaIsLinear())) {
        return nullptr;
    }

    SkColorType colorType = kUnknown_SkColorType;
    if (!GrPixelConfigToColorType(backendTex.config(), &colorType)) {
        return nullptr;
    }

    SkImageInfo info = SkImageInfo::Make(backendTex.width(), backendTex.height(), colorType,
                                         alphaType, std::move(colorSpace));
    return std::unique_ptr<SkImageGenerator>(
            new GrBackendTextureImageGenerator(info, backendTex, origin, releaseProc, releaseCtx));
}

GrBackendTextureImageGenerator::GrBackendTextureImageGenerator(const SkImageInfo& info,
                                                               const GrBackendTexture& backendTex,
                                                               GrSurfaceOrigin origin,
                                                               ReleaseProc releaseProc,
                                                               ReleaseCtx releaseCtx)
        : INHERITED(info)
        , fTexture(backendTex)
        , fSurfaceOrigin(origin) {
    if (releaseProc) {
        fRefHelper = new RefHelper(releaseProc, releaseCtx);
    } else {
        fRefHelper = nullptr;
    }
}

GrBackendTextureImageGenerator::~GrBackendTextureImageGenerator() {
    fRefHelper->unref();
}

bool GrBackendTextureImageGenerator::onGetPixels(const SkImageInfo& info, void* pixels,
                                                 size_t rowBytes, SkPMColor ctable[],
                                                 int* ctableCount) {
    // TODO: Is there any way to implement this? I don't think so.
    return false;
}

bool GrBackendTextureImageGenerator::onGetPixels(const SkImageInfo& info, void* pixels,
                                                 size_t rowBytes, const Options& opts) {

    // TODO: Is there any way to implement this? I don't think so.
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<SkImageGenerator>
SkImageGenerator::MakeFromBackendTexture(const GrBackendTexture& backendTex, GrSurfaceOrigin origin,
                                         SkAlphaType alphaType, sk_sp<SkColorSpace> colorSpace,
                                         ReleaseProc releaseProc, ReleaseCtx releaseCtx) {
    return GrBackendTextureImageGenerator::Make(backendTex, origin, alphaType,
                                                std::move(colorSpace), releaseProc, releaseCtx);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU
void GrBackendTextureImageGenerator::ReleaseRefHelper_TextureReleaseProc(ReleaseCtx ctx) {
    RefHelper* refHelper = static_cast<RefHelper*>(ctx);
    SkASSERT(refHelper);
    refHelper->unref();
}

sk_sp<GrTextureProxy> GrBackendTextureImageGenerator::onGenerateTexture(GrContext* ctx,
                                                                        const SkImageInfo& info,
                                                                        const SkIPoint& origin) {
    SkASSERT(ctx);

    if (ctx->contextPriv().getBackend() != fTexture.backend()) {
        return nullptr;
    }

    // TODO: Use GrTextureAdjuster (or similar) to repsect incoming info/origin?

    sk_sp<GrTexture> tex = ctx->resourceProvider()->wrapBackendTexture(
            fTexture, fSurfaceOrigin, kNone_GrBackendTextureFlag, 0, kBorrow_GrWrapOwnership);
    if (!tex) {
        return nullptr;
    }

    if (fRefHelper) {
        tex->setRelease(ReleaseRefHelper_TextureReleaseProc, fRefHelper);
        fRefHelper->ref();
    }

    return GrSurfaceProxy::MakeWrapped(std::move(tex));
}
#endif
