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

#include <memory>

class SkContextPriv;
class SkContextPrivConst;
class SkResourceCache;
class SkSharedContext;
class SkStrikeCache;
class SkTraceMemoryDump;
class SkTypefaceCache;
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

private:
    SkContext(const SkContextOptions&);

    friend class SkContextPrivConst;
    friend class SkContextPriv;
    friend class SkContextCtorAccessor;

    SkResourceCache* resourceCache() const;
    SkStrikeCache* fontCache() const;
    SkTypefaceCache* typefaceCache() const;

    size_t fontCacheLimit() const;
    size_t setFontCacheLimit(size_t bytes);
    size_t fontCacheUsed() const;

    int fontCacheCountLimit() const;
    int setFontCacheCountLimit(int count);
    int fontCacheCountUsed() const;

    void purgeFontCache();
    void purgePinnedFontCache();

    void dumpMemoryStatistics(SkTraceMemoryDump* dump);

    sk_sp<SkSharedContext> fSharedContext;
};

#endif  // SkContext_DEFINED
