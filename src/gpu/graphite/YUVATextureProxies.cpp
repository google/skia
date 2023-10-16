/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/YUVATextureProxies.h"

#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

YUVATextureProxies::YUVATextureProxies(const Recorder* recorder,
                                       const SkYUVAInfo& yuvaInfo,
                                       SkSpan<sk_sp<TextureProxy>> proxies)
        : fYUVAInfo(yuvaInfo) {
    int n = yuvaInfo.numPlanes();
    if (n == 0 || (size_t)n > proxies.size()) {
        *this = {};
        SkASSERT(!this->isValid());
        return;
    }
    uint32_t textureChannelMasks[SkYUVAInfo::kMaxPlanes];
    for (int i = 0; i < n; ++i) {
        if (!proxies[i]) {
            *this = {};
            SkASSERT(!this->isValid());
            return;
        }
        textureChannelMasks[i] = recorder->priv().caps()->channelMask(proxies[i]->textureInfo());
    }
    fYUVALocations = yuvaInfo.toYUVALocations(textureChannelMasks);
    if (fYUVALocations[0].fPlane < 0) {
        *this = {};
        SkASSERT(!this->isValid());
        return;
    }
    fMipmapped = Mipmapped::kYes;
    for (int i = 0; i < n; ++i) {
        if (proxies[i]->mipmapped() == Mipmapped::kNo) {
            fMipmapped = Mipmapped::kNo;
        }
        fProxies[i] = std::move(proxies[i]);
    }
    SkASSERT(this->isValid());
}

YUVATextureProxies::YUVATextureProxies(const Recorder* recorder,
                                       const SkYUVAInfo& yuvaInfo,
                                       SkSpan<TextureProxyView> views)
        : fYUVAInfo(yuvaInfo) {
    uint32_t pixmapChannelMasks[SkYUVAInfo::kMaxPlanes];
    int n = yuvaInfo.numPlanes();
    if (n == 0 || (size_t)n > views.size()) {
        *this = {};
        SkASSERT(!this->isValid());
        return;
    }
    fMipmapped = Mipmapped::kYes;
    for (int i = 0; i < n; ++i) {
        if (!views[i]) {
            *this = {};
            SkASSERT(!this->isValid());
            return;
        }
        pixmapChannelMasks[i] =
                recorder->priv().caps()->channelMask(views[i].proxy()->textureInfo());
        if (views[i].proxy()->mipmapped() == Mipmapped::kNo) {
            fMipmapped = Mipmapped::kNo;
        }
    }
    // Initial locations refer to the CPU pixmap channels.
    fYUVALocations = yuvaInfo.toYUVALocations(pixmapChannelMasks);
    if (fYUVALocations[0].fPlane < 0) {
        *this = {};
        SkASSERT(!this->isValid());
        return;
    }

    // Run each location through the proxy view's swizzle to get the actual texture format channel.
    for (int i = 0; i < SkYUVAInfo::kYUVAChannelCount; ++i) {
        int plane = fYUVALocations[i].fPlane;
        if (plane >= 0) {
            int chanAsIdx = static_cast<int>(fYUVALocations[i].fChannel);
            switch (views[plane].swizzle()[chanAsIdx]) {
                case 'r': fYUVALocations[i].fChannel = SkColorChannel::kR; break;
                case 'g': fYUVALocations[i].fChannel = SkColorChannel::kG; break;
                case 'b': fYUVALocations[i].fChannel = SkColorChannel::kB; break;
                case 'a': fYUVALocations[i].fChannel = SkColorChannel::kA; break;

                default:
                    SkDEBUGFAILF("Unexpected swizzle value: %c", views[i].swizzle()[chanAsIdx]);
                    *this = {};
                    SkASSERT(!this->isValid());
                    return;
            }
        }
    }
    for (int i = 0; i < n; ++i) {
        fProxies[i] = views[i].detachProxy();
    }
    SkASSERT(this->isValid());
}

}  // namespace skgpu::graphite
