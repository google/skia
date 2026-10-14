/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/text/StrikeForGPU.h"

#include "src/core/SkDescriptor.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"

#include <memory>
#include <utility>

namespace sktext {
// -- SkStrikePromise ------------------------------------------------------------------------------
SkStrikePromise::SkStrikePromise(sktext::SkStrikePromise&&) = default;
SkStrikePromise& SkStrikePromise::operator=(sktext::SkStrikePromise&&) = default;
SkStrikePromise::SkStrikePromise(sk_sp<SkStrike>&& strike)
        : fStrikeInfo{std::move(strike)} {}
SkStrikePromise::SkStrikePromise(const SkStrikeSpec& spec)
        : fStrikeInfo{std::make_unique<SkStrikeSpec>(spec)} {}

SkStrike* SkStrikePromise::strike() {
    if (std::holds_alternative<std::unique_ptr<SkAutoDescriptor>>(fStrikeInfo)) {
        std::unique_ptr<SkAutoDescriptor> desc =
                std::exchange(std::get<std::unique_ptr<SkAutoDescriptor>>(fStrikeInfo), nullptr);
        fStrikeInfo = SkStrikeCache::GlobalStrikeCache()->findStrike(*desc->getDesc());
    } else if (std::holds_alternative<std::unique_ptr<SkStrikeSpec>>(fStrikeInfo)) {
        // Turn the strike spec into a strike.
        std::unique_ptr<SkStrikeSpec> spec =
                std::exchange(std::get<std::unique_ptr<SkStrikeSpec>>(fStrikeInfo), nullptr);
        fStrikeInfo = SkStrikeCache::GlobalStrikeCache()->findOrCreateStrike(*spec);
    }
    return std::get<sk_sp<SkStrike>>(fStrikeInfo).get();
}

void SkStrikePromise::resetStrike() {
    if (std::holds_alternative<sk_sp<SkStrike>>(fStrikeInfo)) {
        if (sk_sp<SkStrike>& strike = std::get<sk_sp<SkStrike>>(fStrikeInfo); strike != nullptr) {
            fStrikeInfo = std::make_unique<SkAutoDescriptor>(strike->getDescriptor());
        }
    }
}

const SkDescriptor& SkStrikePromise::descriptor() const {
    if (std::holds_alternative<std::unique_ptr<SkAutoDescriptor>>(fStrikeInfo)) {
        return *std::get<std::unique_ptr<SkAutoDescriptor>>(fStrikeInfo)->getDesc();
    }
    if (std::holds_alternative<std::unique_ptr<SkStrikeSpec>>(fStrikeInfo)) {
        return std::get<std::unique_ptr<SkStrikeSpec>>(fStrikeInfo)->descriptor();
    }
    return std::get<sk_sp<SkStrike>>(fStrikeInfo)->getDescriptor();
}

void SkStrikePromise::flatten(SkWriteBuffer& buffer) const {
    this->descriptor().flatten(buffer);
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
