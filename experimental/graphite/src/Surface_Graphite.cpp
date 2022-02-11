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

Surface::Surface(sk_sp<Device> device)
        : SkSurface_Base(device->width(), device->height(), &device->surfaceProps())
        , fDevice(std::move(device)) {
}

Surface::~Surface() {}

SkCanvas* Surface::onNewCanvas() { return new SkCanvas(fDevice); }

sk_sp<SkSurface> Surface::onNewSurface(const SkImageInfo& ii) {
    return MakeGraphite(fDevice->recorder(), ii);
}

sk_sp<SkImage> Surface::onNewImageSnapshot(const SkIRect* subset) {
    SkImageInfo ii = subset ? this->imageInfo().makeDimensions(subset->size())
                            : this->imageInfo();

    return sk_sp<Image_Graphite>(new Image_Graphite(ii));
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

} // namespace skgpu
