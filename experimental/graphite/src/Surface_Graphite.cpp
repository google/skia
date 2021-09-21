/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Surface_Graphite.h"

#include "experimental/graphite/src/Device.h"

namespace skgpu {

Surface_Graphite::Surface_Graphite(sk_sp<Device> device)
        : SkSurface_Base(device->width(), device->height(), &device->surfaceProps())
        , fDevice(std::move(device)) {
}

Surface_Graphite::~Surface_Graphite() {}

SkCanvas* Surface_Graphite::onNewCanvas() {
    return nullptr;
}

sk_sp<SkSurface> Surface_Graphite::onNewSurface(const SkImageInfo&) {
    return nullptr;
}

void Surface_Graphite::onWritePixels(const SkPixmap&, int x, int y) {}

void Surface_Graphite::onCopyOnWrite(ContentChangeMode) {}

} // namespace skgpu
