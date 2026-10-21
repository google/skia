/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSharedContext_DEFINED
#define SkSharedContext_DEFINED

#include "include/core/SkRefCnt.h"

#include <cstddef>
#include <cstdint>
#include <memory>

class SkOpenTypeSVGDecoder;
class SkStrikeCache;
class SkSynchronizedResourceCache;
class SkTypefaceCache;
struct SkContextOptions;

using OpenTypeSVGDecoderFactory = std::unique_ptr<SkOpenTypeSVGDecoder> (*)(const uint8_t* svg,
                                                                            size_t length);

class SkSharedContext : public SkRefCnt {
public:
    explicit SkSharedContext(const SkContextOptions& options);
    ~SkSharedContext() override;

    const SkSynchronizedResourceCache* synchronizedResourceCache() const {
        return fResourceCache.get();
    }
    SkSynchronizedResourceCache* synchronizedResourceCache() { return fResourceCache.get(); }

    const SkStrikeCache* strikeCache() const { return fStrikeCache.get(); }
    SkStrikeCache* strikeCache() { return fStrikeCache.get(); }

    const SkTypefaceCache* typefaceCache() const { return fTypefaceCache.get(); }
    SkTypefaceCache* typefaceCache() { return fTypefaceCache.get(); }

    size_t fontCacheLimit() const;
    size_t setFontCacheLimit(size_t bytes);
    size_t fontCacheUsed() const;

    int fontCacheCountLimit() const;
    int setFontCacheCountLimit(int count);
    int fontCacheCountUsed() const;

    void purgeFontCache();
    void purgePinnedFontCache();

    size_t resourceCacheTotalBytesUsed() const;
    size_t resourceCacheTotalByteLimit() const;
    size_t setResourceCacheTotalByteLimit(size_t newLimit);

    size_t resourceCacheSingleAllocationByteLimit() const;
    size_t setResourceCacheSingleAllocationByteLimit(size_t newLimit);

    void purgeResourceCache();

    int typefaceCacheCountLimit() const { return fTypefaceCacheCountLimit; }
    int setTypefaceCacheCountLimit(int count) {
        const int prev = fTypefaceCacheCountLimit;
        fTypefaceCacheCountLimit = count;
        return prev;
    }

    OpenTypeSVGDecoderFactory setSVGDecoderFactory(OpenTypeSVGDecoderFactory factory) {
        const OpenTypeSVGDecoderFactory prev = fSVGDecoderFactory;
        fSVGDecoderFactory = std::move(factory);
        return prev;
    }

    OpenTypeSVGDecoderFactory svgDecoderFactory() const { return fSVGDecoderFactory; }

private:
    int fTypefaceCacheCountLimit = 1024;  // historical default value
    OpenTypeSVGDecoderFactory fSVGDecoderFactory = nullptr;

    std::unique_ptr<SkSynchronizedResourceCache> fResourceCache;
    std::unique_ptr<SkStrikeCache> fStrikeCache;
    std::unique_ptr<SkTypefaceCache> fTypefaceCache;
};

#endif  // SkSharedContext_DEFINED
