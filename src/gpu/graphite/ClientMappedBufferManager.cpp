/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ClientMappedBufferManager.h"

//////////////////////////////////////////////////////////////////////////////

DECLARE_SKMESSAGEBUS_MESSAGE(skgpu::graphite::ClientMappedBufferManager::BufferFinishedMessage,
                             skgpu::graphite::Context::ContextID,
                             false)

namespace skgpu::graphite {
bool SkShouldPostMessageToBus(const ClientMappedBufferManager::BufferFinishedMessage& m,
                              Context::ContextID potentialRecipient) {
    return m.fIntendedRecipient == potentialRecipient;
}
}

