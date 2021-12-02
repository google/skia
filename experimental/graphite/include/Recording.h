/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Recording_DEFINED
#define skgpu_Recording_DEFINED

#include "include/core/SkRefCnt.h"

namespace skgpu {

class CommandBuffer;

class Recording final {
public:
    ~Recording();

protected:
private:
    friend class Context; // for access fCommandBuffer
    friend class Recorder; // for ctor
    Recording(sk_sp<CommandBuffer>);

    sk_sp<CommandBuffer> fCommandBuffer;
};

} // namespace skgpu

#endif // skgpu_Recording_DEFINED
