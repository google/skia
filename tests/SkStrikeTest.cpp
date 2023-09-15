/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkExecutor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkZip.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMask.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTaskGroup.h"
#include "src/core/SkWriteBuffer.h"
#include "src/text/StrikeForGPU.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <memory>
#include <tuple>
#include <vector>

using namespace skia_private;

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
                STArray<64, SkPackedGlyphID> acceptedPackedGlyphIDs;
                STArray<64, SkPoint> acceptedPositions;
                STArray<64, SkMask::Format> acceptedFormats;
                acceptedPackedGlyphIDs.resize(glyphCount);
                acceptedPositions.resize(glyphCount);
                const auto acceptedBuffer = SkMakeZip(acceptedPackedGlyphIDs, acceptedPositions);

                // Rejected buffers.
                STArray<64, SkGlyphID> rejectedGlyphIDs;
                STArray<64, SkPoint> rejectedPositions;
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

class SkGlyphTestPeer {
public:
    static void SetGlyph(SkGlyph* glyph) {
        // Tweak the bounds to make them unique based on glyph id.
        const SkGlyphID uniquify = glyph->getGlyphID();
        glyph->fAdvanceX = 10;
        glyph->fAdvanceY = 11;
        glyph->fLeft = -1 - uniquify;
        glyph->fTop = -2;
        glyph->fWidth = 8;
        glyph->fHeight = 9;
        glyph->fMaskFormat = SkMask::Format::kA8_Format;
    }
};

class SkStrikeTestingPeer {
public:
    static SkGlyph* GetGlyph(SkStrike* strike, SkPackedGlyphID packedID) {
        SkAutoMutexExclusive m{strike->fStrikeLock};
        return strike->glyph(packedID);
    }
};

DEF_TEST(SkStrike_FlattenByType, reporter) {
    std::vector<SkGlyph> imagesToSend;
    std::vector<SkGlyph> pathsToSend;
    std::vector<SkGlyph> drawablesToSend;
    SkArenaAlloc alloc{256};

    // Make a mask glyph and put it in the glyphs to send.
    const SkPackedGlyphID maskPackedGlyphID((SkGlyphID)10);
    SkGlyph maskGlyph{maskPackedGlyphID};
    SkGlyphTestPeer::SetGlyph(&maskGlyph);

    static constexpr uint8_t X = 0xff;
    static constexpr uint8_t O = 0x00;
    uint8_t imageData[][8] = {
            {X,X,X,X,X,X,X,X},
            {X,O,O,O,O,O,O,X},
            {X,O,O,O,O,O,O,X},
            {X,O,O,O,O,O,O,X},
            {X,O,O,X,X,O,O,X},
            {X,O,O,O,O,O,O,X},
            {X,O,O,O,O,O,O,X},
            {X,O,O,O,O,O,O,X},
            {X,X,X,X,X,X,X,X},
    };
    maskGlyph.setImage(&alloc, imageData);
    imagesToSend.emplace_back(maskGlyph);

    // Make a path glyph and put it in the glyphs to send.
    const SkPackedGlyphID pathPackedGlyphID((SkGlyphID)11);
    SkGlyph pathGlyph{pathPackedGlyphID};
    SkGlyphTestPeer::SetGlyph(&pathGlyph);
    SkPath path;
    path.addRect(pathGlyph.rect());
    pathGlyph.setPath(&alloc, &path, false);
    pathsToSend.emplace_back(pathGlyph);

    // Make a drawable glyph and put it in the glyphs to send.
    const SkPackedGlyphID drawablePackedGlyphID((SkGlyphID)12);
    SkGlyph drawableGlyph{drawablePackedGlyphID};
    SkGlyphTestPeer::SetGlyph(&drawableGlyph);
    class TestDrawable final : public SkDrawable {
    public:
        TestDrawable(SkRect rect) : fRect(rect) {}

    private:
        const SkRect fRect;
        SkRect onGetBounds() override { return fRect;  }
        size_t onApproximateBytesUsed() override {
            return 0;
        }
        void onDraw(SkCanvas* canvas) override {
            SkPaint paint;
            canvas->drawRect(fRect, paint);
        }
    };

    sk_sp<SkDrawable> drawable = sk_make_sp<TestDrawable>(drawableGlyph.rect());
    REPORTER_ASSERT(reporter, !drawableGlyph.setDrawableHasBeenCalled());
    drawableGlyph.setDrawable(&alloc, drawable);
    REPORTER_ASSERT(reporter, drawableGlyph.setDrawableHasBeenCalled());
    REPORTER_ASSERT(reporter, drawableGlyph.drawable() != nullptr);
    drawablesToSend.emplace_back(drawableGlyph);

    // Test the FlattenGlyphsByType method.
    SkBinaryWriteBuffer writeBuffer({});
    SkStrike::FlattenGlyphsByType(writeBuffer, imagesToSend, pathsToSend, drawablesToSend);
    auto data = writeBuffer.snapshotAsData();

    // Make a strike to merge into.
    SkStrikeCache strikeCache;
    auto dstTypeface = SkTypeface::MakeFromName("monospace", SkFontStyle());
    SkFont font{dstTypeface};
    SkStrikeSpec spec = SkStrikeSpec::MakeWithNoDevice(font);
    sk_sp<SkStrike> strike = spec.findOrCreateStrike(&strikeCache);

    // Test the mergeFromBuffer method.
    SkReadBuffer readBuffer{data->data(), data->size()};
    strike->mergeFromBuffer(readBuffer);

    // Check mask glyph.
    SkGlyph* dstMaskGlyph = SkStrikeTestingPeer::GetGlyph(strike.get(), maskPackedGlyphID);
    REPORTER_ASSERT(reporter, maskGlyph.rect() == dstMaskGlyph->rect());
    REPORTER_ASSERT(reporter, dstMaskGlyph->setImageHasBeenCalled());
    REPORTER_ASSERT(reporter, dstMaskGlyph->image() != nullptr);

    // Check path glyph.
    SkGlyph* dstPathGlyph = SkStrikeTestingPeer::GetGlyph(strike.get(), pathPackedGlyphID);
    REPORTER_ASSERT(reporter, pathGlyph.rect() == dstPathGlyph->rect());
    REPORTER_ASSERT(reporter, dstPathGlyph->setPathHasBeenCalled());
    REPORTER_ASSERT(reporter, dstPathGlyph->path() != nullptr);

    // Check drawable glyph.
    SkGlyph* dstDrawableGlyph = SkStrikeTestingPeer::GetGlyph(strike.get(),drawablePackedGlyphID);
    REPORTER_ASSERT(reporter, drawableGlyph.rect() == dstDrawableGlyph->rect());
    REPORTER_ASSERT(reporter, dstDrawableGlyph->setDrawableHasBeenCalled());
    REPORTER_ASSERT(reporter, dstDrawableGlyph->drawable() != nullptr);
}
