/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStrikeCache_DEFINED
#define SkStrikeCache_DEFINED

#include <unordered_map>
#include <unordered_set>

#include "include/private/SkSpinlock.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkDescriptor.h"
#include "src/core/SkScalerCache.h"

class SkTraceMemoryDump;

#ifndef SK_DEFAULT_FONT_CACHE_COUNT_LIMIT
    #define SK_DEFAULT_FONT_CACHE_COUNT_LIMIT   2048
#endif

#ifndef SK_DEFAULT_FONT_CACHE_LIMIT
    #define SK_DEFAULT_FONT_CACHE_LIMIT     (2 * 1024 * 1024)
#endif

#ifndef SK_DEFAULT_FONT_CACHE_POINT_SIZE_LIMIT
    #define SK_DEFAULT_FONT_CACHE_POINT_SIZE_LIMIT  256
#endif

///////////////////////////////////////////////////////////////////////////////

class SkStrikePinner {
public:
    virtual ~SkStrikePinner() = default;
    virtual bool canDelete() = 0;
};

class SkStrikeCache final : public SkStrikeForGPUCacheInterface {
public:
    SkStrikeCache() = default;
    ~SkStrikeCache() override;

    class Strike final : public SkRefCnt, public SkStrikeForGPU {
    public:
        Strike(SkStrikeCache* strikeCache,
               const SkDescriptor& desc,
               std::unique_ptr<SkScalerContext> scaler,
               const SkFontMetrics* metrics,
               std::unique_ptr<SkStrikePinner> pinner)
                : fStrikeCache{strikeCache}
                , fScalerCache{desc, std::move(scaler), metrics}
                , fPinner{std::move(pinner)} {}

        SkGlyph* mergeGlyphAndImage(SkPackedGlyphID toID, const SkGlyph& from) {
            auto [glyph, delta] = fScalerCache.mergeGlyphAndImage(toID, from);
            fMemoryUsed += delta;
            SkASSERT(fScalerCache.recalculateMemoryUsed() == fMemoryUsed);
            return glyph;
        }

        const SkPath* mergePath(SkGlyph* glyph, const SkPath* path) {
            auto [glyphPath, delta] = fScalerCache.mergePath(glyph, path);
            fMemoryUsed += delta;
            SkASSERT(fScalerCache.recalculateMemoryUsed() == fMemoryUsed);
            return glyphPath;
        }

        SkScalerContext* getScalerContext() const {
            return fScalerCache.getScalerContext();
        }

        void findIntercepts(const SkScalar bounds[2], SkScalar scale, SkScalar xPos,
                            SkGlyph* glyph, SkScalar* array, int* count) {
            fScalerCache.findIntercepts(bounds, scale, xPos, glyph, array, count);
        }

        const SkFontMetrics& getFontMetrics() const {
            return fScalerCache.getFontMetrics();
        }

        SkSpan<const SkGlyph*> metrics(SkSpan<const SkGlyphID> glyphIDs,
                                       const SkGlyph* results[]) {
            auto [glyphs, delta] = fScalerCache.metrics(glyphIDs, results);
            fMemoryUsed += delta;
            SkASSERT(fScalerCache.recalculateMemoryUsed() == fMemoryUsed);
            return glyphs;
        }

        SkSpan<const SkGlyph*> preparePaths(SkSpan<const SkGlyphID> glyphIDs,
                                            const SkGlyph* results[]) {
            auto [glyphs, delta] = fScalerCache.preparePaths(glyphIDs, results);
            fMemoryUsed += delta;
            SkASSERT(fScalerCache.recalculateMemoryUsed() == fMemoryUsed);
            return glyphs;
        }

        SkSpan<const SkGlyph*> prepareImages(SkSpan<const SkPackedGlyphID> glyphIDs,
                                             const SkGlyph* results[]) {
            auto [glyphs, delta] = fScalerCache.prepareImages(glyphIDs, results);
            fMemoryUsed += delta;
            SkASSERT(fScalerCache.recalculateMemoryUsed() == fMemoryUsed);
            return glyphs;
        }

        void prepareForDrawingMasksCPU(SkDrawableGlyphBuffer* drawables) {
            size_t delta = fScalerCache.prepareForDrawingMasksCPU(drawables);
            fMemoryUsed += delta;
            SkASSERT(fScalerCache.recalculateMemoryUsed() == fMemoryUsed);
        }

        const SkGlyphPositionRoundingSpec& roundingSpec() const override {
            return fScalerCache.roundingSpec();
        }

        const SkDescriptor& getDescriptor() const override {
            return fScalerCache.getDescriptor();
        }

        void prepareForMaskDrawing(
                SkDrawableGlyphBuffer* drawbles, SkSourceGlyphBuffer* rejects) override {
            size_t delta = fScalerCache.prepareForMaskDrawing(drawbles, rejects);
            fMemoryUsed += delta;
            SkASSERT(fScalerCache.recalculateMemoryUsed() == fMemoryUsed);
        }

        void prepareForSDFTDrawing(
                SkDrawableGlyphBuffer* drawbles, SkSourceGlyphBuffer* rejects) override {
            size_t delta = fScalerCache.prepareForSDFTDrawing(drawbles, rejects);
            fMemoryUsed += delta;
            SkASSERT(fScalerCache.recalculateMemoryUsed() == fMemoryUsed);
        }

        void prepareForPathDrawing(
                SkDrawableGlyphBuffer* drawbles, SkSourceGlyphBuffer* rejects) override {
            size_t delta = fScalerCache.prepareForPathDrawing(drawbles, rejects);
            fMemoryUsed += delta;
            SkASSERT(fScalerCache.recalculateMemoryUsed() == fMemoryUsed);
        }

