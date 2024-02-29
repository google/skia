/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/core/SkSerialProcs.h"
#include "include/private/base/SkAssert.h"
#include "include/private/chromium/Slug.h"
#include "src/core/SkReadBuffer.h"

#include <atomic>
#include <cstdint>

// This file contains Slug methods that need to be defined on CPU and GPU builds, even though
// Slugs aren't fully implemented in the CPU backend (yet?)

namespace sktext::gpu {

sk_sp<Slug> Slug::MakeFromBuffer(SkReadBuffer& buffer) {
    auto procs = buffer.getDeserialProcs();
    if (procs.fSlugProc) {
        return procs.fSlugProc(buffer, procs.fSlugCtx);
    }
    SkDEBUGFAIL("Should have set serial procs");
    return nullptr;
}

uint32_t Slug::NextUniqueID() {
    static std::atomic<uint32_t> nextUnique = 1;
    return nextUnique++;
}

}  // namespace sktext::gpu
