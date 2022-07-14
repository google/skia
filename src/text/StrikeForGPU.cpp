/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/text/StrikeForGPU.h"

#include "include/private/chromium/SkChromeRemoteGlyphCache.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkWriteBuffer.h"

namespace sktext {
// -- StrikeRef ------------------------------------------------------------------------------------
StrikeRef::StrikeRef(sk_sp<SkStrike>&& strike) : fStrike{std::move(strike)} {
    SkASSERT(std::get<sk_sp<SkStrike>>(fStrike) != nullptr);
}

StrikeRef::StrikeRef(StrikeForGPU* strike) : fStrike {strike} {
    SkASSERT(std::get<StrikeForGPU*>(fStrike) != nullptr);
}

StrikeRef::StrikeRef(StrikeRef&&) = default;
StrikeRef& StrikeRef::operator=(StrikeRef&&) = default;

void StrikeRef::flatten(SkWriteBuffer& buffer) const {
    if (std::holds_alternative<std::monostate>(fStrike)) {
        SK_ABORT("Can't flatten. getStrikeAndSetToNullptr has already been called.");
    }

    if (const sk_sp<SkStrike>* strikePtr = std::get_if<sk_sp<SkStrike>>(&fStrike)) {
        (*strikePtr)->getDescriptor().flatten(buffer);
    } else if (StrikeForGPU*const* GPUstrikePtr = std::get_if<StrikeForGPU*>(&fStrike)) {
        (*GPUstrikePtr)->getDescriptor().flatten(buffer);
    }
}

std::optional<StrikeRef> StrikeRef::MakeFromBuffer(SkReadBuffer& buffer,
                                                   const SkStrikeClient* client) {
    auto descriptor = SkAutoDescriptor::MakeFromBuffer(buffer);
    if (!buffer.validate(descriptor.has_value())) {
        return std::nullopt;
    }

    // If there is a client, then this from a different process. Translate the typeface id from
    // that process to this process.
    if (client != nullptr) {
        if (!client->translateTypefaceID(&descriptor.value())) {
            return std::nullopt;
        }
    }

    sk_sp<SkStrike> strike = SkStrikeCache::GlobalStrikeCache()->findStrike(*descriptor->getDesc());
    if (!buffer.validate(strike != nullptr)) {
        return std::nullopt;
    }

    return StrikeRef{std::move(strike)};
}

sk_sp<SkStrike> StrikeRef::getStrikeAndSetToNullptr() {
    if (std::holds_alternative<sk_sp<SkStrike>>(fStrike)) {
        // Force a copy out of the variant because there is no more efficient way to do it.
        sk_sp<SkStrike> strike = std::get<sk_sp<SkStrike>>(fStrike);
        fStrike = std::monostate();
        return strike;
    }
    return nullptr;
}

StrikeForGPU* StrikeRef::asStrikeForGPU() {
    if (sk_sp<SkStrike>* skStrike = std::get_if<sk_sp<SkStrike>>(&fStrike)) {
        return skStrike->get();
    } else if (StrikeForGPU** remoteStrike = std::get_if<StrikeForGPU*>(&fStrike)) {
        return *remoteStrike;
    }

    SK_ABORT("Variant must be sk_sp<SkStrike> or StrikeForGPU*.");
}
}  // namespace sktext