        void onAboutToExitScope() override {
            fStrikeCache->attachStrike(this);
        }

        SkStrikeCache* const            fStrikeCache;
        Strike*                         fNext{nullptr};
        Strike*                         fPrev{nullptr};
        SkScalerCache                   fScalerCache;
        std::unique_ptr<SkStrikePinner> fPinner;
        size_t                          fMemoryUsed{sizeof(SkScalerCache)};
    };  // Strike

    class ExclusiveStrikePtr {
    public:
        explicit ExclusiveStrikePtr(Strike*);
        ExclusiveStrikePtr();
        ExclusiveStrikePtr(const ExclusiveStrikePtr&) = delete;
        ExclusiveStrikePtr& operator = (const ExclusiveStrikePtr&) = delete;
        ExclusiveStrikePtr(ExclusiveStrikePtr&&);
        ExclusiveStrikePtr& operator = (ExclusiveStrikePtr&&);
        ~ExclusiveStrikePtr();

        Strike* get() const;
        Strike* operator -> () const;
        Strike& operator *  () const;
        explicit operator bool () const;
        friend bool operator == (const ExclusiveStrikePtr&, const ExclusiveStrikePtr&);
        friend bool operator == (const ExclusiveStrikePtr&, decltype(nullptr));
        friend bool operator == (decltype(nullptr), const ExclusiveStrikePtr&);

    private:
        Strike* fStrike;
    };

    static SkStrikeCache* GlobalStrikeCache();

    ExclusiveStrikePtr findStrikeExclusive(const SkDescriptor&);

    ExclusiveStrikePtr createStrikeExclusive(
            const SkDescriptor& desc,
            std::unique_ptr<SkScalerContext> scaler,
            SkFontMetrics* maybeMetrics = nullptr,
            std::unique_ptr<SkStrikePinner> = nullptr);

    ExclusiveStrikePtr findOrCreateStrikeExclusive(
            const SkDescriptor& desc,
            const SkScalerContextEffects& effects,
            const SkTypeface& typeface);

    SkScopedStrikeForGPU findOrCreateScopedStrike(const SkDescriptor& desc,
                                                  const SkScalerContextEffects& effects,
                                                  const SkTypeface& typeface) override;

    static void PurgeAll();
    static void ValidateGlyphCacheDataSize();
    static void Dump();

    // Dump memory usage statistics of all the attaches caches in the process using the
    // SkTraceMemoryDump interface.
    static void DumpMemoryStatistics(SkTraceMemoryDump* dump);

    void purgeAll(); // does not change budget

    int getCacheCountLimit() const;
    int setCacheCountLimit(int limit);
    int getCacheCountUsed() const;

    size_t getCacheSizeLimit() const;
    size_t setCacheSizeLimit(size_t limit);
    size_t getTotalMemoryUsed() const;

    int  getCachePointSizeLimit() const;
    int  setCachePointSizeLimit(int limit);
#ifdef SK_DEBUG
    // Make sure that each glyph cache's memory tracking and actual memory used are in sync.
    void validateGlyphCacheDataSize() const;
#else
    void validateGlyphCacheDataSize() const {}
#endif

private:
#ifdef SK_DEBUG
    // A simple accounting of what each glyph cache reports and the strike cache total.
    void validate() const SK_REQUIRES(fLock);
#else
    void validate() const {}
#endif

    Strike* findAndDetachStrike(const SkDescriptor&) SK_EXCLUDES(fLock);
    Strike* createStrike(
            const SkDescriptor& desc,
            std::unique_ptr<SkScalerContext> scaler,
            SkFontMetrics* maybeMetrics = nullptr,
            std::unique_ptr<SkStrikePinner> = nullptr);
    Strike* findOrCreateStrike(
            const SkDescriptor& desc,
            const SkScalerContextEffects& effects,
            const SkTypeface& typeface) SK_EXCLUDES(fLock);
    void attachStrike(Strike* strike) SK_EXCLUDES(fLock);

    // The following methods can only be called when mutex is already held.
    void internalDetachStrike(Strike* strike) SK_REQUIRES(fLock);
    void internalAttachToHead(Strike* strike) SK_REQUIRES(fLock);

    // Checkout budgets, modulated by the specified min-bytes-needed-to-purge,
    // and attempt to purge caches to match.
    // Returns number of bytes freed.
    size_t internalPurge(size_t minBytesNeeded = 0) SK_REQUIRES(fLock);

    void forEachStrike(std::function<void(const Strike&)> visitor) const;

    mutable SkSpinlock fLock;
    Strike* fHead SK_GUARDED_BY(fLock) {nullptr};
    Strike* fTail SK_GUARDED_BY(fLock) {nullptr};
    size_t  fCacheSizeLimit{SK_DEFAULT_FONT_CACHE_LIMIT};
    size_t  fTotalMemoryUsed SK_GUARDED_BY(fLock) {0};
    int32_t fCacheCountLimit{SK_DEFAULT_FONT_CACHE_COUNT_LIMIT};
    int32_t fCacheCount SK_GUARDED_BY(fLock) {0};
    int32_t fPointSizeLimit{SK_DEFAULT_FONT_CACHE_POINT_SIZE_LIMIT};
};

using SkExclusiveStrikePtr = SkStrikeCache::ExclusiveStrikePtr;
using SkStrike = SkStrikeCache::Strike;

#endif  // SkStrikeCache_DEFINED
