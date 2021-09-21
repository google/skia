/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/sktbd/include/SkStuff.h"

#include "experimental/sktbd/src/Device.h"

sk_sp<SkSurface> MakeTBD(const SkImageInfo& ii) {

    sk_sp<sktbd::Device> device = sk_make_sp<sktbd::Device>(ii);
    if (!device) {
        return nullptr;
    }

    // TODO: create a new SkSurface_TBD class that wraps a sktbd::Device. skgpu::BaseDevice
    // carries a lot of baggage.
    //return sk_make_sp<SkSurface_TBD>(std::move(device));
    return nullptr;
}
