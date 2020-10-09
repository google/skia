/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrAttachment.h"

#include "include/private/GrResourceKey.h"
#include "src/gpu/GrCaps.h"

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

void GrAttachment::ComputeScratchKey(const GrCaps& caps,
                                     const GrBackendFormat& format,
                                     SkISize dimensions,
                                     UsageFlags requiredUsage,
                                     int sampleCnt,
                                     GrProtected isProtected,
                                     GrScratchKey* key) {
    static const GrScratchKey::ResourceType kType = GrScratchKey::GenerateResourceType();
    SkASSERT(!dimensions.isEmpty());
    SkASSERT(sampleCnt > 1);

    SkASSERT(static_cast<uint32_t>(isProtected) <= 1);
    SkASSERT(static_cast<uint32_t>(sampleCnt) < (1u << (32 - 1)));

    uint64_t formatKey = caps.computeFormatKey(format);

    GrScratchKey::Builder builder(key, kType, 6);
    builder[0] = dimensions.width();
    builder[1] = dimensions.height();
    builder[2] = (uint32_t)requiredUsage;
    builder[3] = formatKey & 0xFFFFFFFF;
    builder[4] = (formatKey >> 32) & 0xFFFFFFFF;
    builder[5] =
            (static_cast<uint32_t>(isProtected) << 0) | (static_cast<uint32_t>(sampleCnt) << 1);
}
