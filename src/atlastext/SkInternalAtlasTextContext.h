/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkInternalAtlasTextContext_DEFINED
#define SkInternalAtlasTextContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkArenaAlloc.h"
#include "src/core/SkArenaAllocList.h"
#include "src/gpu/GrDeferredUpload.h"

class GrContext;
class GrStrikeCache;
class GrTextBlobCache;

class SkAtlasTextRenderer;
class SkMatrix;

/**
 * The implementation of SkAtlasTextContext. This exists to hide the details from the public class.
 * and to be able to use other private types.
 */
class SkInternalAtlasTextContext : public GrDeferredUploadTarget {
public:
    static std::unique_ptr<SkInternalAtlasTextContext> Make(sk_sp<SkAtlasTextRenderer>);

    ~SkInternalAtlasTextContext() override;

    SkAtlasTextRenderer* renderer() const { return fRenderer.get(); }

    GrContext* grContext() const { return fGrContext.get(); }
    GrStrikeCache* glyphCache();
    GrTextBlobCache* textBlobCache();

    const GrTokenTracker* tokenTracker() final { return &fTokenTracker; }
    GrDeferredUploadToken addInlineUpload(GrDeferredTextureUploadFn&&) final;
    GrDeferredUploadToken addASAPUpload(GrDeferredTextureUploadFn&&) final;

    void recordDraw(const void* vertexData, int glyphCnt, const SkMatrix&, void* targetHandle);

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

    GrTokenTracker fTokenTracker;
    SkArenaAllocList<InlineUpload> fInlineUploads;
    SkArenaAllocList<Draw> fDraws;
    SkArenaAllocList<GrDeferredTextureUploadFn> fASAPUploads;
    SkArenaAlloc fArena{1024 * 40};
    sk_sp<GrContext> fGrContext;
    AtlasTexture fDistanceFieldAtlas;
};

#endif
