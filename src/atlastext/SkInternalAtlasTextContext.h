/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkInternalAtlasTextContext_DEFINED
#define SkInternalAtlasTextContext_DEFINED

#include "SkArenaAlloc.h"
#include "SkRefCnt.h"
#include "GrContext.h"
#include "GrDeferredUpload.h"

class SkAtlasTextRenderer;
class GrContext;
class GrAtlasGlyphCache;
class GrTextBlobCache;

class SkInternalAtlasTextContext : public GrDeferredUploadTarget {
public:
    static std::unique_ptr<SkInternalAtlasTextContext> Make(sk_sp<SkAtlasTextRenderer>);

    ~SkInternalAtlasTextContext() override;

    SkAtlasTextRenderer* renderer() const { return fRenderer.get(); }

    GrContext* grContext() const { return fGrContext.get(); }
    GrAtlasGlyphCache* atlasGlyphCache();
    GrTextBlobCache* textBlobCache();

    GrDeferredUploadToken addInlineUpload(GrDeferredTextureUploadFn&&) override;

    GrDeferredUploadToken addASAPUpload(GrDeferredTextureUploadFn&&) override;

    void recordDraw(const void* vertexData, int glyphCnt, void* targetHandle);

    void flush();

private:
    class DeferredUploader;
    SkInternalAtlasTextContext() = delete;
    SkInternalAtlasTextContext(const SkInternalAtlasTextContext&) = delete;
    SkInternalAtlasTextContext& operator=(const SkInternalAtlasTextContext&) = delete;

    SkInternalAtlasTextContext(sk_sp<SkAtlasTextRenderer>);

    sk_sp<SkAtlasTextRenderer> fRenderer;

    struct AtlasTexture {
        void* fTextureHandle = nullptr;
        GrTextureProxy* fProxy = nullptr;
    };

    struct Draw {
        int fGlyphCnt;
        GrDeferredUploadToken fToken;
        void* fTargetHandle;
        const void* fVertexData;
    };

    struct InlineUpload {
        GrDeferredTextureUploadFn fUpload;
        GrDeferredUploadToken fToken;
    };

    SkArenaAlloc::List<InlineUpload> fInlineUploads;
    SkArenaAlloc::List<Draw> fDraws;
    SkArenaAlloc::List<GrDeferredTextureUploadFn> fASAPUploads;
    SkArenaAlloc fArena{1024*40};
    sk_sp<GrContext> fGrContext;
    AtlasTexture fDistanceFieldAtlas;
};

#endif
