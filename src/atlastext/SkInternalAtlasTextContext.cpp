/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkInternalAtlasTextContext.h"
#include "GrContext.h"
#include "SkAtlasTextContext.h"
#include "SkAtlasTextRenderer.h"
#include "text/GrAtlasGlyphCache.h"

SkAtlasTextRenderer* SkGetAtlasTextRendererFromInternalContext(
        class SkInternalAtlasTextContext& internal) {
    return internal.renderer();
}

//////////////////////////////////////////////////////////////////////////////

std::unique_ptr<SkInternalAtlasTextContext> SkInternalAtlasTextContext::Make(
        sk_sp<SkAtlasTextRenderer> renderer) {
    return std::unique_ptr<SkInternalAtlasTextContext>(
            new SkInternalAtlasTextContext(std::move(renderer)));
}

SkInternalAtlasTextContext::SkInternalAtlasTextContext(sk_sp<SkAtlasTextRenderer> renderer)
        : fRenderer(std::move(renderer)) {
    GrContextOptions options;
    options.fAllowMultipleGlyphCacheTextures = GrContextOptions::Enable::kNo;
    options.fMinDistanceFieldFontSize = 0.f;
    options.fGlyphsAsPathsFontSize = SK_ScalarInfinity;
    options.fDistanceFieldGlyphVerticesAlwaysHaveW = GrContextOptions::Enable::kYes;
    fGrContext = GrContext::MakeMock(nullptr, options);
}

SkInternalAtlasTextContext::~SkInternalAtlasTextContext() {
    if (fDistanceFieldAtlas.fProxy) {
        SkASSERT(1 == fGrContext->getAtlasGlyphCache()->getAtlasPageCount(kA8_GrMaskFormat));
        fRenderer->deleteTexture(fDistanceFieldAtlas.fTextureHandle);
    }
}

GrAtlasGlyphCache* SkInternalAtlasTextContext::atlasGlyphCache() {
    return fGrContext->getAtlasGlyphCache();
}

GrTextBlobCache* SkInternalAtlasTextContext::textBlobCache() {
    return fGrContext->getTextBlobCache();
}

GrDeferredUploadToken SkInternalAtlasTextContext::addInlineUpload(
        GrDeferredTextureUploadFn&& upload) {
    auto token = this->nextDrawToken();
    fInlineUploads.append(&fArena, InlineUpload{std::move(upload), token});
    return token;
}

GrDeferredUploadToken SkInternalAtlasTextContext::addASAPUpload(
        GrDeferredTextureUploadFn&& upload) {
    fASAPUploads.append(&fArena, std::move(upload));
    return this->nextTokenToFlush();
}

void SkInternalAtlasTextContext::recordDraw(const void* srcVertexData, int glyphCnt,
                                            const SkMatrix& matrix, void* targetHandle) {
    auto vertexDataSize = sizeof(SkAtlasTextRenderer::SDFVertex) * 4 * glyphCnt;
    auto vertexData = fArena.makeArrayDefault<char>(vertexDataSize);
    memcpy(vertexData, srcVertexData, vertexDataSize);
    for (int i = 0; i < 4 * glyphCnt; ++i) {
        auto* vertex = reinterpret_cast<SkAtlasTextRenderer::SDFVertex*>(vertexData) + i;
        // GrAtlasTextContext encodes a texture index into the lower bit of each texture coord.
        // This isn't expected by SkAtlasTextRenderer subclasses.
        vertex->fTextureCoord.fX /= 2;
        vertex->fTextureCoord.fY /= 2;
        matrix.mapHomogeneousPoints(&vertex->fPosition, &vertex->fPosition, 1);
    }
    fDraws.append(&fArena, Draw{glyphCnt, this->issueDrawToken(), targetHandle, vertexData});
}

void SkInternalAtlasTextContext::flush() {
    auto* atlasGlyphCache = fGrContext->getAtlasGlyphCache();
    if (!fDistanceFieldAtlas.fProxy) {
        SkASSERT(1 == atlasGlyphCache->getAtlasPageCount(kA8_GrMaskFormat));
        fDistanceFieldAtlas.fProxy = atlasGlyphCache->getProxies(kA8_GrMaskFormat)->get();
        fDistanceFieldAtlas.fTextureHandle =
                fRenderer->createTexture(SkAtlasTextRenderer::AtlasFormat::kA8,
                                         fDistanceFieldAtlas.fProxy->width(),
                                         fDistanceFieldAtlas.fProxy->height());
    }
    GrDeferredTextureUploadWritePixelsFn writePixelsFn =
            [this](GrTextureProxy* proxy, int left, int top, int width, int height,
                   GrPixelConfig config, const void* data, size_t rowBytes) -> bool {
        SkASSERT(kAlpha_8_GrPixelConfig == config);
        SkASSERT(proxy == this->fDistanceFieldAtlas.fProxy);
        void* handle = fDistanceFieldAtlas.fTextureHandle;
        this->fRenderer->setTextureData(handle, data, left, top, width, height, rowBytes);
        return true;
    };
    for (const auto& upload : fASAPUploads) {
        upload(writePixelsFn);
    }
    auto inlineUpload = fInlineUploads.begin();
    for (const auto& draw : fDraws) {
        while (inlineUpload != fInlineUploads.end() && inlineUpload->fToken == draw.fToken) {
            inlineUpload->fUpload(writePixelsFn);
            ++inlineUpload;
        }
        auto vertices = reinterpret_cast<const SkAtlasTextRenderer::SDFVertex*>(draw.fVertexData);
        fRenderer->drawSDFGlyphs(draw.fTargetHandle, fDistanceFieldAtlas.fTextureHandle, vertices,
                                 draw.fGlyphCnt);
        this->flushToken();
    }
    fASAPUploads.reset();
    fInlineUploads.reset();
    fDraws.reset();
    fArena.reset();
}
