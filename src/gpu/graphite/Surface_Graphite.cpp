/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Surface_Graphite.h"

#include "include/core/SkCapabilities.h"
#include "include/core/SkColorSpace.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "src/core/SkSurfacePriv.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"

namespace skgpu::graphite {

Surface::Surface(sk_sp<Device> device)
        : SkSurface_Base(device->width(), device->height(), &device->surfaceProps())
        , fDevice(std::move(device)) {
}

Surface::~Surface() {}

SkImageInfo Surface::imageInfo() const {
    return fDevice->imageInfo();
}

Recorder* Surface::onGetRecorder() const { return fDevice->recorder(); }

TextureProxyView Surface::readSurfaceView() const {
    return fDevice->readSurfaceView();
}

SkCanvas* Surface::onNewCanvas() { return new SkCanvas(fDevice); }

sk_sp<SkSurface> Surface::onNewSurface(const SkImageInfo& ii) {
    return SkSurfaces::RenderTarget(fDevice->recorder(), ii, Mipmapped::kNo, &this->props());
}

sk_sp<SkImage> Surface::onNewImageSnapshot(const SkIRect* subset) {
    TextureProxyView srcView = fDevice->readSurfaceView();
    if (!srcView) {
        return nullptr;
    }

    return this->makeImageCopy(subset, srcView.mipmapped());
}

sk_sp<SkImage> Surface::asImage() const {
    if (this->hasCachedImage()) {
        SKGPU_LOG_W(
                "Intermingling makeImageSnapshot and asImage calls may produce "
                "unexpected results. Please use either the old _or_ new API.");
    }
    TextureProxyView srcView = fDevice->readSurfaceView();
    if (!srcView) {
        return nullptr;
    }

    return sk_sp<Image>(new Image(kNeedNewImageUniqueID,
                                  std::move(srcView),
                                  this->imageInfo().colorInfo()));
}

sk_sp<SkImage> Surface::makeImageCopy(const SkIRect* subset, Mipmapped mipmapped) const {
    if (this->hasCachedImage()) {
        SKGPU_LOG_W(
                "Intermingling makeImageSnapshot and asImage calls may produce "
                "unexpected results. Please use either the old _or_ new API.");
    }
    TextureProxyView srcView = fDevice->createCopy(subset, mipmapped);
    if (!srcView) {
        return nullptr;
    }

    return sk_sp<Image>(new Image(kNeedNewImageUniqueID,
                                  std::move(srcView),
                                  this->imageInfo().colorInfo()));
}

void Surface::onWritePixels(const SkPixmap& pixmap, int x, int y) {
    fDevice->writePixels(pixmap, x, y);
}

bool Surface::onCopyOnWrite(ContentChangeMode) { return true; }

void Surface::onAsyncRescaleAndReadPixels(const SkImageInfo& info,
                                          SkIRect srcRect,
                                          RescaleGamma rescaleGamma,
                                          RescaleMode rescaleMode,
                                          ReadPixelsCallback callback,
                                          ReadPixelsContext context) {
    // Not supported for Graphite. Use Context::asyncRescaleAndReadPixels instead.
    callback(context, nullptr);
}

void Surface::onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace yuvColorSpace,
                                                bool readAlpha,
                                                sk_sp<SkColorSpace> dstColorSpace,
                                                SkIRect srcRect,
                                                SkISize dstSize,
                                                RescaleGamma rescaleGamma,
                                                RescaleMode rescaleMode,
                                                ReadPixelsCallback callback,
                                                ReadPixelsContext context) {
    // Not supported for Graphite. Use Context::asyncRescaleAndReadPixelsYUV420 instead.
    callback(context, nullptr);
}

sk_sp<const SkCapabilities> Surface::onCapabilities() {
    return fDevice->recorder()->priv().caps()->capabilities();
}

TextureProxy* Surface::backingTextureProxy() { return fDevice->target(); }

