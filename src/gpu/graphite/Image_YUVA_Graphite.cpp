/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Image_YUVA_Graphite.h"

#include "include/core/SkColorSpace.h"
#include "src/gpu/graphite/Log.h"

namespace skgpu::graphite {

Image_YUVA::Image_YUVA(uint32_t uniqueID,
                       YUVATextureProxies proxies,
                       const SkColorInfo& colorInfo)
        : Image_Base(SkImageInfo::Make(proxies.yuvaInfo().dimensions(),
                                       colorInfo),
                     uniqueID)
        , fYUVAProxies(std::move(proxies)) {
    SkASSERT(fYUVAProxies.isValid());
}

}
