/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/text/StrikeForGPU.h"

#include <memory>
#include <utility>

#include "include/private/chromium/SkChromeRemoteGlyphCache.h"
#include "src/core/SkDescriptor.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkWriteBuffer.h"

namespace sktext {
// -- SkStrikePromise ------------------------------------------------------------------------------
SkStrikePromise::SkStrikePromise(sktext::SkStrikePromise&&) = default;
SkStrikePromise& SkStrikePromise::operator=(sktext::SkStrikePromise&&) = default;
SkStrikePromise::SkStrikePromise(sk_sp<SkStrike>&& strike)
        : fStrikeOrSpec{std::move(strike)} {}
SkStrikePromise::SkStrikePromise(const SkStrikeSpec& spec)
        : fStrikeOrSpec{std::make_unique<SkStrikeSpec>(spec)} {}

SkStrike* SkStrikePromise::strike() {
    if (std::holds_alternative<std::unique_ptr<SkStrikeSpec>>(fStrikeOrSpec)) {
        // Turn the strike spec into a strike.
        std::unique_ptr<SkStrikeSpec> spec =
            std::exchange(std::get<std::unique_ptr<SkStrikeSpec>>(fStrikeOrSpec), nullptr);

        fStrikeOrSpec = SkStrikeCache::GlobalStrikeCache()->findOrCreateStrike(*spec);
    }
    return std::get<sk_sp<SkStrike>>(fStrikeOrSpec).get();
}

void SkStrikePromise::resetStrike() {
    fStrikeOrSpec = sk_sp<SkStrike>();
}

const SkDescriptor& SkStrikePromise::descriptor() const {
    if (std::holds_alternative<std::unique_ptr<SkStrikeSpec>>(fStrikeOrSpec)) {
        return std::get<std::unique_ptr<SkStrikeSpec>>(fStrikeOrSpec)->descriptor();
    }

    return std::get<sk_sp<SkStrike>>(fStrikeOrSpec)->getDescriptor();
}

void SkStrikePromise::flatten(SkWriteBuffer& buffer) const {
    this->descriptor().flatten(buffer);
}

std::optional<SkStrikePromise> SkStrikePromise::MakeFromBuffer(
        SkReadBuffer& buffer, const SkStrikeClient* client, SkStrikeCache* strikeCache) {
    std::optional<SkAutoDescriptor> descriptor = SkAutoDescriptor::MakeFromBuffer(buffer);
    if (!buffer.validate(descriptor.has_value())) {
        return std::nullopt;
    }

    // If there is a client, then this from a different process. Translate the SkTypefaceID from
    // the strike server (Renderer) process to strike client (GPU) process.
    if (client != nullptr) {
        if (!client->translateTypefaceID(&descriptor.value())) {
            return std::nullopt;
        }
    }

    sk_sp<SkStrike> strike = strikeCache->findStrike(*descriptor->getDesc());
    SkASSERT(strike != nullptr);
    if (!buffer.validate(strike != nullptr)) {
        return std::nullopt;
    }

    return SkStrikePromise{std::move(strike)};
}

// -- StrikeMutationMonitor ------------------------------------------------------------------------
StrikeMutationMonitor::StrikeMutationMonitor(StrikeForGPU* strike)
        : fStrike{strike} {
    fStrike->lock();
}

StrikeMutationMonitor::~StrikeMutationMonitor() {
    fStrike->unlock();
}
}  // namespace sktext
