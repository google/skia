/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "InitDawnMTL.h"
#include "Metal/Metal.h"

namespace dawn_native {
    namespace metal {
        void Init(id<MTLDevice> metalDevice, dawnProcTable* procs, dawnDevice* device);
    }
}

void InitDawnMTLSystemDefaultDevice(dawnProcTable* procs, dawnDevice* device) {
    dawn_native::metal::Init(MTLCreateSystemDefaultDevice(), procs, device);
}
