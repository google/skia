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
        , fTextureProxy(nullptr)
        , fOwningContextID(SK_InvalidGenID)
        , fBackendTexture(backendTex)
        , fSurfaceOrigin(origin)
        , fReleaseProc(releaseProc)
        , fReleaseCtx(releaseCtx) { }

GrBackendTextureImageGenerator::~GrBackendTextureImageGenerator() {
    if (fReleaseProc) {
        fReleaseProc(fReleaseCtx);
    }
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

#if SK_SUPPORT_GPU
sk_sp<GrTextureProxy> GrBackendTextureImageGenerator::onGenerateTexture(GrContext* ctx,
                                                                        const SkImageInfo& info,
                                                                        const SkIPoint& origin) {
    SkASSERT(ctx);

    // TODO: Use GrTextureAdjuster (or similar) to repsect incoming info/origin?

    if (ctx->uniqueID() == fOwningContextID) {
        return fTextureProxy;
    }

    if (ctx->contextPriv().getBackend() != fBackendTexture.backend()) {
        return nullptr;
    }

    uint32_t uninitializedID = SK_InvalidGenID;
    if (fOwningContextID.compare_exchange(&uninitializedID, ctx->uniqueID())) {
        // Ownership of this generator is now granted to ctx
        sk_sp<GrTexture> tex = ctx->resourceProvider()->wrapBackendTexture(
                fBackendTexture, fSurfaceOrigin, kNone_GrBackendTextureFlag, 0);
        if (!tex) {
            return nullptr;
        }

        // Once we create a texture, we transfer the responsibility of calling the release proc.
        if (fReleaseProc) {
            tex->setRelease(fReleaseProc, fReleaseCtx);
            fReleaseProc = nullptr;
        }

        fTextureProxy = GrSurfaceProxy::MakeWrapped(std::move(tex));
        return fTextureProxy;
    } else {
        // Some other context got here first
        return nullptr;
    }
}
#endif
