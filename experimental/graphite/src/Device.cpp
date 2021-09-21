/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Device.h"

#include "experimental/graphite/src/SurfaceDrawContext.h"

namespace skgpu {

sk_sp<Device> Device::Make(const SkImageInfo& ii) {
    sk_sp<SurfaceDrawContext> sdc = SurfaceDrawContext::Make(ii);
    if (!sdc) {
        return nullptr;
    }

    return sk_sp<Device>(new Device(std::move(sdc)));
}

Device::Device(sk_sp<SurfaceDrawContext> sdc)
    : SkBaseDevice(sdc->imageInfo(), SkSurfaceProps())
    , fSDC(std::move(sdc)) {
}

} // namespace skgpu
