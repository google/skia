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
#include "src/core/SkStrike.h"

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

    // Routines to find suitable data when working in a remote cache situation. These are
    // suitable as substitutes for similar calls in SkScalerContext.
    bool desperationSearchForImage(const SkDescriptor& desc,
                                   SkGlyph* glyph,
                                   SkStrike* targetCache);
    bool desperationSearchForPath(const SkDescriptor& desc, SkGlyphID glyphID, SkPath* path);

    SkScopedStrikeForGPU findOrCreateScopedStrike(const SkDescriptor& desc,
                                                  const SkScalerContextEffects& effects,
                                                  const SkTypeface& typeface) override;

    static std::unique_ptr<SkScalerContext> CreateScalerContext(
            const SkDescriptor&, const SkScalerContextEffects&, const SkTypeface&);

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
    // A simple accounting of what each glyph cache reports and the strike cache total.
    void validate() const SK_REQUIRES(fLock);
    // Make sure that each glyph cache's memory tracking and actual memory used are in sync.
    void validateGlyphCacheDataSize() const;
#else
    void validate() const {}
    void validateGlyphCacheDataSize() const {}
#endif

private:
    Node* findAndDetachStrike(const SkDescriptor&);
    Node* createStrike(
            const SkDescriptor& desc,
            std::unique_ptr<SkScalerContext> scaler,
            SkFontMetrics* maybeMetrics = nullptr,
            std::unique_ptr<SkStrikePinner> = nullptr);
    Node* findOrCreateStrike(
            const SkDescriptor& desc,
            const SkScalerContextEffects& effects,
            const SkTypeface& typeface);
    void attachNode(Node* node);

    // The following methods can only be called when mutex is already held.
    Node* internalGetHead() const SK_REQUIRES(fLock) { return fHead; }
    Node* internalGetTail() const SK_REQUIRES(fLock) { return fTail; }
    void internalDetachCache(Node*) SK_REQUIRES(fLock);
    void internalAttachToHead(Node*) SK_REQUIRES(fLock);

    // Checkout budgets, modulated by the specified min-bytes-needed-to-purge,
    // and attempt to purge caches to match.
    // Returns number of bytes freed.
    size_t internalPurge(size_t minBytesNeeded = 0) SK_REQUIRES(fLock);

    void forEachStrike(std::function<void(const SkStrike&)> visitor) const;

    mutable SkSpinlock fLock;
    Node*              fHead SK_GUARDED_BY(fLock) {nullptr};
    Node*              fTail SK_GUARDED_BY(fLock) {nullptr};
    size_t             fTotalMemoryUsed{0};
    size_t             fCacheSizeLimit{SK_DEFAULT_FONT_CACHE_LIMIT};
    int32_t            fCacheCountLimit{SK_DEFAULT_FONT_CACHE_COUNT_LIMIT};
    int32_t            fCacheCount{0};
    int32_t            fPointSizeLimit{SK_DEFAULT_FONT_CACHE_POINT_SIZE_LIMIT};
};

using SkExclusiveStrikePtr = SkStrikeCache::ExclusiveStrikePtr;

#endif  // SkStrikeCache_DEFINED
