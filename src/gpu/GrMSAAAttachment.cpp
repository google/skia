/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrMSAAAttachment.h"

#include "src/gpu/GrCaps.h"

void GrMSAAAttachment::ComputeScratchKey(const GrCaps& caps,
                                         const GrBackendFormat& format,
                                         SkISize dimensions,
                                         int sampleCnt,
                                         GrProtected isProtected,
                                         GrScratchKey* key) {
    static const GrScratchKey::ResourceType kType = GrScratchKey::GenerateResourceType();
    SkASSERT(!dimensions.isEmpty());
    SkASSERT(sampleCnt > 1);

    SkASSERT(static_cast<uint32_t>(isProtected) <= 1);
    SkASSERT(static_cast<uint32_t>(sampleCnt) < (1u << (32 - 1)));

    uint64_t formatKey = caps.computeFormatKey(format);

    GrScratchKey::Builder builder(key, kType, 5);
    builder[0] = dimensions.width();
    builder[1] = dimensions.height();
    builder[2] = formatKey & 0xFFFFFFFF;
    builder[3] = (formatKey >> 32) & 0xFFFFFFFF;
    builder[4] = (static_cast<uint32_t>(isProtected) << 0) |
                 (static_cast<uint32_t>(sampleCnt) << 1);
}
