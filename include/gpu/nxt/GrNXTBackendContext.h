/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNXTBackendContext_DEFINED
#define GrNXTBackendContext_DEFINED

#include "SkRefCnt.h"
#include "nxt/nxtcpp.h"

struct GrNXTBackendContext : public SkRefCnt {
    GrNXTBackendContext(nxt::Device device, nxt::Queue queue)
      : fDevice(device.Clone())
      , fQueue(queue.Clone()) {}
    nxt::Device       fDevice;
    nxt::Queue        fQueue;

    ~GrNXTBackendContext() override;
};

#endif
