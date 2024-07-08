/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrClientMappedBufferManager.h"

#include "src/core/SkMessageBus.h"

DECLARE_SKMESSAGEBUS_MESSAGE(GrClientMappedBufferManager::BufferFinishedMessage,
                             GrDirectContext::DirectContextID,
                             false)

bool SkShouldPostMessageToBus(const GrClientMappedBufferManager::BufferFinishedMessage& m,
                              GrDirectContext::DirectContextID potentialRecipient) {
    return m.fIntendedRecipient == potentialRecipient;
}