sk_sp<SkSurface> Surface::MakeGraphite(Recorder* recorder,
                                       const SkImageInfo& info,
                                       skgpu::Budgeted budgeted,
                                       Mipmapped mipmapped,
                                       const SkSurfaceProps* props) {
    sk_sp<Device> device = Device::Make(recorder, info, budgeted, mipmapped,
                                        Protected::kNo,
                                        SkSurfacePropsCopyOrDefault(props),
                                        /* addInitialClear= */ true);
    if (!device) {
        return nullptr;
    }

    if (!device->target()->instantiate(recorder->priv().resourceProvider())) {
        return nullptr;
    }
    return sk_make_sp<Surface>(std::move(device));
}

void Flush(sk_sp<SkSurface> surface) {
    return Flush(surface.get());
}

void Flush(SkSurface* surface) {
    if (!surface) {
        return;
    }
    auto sb = asSB(surface);
    if (!sb->isGraphiteBacked()) {
        return;
    }
    auto gs = static_cast<Surface*>(surface);
    gs->fDevice->flushPendingWorkToRecorder();
}

} // namespace skgpu::graphite

using namespace skgpu::graphite;

namespace {

bool validate_backend_texture(const Caps* caps,
                              const BackendTexture& texture,
                              const SkColorInfo& info) {
    if (!texture.isValid() ||
        texture.dimensions().width() <= 0 ||
        texture.dimensions().height() <= 0) {
        return false;
    }

    if (!SkColorInfoIsValid(info)) {
        return false;
    }

    if (!caps->isRenderable(texture.info())) {
        return false;
    }

    return caps->areColorTypeAndTextureInfoCompatible(info.colorType(), texture.info());
}

} // anonymous namespace

namespace SkSurfaces {
sk_sp<SkImage> AsImage(sk_sp<const SkSurface> surface) {
    if (!surface) {
        return nullptr;
    }
    auto sb = asConstSB(surface.get());
    if (!sb->isGraphiteBacked()) {
        return nullptr;
    }
    auto gs = static_cast<const Surface*>(surface.get());
    return gs->asImage();
}

sk_sp<SkImage> AsImageCopy(sk_sp<const SkSurface> surface,
                           const SkIRect* subset,
                           skgpu::Mipmapped mipmapped) {
    if (!surface) {
        return nullptr;
    }
    auto sb = asConstSB(surface.get());
    if (!sb->isGraphiteBacked()) {
        return nullptr;
    }
    auto gs = static_cast<const Surface*>(surface.get());
    return gs->makeImageCopy(subset, mipmapped);
}

sk_sp<SkSurface> RenderTarget(Recorder* recorder,
                              const SkImageInfo& info,
                              skgpu::Mipmapped mipmapped,
                              const SkSurfaceProps* props) {
    // The client is getting the ref on this surface so it must be unbudgeted.
    return skgpu::graphite::Surface::MakeGraphite(
            recorder, info, skgpu::Budgeted::kNo, mipmapped, props);
}

sk_sp<SkSurface> WrapBackendTexture(Recorder* recorder,
                                    const BackendTexture& backendTex,
                                    SkColorType ct,
                                    sk_sp<SkColorSpace> cs,
                                    const SkSurfaceProps* props,
                                    TextureReleaseProc releaseP,
                                    ReleaseContext releaseC) {
    auto releaseHelper = skgpu::RefCntedCallback::Make(releaseP, releaseC);

    if (!recorder) {
        return nullptr;
    }

    const Caps* caps = recorder->priv().caps();

    SkColorInfo info(ct, kPremul_SkAlphaType, std::move(cs));

    if (!validate_backend_texture(caps, backendTex, info)) {
        SKGPU_LOG_E("validate_backend_texture failed: backendTex.info = %s; colorType = %d",
                    backendTex.info().toString().c_str(),
                    info.colorType());
        return nullptr;
    }

    sk_sp<Texture> texture = recorder->priv().resourceProvider()->createWrappedTexture(backendTex);
    if (!texture) {
        return nullptr;
    }
    texture->setReleaseCallback(std::move(releaseHelper));

    sk_sp<TextureProxy> proxy = TextureProxy::Wrap(std::move(texture));

    sk_sp<Device> device = Device::Make(recorder,
                                        std::move(proxy),
                                        info,
                                        SkSurfacePropsCopyOrDefault(props),
                                        /* addInitialClear= */ false);
    if (!device) {
        return nullptr;
    }

    return sk_make_sp<Surface>(std::move(device));
}

}  // namespace SkSurfaces
