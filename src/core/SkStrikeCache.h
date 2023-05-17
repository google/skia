/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStrikeCache_DEFINED
#define SkStrikeCache_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkLoadUserConfig.h" // IWYU pragma: keep
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkThreadAnnotations.h"
#include "src/core/SkStrike.h"
#include "src/core/SkTHash.h"
#include "src/text/StrikeForGPU.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>

class SkDescriptor;
class SkStrikeSpec;
class SkTraceMemoryDump;
struct SkFontMetrics;

//  SK_DEFAULT_FONT_CACHE_COUNT_LIMIT and SK_DEFAULT_FONT_CACHE_LIMIT can be set using -D on your
//  compiler commandline, or by using the defines in SkUserConfig.h
#ifndef SK_DEFAULT_FONT_CACHE_COUNT_LIMIT
    #define SK_DEFAULT_FONT_CACHE_COUNT_LIMIT   2048
#endif

#ifndef SK_DEFAULT_FONT_CACHE_LIMIT
    #define SK_DEFAULT_FONT_CACHE_LIMIT     (2 * 1024 * 1024)
#endif

///////////////////////////////////////////////////////////////////////////////

class SkStrikeCache final : public sktext::StrikeForGPUCacheInterface {
public:
    SkStrikeCache() = default;

    static SkStrikeCache* GlobalStrikeCache();

    sk_sp<SkStrike> findStrike(const SkDescriptor& desc) SK_EXCLUDES(fLock);

    sk_sp<SkStrike> createStrike(
            const SkStrikeSpec& strikeSpec,
            SkFontMetrics* maybeMetrics = nullptr,
            std::unique_ptr<SkStrikePinner> = nullptr) SK_EXCLUDES(fLock);

    sk_sp<SkStrike> findOrCreateStrike(const SkStrikeSpec& strikeSpec) SK_EXCLUDES(fLock);

    sk_sp<sktext::StrikeForGPU> findOrCreateScopedStrike(
            const SkStrikeSpec& strikeSpec) override SK_EXCLUDES(fLock);

    static void PurgeAll();
    static void Dump();

    // Dump memory usage statistics of all the attaches caches in the process using the
    // SkTraceMemoryDump interface.
    static void DumpMemoryStatistics(SkTraceMemoryDump* dump);

    void purgeAll() SK_EXCLUDES(fLock); // does not change budget
    void purgePinned(size_t minBytesNeeded = 0) SK_EXCLUDES(fLock);

    int getCacheCountLimit() const SK_EXCLUDES(fLock);
    int setCacheCountLimit(int limit) SK_EXCLUDES(fLock);
    int getCacheCountUsed() const SK_EXCLUDES(fLock);

    size_t getCacheSizeLimit() const SK_EXCLUDES(fLock);
    size_t setCacheSizeLimit(size_t limit) SK_EXCLUDES(fLock);
    size_t getTotalMemoryUsed() const SK_EXCLUDES(fLock);

private:
    friend class SkStrike;  // for SkStrike::updateDelta
    static constexpr char kGlyphCacheDumpName[] = "skia/sk_glyph_cache";
    sk_sp<SkStrike> internalFindStrikeOrNull(const SkDescriptor& desc) SK_REQUIRES(fLock);
    sk_sp<SkStrike> internalCreateStrike(
            const SkStrikeSpec& strikeSpec,
            SkFontMetrics* maybeMetrics = nullptr,
            std::unique_ptr<SkStrikePinner> = nullptr) SK_REQUIRES(fLock);

    // The following methods can only be called when mutex is already held.
    void internalRemoveStrike(SkStrike* strike) SK_REQUIRES(fLock);
    void internalAttachToHead(sk_sp<SkStrike> strike) SK_REQUIRES(fLock);

    // Checkout budgets, modulated by the specified min-bytes-needed-to-purge,
    // and attempt to purge caches to match.
    // Returns number of bytes freed.
    size_t internalPurge(size_t minBytesNeeded = 0, bool checkPinners = false) SK_REQUIRES(fLock);

    // A simple accounting of what each glyph cache reports and the strike cache total.
    void validate() const SK_REQUIRES(fLock);

    void forEachStrike(std::function<void(const SkStrike&)> visitor) const SK_EXCLUDES(fLock);

    mutable SkMutex fLock;
    SkStrike* fHead SK_GUARDED_BY(fLock) {nullptr};
    SkStrike* fTail SK_GUARDED_BY(fLock) {nullptr};
    struct StrikeTraits {
        static const SkDescriptor& GetKey(const sk_sp<SkStrike>& strike);
        static uint32_t Hash(const SkDescriptor& descriptor);
    };
    skia_private::THashTable<sk_sp<SkStrike>, SkDescriptor, StrikeTraits> fStrikeLookup
            SK_GUARDED_BY(fLock);

    size_t  fCacheSizeLimit{SK_DEFAULT_FONT_CACHE_LIMIT};
    size_t  fTotalMemoryUsed SK_GUARDED_BY(fLock) {0};
    int32_t fCacheCountLimit{SK_DEFAULT_FONT_CACHE_COUNT_LIMIT};
    int32_t fCacheCount SK_GUARDED_BY(fLock) {0};
    int32_t fPinnerCount SK_GUARDED_BY(fLock) {0};
};

#endif  // SkStrikeCache_DEFINED
