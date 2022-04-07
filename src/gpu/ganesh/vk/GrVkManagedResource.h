/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkManagedResource_DEFINED
#define GrVkManagedResource_DEFINED

#include "src/gpu/ganesh/GrManagedResource.h"

class GrVkGpu;

class GrVkManagedResource : public GrManagedResource {
public:
    GrVkManagedResource(const GrVkGpu* gpu) : fGpu(gpu) {}

protected:
    const GrVkGpu* fGpu;  // pointer to gpu object that can be used
                          // in subclass's freeGPUData()

private:
    using INHERITED = GrManagedResource;
};

class GrVkRecycledResource : public GrRecycledResource {
public:
    GrVkRecycledResource(GrVkGpu* gpu) : fGpu(gpu) {}

protected:
    GrVkGpu* fGpu;  // pointer to gpu object that can be used
                    // in subclass's freeGPUData() and onRecycle().
                    // mustn't be const
};

#endif
