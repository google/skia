/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkStrike.h"

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkTypeface.h"
#include "include/private/chromium/SkChromeRemoteGlyphCache.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTaskGroup.h"
#include "src/core/SkTextBlobTrace.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

using namespace skia_private;

static void do_font_stuff(SkFont* font) {
    SkPaint defaultPaint;
    for (SkScalar i = 8; i < 64; i++) {
        font->setSize(i);
        auto strikeSpec = SkStrikeSpec::MakeMask(
                *font,  defaultPaint, SkSurfaceProps(0, kUnknown_SkPixelGeometry),
                SkScalerContextFlags::kNone, SkMatrix::I());
        SkPackedGlyphID glyphs['z'];
        for (int c = ' '; c < 'z'; c++) {
            glyphs[c] = SkPackedGlyphID{font->unicharToGlyph(c)};
        }
        constexpr size_t glyphCount = 'z' - ' ';
        SkSpan<const SkPackedGlyphID> glyphIDs{&glyphs[SkTo<int>(' ')], glyphCount};
        SkBulkGlyphMetricsAndImages images{strikeSpec};
        for (int lookups = 0; lookups < 10; lookups++) {
            (void)images.glyphs(glyphIDs);
        }
    }
}

class SkGlyphCacheBasic : public Benchmark {
public:
    explicit SkGlyphCacheBasic(size_t cacheSize) : fCacheSize(cacheSize) { }

protected:
    const char* onGetName() override {
        fName.printf("SkGlyphCacheBasic%dK", (int)(fCacheSize >> 10));
        return fName.c_str();
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    void onDraw(int loops, SkCanvas*) override {
        size_t oldCacheLimitSize = SkGraphics::GetFontCacheLimit();
        SkGraphics::SetFontCacheLimit(fCacheSize);
        SkFont font;
        font.setEdging(SkFont::Edging::kAntiAlias);
        font.setSubpixel(true);
        font.setTypeface(ToolUtils::CreatePortableTypeface("serif", SkFontStyle::Italic()));

        for (int work = 0; work < loops; work++) {
            do_font_stuff(&font);
        }
        SkGraphics::SetFontCacheLimit(oldCacheLimitSize);
    }

private:
    using INHERITED = Benchmark;
    const size_t fCacheSize;
    SkString fName;
};

class SkGlyphCacheStressTest : public Benchmark {
public:
    explicit SkGlyphCacheStressTest(int cacheSize) : fCacheSize(cacheSize) { }

protected:
    const char* onGetName() override {
        fName.printf("SkGlyphCacheStressTest%dK", (int)(fCacheSize >> 10));
        return fName.c_str();
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    void onDraw(int loops, SkCanvas*) override {
        size_t oldCacheLimitSize = SkGraphics::GetFontCacheLimit();
        SkGraphics::SetFontCacheLimit(fCacheSize);
        sk_sp<SkTypeface> typefaces[] = {
                ToolUtils::CreatePortableTypeface("serif", SkFontStyle::Italic()),
                ToolUtils::CreatePortableTypeface("sans-serif", SkFontStyle::Italic())};

        for (int work = 0; work < loops; work++) {
            SkTaskGroup().batch(16, [&](int threadIndex) {
                SkFont font;
                font.setEdging(SkFont::Edging::kAntiAlias);
                font.setSubpixel(true);
                font.setTypeface(typefaces[threadIndex % 2]);
                do_font_stuff(&font);
            });
        }
        SkGraphics::SetFontCacheLimit(oldCacheLimitSize);
    }

private:
    using INHERITED = Benchmark;
    const size_t fCacheSize;
    SkString fName;
};

DEF_BENCH( return new SkGlyphCacheBasic(256 * 1024); )
DEF_BENCH( return new SkGlyphCacheBasic(32 * 1024 * 1024); )
DEF_BENCH( return new SkGlyphCacheStressTest(256 * 1024); )
DEF_BENCH( return new SkGlyphCacheStressTest(32 * 1024 * 1024); )

namespace {
class DiscardableManager : public SkStrikeServer::DiscardableHandleManager,
                           public SkStrikeClient::DiscardableHandleManager {
public:
    DiscardableManager() { sk_bzero(&fCacheMissCount, sizeof(fCacheMissCount)); }
    ~DiscardableManager() override = default;

    // Server implementation.
    SkDiscardableHandleId createHandle() override {
        SkAutoMutexExclusive l(fMutex);

        // Handles starts as locked.
        fLockedHandles.add(++fNextHandleId);
        return fNextHandleId;
    }
    bool lockHandle(SkDiscardableHandleId id) override {
        SkAutoMutexExclusive l(fMutex);

        if (id <= fLastDeletedHandleId) return false;
        fLockedHandles.add(id);
        return true;
    }

    // Client implementation.
    bool deleteHandle(SkDiscardableHandleId id) override {
        SkAutoMutexExclusive l(fMutex);

        return id <= fLastDeletedHandleId;
    }

    void notifyCacheMiss(SkStrikeClient::CacheMissType type, int fontSize) override {
        SkAutoMutexExclusive l(fMutex);

        fCacheMissCount[type]++;
    }
    bool isHandleDeleted(SkDiscardableHandleId id) override {
        SkAutoMutexExclusive l(fMutex);

        return id <= fLastDeletedHandleId;
    }

    void unlockAll() {
        SkAutoMutexExclusive l(fMutex);

        fLockedHandles.reset();
    }
    void unlockAndDeleteAll() {
        SkAutoMutexExclusive l(fMutex);

        fLockedHandles.reset();
        fLastDeletedHandleId = fNextHandleId;
    }
    const THashSet<SkDiscardableHandleId>& lockedHandles() const {
        SkAutoMutexExclusive l(fMutex);

        return fLockedHandles;
    }
    SkDiscardableHandleId handleCount() {
        SkAutoMutexExclusive l(fMutex);

        return fNextHandleId;
    }
    int cacheMissCount(uint32_t type) {
        SkAutoMutexExclusive l(fMutex);

        return fCacheMissCount[type];
    }
    bool hasCacheMiss() const {
        SkAutoMutexExclusive l(fMutex);

        for (uint32_t i = 0; i <= SkStrikeClient::CacheMissType::kLast; ++i) {
            if (fCacheMissCount[i] > 0) return true;
        }
        return false;
    }
    void resetCacheMissCounts() {
        SkAutoMutexExclusive l(fMutex);
        sk_bzero(&fCacheMissCount, sizeof(fCacheMissCount));
    }

private:
    // The tests below run in parallel on multiple threads and use the same
    // process global SkStrikeCache. So the implementation needs to be
    // thread-safe.
    mutable SkMutex fMutex;

    SkDiscardableHandleId fNextHandleId = 0u;
    SkDiscardableHandleId fLastDeletedHandleId = 0u;
    THashSet<SkDiscardableHandleId> fLockedHandles;
    int fCacheMissCount[SkStrikeClient::CacheMissType::kLast + 1u];
};

class DiffCanvasBench : public Benchmark {
    SkString fBenchName;
    std::function<std::unique_ptr<SkStreamAsset>()> fDataProvider;
    std::vector<SkTextBlobTrace::Record> fTrace;
    sk_sp<DiscardableManager> fDiscardableManager;
    SkTLazy<SkStrikeServer> fServer;

