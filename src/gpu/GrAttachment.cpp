/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrAttachment.h"

#include "include/private/GrResourceKey.h"
#include "src/gpu/GrCaps.h"

void GrAttachment::ComputeSharedAttachmentUniqueKey(const GrCaps& caps,
                                                    const GrBackendFormat& format,
                                                    SkISize dimensions,
                                                    UsageFlags requiredUsage,
                                                    int sampleCnt,
                                                    GrProtected isProtected,
                                                    GrUniqueKey* key) {
    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();

    SkASSERT(!dimensions.isEmpty());

    SkASSERT(static_cast<uint32_t>(isProtected) <= 1);
    SkASSERT(static_cast<uint32_t>(requiredUsage) < (1u << 8));
    SkASSERT(static_cast<uint32_t>(sampleCnt) < (1u << (32 - 9)));

    uint64_t formatKey = caps.computeFormatKey(format);

    GrUniqueKey::Builder builder(key, kDomain, 5);
    builder[0] = dimensions.width();
    builder[1] = dimensions.height();
    builder[2] = formatKey & 0xFFFFFFFF;
    builder[3] = (formatKey >> 32) & 0xFFFFFFFF;
    builder[4] = (static_cast<uint32_t>(isProtected) << 0) |
                 (static_cast<uint32_t>(requiredUsage) << 1) |
                 (static_cast<uint32_t>(sampleCnt) << 9);
}
