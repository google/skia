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

SkAtlasTextRenderer* SkGetAtlasTextRendererFromInternalContext(class SkInternalAtlasTextContext& internal) {
    return internal.renderer();
}


//////////////////////////////////////////////////////////////////////////////

std::unique_ptr<SkInternalAtlasTextContext> SkInternalAtlasTextContext::Make(std::unique_ptr<SkAtlasTextRenderer> renderer) {
    return std::unique_ptr<SkInternalAtlasTextContext>(new SkInternalAtlasTextContext(std::move(renderer)));
}

SkInternalAtlasTextContext::SkInternalAtlasTextContext(std::unique_ptr<SkAtlasTextRenderer> renderer)
        : fRenderer(std::move(renderer)), fGrContext(GrContext::MakeMock(nullptr)) {}


GrAtlasGlyphCache* SkInternalAtlasTextContext::atlasGlyphCache() {
    return fGrContext->getAtlasGlyphCache();
}

GrTextBlobCache* SkInternalAtlasTextContext::textBlobCache() {
    return fGrContext->getTextBlobCache();
}

GrDeferredUploadToken SkInternalAtlasTextContext::addInlineUpload(GrDeferredTextureUploadFn&& upload) {
    auto token = this->nextDrawToken();
    fInlineUploads.append(&fArena, InlineUpload{std::move(upload), token});
    return token;
}

GrDeferredUploadToken SkInternalAtlasTextContext::addASAPUpload(GrDeferredTextureUploadFn&& upload) {
    fASAPUploads.append(&fArena, std::move(upload));
    return this->nextTokenToFlush();
}

