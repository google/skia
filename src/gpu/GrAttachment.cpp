/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrAttachment.h"

#include "include/private/GrResourceKey.h"
#include "src/gpu/GrBackendUtils.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDataUtils.h"
#include "src/gpu/GrGpu.h"

size_t GrAttachment::onGpuMemorySize() const {
    GrBackendFormat format = this->backendFormat();
    SkImage::CompressionType compression = GrBackendFormatToCompressionType(format);

    uint64_t size = GrNumBlocks(compression, this->dimensions());
    size *= GrBackendFormatBytesPerBlock(this->backendFormat());
    size *= this->numSamples();
    return size;
}

static void build_key(GrResourceKey::Builder* builder,
                      const GrCaps& caps,
                      const GrBackendFormat& format,
                      SkISize dimensions,
                      GrAttachment::UsageFlags requiredUsage,
                      int sampleCnt,
                      GrProtected isProtected) {
    SkASSERT(!dimensions.isEmpty());

    SkASSERT(static_cast<uint32_t>(isProtected) <= 1);
    SkASSERT(static_cast<uint32_t>(requiredUsage) < (1u << 8));
    SkASSERT(static_cast<uint32_t>(sampleCnt) < (1u << (32 - 9)));

    uint64_t formatKey = caps.computeFormatKey(format);
    (*builder)[0] = dimensions.width();
    (*builder)[1] = dimensions.height();
    (*builder)[2] = formatKey & 0xFFFFFFFF;
    (*builder)[3] = (formatKey >> 32) & 0xFFFFFFFF;
    (*builder)[4] = (static_cast<uint32_t>(isProtected) << 0) |
                    (static_cast<uint32_t>(requiredUsage) << 1) |
                    (static_cast<uint32_t>(sampleCnt) << 9);
}

void GrAttachment::ComputeSharedAttachmentUniqueKey(const GrCaps& caps,
                                                    const GrBackendFormat& format,
                                                    SkISize dimensions,
                                                    UsageFlags requiredUsage,
                                                    int sampleCnt,
                                                    GrProtected isProtected,
                                                    GrUniqueKey* key) {
    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();

    GrUniqueKey::Builder builder(key, kDomain, 5);
    build_key(&builder, caps, format, dimensions, requiredUsage, sampleCnt, isProtected);
}

void GrAttachment::ComputeScratchKey(const GrCaps& caps,
                                     const GrBackendFormat& format,
                                     SkISize dimensions,
                                     UsageFlags requiredUsage,
                                     int sampleCnt,
                                     GrProtected isProtected,
                                     GrScratchKey* key) {
    static const GrScratchKey::ResourceType kType = GrScratchKey::GenerateResourceType();

    SkASSERT(sampleCnt > 1);

    GrScratchKey::Builder builder(key, kType, 5);
    build_key(&builder, caps, format, dimensions, requiredUsage, sampleCnt, isProtected);
}

void GrAttachment::computeScratchKey(GrScratchKey* key) const {
    if (fSupportedUsages & UsageFlags::kColorAttachment) {
        auto isProtected = this->isProtected() ? GrProtected::kYes : GrProtected::kNo;
        ComputeScratchKey(*this->getGpu()->caps(), this->backendFormat(), this->dimensions(),
                          fSupportedUsages, this->numSamples(), isProtected, key);
    }
}
