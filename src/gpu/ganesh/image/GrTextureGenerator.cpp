/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImageGenerator.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/GrExternalTextureGenerator.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrTextureGenerator.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include <cstdint>
#include <memory>
#include <utility>

enum class GrImageTexGenPolicy : int;

namespace skgpu {
enum class Mipmapped : bool;
}

static void dispose_external_texture(void *context) {
    // Reify the unique_ptr so that we delete the `GrExternalTexture` at the end of scope.
    auto texture = std::unique_ptr<GrExternalTexture>(reinterpret_cast<GrExternalTexture *>(context));
    texture->dispose();
}

GrTextureGenerator::GrTextureGenerator(const SkImageInfo& info, uint32_t uniqueID)
        : SkImageGenerator(info, uniqueID) {}

GrSurfaceProxyView GrTextureGenerator::generateTexture(GrRecordingContext* ctx,
                                                       const SkImageInfo& info,
                                                       skgpu::Mipmapped mipmapped,
                                                       GrImageTexGenPolicy texGenPolicy) {
    SkASSERT_RELEASE(fInfo.dimensions() == info.dimensions());

    if (!ctx || ctx->abandoned()) {
        return {};
    }

    return this->onGenerateTexture(ctx, info, mipmapped, texGenPolicy);
}

GrExternalTextureGenerator::GrExternalTextureGenerator(const SkImageInfo& info) : GrTextureGenerator(info) {}

GrSurfaceProxyView GrExternalTextureGenerator::onGenerateTexture(GrRecordingContext* ctx,
                                                                 const SkImageInfo& info,
                                                                 skgpu::Mipmapped mipmapped,
                                                                 GrImageTexGenPolicy texGenPolicy) {
    std::unique_ptr<GrExternalTexture> externalTexture = generateExternalTexture(ctx, mipmapped);
    GrBackendTexture backendTexture = externalTexture->getBackendTexture();
    const GrBackendFormat& format = backendTexture.getBackendFormat();
    const GrColorType colorType = SkColorTypeToGrColorType(info.colorType());
    if (!ctx->priv().caps()->areColorTypeAndFormatCompatible(colorType, format)) {
        return {};
    }

    auto cleanupCallback = skgpu::RefCntedCallback::Make(dispose_external_texture, externalTexture.release());
    sk_sp<GrSurfaceProxy> proxy = ctx->priv().proxyProvider()->wrapBackendTexture(
            backendTexture,
            kBorrow_GrWrapOwnership,
            GrWrapCacheable::kYes,
            kRead_GrIOType,
            std::move(cleanupCallback));
    if (!proxy) {
        return {};
    }
    static constexpr auto kOrigin = kTopLeft_GrSurfaceOrigin;
    skgpu::Swizzle swizzle = ctx->priv().caps()->getReadSwizzle(format, colorType);
    return GrSurfaceProxyView(std::move(proxy), kOrigin, swizzle);
}
