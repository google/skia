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

Recorder* Surface::onGetRecorder() {
    return fDevice->recorder();
}

SkCanvas* Surface::onNewCanvas() { return new SkCanvas(fDevice); }

sk_sp<SkSurface> Surface::onNewSurface(const SkImageInfo& ii) {
    return MakeGraphite(fDevice->recorder(), ii, Mipmapped::kNo, &this->props());
}

sk_sp<SkImage> Surface::onNewImageSnapshot(const SkIRect* subset) {
    SkImageInfo ii = subset ? this->imageInfo().makeDimensions(subset->size())
                            : this->imageInfo();

    // TODO: we need to resolve Graphite's Surface/Image story then expand the handling
    // in here.
    TextureProxyView srcView = fDevice->readSurfaceView();
    if (!srcView) {
        return nullptr;
    }

    return sk_sp<Image>(new Image(std::move(srcView), ii.colorInfo()));
}

void Surface::onWritePixels(const SkPixmap& pixmap, int x, int y) {
    fDevice->writePixels(pixmap, x, y);
}

bool Surface::onCopyOnWrite(ContentChangeMode) { return true; }

bool Surface::onReadPixels(Context* context,
                           Recorder* recorder,
                           const SkPixmap& dst,
                           int srcX,
                           int srcY) {
    return fDevice->readPixels(context, recorder, dst, srcX, srcY);
}

sk_sp<const SkCapabilities> Surface::onCapabilities() {
    return fDevice->recorder()->priv().caps()->capabilities();
}

#if GRAPHITE_TEST_UTILS && SK_SUPPORT_GPU
GrSemaphoresSubmitted Surface::onFlush(BackendSurfaceAccess,
                                       const GrFlushInfo&,
                                       const GrBackendSurfaceMutableState*) {
    fDevice->flushPendingWorkToRecorder();
    return GrSemaphoresSubmitted::kNo;
}
#endif

} // namespace skgpu::graphite

using namespace skgpu::graphite;

namespace {

bool validate_backend_texture(const Caps* caps,
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

} // anonymous namespace

sk_sp<SkSurface> SkSurface::MakeGraphite(Recorder* recorder,
                                         const SkImageInfo& info,
                                         Mipmapped mipmapped,
                                         const SkSurfaceProps* props) {

    sk_sp<Device> device = Device::Make(recorder, info, SkBudgeted::kNo,
                                        mipmapped,
                                        SkSurfacePropsCopyOrDefault(props),
                                        /* addInitialClear= */ true);
    if (!device) {
        return nullptr;
    }

    return sk_make_sp<Surface>(std::move(device));
}

sk_sp<SkSurface> SkSurface::MakeGraphiteFromBackendTexture(Recorder* recorder,
                                                           const BackendTexture& beTexture,
                                                           SkColorType colorType,
                                                           sk_sp<SkColorSpace> colorSpace,
                                                           const SkSurfaceProps* props) {

    if (!recorder) {
        return nullptr;
    }

    if (!validate_backend_texture(recorder->priv().caps(), beTexture, colorType)) {
        return nullptr;
    }

    sk_sp<Texture> texture = recorder->priv().resourceProvider()->createWrappedTexture(beTexture);
    if (!texture) {
        return nullptr;
    }

    sk_sp<TextureProxy> proxy(new TextureProxy(std::move(texture)));

    sk_sp<Device> device = Device::Make(recorder,
                                        std::move(proxy),
                                        { colorType, kPremul_SkAlphaType, std::move(colorSpace) },
                                        SkSurfacePropsCopyOrDefault(props),
                                        /* addInitialClear= */ false);
    if (!device) {
        return nullptr;
    }

    return sk_make_sp<Surface>(std::move(device));
}
