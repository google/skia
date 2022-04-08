/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/SkStuff.h"

#include "experimental/graphite/include/BackendTexture.h"
#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/include/Recorder.h"
#include "experimental/graphite/src/Caps.h"
#include "experimental/graphite/src/ContextPriv.h"
#include "experimental/graphite/src/Device.h"
#include "experimental/graphite/src/Gpu.h"
#include "experimental/graphite/src/RecorderPriv.h"
#include "experimental/graphite/src/ResourceProvider.h"
#include "experimental/graphite/src/Surface_Graphite.h"
#include "experimental/graphite/src/Texture.h"
#include "experimental/graphite/src/TextureProxy.h"
#include "include/core/SkColorSpace.h"

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
