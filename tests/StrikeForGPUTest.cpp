/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkRefCnt.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkWriteBuffer.h"
#include "src/text/StrikeForGPU.h"
#include "tests/Test.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <utility>

using namespace sktext;

DEF_TEST(SkStrikePromise_Basic, reporter) {
    auto strikeCache = std::make_unique<SkStrikeCache>();
    auto [strikeSpec, _] = SkStrikeSpec::MakeCanonicalized(SkFont());

    class Pinner : public ::SkStrikePinner {
    public:
        // Changing canDelete to return true causes this test to expectedly fail.
        bool canDelete() override { return false; }
    };

    intptr_t toCompareWith;
    sk_sp<SkData> data;

    // Ensure that the ref in srcPromise is dropped.
    {
        // Make a strike with a Pinner.
        auto strike = strikeCache->createStrike(strikeSpec, nullptr, std::make_unique<Pinner>());
        toCompareWith = reinterpret_cast<intptr_t>(strike.get());
        SkStrikePromise srcPromise(std::move(strike));
        SkBinaryWriteBuffer writeBuffer({});
        srcPromise.flatten(writeBuffer);
        data = writeBuffer.snapshotAsData();
    }

    // Remove all unpinned strikes.
    strikeCache->purgeAll();

    SkReadBuffer readBuffer{data->data(), data->size()};
    std::optional<SkStrikePromise> dstPromise = SkStrikePromise::MakeFromBuffer(
            readBuffer, nullptr, strikeCache.get());

    REPORTER_ASSERT(reporter, dstPromise.has_value());
    REPORTER_ASSERT(reporter,
        reinterpret_cast<intptr_t>(dstPromise->strike()) == toCompareWith);
}
