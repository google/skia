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

// -- StrikeMutationMonitor ------------------------------------------------------------------------
StrikeMutationMonitor::StrikeMutationMonitor(StrikeForGPU* strike)
        : fStrike{strike} {
    fStrike->lock();
}

StrikeMutationMonitor::~StrikeMutationMonitor() {
    fStrike->unlock();
}
}  // namespace sktext
