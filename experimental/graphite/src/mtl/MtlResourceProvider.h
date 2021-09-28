/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlResourceProvider_DEFINED
#define skgpu_MtlResourceProvider_DEFINED

#include "experimental/graphite/src/ResourceProvider.h"

#import <Metal/Metal.h>

namespace skgpu::mtl {

class ResourceProvider final : public skgpu::ResourceProvider {
public:
    ResourceProvider();
    ~ResourceProvider() override {}

private:
};

} // namespace skgpu::mtl

#endif // skgpu_MtlResourceProvider_DEFINED
