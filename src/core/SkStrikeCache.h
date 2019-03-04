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

#include "SkDescriptor.h"
#include "SkStrike.h"
#include "SkSpinlock.h"
#include "SkTemplates.h"

class SkStrike;
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

class SkStrikeCache final : public SkStrikeCacheInterface {
    class Node;

public:
    SkStrikeCache() = default;
    ~SkStrikeCache() override;

    class ExclusiveStrikePtr {
    public:
        explicit ExclusiveStrikePtr(Node*);
        ExclusiveStrikePtr();
        ExclusiveStrikePtr(const ExclusiveStrikePtr&) = delete;
        ExclusiveStrikePtr& operator = (const ExclusiveStrikePtr&) = delete;
        ExclusiveStrikePtr(ExclusiveStrikePtr&&);
        ExclusiveStrikePtr& operator = (ExclusiveStrikePtr&&);
        ~ExclusiveStrikePtr();

        SkStrike* get() const;
        SkStrike* operator -> () const;
        SkStrike& operator *  () const;
        explicit operator bool () const;
        friend bool operator == (const ExclusiveStrikePtr&, const ExclusiveStrikePtr&);
        friend bool operator == (const ExclusiveStrikePtr&, decltype(nullptr));
        friend bool operator == (decltype(nullptr), const ExclusiveStrikePtr&);

    private:
        Node* fNode;
    };

    static SkStrikeCache* GlobalStrikeCache();

    static ExclusiveStrikePtr FindStrikeExclusive(const SkDescriptor&);
    ExclusiveStrikePtr findStrikeExclusive(const SkDescriptor&);
    Node* findAndDetachStrike(const SkDescriptor&);

    static ExclusiveStrikePtr CreateStrikeExclusive(
            const SkDescriptor& desc,
            std::unique_ptr<SkScalerContext> scaler,
            SkFontMetrics* maybeMetrics = nullptr,
            std::unique_ptr<SkStrikePinner> = nullptr);

    ExclusiveStrikePtr createStrikeExclusive(
            const SkDescriptor& desc,
            std::unique_ptr<SkScalerContext> scaler,
            SkFontMetrics* maybeMetrics = nullptr,
            std::unique_ptr<SkStrikePinner> = nullptr);

    Node* createStrike(
            const SkDescriptor& desc,
            std::unique_ptr<SkScalerContext> scaler,
            SkFontMetrics* maybeMetrics = nullptr,
            std::unique_ptr<SkStrikePinner> = nullptr);

    static ExclusiveStrikePtr FindOrCreateStrikeExclusive(
            const SkDescriptor& desc,
            const SkScalerContextEffects& effects,
            const SkTypeface& typeface);

    ExclusiveStrikePtr findOrCreateStrikeExclusive(
            const SkDescriptor& desc,
            const SkScalerContextEffects& effects,
            const SkTypeface& typeface);

    Node* findOrCreateStrike(
            const SkDescriptor& desc,
            const SkScalerContextEffects& effects,
            const SkTypeface& typeface);

    // Routines to find suitable data when working in a remote cache situation. These are
    // suitable as substitutes for similar calls in SkScalerContext.
    bool desperationSearchForImage(const SkDescriptor& desc,
                                   SkGlyph* glyph,
                                   SkStrike* targetCache);
    bool desperationSearchForPath(const SkDescriptor& desc, SkGlyphID glyphID, SkPath* path);

    SkScopedStrike findOrCreateScopedStrike(const SkDescriptor& desc,
                                            const SkScalerContextEffects& effects,
                                            const SkTypeface& typeface) override;

    static ExclusiveStrikePtr FindOrCreateStrikeExclusive(
            const SkFont& font,
            const SkPaint& paint,
            const SkSurfaceProps& surfaceProps,
            SkScalerContextFlags scalerContextFlags,
            const SkMatrix& deviceMatrix);

    Node* findOrCreateStrike(
            const SkFont& font,
            const SkPaint& paint,
            const SkSurfaceProps& surfaceProps,
            SkScalerContextFlags scalerContextFlags,
            const SkMatrix& deviceMatrix);

    // cons up a default paint, which is only needed for patheffects/maskfilter
    static ExclusiveStrikePtr FindOrCreateStrikeWithNoDeviceExclusive(const SkFont&);

    static ExclusiveStrikePtr FindOrCreateStrikeWithNoDeviceExclusive(const SkFont& font,
                                                                      const SkPaint& paint);

    static std::unique_ptr<SkScalerContext> CreateScalerContext(
            const SkDescriptor&, const SkScalerContextEffects&, const SkTypeface&);

    static void PurgeAll();
    static void ValidateGlyphCacheDataSize();
    static void Dump();

    // Dump memory usage statistics of all the attaches caches in the process using the
    // SkTraceMemoryDump interface.
    static void DumpMemoryStatistics(SkTraceMemoryDump* dump);

    // call when a glyphcache is available for caching (i.e. not in use)
    void attachNode(Node* node);

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
    // A simple accounting of what each glyph cache reports and the strike cache total.
    void validate() const;
    // Make sure that each glyph cache's memory tracking and actual memory used are in sync.
    void validateGlyphCacheDataSize() const;
#else
    void validate() const {}
    void validateGlyphCacheDataSize() const {}
#endif

private:

    // The following methods can only be called when mutex is already held.
    Node* internalGetHead() const { return fHead; }
    Node* internalGetTail() const { return fTail; }
    void internalDetachCache(Node*);
    void internalAttachToHead(Node*);

    // Checkout budgets, modulated by the specified min-bytes-needed-to-purge,
    // and attempt to purge caches to match.
    // Returns number of bytes freed.
    size_t internalPurge(size_t minBytesNeeded = 0);

    void forEachStrike(std::function<void(const SkStrike&)> visitor) const;

    mutable SkSpinlock fLock;
    Node*              fHead{nullptr};
    Node*              fTail{nullptr};
    size_t             fTotalMemoryUsed{0};
    size_t             fCacheSizeLimit{SK_DEFAULT_FONT_CACHE_LIMIT};
    int32_t            fCacheCountLimit{SK_DEFAULT_FONT_CACHE_COUNT_LIMIT};
    int32_t            fCacheCount{0};
    int32_t            fPointSizeLimit{SK_DEFAULT_FONT_CACHE_POINT_SIZE_LIMIT};
};

using SkExclusiveStrikePtr = SkStrikeCache::ExclusiveStrikePtr;

#endif  // SkStrikeCache_DEFINED
