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
#include "src/gpu/SkBackingFit.h"
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
        , fDevice(std::move(device))
        , fImageView(Image::WrapDevice(fDevice)) {}

Surface::~Surface() {
    // Mark the device immutable when the Surface is destroyed to flush any pending work to the
    // recorder and to flag the device so that any linked image views can detach from the Device
    // when they are next drawn.
    fDevice->setImmutable();
}

SkImageInfo Surface::imageInfo() const {
    return fDevice->imageInfo();
}

Recorder* Surface::onGetRecorder() const { return fDevice->recorder(); }

TextureProxyView Surface::readSurfaceView() const {
    return fDevice->readSurfaceView();
}

SkCanvas* Surface::onNewCanvas() { return new SkCanvas(fDevice); }

sk_sp<SkSurface> Surface::onNewSurface(const SkImageInfo& ii) {
    return fDevice->makeSurface(ii, this->props());
}

sk_sp<SkImage> Surface::onNewImageSnapshot(const SkIRect* subset) {
    return this->makeImageCopy(subset, fDevice->target()->mipmapped());
}

sk_sp<Image> Surface::asImage() const {
    if (this->hasCachedImage()) {
        SKGPU_LOG_W("Intermingling makeImageSnapshot and asImage calls may produce "
                    "unexpected results. Please use either the old _or_ new API.");
    }
    return fImageView;
}

sk_sp<Image> Surface::makeImageCopy(const SkIRect* subset, Mipmapped mipmapped) const {
    if (this->hasCachedImage()) {
        SKGPU_LOG_W("Intermingling makeImageSnapshot and asImage calls may produce "
                    "unexpected results. Please use either the old _or_ new API.");
    }

    SkIRect srcRect = subset ? *subset : SkIRect::MakeSize(this->imageInfo().dimensions());
    // NOTE: Must copy through fDevice and not fImageView if the surface's texture is not sampleable
    return fDevice->makeImageCopy(srcRect, Budgeted::kNo, mipmapped, SkBackingFit::kExact);
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

TextureProxy* Surface::backingTextureProxy() const { return fDevice->target(); }

sk_sp<Surface> Surface::Make(Recorder* recorder,
                             const SkImageInfo& info,
                             std::string_view label,
                             Budgeted budgeted,
                             Mipmapped mipmapped,
                             SkBackingFit backingFit,
                             const SkSurfaceProps* props,
                             LoadOp initialLoadOp,
                             bool registerWithRecorder) {
    sk_sp<Device> device = Device::Make(recorder,
                                        info,
                                        budgeted,
                                        mipmapped,
                                        backingFit,
                                        SkSurfacePropsCopyOrDefault(props),
                                        initialLoadOp,
                                        std::move(label),
                                        registerWithRecorder);
    if (!device) {
        return nullptr;
    }
    // A non-budgeted surface should be fully instantiated before we return it
    // to the client.
    SkASSERT(budgeted == Budgeted::kYes || device->target()->isInstantiated());
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
    return skgpu::graphite::Surface::Make(recorder, info, "SkSurfaceRenderTarget",
                                          skgpu::Budgeted::kNo, mipmapped, SkBackingFit::kExact,
                                          props);
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
    SkISize deviceSize = proxy->dimensions();
    // Use kLoad for this device to preserve the existing contents of the wrapped backend texture.
    sk_sp<Device> device = Device::Make(recorder,
                                        std::move(proxy),
                                        deviceSize,
                                        info,
                                        SkSurfacePropsCopyOrDefault(props),
                                        LoadOp::kLoad);
    return device ? sk_make_sp<Surface>(std::move(device)) : nullptr;
}

}  // namespace SkSurfaces
