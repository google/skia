/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/SkStuff.h"

#include "experimental/graphite/src/Device.h"

sk_sp<SkSurface> MakeGraphite(const SkImageInfo& ii) {

    sk_sp<skgpu::Device> device = sk_make_sp<skgpu::Device>(ii);
    if (!device) {
        return nullptr;
    }

    // TODO: create a new SkSurface_Graphite class that wraps a skgpu::Device.
    // skgpu::BaseDevice carries a lot of baggage.
    //return sk_make_sp<SkSurface_Graphite>(std::move(device));
    return nullptr;
}
