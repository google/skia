/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkFontMetricsPriv.h"

#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkWriteBuffer.h"
#include "tests/Test.h"
#include "tools/fonts/FontToolUtils.h"

#include <optional>

DEF_TEST(SkFontMetricsPriv_Basic, reporter) {
    auto typeface = ToolUtils::CreateTestTypeface("monospace", SkFontStyle());
    SkFont font{typeface};
    SkStrikeSpec spec = SkStrikeSpec::MakeWithNoDevice(font);
    auto context = spec.createScalerContext();
    SkFontMetrics srcMetrics;

    // Check that font metrics round-trip.
    context->getFontMetrics(&srcMetrics);

    SkBinaryWriteBuffer writeBuffer({});
    SkFontMetricsPriv::Flatten(writeBuffer, srcMetrics);

    auto data = writeBuffer.snapshotAsData();

    SkReadBuffer readBuffer{data->data(), data->size()};

    std::optional<SkFontMetrics> dstMetrics = SkFontMetricsPriv::MakeFromBuffer(readBuffer);

    REPORTER_ASSERT(reporter, dstMetrics.has_value());
    REPORTER_ASSERT(reporter, srcMetrics == dstMetrics.value());

    // Check that a broken buffer is detected.
    // Must be multiple of 4 for a valid buffer.
    std::uint8_t brokenData[] = {1, 2, 3, 4, 5, 6, 7, 8};
    SkReadBuffer brokenBuffer{brokenData, std::size(brokenData)};

    dstMetrics = SkFontMetricsPriv::MakeFromBuffer(brokenBuffer);
    REPORTER_ASSERT(reporter, !dstMetrics.has_value());
}


