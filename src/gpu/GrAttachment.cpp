/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrAttachment.h"

#include "include/private/GrResourceKey.h"

void GrAttachment::ComputeSharedAttachmentUniqueKey(SkISize dimensions,
                                                    UsageFlags requiredUsage,
                                                    int sampleCnt,
                                                    GrUniqueKey* key) {
    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();

    GrUniqueKey::Builder builder(key, kDomain, 4);
    builder[0] = dimensions.width();
    builder[1] = dimensions.height();
    builder[2] = (uint32_t)requiredUsage;
    builder[3] = sampleCnt;
}
