/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#if SK_SUPPORT_GPU

#include "GrContextPriv.h"
#include "Test.h"
#include "text/GrAtlasGlyphCache.h"

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(AtlasGlyphCache, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    auto proxyProvider = context->contextPriv().proxyProvider();
    auto drawingManager = context->contextPriv().drawingManager();

    GrAtlasGlyphCache cache(proxyProvider, 0, GrDrawOpAtlas::AllowMultitexturing::kYes);

    GrOnFlushResourceProvider onFlushResourceProvider(drawingManager);

    cache.preFlush(&onFlushResourceProvider, nullptr, 0, nullptr);

    cache.postFlush(GrDeferredUploadToken::AlreadyFlushedToken(), nullptr, 0);


}

#endif
