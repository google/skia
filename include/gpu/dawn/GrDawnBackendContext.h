/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnBackendContext_DEFINED
#define GrDawnBackendContext_DEFINED

#include "SkRefCnt.h"

struct dawnDeviceImpl;
struct dawnQueueImpl;

struct GrDawnBackendContext : public SkRefCnt {
    GrDawnBackendContext(dawnDeviceImpl* device, dawnQueueImpl* queue)
      : fDevice(device), fQueue(queue) {}
    dawnDeviceImpl*  fDevice;
    dawnQueueImpl*   fQueue;
    ~GrDawnBackendContext() override;
};

#endif
