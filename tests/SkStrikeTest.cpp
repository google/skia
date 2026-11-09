/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkExecutor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkMaskFilter.h"
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
#include "include/private/SkMutex.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTo.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkExecutors.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMask.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTaskGroup.h"
#include "src/core/SkWriteBuffer.h"
#include "src/core/SkZip.h"
#include "src/effects/SkEmbossMaskFilter.h"
#include "src/text/StrikeForGPU.h"
#include "src/text/gpu/SDFMaskFilter.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/fonts/TestEmptyTypeface.h"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <memory>
#include <optional>
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
        if (!SkIsFinite(pos.x(), pos.y())) {
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
            ToolUtils::CreatePortableTypeface("serif", SkFontStyle::Italic());
    static constexpr int kThreadCount = 4;

    Barrier barrier{kThreadCount};

    SkFont font;
    font.setEdging(SkFont::Edging::kAntiAlias);
    font.setSubpixel(true);
    font.setTypeface(typeface);

    std::array<SkGlyphID, 'z'> glyphs;
    std::array<SkPoint, 'z'> pos;
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
    auto executor = SkFunctionalExecutors::MakeFIFOThreadPool(kThreadCount);
    for (int tries = 0; tries < 100; tries++) {
        SkStrike strike{&strikeCache, strikeSpec, strikeSpec.createScalerContext(), nullptr,
                        nullptr};

        SkTaskGroup taskGroup(*executor);
        for (int threadIndex = 0; threadIndex < kThreadCount; ++threadIndex) {
            taskGroup.add([&, threadIndex]() {
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
            });
        }
        taskGroup.wait();
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
    SkPath path = SkPath::Rect(pathGlyph.rect());
    pathGlyph.setPath(&alloc, &path, false, false);
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
    auto dstTypeface = ToolUtils::CreateTestTypeface("monospace", SkFontStyle());
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

DEF_TEST(SkScalerContext_GetImageNonDeterministicBounds, reporter) {
    class NonDeterministicTypeface final : public TestEmptyTypeface {
    public:
        explicit NonDeterministicTypeface(SkRect secondBounds) : fSecondBounds(secondBounds) {}

    protected:
        std::unique_ptr<SkScalerContext> onCreateScalerContext(
                const SkScalerContextEffects& effects, const SkDescriptor* desc) const override {
            class ScalerContext final : public SkScalerContext {
            public:
                ScalerContext(NonDeterministicTypeface& tf,
                              const SkScalerContextEffects& effects,
                              const SkDescriptor* desc,
                              SkRect secondBounds)
                        : SkScalerContext(tf, effects, desc), fSecondBounds(secondBounds) {}

            protected:
                GlyphMetrics generateMetrics(const SkGlyph& glyph, SkArenaAlloc*) override {
                    GlyphMetrics mx(glyph.maskFormat());
                    // Force the raster getImage() path instead of drawing from a glyph path
                    mx.neverRequestPath = true;
                    // Return valid 10x10 bounds on initial makeGlyph(), then fSecondBounds when
                    // getImage() reloads the unfiltered glyph for mask filtering
                    mx.bounds =
                            (fMetricsCallCount++ == 0) ? SkRect::MakeWH(10, 10) : fSecondBounds;
                    return mx;
                }

                void generateImage(const SkGlyph& glyph, void* imageBuffer) override {
                    // Intentionally use rowBytes() * height() like SkScalerContextFTUtils does;
                    // if getImage() fails to reject an imageTooLarge() or empty unfiltered glyph,
                    // this overflows the small origGlyph buffer or dereferences nullptr
                    memset(imageBuffer, 0xAA, glyph.rowBytes() * glyph.height());
                }

                std::optional<GeneratedPath> generatePath(const SkGlyph&) override {
                    return std::nullopt;
                }

                void generateFontMetrics(SkFontMetrics*) override {}

            private:
                const SkRect fSecondBounds;
                int fMetricsCallCount = 0;
            };

            return std::make_unique<ScalerContext>(
                    *const_cast<NonDeterministicTypeface*>(this), effects, desc, fSecondBounds);
        }

    private:
        const SkRect fSecondBounds;
    };

    for (const SkRect& secondBounds : {
                 // Exceeds kMaxGlyphWidth so imageTooLarge() is true and imageSize() returns 0
                 SkRect::MakeWH(1 << 14, 10),
                 // Zero-area unfiltered glyph on the second load
                 SkRect::MakeEmpty(),
                 // Unfiltered bounds disjoint horizontally and vertically from the 10x10 dstMask
                 SkRect::MakeXYWH(500, 0, 10, 10),
                 SkRect::MakeXYWH(0, 500, 10, 10),
         }) {
        sk_sp<SkTypeface> tf = sk_make_sp<NonDeterministicTypeface>(secondBounds);
        SkFont font(tf, 16.0f);
        SkPaint paint;
        // A mask filter triggers the second makeGlyph() call inside SkScalerContext::getImage
        paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 1.0f));

        SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(
                font, paint, SkSurfaceProps(0, kUnknown_SkPixelGeometry),
                SkScalerContextFlags::kNone, SkMatrix::I());
        std::unique_ptr<SkScalerContext> context = strikeSpec.createScalerContext();

        SkArenaAlloc alloc{256};
        SkGlyph glyph = context->makeGlyph(SkPackedGlyphID((SkGlyphID)1), &alloc);
        REPORTER_ASSERT(reporter, !glyph.isEmpty());
        REPORTER_ASSERT(reporter, !glyph.imageTooLarge());

        REPORTER_ASSERT(reporter, glyph.setImage(&alloc, context.get()));
        REPORTER_ASSERT(reporter, glyph.image() != nullptr);

        const uint8_t* bytes = static_cast<const uint8_t*>(glyph.image());
        for (size_t i = 0; i < glyph.imageSize(); ++i) {
            REPORTER_ASSERT(reporter, bytes[i] == 0);
        }
    }
}

