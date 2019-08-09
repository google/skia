/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "src/core/SkStrike.h"

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkRemoteGlyphCache.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTaskGroup.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

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
        font.setTypeface(ToolUtils::create_portable_typeface("serif", SkFontStyle::Italic()));

        for (int work = 0; work < loops; work++) {
            do_font_stuff(&font);
        }
        SkGraphics::SetFontCacheLimit(oldCacheLimitSize);
    }

private:
    typedef Benchmark INHERITED;
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
                ToolUtils::create_portable_typeface("serif", SkFontStyle::Italic()),
                ToolUtils::create_portable_typeface("sans-serif", SkFontStyle::Italic())};

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
    typedef Benchmark INHERITED;
    const size_t fCacheSize;
    SkString fName;
};

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

    void notifyCacheMiss(SkStrikeClient::CacheMissType type) override {
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
    const SkTHashSet<SkDiscardableHandleId>& lockedHandles() const {
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
    SkTHashSet<SkDiscardableHandleId> fLockedHandles;
    int fCacheMissCount[SkStrikeClient::CacheMissType::kLast + 1u];
};


class SkDiffCanvasBench : public Benchmark {
public:
    SkDiffCanvasBench(const std::string& traceName) {
        fBenchName = std::string("SkDiffBench-") + traceName;
        fTraceName = std::string("diff_canvas_traces/") + traceName + ".trace";
    }

    const char* onGetName() override {
        return fBenchName.c_str();
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

protected:
    void onDraw(int loops, SkCanvas*) override {
        const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
        SkTextBlobCacheDiffCanvas canvas{1024, 1024, props, fServer.get()};
        loops *= 100;
        while (loops --> 0) {
            for (const auto& record : fTrace) {
                canvas.drawTextBlob(
                        record.blob.get(), record.offset.x(), record.offset.y(),record.paint);
            }
        }
    }

protected:
    void onDelayedSetup() override {
        auto stream = GetResourceAsStream(fTraceName.c_str());
        fDiscardableManager = sk_make_sp<DiscardableManager>();
        fServer.init(fDiscardableManager.get());
        fTrace = SkStrikeServer::CreateBlobTrace(stream.get());
    }

    std::vector<SkStrikeServer::BlobTraceRecord> fTrace;
    std::string fBenchName;
    std::string fTraceName;
    sk_sp<DiscardableManager> fDiscardableManager;
    SkTLazy<SkStrikeServer> fServer;
};

DEF_BENCH( return new SkGlyphCacheBasic(256 * 1024); )
DEF_BENCH( return new SkGlyphCacheBasic(32 * 1024 * 1024); )
DEF_BENCH( return new SkGlyphCacheStressTest(256 * 1024); )
DEF_BENCH( return new SkGlyphCacheStressTest(32 * 1024 * 1024); )

DEF_BENCH( return new SkDiffCanvasBench{"wikicat"});
DEF_BENCH( return new SkDiffCanvasBench{"espn"});
DEF_BENCH( return new SkDiffCanvasBench{"google"});
DEF_BENCH( return new SkDiffCanvasBench{"qq"});
DEF_BENCH( return new SkDiffCanvasBench{"taobao"});
DEF_BENCH( return new SkDiffCanvasBench{"yahoo"});
DEF_BENCH( return new SkDiffCanvasBench{"amazon"});
DEF_BENCH( return new SkDiffCanvasBench{"youtube"});
