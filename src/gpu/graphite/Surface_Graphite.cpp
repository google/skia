/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Surface_Graphite.h"

#include "include/core/SkCapabilities.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/Image_Graphite.h"
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

Recorder* Surface::onGetRecorder() {
    return fDevice->recorder();
}

TextureProxyView Surface::readSurfaceView() const {
    return fDevice->readSurfaceView();
}

SkCanvas* Surface::onNewCanvas() { return new SkCanvas(fDevice); }

sk_sp<SkSurface> Surface::onNewSurface(const SkImageInfo& ii) {
    return SkSurface::MakeGraphite(fDevice->recorder(), ii, Mipmapped::kNo, &this->props());
}

sk_sp<SkImage> Surface::onNewImageSnapshot(const SkIRect* subset) {
    TextureProxyView srcView = fDevice->readSurfaceView();
    if (!srcView) {
        return nullptr;
    }

    return this->onMakeImageCopy(subset, srcView.mipmapped());
}

sk_sp<SkImage> Surface::onAsImage() {
    TextureProxyView srcView = fDevice->readSurfaceView();
    if (!srcView) {
        return nullptr;
    }

    return sk_sp<Image>(new Image(std::move(srcView), this->imageInfo().colorInfo()));
}

sk_sp<SkImage> Surface::onMakeImageCopy(const SkIRect* subset, Mipmapped mipmapped) {
    TextureProxyView srcView = fDevice->createCopy(subset, mipmapped);
    if (!srcView) {
        return nullptr;
    }

    return sk_sp<Image>(new Image(std::move(srcView), this->imageInfo().colorInfo()));
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
    fDevice->asyncRescaleAndReadPixels(info,
                                       srcRect,
                                       rescaleGamma,
                                       rescaleMode,
                                       callback,
                                       context);
}

void Surface::onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace yuvColorSpace,
                                                sk_sp<SkColorSpace> dstColorSpace,
                                                SkIRect srcRect,
                                                SkISize dstSize,
                                                RescaleGamma rescaleGamma,
                                                RescaleMode rescaleMode,
                                                ReadPixelsCallback callback,
                                                ReadPixelsContext context) {
    fDevice->asyncRescaleAndReadPixelsYUV420(yuvColorSpace,
                                             dstColorSpace,
                                             srcRect,
                                             dstSize,
                                             rescaleGamma,
                                             rescaleMode,
                                             callback,
                                             context);
}

sk_sp<const SkCapabilities> Surface::onCapabilities() {
    return fDevice->recorder()->priv().caps()->capabilities();
}

#if GRAPHITE_TEST_UTILS && SK_SUPPORT_GPU
GrSemaphoresSubmitted Surface::onFlush(BackendSurfaceAccess,
                                       const GrFlushInfo&,
                                       const skgpu::MutableTextureState*) {
    fDevice->flushPendingWorkToRecorder();
    return GrSemaphoresSubmitted::kNo;
}
#endif

TextureProxy* Surface::backingTextureProxy() { return fDevice->target(); }

sk_sp<SkSurface> Surface::MakeGraphite(Recorder* recorder,
                                       const SkImageInfo& info,
                                       skgpu::Budgeted budgeted,
                                       Mipmapped mipmapped,
                                       const SkSurfaceProps* props) {
    sk_sp<Device> device = Device::Make(recorder, info, budgeted, mipmapped,
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

sk_sp<SkSurface> SkSurface::MakeGraphite(Recorder* recorder,
                                         const SkImageInfo& info,
                                         skgpu::Mipmapped mipmapped,
                                         const SkSurfaceProps* props) {
    // The client is getting the ref on this surface so it must be unbudgeted.
    return skgpu::graphite::Surface::MakeGraphite(
            recorder, info, skgpu::Budgeted::kNo, mipmapped, props);
}

sk_sp<SkSurface> SkSurface::MakeGraphiteFromBackendTexture(Recorder* recorder,
                                                           const BackendTexture& backendTex,
                                                           SkColorType ct,
                                                           sk_sp<SkColorSpace> cs,
                                                           const SkSurfaceProps* props) {

    if (!recorder) {
        return nullptr;
    }

    const Caps* caps = recorder->priv().caps();

    SkColorInfo info(ct, kPremul_SkAlphaType, std::move(cs));

    if (!validate_backend_texture(caps, backendTex, info)) {
        return nullptr;
    }

    sk_sp<Texture> texture = recorder->priv().resourceProvider()->createWrappedTexture(backendTex);
    if (!texture) {
        return nullptr;
    }

    sk_sp<TextureProxy> proxy(new TextureProxy(std::move(texture)));

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