namespace {
// Creates a synthetic typeface whose first and subsequent glyph metric queries return
// caller-specified bounds, mask formats, and optional outline paths, while generateImage() fills
// raster glyph buffers with 0xAA
sk_sp<SkTypeface> make_sequenced_typeface(SkRect firstBounds,
                                          SkMask::Format firstFormat,
                                          SkRect secondBounds,
                                          SkMask::Format secondFormat,
                                          std::optional<SkPath> firstPath = std::nullopt,
                                          std::optional<SkPath> secondPath = std::nullopt) {
    class SequencedTypeface final : public TestEmptyTypeface {
    public:
        SequencedTypeface(SkRect firstBounds,
                          SkMask::Format firstFormat,
                          SkRect secondBounds,
                          SkMask::Format secondFormat,
                          std::optional<SkPath> firstPath,
                          std::optional<SkPath> secondPath)
                : fFirstBounds(firstBounds)
                , fFirstFormat(firstFormat)
                , fSecondBounds(secondBounds)
                , fSecondFormat(secondFormat)
                , fFirstPath(std::move(firstPath))
                , fSecondPath(std::move(secondPath)) {}

    protected:
        std::unique_ptr<SkScalerContext> onCreateScalerContext(
                const SkScalerContextEffects& effects, const SkDescriptor* desc) const override {
            class ScalerContext final : public SkScalerContext {
            public:
                ScalerContext(SequencedTypeface& tf,
                              const SkScalerContextEffects& effects,
                              const SkDescriptor* desc)
                        : SkScalerContext(tf, effects, desc), fTypeface(tf) {}

            protected:
                GlyphMetrics generateMetrics(const SkGlyph&, SkArenaAlloc*) override {
                    const bool isFirst = (fMetricsCallCount++ == 0);
                    GlyphMetrics mx(isFirst ? fTypeface.fFirstFormat : fTypeface.fSecondFormat);
                    const std::optional<SkPath>& path =
                            isFirst ? fTypeface.fFirstPath : fTypeface.fSecondPath;
                    if (path.has_value()) {
                        mx.generatedPath = GeneratedPath{*path, false};
                    } else {
                        // Force the raster getImage() path instead of drawing from a glyph path
                        mx.neverRequestPath = true;
                    }
                    mx.bounds = isFirst ? fTypeface.fFirstBounds : fTypeface.fSecondBounds;
                    return mx;
                }

                void generateImage(const SkGlyph& glyph, void* imageBuffer) override {
                    memset(imageBuffer, 0xAA, glyph.rowBytes() * glyph.height());
                }

                std::optional<GeneratedPath> generatePath(const SkGlyph&) override {
                    return std::nullopt;
                }

                void generateFontMetrics(SkFontMetrics*) override {}

            private:
                const SequencedTypeface& fTypeface;
                int fMetricsCallCount = 0;
            };

            return std::make_unique<ScalerContext>(
                    *const_cast<SequencedTypeface*>(this), effects, desc);
        }

    private:
        const SkRect fFirstBounds;
        const SkMask::Format fFirstFormat;
        const SkRect fSecondBounds;
        const SkMask::Format fSecondFormat;
        const std::optional<SkPath> fFirstPath;
        const std::optional<SkPath> fSecondPath;
    };

    return sk_make_sp<SequencedTypeface>(firstBounds,
                                         firstFormat,
                                         secondBounds,
                                         secondFormat,
                                         std::move(firstPath),
                                         std::move(secondPath));
}
}  // namespace

