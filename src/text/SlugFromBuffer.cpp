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

namespace sktext::gpu {

sk_sp<Slug> Slug::MakeFromBuffer(SkReadBuffer& buffer) {
    auto procs = buffer.getDeserialProcs();
    if (procs.fSlugProc) {
        return procs.fSlugProc(buffer, procs.fSlugCtx);
    }
    SkDEBUGFAIL("Should have set serial procs");
    return nullptr;
}

}  // namespace sktext::gpu
