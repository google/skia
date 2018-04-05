/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "InitNXTMTL.h"
#include "Metal/Metal.h"

namespace backend {
    namespace metal {
        void Init(id<MTLDevice> metalDevice, nxtProcTable* procs, nxtDevice* device);
    }
}

void InitNXTMTLSystemDefaultDevice(nxtProcTable* procs, nxtDevice* device) {
    backend::metal::Init(MTLCreateSystemDefaultDevice(), procs, device);
}
