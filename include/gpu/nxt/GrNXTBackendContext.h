/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNXTBackendContext_DEFINED
#define GrNXTBackendContext_DEFINED

#include "SkRefCnt.h"

struct nxtDeviceImpl;
struct nxtQueueImpl;

struct GrNXTBackendContext : public SkRefCnt {
    GrNXTBackendContext(nxtDeviceImpl* device, nxtQueueImpl* queue)
      : fDevice(device), fQueue(queue) {}
    nxtDeviceImpl*  fDevice;
    nxtQueueImpl*   fQueue;
    ~GrNXTBackendContext() override;
};

#endif
