/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/mtl/MtlTexture.h"

#include "include/gpu/MutableTextureState.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/gpu/graphite/mtl/MtlGraphiteTypes.h"
#include "src/core/SkMipmap.h"
#include "src/gpu/graphite/mtl/MtlCaps.h"
#include "src/gpu/graphite/mtl/MtlGraphiteUtils.h"
#include "src/gpu/graphite/mtl/MtlSharedContext.h"
#include "src/gpu/mtl/MtlUtilsPriv.h"

namespace skgpu::graphite {

sk_cfp<id<MTLTexture>> MtlTexture::MakeMtlTexture(const MtlSharedContext* sharedContext,
                                                  SkISize dimensions,
                                                  const TextureInfo& info) {
    const Caps* caps = sharedContext->caps();
    if (dimensions.width() > caps->maxTextureSize() ||
        dimensions.height() > caps->maxTextureSize()) {
        return nullptr;
    }

    const auto& mtlInfo = TextureInfoPriv::Get<MtlTextureInfo>(info);
    SkASSERT(!mtlInfo.fFramebufferOnly);

    if (mtlInfo.fUsage & MTLTextureUsageShaderRead && !caps->isTexturable(info)) {
        return nullptr;
    }

    if (mtlInfo.fUsage & MTLTextureUsageRenderTarget && !caps->isRenderable(info)) {
        return nullptr;
    }

    if (mtlInfo.fUsage & MTLTextureUsageShaderWrite && !caps->isStorage(info)) {
        return nullptr;
    }

    int numMipLevels = 1;
    if (info.mipmapped() == Mipmapped::kYes) {
        numMipLevels = SkMipmap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;
    }

    sk_cfp<MTLTextureDescriptor*> desc([[MTLTextureDescriptor alloc] init]);
    (*desc).textureType = (info.numSamples() > 1) ? MTLTextureType2DMultisample : MTLTextureType2D;
    (*desc).pixelFormat = mtlInfo.fFormat;
    (*desc).width = dimensions.width();
    (*desc).height = dimensions.height();
    (*desc).depth = 1;
    (*desc).mipmapLevelCount = numMipLevels;
    (*desc).sampleCount = info.numSamples();
    (*desc).arrayLength = 1;
    (*desc).usage = mtlInfo.fUsage;
    (*desc).storageMode = mtlInfo.fStorageMode;

    sk_cfp<id<MTLTexture>> texture([sharedContext->device() newTextureWithDescriptor:desc.get()]);
    return texture;
}

static bool has_transient_usage(const TextureInfo& info) {
    if (@available(macOS 11.0, iOS 10.0, tvOS 10.0, *)) {
        const auto& mtlInfo = TextureInfoPriv::Get<MtlTextureInfo>(info);
        return mtlInfo.fStorageMode == MTLStorageModeMemoryless;
    }
    return false;
}

MtlTexture::MtlTexture(const MtlSharedContext* sharedContext,
                       SkISize dimensions,
                       const TextureInfo& info,
                       sk_cfp<id<MTLTexture>> texture,
                       Ownership ownership)
        : Texture(sharedContext,
                  dimensions,
                  info,
                  /*isTransient=*/has_transient_usage(info),
                  /*mutableState=*/nullptr,
                  ownership)
        , fTexture(std::move(texture)) {}

sk_sp<Texture> MtlTexture::Make(const MtlSharedContext* sharedContext,
                                SkISize dimensions,
                                const TextureInfo& info) {
    sk_cfp<id<MTLTexture>> texture = MakeMtlTexture(sharedContext, dimensions, info);
    if (!texture) {
        return nullptr;
    }
    return sk_sp<Texture>(new MtlTexture(sharedContext,
                                         dimensions,
                                         info,
                                         std::move(texture),
                                         Ownership::kOwned));
}

sk_sp<Texture> MtlTexture::MakeWrapped(const MtlSharedContext* sharedContext,
                                       SkISize dimensions,
                                       const TextureInfo& info,
                                       sk_cfp<id<MTLTexture>> texture) {
    return sk_sp<Texture>(new MtlTexture(sharedContext,
                                         dimensions,
                                         info,
                                         std::move(texture),
                                         Ownership::kWrapped));
}

void MtlTexture::freeGpuData() {
    fTexture.reset();
}


void MtlTexture::setBackendLabel(char const* label) {
    SkASSERT(label);
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    NSString* labelStr = @(label);
    this->mtlTexture().label = labelStr;
#endif
}

} // namespace skgpu::graphite
