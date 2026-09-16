/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

namespace skgpu::graphite {

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasDimensionsAndStride,
                                         reporter,
                                         context,
                                         CtsEnforcement::kToBeDetermined) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());
    // Initial allocation (e.g. 512 bytes = 8 tiles of 8x8)
    REPORTER_ASSERT(reporter, atlasManager.requestAlphaSpace(512) != nullptr);
    {
        auto alloc = atlasManager.finalizeRun();
        REPORTER_ASSERT(reporter, alloc.has_value());
        auto [alphaIndex, texPage] = *alloc;
        REPORTER_ASSERT(reporter, alphaIndex == 0);
        REPORTER_ASSERT(reporter, texPage == 0);
    }
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 1);

    // Initial page dimensions: 8192 x 1
    sk_sp<TextureProxy> proxy = atlasManager.getPageProxy(0);
    REPORTER_ASSERT(reporter, proxy != nullptr);
    REPORTER_ASSERT(reporter, proxy->dimensions().width() == SparseStripConfig::kAtlasWidth);
    REPORTER_ASSERT(reporter, proxy->dimensions().height() == 1);

    // Verify row byte stride alignment (32,768 bytes/row = 8192 pixels * 4 bytes/pixel)
    int32_t strideBytes = SparseStripConfig::kAtlasWidthBytes;
    REPORTER_ASSERT(reporter, strideBytes == 32768);

    // Subsequent allocation on same row/page
    REPORTER_ASSERT(reporter, atlasManager.requestAlphaSpace(1024) != nullptr);
    {
        auto alloc = atlasManager.finalizeRun();
        REPORTER_ASSERT(reporter, alloc.has_value());
        auto [alphaIndex, texPage] = *alloc;
        REPORTER_ASSERT(reporter, alphaIndex == 512);
        REPORTER_ASSERT(reporter, texPage == 0);
    }
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 1);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasDynamicGrowthAndDoubling,
                                         reporter,
                                         context,
                                         CtsEnforcement::kToBeDetermined) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());

    // Page 0: allocate exactly 1 row (32,768 bytes)
    REPORTER_ASSERT(reporter,
                    atlasManager.requestAlphaSpace(SparseStripConfig::kAtlasWidthBytes) != nullptr);
    {
        auto alloc = atlasManager.finalizeRun();
        REPORTER_ASSERT(reporter, alloc.has_value());
        auto [alphaIndex, texPage] = *alloc;
        REPORTER_ASSERT(reporter, texPage == 0);
    }
    REPORTER_ASSERT(reporter, atlasManager.getPageProxy(0)->dimensions().height() == 1);

    // Page 1: allocating 1 more tile exceeds Page 0, triggers doubling -> 2 rows (65,536 bytes)
    REPORTER_ASSERT(reporter, atlasManager.requestAlphaSpace(64) != nullptr);
    {
        auto alloc = atlasManager.finalizeRun();
        REPORTER_ASSERT(reporter, alloc.has_value());
        auto [alphaIndex, texPage] = *alloc;
        REPORTER_ASSERT(reporter, alphaIndex == 0);  // Atomic on new page, starts at 0
        REPORTER_ASSERT(reporter, texPage == 1);
    }
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 2);
    REPORTER_ASSERT(reporter, atlasManager.getPageProxy(1)->dimensions().height() == 2);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasStrictTwoPageLimit,
                                         reporter,
                                         context,
                                         CtsEnforcement::kToBeDetermined) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());

    // Page 0: 1 row
    REPORTER_ASSERT(reporter,
                    atlasManager.requestAlphaSpace(SparseStripConfig::kAtlasWidthBytes) != nullptr);
    {
        auto alloc = atlasManager.finalizeRun();
        REPORTER_ASSERT(reporter, alloc.has_value());
        auto [alphaIndex, texPage] = *alloc;
        REPORTER_ASSERT(reporter, texPage == 0);
    }
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 1);

    // Page 1: 2 rows
    REPORTER_ASSERT(
            reporter,
            atlasManager.requestAlphaSpace(SparseStripConfig::kAtlasWidthBytes * 2) != nullptr);
    {
        auto alloc = atlasManager.finalizeRun();
        REPORTER_ASSERT(reporter, alloc.has_value());
        auto [alphaIndex, texPage] = *alloc;
        REPORTER_ASSERT(reporter, texPage == 1);
    }
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 2);

    // Attempting to exceed Page 1: we only ever allow 2 pages globally across all endcaps.
    // Should fail gracefully!
    REPORTER_ASSERT(reporter, atlasManager.requestAlphaSpace(64) == nullptr);
    auto alloc = atlasManager.finalizeRun();
    REPORTER_ASSERT(reporter, !alloc.has_value());
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 2);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasEndCapAtomicity,
                                         reporter,
                                         context,
                                         CtsEnforcement::kToBeDetermined) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());

    // Allocate almost all of Page 0, leaving 64 bytes
    REPORTER_ASSERT(
            reporter,
            atlasManager.requestAlphaSpace(SparseStripConfig::kAtlasWidthBytes - 64) != nullptr);
    {
        auto alloc = atlasManager.finalizeRun();
        REPORTER_ASSERT(reporter, alloc.has_value());
        auto [alphaIndex, texPage] = *alloc;
        REPORTER_ASSERT(reporter, texPage == 0);
        REPORTER_ASSERT(reporter, alphaIndex == 0);
    }

    // Request 128 bytes (which cannot fit in the remaining 64 bytes of Page 0).
    // The allocation must be atomic to Page 1: entire 128 bytes at offset 0 of Page 1.
    REPORTER_ASSERT(reporter, atlasManager.requestAlphaSpace(128) != nullptr);
    {
        auto alloc = atlasManager.finalizeRun();
        REPORTER_ASSERT(reporter, alloc.has_value());
        auto [alphaIndex, texPage] = *alloc;
        REPORTER_ASSERT(reporter, texPage == 1);
        REPORTER_ASSERT(reporter, alphaIndex == 0);
    }
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 2);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasFlipFlopAtFlush,
                                         reporter,
                                         context,
                                         CtsEnforcement::kToBeDetermined) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());

    // Slot 0: 1 row (32,768 bytes)
    REPORTER_ASSERT(reporter,
                    atlasManager.requestAlphaSpace(SparseStripConfig::kAtlasWidthBytes) != nullptr);
    {
        auto alloc = atlasManager.finalizeRun();
        REPORTER_ASSERT(reporter, alloc.has_value());
        auto [alphaIndex, texPage] = *alloc;
        REPORTER_ASSERT(reporter, texPage == 0);
    }
    REPORTER_ASSERT(reporter, atlasManager.activeSlot() == 0);
    sk_sp<TextureProxy> initialSlot0 = atlasManager.getPageProxy(0);
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 1);

    // Slot 1: 2 rows (65,536 bytes) via XOR flip-flop (0 ^ 1 = 1)
    REPORTER_ASSERT(reporter, atlasManager.requestAlphaSpace(64) != nullptr);
    {
        auto alloc = atlasManager.finalizeRun();
        REPORTER_ASSERT(reporter, alloc.has_value());
        auto [alphaIndex, texPage] = *alloc;
        REPORTER_ASSERT(reporter, texPage == 1);
    }
    REPORTER_ASSERT(reporter, atlasManager.activeSlot() == 1);
    sk_sp<TextureProxy> initialSlot1 = atlasManager.getPageProxy(1);
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 2);

    // Flush: recordUploads(nullptr) retires older slot 0 (1 ^ 1 = 0).
    // Slot 1 stays right in place in slot 1 (no moving needed)!
    atlasManager.recordUploads(nullptr);
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 1);
    REPORTER_ASSERT(reporter, atlasManager.getPageProxy(0) == nullptr);
    REPORTER_ASSERT(reporter, atlasManager.getPageProxy(1) == initialSlot1);
    REPORTER_ASSERT(reporter, atlasManager.activeSlot() == 1);

    // Continue allocating in current active slot (Slot 1):
    REPORTER_ASSERT(reporter, atlasManager.requestAlphaSpace(64) != nullptr);
    {
        auto alloc = atlasManager.finalizeRun();
        REPORTER_ASSERT(reporter, alloc.has_value());
        auto [alphaIndex, texPage] = *alloc;
        REPORTER_ASSERT(reporter, texPage == 1);
    }
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 1);

    // Fill Slot 1 and trigger flip-flop to Slot 0 (1 ^ 1 = 0) -> doubled to 4 rows!
    REPORTER_ASSERT(
            reporter,
            atlasManager.requestAlphaSpace(SparseStripConfig::kAtlasWidthBytes * 2) != nullptr);
    {
        auto alloc = atlasManager.finalizeRun();
        REPORTER_ASSERT(reporter, alloc.has_value());
        auto [alphaIndex, texPage] = *alloc;
        REPORTER_ASSERT(reporter, texPage == 0);
    }
    REPORTER_ASSERT(reporter, atlasManager.activeSlot() == 0);
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 2);
    REPORTER_ASSERT(reporter, atlasManager.getPageProxy(0)->dimensions().height() == 4);

    // Second flush: retires older slot 1 (0 ^ 1 = 1). Slot 0 remains active!
    atlasManager.recordUploads(nullptr);
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 1);
    REPORTER_ASSERT(reporter, atlasManager.getPageProxy(1) == nullptr);
    REPORTER_ASSERT(reporter, atlasManager.getPageProxy(0)->dimensions().height() == 4);
    REPORTER_ASSERT(reporter, atlasManager.activeSlot() == 0);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_AtlasStraddleAndMemmove,
                                         reporter,
                                         context,
                                         CtsEnforcement::kToBeDetermined) {
    auto recorder = context->makeRecorder();
    AlphaAtlasManager atlasManager(recorder.get());

    // Page 0 has 1 row (32,768 bytes).
    // Fill Page 0 up to 64 bytes before the end.
    int32_t almostFull = SparseStripConfig::kAtlasWidthBytes - 64;
    REPORTER_ASSERT(reporter, atlasManager.requestAlphaSpace(almostFull) != nullptr);
    {
        auto alloc = atlasManager.finalizeRun();
        REPORTER_ASSERT(reporter, alloc.has_value());
        auto [alphaIndex, texPage] = *alloc;
        REPORTER_ASSERT(reporter, texPage == 0);
        REPORTER_ASSERT(reporter, alphaIndex == 0);
    }

    // Now start a run of 128 bytes (two 64-byte chunks).
    // Chunk 1 (64 bytes): fills the rest of Page 0.
    uint8_t* ptr1 = atlasManager.requestAlphaSpace(64);
    REPORTER_ASSERT(reporter, ptr1 != nullptr);
    for (int i = 0; i < 64; ++i) {
        ptr1[i] = static_cast<uint8_t>(i + 1);
    }

    // Chunk 2 (64 bytes): straddles past Page 0's capacity into overrun!
    uint8_t* ptr2 = atlasManager.requestAlphaSpace(64);
    REPORTER_ASSERT(reporter, ptr2 != nullptr);
    for (int i = 0; i < 64; ++i) {
        ptr2[i] = static_cast<uint8_t>(i + 65);
    }

    // Finalize the run: should detect straddle, create Page 1, and copy 128 bytes to offset 0 of
    // Page 1!
    {
        auto alloc = atlasManager.finalizeRun();
        REPORTER_ASSERT(reporter, alloc.has_value());
        auto [alphaIndex, texPage] = *alloc;
        REPORTER_ASSERT(reporter, texPage == 1);
        REPORTER_ASSERT(reporter, alphaIndex == 0);
    }
    REPORTER_ASSERT(reporter, atlasManager.numPages() == 2);
    REPORTER_ASSERT(reporter, atlasManager.activeSlot() == 1);

    // Verify that the written bytes were accurately moved into Page 1 at offset 0
    sk_sp<TextureProxy> proxy1 = atlasManager.getPageProxy(1);
    REPORTER_ASSERT(reporter, proxy1 != nullptr);
    REPORTER_ASSERT(reporter, proxy1->dimensions().height() == 2);  // doubled rows

    const uint8_t* page1Data = atlasManager.getPageData(1);
    REPORTER_ASSERT(reporter, page1Data != nullptr);
    for (int i = 0; i < 128; ++i) {
        REPORTER_ASSERT(reporter, page1Data[i] == static_cast<uint8_t>(i + 1));
    }
}

}  // namespace skgpu::graphite