// Verifies that getImage() zeroes the destination buffer instead of overflowing 1-bit rows
// or aborting when consecutive glyph loads diverge in bounds or mask format under a mask filter
DEF_TEST(SkScalerContext_GetImageMaskFormatMismatchAndSubByte, reporter) {
    // 1-bit kBW_Format with a no-op blur (sigma 0.1) and wider second-load bounds: a byte-per-pixel
    // fallback copy would write 64 bytes per row into 8-byte (64-bit) destination rows
    {
        sk_sp<SkTypeface> tf = make_sequenced_typeface(SkRect::MakeWH(64, 10),
                                                       SkMask::kBW_Format,
                                                       SkRect::MakeWH(72, 20),
                                                       SkMask::kBW_Format);
        SkFont font(tf, 16.0f);
        font.setEdging(SkFont::Edging::kAlias);
        SkPaint paint;
        paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 0.1f));

        SkStrikeSpec strikeSpec =
                SkStrikeSpec::MakeMask(font,
                                       paint,
                                       SkSurfaceProps(0, kUnknown_SkPixelGeometry),
                                       SkScalerContextFlags::kNone,
                                       SkMatrix::I());
        std::unique_ptr<SkScalerContext> context = strikeSpec.createScalerContext();

        char storage[1024];
        memset(storage, 0x5A, sizeof(storage));
        SkArenaAlloc alloc(storage, sizeof(storage), 1024);
        SkGlyph glyph = context->makeGlyph(SkPackedGlyphID((SkGlyphID)1), &alloc);
        REPORTER_ASSERT(reporter, glyph.maskFormat() == SkMask::kBW_Format);
        REPORTER_ASSERT(reporter, glyph.imageSize() > 0 && glyph.imageSize() == 80);
        REPORTER_ASSERT(reporter, glyph.setImage(&alloc, context.get()));

        const uint8_t* bytes = static_cast<const uint8_t*>(glyph.image());
        for (size_t i = 0; i < glyph.imageSize(); ++i) {
            REPORTER_ASSERT(reporter, bytes[i] == 0);
        }
    }

    // filterMask() succeeds on initial kA8 bounds (setting origGlyph to k3D_Format), then rejects
    // the second load's kBW_Format glyph, leaving srcMask.fFormat != origGlyph.fMaskFormat
    {
        SkEmbossMaskFilter::Light light{{1.0f, 1.0f, 1.0f}, 0, 128, 16};
        sk_sp<SkTypeface> tf = make_sequenced_typeface(SkRect::MakeWH(10, 10),
                                                       SkMask::kA8_Format,
                                                       SkRect::MakeWH(10, 10),
                                                       SkMask::kBW_Format);
        SkFont font(tf, 16.0f);
        SkPaint paint;
        paint.setMaskFilter(SkEmbossMaskFilter::Make(1.0f, light));

        SkStrikeSpec strikeSpec =
                SkStrikeSpec::MakeMask(font,
                                       paint,
                                       SkSurfaceProps(0, kUnknown_SkPixelGeometry),
                                       SkScalerContextFlags::kNone,
                                       SkMatrix::I());
        std::unique_ptr<SkScalerContext> context = strikeSpec.createScalerContext();

        char storage[1024];
        memset(storage, 0x5A, sizeof(storage));
        SkArenaAlloc alloc(storage, sizeof(storage), 1024);
        SkGlyph glyph = context->makeGlyph(SkPackedGlyphID((SkGlyphID)1), &alloc);
        REPORTER_ASSERT(reporter, glyph.maskFormat() == SkMask::k3D_Format);
        REPORTER_ASSERT(reporter, glyph.imageSize() > 0 && glyph.imageSize() == 300);
        REPORTER_ASSERT(reporter, glyph.setImage(&alloc, context.get()));

        const uint8_t* bytes = static_cast<const uint8_t*>(glyph.image());
        for (size_t i = 0; i < glyph.imageSize(); ++i) {
            REPORTER_ASSERT(reporter, bytes[i] == 0);
        }
    }
}

