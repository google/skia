/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNXTBackendContext_DEFINED
#define GrNXTBackendContext_DEFINED

#include "SkRefCnt.h"

struct dawnDeviceImpl;
struct dawnQueueImpl;

struct GrNXTBackendContext : public SkRefCnt {
    GrNXTBackendContext(dawnDeviceImpl* device, dawnQueueImpl* queue)
      : fDevice(device), fQueue(queue) {}
    dawnDeviceImpl*  fDevice;
    dawnQueueImpl*   fQueue;
    ~GrNXTBackendContext() override;
};

#endif
