/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkContext_DEFINED
#define SkContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkAPI.h"

#include <cstddef>
#include <cstdint>
#include <memory>

class SkCapture;
class SkCaptureManager;
class SkContextPriv;
class SkContextPrivConst;
class SkGPUContext;
class SkOpenTypeSVGDecoder;
class SkSharedContext;
class SkTraceMemoryDump;
struct SkContextOptions;

/**
 * @brief SkContext is a central context object that serves as the mandatory first point of
 * initialization for all Skia clients. SkContext consolidates global state, management of shared
 * resources, and internal caches.
 */
class SK_API SkContext final {
public:
    SkContext(const SkContext&) = delete;
    SkContext(SkContext&&) = delete;
    SkContext& operator=(const SkContext&) = delete;
    SkContext& operator=(SkContext&&) = delete;

    ~SkContext();

    SkContextPriv priv();
    SkContextPrivConst priv() const;

    /*
     * Starts the SkCapture. Must have set SkContextOptions::fEnableCapture to start.
     */
    void startCapture();

    /*
     * Ends the SkCapture and returns the collected draws and surface creation.
     */
    sk_sp<SkCapture> endCapture();

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

    int typefaceCacheCountLimit() const;
    int setTypefaceCacheCountLimit(int count);

    void dumpMemoryStatistics(SkTraceMemoryDump* dump);

private:
    explicit SkContext(const SkContextOptions&, std::unique_ptr<SkGPUContext> gpuContext = nullptr);

    friend class SkContextPrivConst;
    friend class SkContextPriv;
    friend class SkContextCtorAccessor;

    sk_sp<SkCaptureManager> fCaptureManager;
    sk_sp<SkSharedContext> fSharedContext;

    std::unique_ptr<SkGPUContext> fGPUContext;
};

#endif  // SkContext_DEFINED
