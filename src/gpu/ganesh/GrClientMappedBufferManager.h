/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrClientMappedBufferManager_DEFINED
#define GrClientMappedBufferManager_DEFINED

#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/AsyncReadTypes.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"

#include <forward_list>

// This is declared as a class rather than an alias to allow for forward declarations
class GrClientMappedBufferManager :
        public skgpu::TClientMappedBufferManager<GrGpuBuffer, GrDirectContext::DirectContextID> {
public:
    GrClientMappedBufferManager(GrDirectContext::DirectContextID ownerID)
            : TClientMappedBufferManager(ownerID) {}
};

bool SkShouldPostMessageToBus(const GrClientMappedBufferManager::BufferFinishedMessage&,
                              GrDirectContext::DirectContextID potentialRecipient);

#endif