    const char* onGetName() override { return fBenchName.c_str(); }

    bool isSuitableFor(Backend b) override { return b == kNonRendering_Backend; }

    void onDraw(int loops, SkCanvas* modelCanvas) override {
        SkSurfaceProps props;
        if (modelCanvas) { modelCanvas->getProps(&props); }
        std::unique_ptr<SkCanvas> canvas = fServer->makeAnalysisCanvas(1024, 1024, props,
                                                                       nullptr, true, true);
        loops *= 100;
        while (loops --> 0) {
            for (const auto& record : fTrace) {
                canvas->drawTextBlob(
                        record.blob.get(), record.offset.x(), record.offset.y(),record.paint);
            }
        }
    }

    void onDelayedSetup() override {
        auto stream = fDataProvider();
        fDiscardableManager = sk_make_sp<DiscardableManager>();
        fServer.init(fDiscardableManager.get());
        fTrace = SkTextBlobTrace::CreateBlobTrace(stream.get(), nullptr);
    }

public:
    DiffCanvasBench(SkString n, std::function<std::unique_ptr<SkStreamAsset>()> f)
        : fBenchName(std::move(n)), fDataProvider(std::move(f)) {}
};
}  // namespace

Benchmark* CreateDiffCanvasBench(
        SkString name, std::function<std::unique_ptr<SkStreamAsset>()> dataSrc) {
    return new DiffCanvasBench(std::move(name), std::move(dataSrc));
}

DEF_BENCH( return CreateDiffCanvasBench(
        SkString("SkDiffBench-lorem_ipsum"),
        [](){ return GetResourceAsStream("diff_canvas_traces/lorem_ipsum.trace"); }));
