/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlCaps_DEFINED
#define skgpu_MtlCaps_DEFINED

#include "experimental/graphite/src/Caps.h"

#import <Metal/Metal.h>

namespace skgpu::mtl {

class Caps final : public skgpu::Caps {
public:
    Caps(id<MTLDevice>);
    ~Caps() final {}

private:
};

} // namespace skgpu::mtl

#endif // skgpu_MtlCaps_DEFINED
