/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrAttachment.h"

#include "include/private/GrResourceKey.h"

void GrAttachment::ComputeSharedUniqueAttachmentKey(SkISize dimensions,
                                                    UsageFlags requiredUsage,
                                                    int sampleCnt,
                                                    GrUniqueKey* key) {
    // We require only 1 type of usage to be requested
    SkASSERT(SkIsPow2((uint32_t)requiredUsage));

    static const GrUniqueKey::Domain kStencilDomain = GrUniqueKey::GenerateDomain();

    GrUniqueKey::Builder builder(key, kStencilDomain, 4);
    builder[0] = dimensions.width();
    builder[1] = dimensions.height();
    builder[2] = (uint32_t)requiredUsage;
    builder[2] = sampleCnt;
}