// Verifies that clipped mask-filtered glyphs zero all 3 k3D planes and step by per-plane strides,
// while still copying overlapping 1-byte-per-pixel kSDF masks
DEF_TEST(SkScalerContext_GetImageClipped3DAndSDF, reporter) {
    {
        SkEmbossMaskFilter::Light light{{1.0f, 1.0f, 1.0f}, 0, 128, 16};
        sk_sp<SkTypeface> tf = make_sequenced_typeface(SkRect::MakeWH(10, 10),
                                                       SkMask::kA8_Format,
                                                       SkRect::MakeWH(10, 6),
                                                       SkMask::kA8_Format);
        SkFont font(tf, 16.0f);
        SkPaint paint;
        paint.setMaskFilter(SkEmbossMaskFilter::Make(1.0f, light));

        SkStrikeSpec strikeSpec =
                SkStrikeSpec::MakeMask(font,
                                       paint,
                                       SkSurfaceProps(0, kUnknown_SkPixelGeometry),
                                       SkScalerContextFlags::kNone,
                                       SkMatrix::I());
        std::unique_ptr<SkScalerContext> context = strikeSpec.createScalerContext();

        char storage[1024];
        memset(storage, 0x5A, sizeof(storage));
        SkArenaAlloc alloc(storage, sizeof(storage), 1024);
        SkGlyph glyph = context->makeGlyph(SkPackedGlyphID((SkGlyphID)1), &alloc);
        REPORTER_ASSERT(reporter, glyph.maskFormat() == SkMask::k3D_Format);
        REPORTER_ASSERT(reporter, glyph.imageSize() == 300);
        REPORTER_ASSERT(reporter, glyph.setImage(&alloc, context.get()));

        const uint8_t* bytes = static_cast<const uint8_t*>(glyph.image());
        for (int plane = 0; plane < 3; ++plane) {
            const uint8_t* planeBytes = bytes + plane * 100;
            // Rows 6..9 lie outside the 10x6 second-load bounds and must be zero in all 3 planes
            for (int i = 60; i < 100; ++i) {
                REPORTER_ASSERT(reporter, planeBytes[i] == 0);
            }
        }
        // Alpha plane (0) carries 0xAA and multiply plane (1) carries ambient >= 128 in rows 0..5
        for (int i = 0; i < 60; ++i) {
            REPORTER_ASSERT(reporter, bytes[i] == 0xAA);
            REPORTER_ASSERT(reporter, bytes[100 + i] >= 128);
        }
    }

#if !defined(SK_DISABLE_SDF_TEXT)
    {
        sk_sp<SkTypeface> tf = make_sequenced_typeface(SkRect::MakeWH(10, 10),
                                                       SkMask::kA8_Format,
                                                       SkRect::MakeWH(8, 10),
                                                       SkMask::kA8_Format);
        SkFont font(tf, 16.0f);
        SkPaint paint;
        paint.setMaskFilter(sktext::gpu::SDFMaskFilter::Make());

        SkStrikeSpec strikeSpec =
                SkStrikeSpec::MakeMask(font,
                                       paint,
                                       SkSurfaceProps(0, kUnknown_SkPixelGeometry),
                                       SkScalerContextFlags::kNone,
                                       SkMatrix::I());
        std::unique_ptr<SkScalerContext> context = strikeSpec.createScalerContext();

        char storage[2048];
        memset(storage, 0x5A, sizeof(storage));
        SkArenaAlloc alloc(storage, sizeof(storage), 2048);
        SkGlyph glyph = context->makeGlyph(SkPackedGlyphID((SkGlyphID)1), &alloc);
        REPORTER_ASSERT(reporter, glyph.maskFormat() == SkMask::kSDF_Format);
        REPORTER_ASSERT(reporter, glyph.width() == 18 && glyph.height() == 18);
        REPORTER_ASSERT(reporter, glyph.setImage(&alloc, context.get()));

        const uint8_t* bytes = static_cast<const uint8_t*>(glyph.image());
        bool hasNonZeroOverlap = false;
        for (int y = 0; y < 18; ++y) {
            for (int x = 0; x < 16; ++x) {
                hasNonZeroOverlap |= (bytes[y * 18 + x] != 0);
            }
            for (int x = 16; x < 18; ++x) {
                REPORTER_ASSERT(reporter, bytes[y * 18 + x] == 0);
            }
        }
        REPORTER_ASSERT(reporter, hasNonZeroOverlap);
    }
#endif
}

