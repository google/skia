/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkExecutor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkZip.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMask.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTaskGroup.h"
#include "src/text/StrikeForGPU.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"

#include <atomic>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <memory>
#include <tuple>

struct SkRect;

using namespace sktext;
using namespace skglyph;

class Barrier {
public:
    Barrier(int threadCount) : fThreadCount(threadCount) { }
    void waitForAll() {
        fThreadCount -= 1;
        while (fThreadCount > 0) { }
    }

private:
    std::atomic<int> fThreadCount;
};

// This should stay in sync with the implementation from SubRunContainer.
static
std::tuple<SkZip<const SkPackedGlyphID, const SkPoint>,
        SkZip<SkGlyphID, SkPoint>,
        SkRect>
prepare_for_mask_drawing(
        StrikeForGPU* strike,
        SkZip<const SkGlyphID, const SkPoint> source,
        SkZip<SkPackedGlyphID, SkPoint> acceptedBuffer,
        SkZip<SkGlyphID, SkPoint> rejectedBuffer) {
    SkGlyphRect boundingRect = skglyph::empty_rect();
    int acceptedSize = 0,
        rejectedSize = 0;
    StrikeMutationMonitor m{strike};
    for (auto [glyphID, pos] : source) {
        if (!SkScalarsAreFinite(pos.x(), pos.y())) {
            continue;
        }
        const SkPackedGlyphID packedID{glyphID};
        switch (const SkGlyphDigest digest = strike->digestFor(kDirectMask, packedID);
                digest.actionFor(kDirectMask)) {
            case GlyphAction::kAccept: {
                const SkGlyphRect glyphBounds = digest.bounds().offset(pos);
                boundingRect = skglyph::rect_union(boundingRect, glyphBounds);
                acceptedBuffer[acceptedSize++] = std::make_tuple(packedID, glyphBounds.leftTop());
                break;
            }
            case GlyphAction::kReject:
                rejectedBuffer[rejectedSize++] = std::make_tuple(glyphID, pos);
                break;
            default:
                break;
        }
    }

    return {acceptedBuffer.first(acceptedSize),
            rejectedBuffer.first(rejectedSize),
            boundingRect.rect()};
}

DEF_TEST(SkStrikeMultiThread, Reporter) {
    sk_sp<SkTypeface> typeface =
            ToolUtils::create_portable_typeface("serif", SkFontStyle::Italic());
    static constexpr int kThreadCount = 4;

    Barrier barrier{kThreadCount};

    SkFont font;
    font.setEdging(SkFont::Edging::kAntiAlias);
    font.setSubpixel(true);
    font.setTypeface(typeface);

    SkGlyphID glyphs['z'];
    SkPoint pos['z'];
    for (int c = ' '; c < 'z'; c++) {
        glyphs[c] = font.unicharToGlyph(c);
        pos[c] = {30.0f * c + 30, 30.0f};
    }
    constexpr size_t glyphCount = 'z' - ' ';
    auto data = SkMakeZip(glyphs, pos).subspan(SkTo<int>(' '), glyphCount);

    SkPaint defaultPaint;
    SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(
            font, defaultPaint, SkSurfaceProps(0, kUnknown_SkPixelGeometry),
            SkScalerContextFlags::kNone, SkMatrix::I());

    SkStrikeCache strikeCache;

    // Make our own executor so the --threads parameter doesn't mess things up.
    auto executor = SkExecutor::MakeFIFOThreadPool(kThreadCount);
    for (int tries = 0; tries < 100; tries++) {
        SkStrike strike{&strikeCache, strikeSpec, strikeSpec.createScalerContext(), nullptr,
                        nullptr};

        auto perThread = [&](int threadIndex) {
            barrier.waitForAll();

            auto local = data.subspan(threadIndex * 2, data.size() - kThreadCount * 2);
            for (int i = 0; i < 100; i++) {
                // Accepted buffers.
                SkSTArray<64, SkPackedGlyphID> acceptedPackedGlyphIDs;
                SkSTArray<64, SkPoint> acceptedPositions;
                SkSTArray<64, SkMask::Format> acceptedFormats;
                acceptedPackedGlyphIDs.resize(glyphCount);
                acceptedPositions.resize(glyphCount);
                const auto acceptedBuffer = SkMakeZip(acceptedPackedGlyphIDs, acceptedPositions);

                // Rejected buffers.
                SkSTArray<64, SkGlyphID> rejectedGlyphIDs;
                SkSTArray<64, SkPoint> rejectedPositions;
                rejectedGlyphIDs.resize(glyphCount);
                rejectedPositions.resize(glyphCount);
                const auto rejectedBuffer = SkMakeZip(rejectedGlyphIDs, rejectedPositions);

                SkZip<const SkGlyphID, const SkPoint> source = local;

                auto [accepted, rejected, bounds] =
                prepare_for_mask_drawing(&strike, source, acceptedBuffer, rejectedBuffer);
                source = rejected;
            }
        };

        SkTaskGroup(*executor).batch(kThreadCount, perThread);
    }
}
