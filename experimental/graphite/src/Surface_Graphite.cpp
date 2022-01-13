/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Surface_Graphite.h"

#include "experimental/graphite/include/Recorder.h"
#include "experimental/graphite/include/SkStuff.h"
#include "experimental/graphite/src/Device.h"
#include "experimental/graphite/src/Image_Graphite.h"

namespace skgpu {

Surface_Graphite::Surface_Graphite(sk_sp<Device> device)
        : SkSurface_Base(device->width(), device->height(), &device->surfaceProps())
        , fDevice(std::move(device)) {
}

Surface_Graphite::~Surface_Graphite() {}

SkCanvas* Surface_Graphite::onNewCanvas() { return new SkCanvas(fDevice); }

sk_sp<SkSurface> Surface_Graphite::onNewSurface(const SkImageInfo& ii) {
    return MakeGraphite(fDevice->recorder(), ii);
}

sk_sp<SkImage> Surface_Graphite::onNewImageSnapshot(const SkIRect* subset) {
    SkImageInfo ii = subset ? this->imageInfo().makeDimensions(subset->size())
                            : this->imageInfo();

    return sk_sp<Image_Graphite>(new Image_Graphite(ii));
}

void Surface_Graphite::onWritePixels(const SkPixmap&, int x, int y) {}
bool Surface_Graphite::onCopyOnWrite(ContentChangeMode) { return true; }

bool Surface_Graphite::onReadPixels(skgpu::Context* context,
                                    const SkPixmap& dst,
                                    int srcX,
                                    int srcY) {
    return fDevice->readPixels(context, dst, srcX, srcY);
}

} // namespace skgpu
