/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/SkStuff.h"

#include "include/core/SkColorSpace.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/Gpu.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"

using namespace skgpu::graphite;

sk_sp<SkSurface> MakeGraphite(Recorder* recorder, const SkImageInfo& ii) {
    sk_sp<Device> device = Device::Make(recorder, ii);
    if (!device) {
        return nullptr;
    }

    return sk_make_sp<Surface>(std::move(device));
}

static bool validate_backend_texture(const Caps* caps,
                                     const BackendTexture& texture,
                                     SkColorType ct) {
    if (!texture.isValid()) {
        return false;
    }

    const TextureInfo& info = texture.info();
    if (!caps->areColorTypeAndTextureInfoCompatible(ct, info)) {
        return false;
    }

    if (!caps->isRenderable(info)) {
        return false;
    }
    return true;
}

sk_sp<SkSurface> MakeGraphiteFromBackendTexture(Recorder* recorder,
                                                const BackendTexture& beTexture,
                                                SkColorType colorType,
                                                sk_sp<SkColorSpace> colorSpace,
                                                const SkSurfaceProps* props) {

    if (!recorder) {
        return nullptr;
    }

    if (!validate_backend_texture(recorder->priv().caps(),
                                  beTexture,
                                  colorType)) {
        return nullptr;
    }

    sk_sp<Texture> texture =
            recorder->priv().resourceProvider()->createWrappedTexture(beTexture);

    if (!texture) {
        return nullptr;
    }

    sk_sp<TextureProxy> proxy(new TextureProxy(std::move(texture)));

    sk_sp<Device> device = Device::Make(recorder,
                                        std::move(proxy),
                                        std::move(colorSpace),
                                        colorType,
                                        kPremul_SkAlphaType);
    if (!device) {
        return nullptr;
    }

    return sk_make_sp<Surface>(std::move(device));
}
