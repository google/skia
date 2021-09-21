/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/SkStuff.h"

#include "experimental/graphite/src/Device.h"
#include "experimental/graphite/src/Surface_Graphite.h"

sk_sp<SkSurface> MakeGraphite(const SkImageInfo& ii) {

    sk_sp<skgpu::Device> device = sk_make_sp<skgpu::Device>(ii);
    if (!device) {
        return nullptr;
    }

    return sk_make_sp<skgpu::Surface_Graphite>(std::move(device));
}
