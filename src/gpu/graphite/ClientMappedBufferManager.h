/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ClientMappedBufferManager_graphite_DEFINED
#define ClientMappedBufferManager_graphite_DEFINED

#include "include/gpu/graphite/Context.h"
#include "src/gpu/AsyncReadTypes.h"
#include "src/gpu/graphite/Buffer.h"

namespace skgpu::graphite {

// This is declared as a class rather than an alias to allow for forward declarations
class ClientMappedBufferManager :
        public skgpu::TClientMappedBufferManager<Buffer, Context::ContextID> {
public:
    ClientMappedBufferManager(Context::ContextID ownerID)
            : TClientMappedBufferManager(ownerID) {}
};

bool SkShouldPostMessageToBus(const ClientMappedBufferManager::BufferFinishedMessage&,
                              Context::ContextID potentialRecipient);

} // namespace skgpu::graphite

#endif
