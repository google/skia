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

class SkStrikeClient;

namespace sktext::gpu {

#if !defined(SK_SLUG_DISABLE_LEGACY_DESERIALIZE) && (defined(SK_GANESH) || defined(SK_GRAPHITE))
// This is implemented in SlugImpl.cpp
sk_sp<Slug> SkMakeSlugFromBuffer(SkReadBuffer& buffer, const SkStrikeClient* client);
#endif

sk_sp<Slug> Slug::MakeFromBuffer(SkReadBuffer& buffer) {
    auto procs = buffer.getDeserialProcs();
    if (procs.fSlugProc) {
        return procs.fSlugProc(buffer, procs.fSlugCtx);
    }
    SkDEBUGFAIL("Should have set serial procs");
#if !defined(SK_SLUG_DISABLE_LEGACY_DESERIALIZE) && (defined(SK_GANESH) || defined(SK_GRAPHITE))
    return SkMakeSlugFromBuffer(buffer, nullptr);
#else
    return nullptr;
#endif
}

}  // namespace sktext::gpu