// Verifies that path-rendered mask-filtered glyphs inspect and rasterize the reloaded unfiltered
// glyph's path and format rather than stale state from the initial glyph load
DEF_TEST(SkScalerContext_GetImageFromPathDivergence, reporter) {
    // Load 1 returns a stroked kA8 outline path while Load 2 diverges to a non-outline kARGB32
    // raster glyph; getImage() must fall back to generateImage() so blur extracts kARGB32 alpha
    // instead of passing kARGB32 to GenerateImageFromPath
    {
        sk_sp<SkTypeface> tf = make_sequenced_typeface(SkRect::MakeWH(10, 10),
                                                       SkMask::kA8_Format,
                                                       SkRect::MakeWH(10, 10),
                                                       SkMask::kARGB32_Format,
                                                       SkPath::Rect(SkRect::MakeWH(10, 10)),
                                                       std::nullopt);
        SkFont font(tf, 16.0f);
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(1.0f);
        paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 1.0f));

        SkStrikeSpec strikeSpec =
                SkStrikeSpec::MakeMask(font,
                                       paint,
                                       SkSurfaceProps(0, kUnknown_SkPixelGeometry),
                                       SkScalerContextFlags::kNone,
                                       SkMatrix::I());
        std::unique_ptr<SkScalerContext> context = strikeSpec.createScalerContext();

        char storage[2048];
        memset(storage, 0x5A, sizeof(storage));
        SkArenaAlloc alloc(storage, sizeof(storage), 2048);
        SkGlyph glyph = context->makeGlyph(SkPackedGlyphID((SkGlyphID)1), &alloc);
        REPORTER_ASSERT(reporter, glyph.maskFormat() == SkMask::kA8_Format);
        REPORTER_ASSERT(reporter, glyph.imageSize() > 0);
        REPORTER_ASSERT(reporter, glyph.setImage(&alloc, context.get()));

        const uint8_t* bytes = static_cast<const uint8_t*>(glyph.image());
        bool hasNonZero = false;
        for (size_t i = 0; i < glyph.imageSize(); ++i) {
            hasNonZero |= (bytes[i] != 0);
        }
        REPORTER_ASSERT(reporter, hasNonZero);
    }

    // Reverse divergence: Load 1 is a non-outline kARGB32 glyph rejected by SkEmbossMaskFilter
    // while Load 2 returns a kA8 outline path; getImage() must not assert on origGlyph.fMaskFormat
    // and must zero origGlyph when emboss produces k3D against origGlyph's kARGB32
    {
        SkEmbossMaskFilter::Light light{{1.0f, 1.0f, 1.0f}, 0, 128, 16};
        sk_sp<SkTypeface> tf = make_sequenced_typeface(SkRect::MakeWH(10, 10),
                                                       SkMask::kARGB32_Format,
                                                       SkRect::MakeWH(10, 10),
                                                       SkMask::kA8_Format,
                                                       std::nullopt,
                                                       SkPath::Rect(SkRect::MakeWH(10, 10)));
        SkFont font(tf, 16.0f);
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(1.0f);
        paint.setMaskFilter(SkEmbossMaskFilter::Make(1.0f, light));

        SkStrikeSpec strikeSpec =
                SkStrikeSpec::MakeMask(font,
                                       paint,
                                       SkSurfaceProps(0, kUnknown_SkPixelGeometry),
                                       SkScalerContextFlags::kNone,
                                       SkMatrix::I());
        std::unique_ptr<SkScalerContext> context = strikeSpec.createScalerContext();

        char storage[2048];
        memset(storage, 0x5A, sizeof(storage));
        SkArenaAlloc alloc(storage, sizeof(storage), 2048);
        SkGlyph glyph = context->makeGlyph(SkPackedGlyphID((SkGlyphID)1), &alloc);
        REPORTER_ASSERT(reporter, glyph.maskFormat() == SkMask::kARGB32_Format);
        REPORTER_ASSERT(reporter, glyph.imageSize() > 0);
        REPORTER_ASSERT(reporter, glyph.setImage(&alloc, context.get()));

        const uint8_t* bytes = static_cast<const uint8_t*>(glyph.image());
        for (size_t i = 0; i < glyph.imageSize(); ++i) {
            REPORTER_ASSERT(reporter, bytes[i] == 0);
        }
    }

    // Load 1 returns a hollow 20x20 stroked frame while Load 2 shifts to a 2x2 inner stroked rect
    // inside the hollow center; rasterizing Load 1's stale path into Load 2's [7..13, 7..13] mask
    // would produce an all-zero image instead of the blurred inner rect
    {
        sk_sp<SkTypeface> tf = make_sequenced_typeface(SkRect::MakeWH(20, 20),
                                                       SkMask::kA8_Format,
                                                       SkRect::MakeXYWH(9, 9, 2, 2),
                                                       SkMask::kA8_Format,
                                                       SkPath::Rect(SkRect::MakeWH(20, 20)),
                                                       SkPath::Rect(SkRect::MakeXYWH(9, 9, 2, 2)));
        SkFont font(tf, 16.0f);
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(2.0f);
        paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 0.5f));

        SkStrikeSpec strikeSpec =
                SkStrikeSpec::MakeMask(font,
                                       paint,
                                       SkSurfaceProps(0, kUnknown_SkPixelGeometry),
                                       SkScalerContextFlags::kNone,
                                       SkMatrix::I());
        std::unique_ptr<SkScalerContext> context = strikeSpec.createScalerContext();

        char storage[4096];
        memset(storage, 0x5A, sizeof(storage));
        SkArenaAlloc alloc(storage, sizeof(storage), 4096);
        SkGlyph glyph = context->makeGlyph(SkPackedGlyphID((SkGlyphID)1), &alloc);
        REPORTER_ASSERT(reporter, glyph.maskFormat() == SkMask::kA8_Format);
        REPORTER_ASSERT(reporter, glyph.imageSize() > 0);
        REPORTER_ASSERT(reporter, glyph.setImage(&alloc, context.get()));

        const uint8_t* bytes = static_cast<const uint8_t*>(glyph.image());
        bool hasNonZero = false;
        for (size_t i = 0; i < glyph.imageSize(); ++i) {
            hasNonZero |= (bytes[i] != 0);
        }
        REPORTER_ASSERT(reporter, hasNonZero);
    }
}
