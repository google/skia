/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAtlasTextTarget.h"
#include "SkAtlasTextContext.h"
#include "SkAtlasTextRenderer.h"
#include "ops/GrAtlasTextOp.h"
#include "SkGr.h"
#include "GrClip.h"

class SkAtlasTextTarget::TextTarget : public GrTextUtils::Target {
public:
    TextTarget(SkAtlasTextTarget* att) : GrTextUtils::Target(att->width(), att->height(), kColorSpaceInfo), fAtt(att) {}

    void addDrawOp(const GrClip&, std::unique_ptr<GrAtlasTextOp> op) override {
        fAtt->addOp(std::move(op));
    }

    void drawPath(const GrClip&, const SkPath&, const SkPaint&,
                          const SkMatrix& viewMatrix, const SkMatrix* pathMatrix,
                          const SkIRect& clipBounds) override {
        SkDebugf("Path glyph??");
    }

    void makeGrPaint(GrMaskFormat, const SkPaint& skPaint, const SkMatrix&,
                             GrPaint* grPaint) override {
        grPaint->setColor4f(SkColorToPremulGrColor4fLegacy(skPaint.getColor()));
    }

private:
    SkAtlasTextTarget* fAtt;
};

//////////////////////////////////////////////////////////////////////////////

SkAtlasTextTarget::~SkAtlasTextTarget() {
    fContext->renderer()->targetDeleted(fHandle);
}

std::unique_ptr<SkAtlasTextTarget> SkAtlasTextTarget::Make(sk_sp<SkAtlasTextContext> context,
                                                           int width, int height, void* handle) {
    return std::unique_ptr<SkAtlasTextTarget>(new SkAtlasTextTarget(std::move(context), width, height, handle));
}

SkAtlasTextTarget::SkAtlasTextTarget(sk_sp<SkAtlasTextContext> context, int width, int height, void* handle)
        : fContext(std::move(context)), fWidth(width), fHeight(height), fHandle(handle), fTarget(new TextTarget()) {}


#include "text/GrAtlasTextContext.h"
#include "SkInternalAtlasTextContext.h"

void SkAtlasTextTarget::drawText(const void* text, size_t byteLength, SkScalar x, SkScalar y) {
    //GrAtlasGlyphCache* atlas = fContext->internal().atlasGlyphCache();
   // GrTextBlobCache* blobCache = fContext->internal().textBlobCache();

    SkPaint paint;
    static const GrColorSpaceInfo kColorSpaceInfo(nullptr, kRGBA_8888_GrPixelConfig);

    GrAtlasTextContext* context = GrAtlasTextContext::Create();
    SkSurfaceProps props(SkSurfaceProps::kUseDistanceFieldFonts_Flag, kBGR_H_SkPixelGeometry);
    context->drawText(fContext->internal().grContext(), fTarget.get(), GrNoClip(), paint, SkMatrix::I(), props, (const char*)text, byteLength, x, y, SkIRect::MakeWH(this->width(), this->height()));
}

void SkAtlasTextTarget::addOp(std::unique_ptr<GrAtlasTextOp> op) {
    // TODO: batch ops here
    fOps.emplace_back(std::move(op));
}

void SkAtlasTextTarget::flush() {
    SkInternalAtlasTextContext& context = fContext->internal();

    for (int i = 0; i < fOps.count(); ++i) {
        fOps[i]->flush(&context, fTarget.get());
    }
}

void GrAtlasTextOp::flush(SkInternalAtlasTextContext* context, GrTextUtils::Target* target) {
    fFontCache->getAtlasPageCount(this->maskFormat());
    const sk_sp<GrTextureProxy>* proxies = fFontCache->getProxies(this->maskFormat());
    int glyphsToFlush = 0;
    size_t vertexSize = sizeof(SkAtlasTextRenderer::SDFVertex) * this->numGlyphs();
    std::unique_ptr<char[]> vertices(new char[vertexSize]);
    FlushInfo flushInfo;
    SkAutoGlyphCache glyphCache;
    GrBlobRegenHelper helper(this, target, &flushInfo);

    for (int i = 0; i < fGeoCount; ++i) {
        fGeoData[i].fBlob->regenInOp(context, context->atlasGlyphCache(), &helper, fGeoData[i].fRun, fGeoData[i].fSubRun, &glyphCache, sizeof(SkAtlasTextRenderer::SDFVertex), SkMatrix::I(), fGeoData[i].fX, fGeoData[i].fY, 0x0, *vertices);
    }
}