/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAtlasTextTarget.h"
#include "SkAtlasTextContext.h"
#include "SkAtlasTextRenderer.h"
#include "text/GrAtlasTextContext.h"
#include "SkInternalAtlasTextContext.h"
#include "ops/GrAtlasTextOp.h"
#include "SkGr.h"
#include "GrClip.h"

SkAtlasTextTarget::SkAtlasTextTarget(sk_sp<SkAtlasTextContext> context, int width, int height, void* handle)
        : fHandle(handle), fContext(std::move(context)), fWidth(width), fHeight(height) {}

SkAtlasTextTarget::~SkAtlasTextTarget() { fContext->renderer()->targetDeleted(fHandle); }

//////////////////////////////////////////////////////////////////////////////

static const GrColorSpaceInfo kColorSpaceInfo(nullptr, kRGBA_8888_GrPixelConfig);

//////////////////////////////////////////////////////////////////////////////

class SkInternalAtlasTextTarget : public GrTextUtils::Target, public SkAtlasTextTarget {
public:
    SkInternalAtlasTextTarget(sk_sp<SkAtlasTextContext> context, int width, int height, void* handle) : GrTextUtils::Target(width, height, kColorSpaceInfo), SkAtlasTextTarget(std::move(context), width, height, handle) {}

    /** GrTextUtils::Target overrides */

    void addDrawOp(const GrClip&, std::unique_ptr<GrAtlasTextOp> op) override;

    void drawPath(const GrClip&, const SkPath&, const SkPaint&,
                          const SkMatrix& viewMatrix, const SkMatrix* pathMatrix,
                          const SkIRect& clipBounds) override {
        SkDebugf("Path glyph??");
    }

    void makeGrPaint(GrMaskFormat, const SkPaint& skPaint, const SkMatrix&,
                     GrPaint* grPaint) override {
        grPaint->setColor4f(SkColorToPremulGrColor4fLegacy(skPaint.getColor()));
    }

    /** SkAtlasTextTarget overrides */

    void drawText(const void* text, size_t byteLength, SkScalar x, SkScalar y) override;
    void flush() override;

private:
    using SkAtlasTextTarget::fWidth;
    using SkAtlasTextTarget::fHeight;
    SkTArray<std::unique_ptr<GrAtlasTextOp>, true> fOps;
};

//////////////////////////////////////////////////////////////////////////////

std::unique_ptr<SkAtlasTextTarget> SkAtlasTextTarget::Make(sk_sp<SkAtlasTextContext> context,
                                                           int width, int height, void* handle) {
    return std::unique_ptr<SkAtlasTextTarget>(new SkInternalAtlasTextTarget(std::move(context), width, height, handle));
}

//////////////////////////////////////////////////////////////////////////////

#include "GrContextPriv.h"
#include "GrDrawingManager.h"

void SkInternalAtlasTextTarget::drawText(const void* text, size_t byteLength, SkScalar x, SkScalar y) {
    SkPaint paint;
    static const GrColorSpaceInfo kColorSpaceInfo(nullptr, kRGBA_8888_GrPixelConfig);

    SkSurfaceProps props(SkSurfaceProps::kUseDistanceFieldFonts_Flag, kBGR_H_SkPixelGeometry);
    auto* grContext = this->context()->internal().grContext();
    auto bounds = SkIRect::MakeWH(fWidth, fHeight);
    auto atlasTextContext = grContext->contextPriv().drawingManager()->getAtlasTextContext();
    atlasTextContext->drawText(grContext, this, GrNoClip(), paint, SkMatrix::I(), props,
                               (const char*)text, byteLength, x, y, bounds);
}

void SkInternalAtlasTextTarget::addDrawOp(const GrClip& clip, std::unique_ptr<GrAtlasTextOp> op) {
    SkASSERT(clip.quickContains(SkRect::MakeIWH(fWidth, fHeight)));
    // The SkAtlasTextRenderer currently only handles SDF glyphs.
    if (op->maskType() != GrAtlasTextOp::kGrayscaleDistanceField_MaskType) {
        return;
    }
    // TODO: batch ops here.
    fOps.emplace_back(std::move(op));
}

void SkInternalAtlasTextTarget::flush() {
    for (int i = 0; i < fOps.count(); ++i) {
        fOps[i]->executeForTextTarget(this);
    }
    this->context()->internal().flush();
}

void GrAtlasTextOp::executeForTextTarget(SkAtlasTextTarget* target) {
    fFontCache->getAtlasPageCount(this->maskFormat());
    //size_t vertexSize = sizeof(SkAtlasTextRenderer::SDFVertex) * this->numGlyphs();
    //std::unique_ptr<char[]> vertices(new char[vertexSize]);
    FlushInfo flushInfo;
    SkAutoGlyphCache glyphCache;
    auto& context = target->context()->internal();
    auto* atlasGlyphCache = context.grContext()->getAtlasGlyphCache();
    for (int i = 0; i < fGeoCount; ++i) {
        GrAtlasTextBlob::VertexRegenerator regenerator(fGeoData[i].fBlob, fGeoData[i].fRun, fGeoData[i].fSubRun, fGeoData[i].fViewMatrix, fGeoData[i].fX, fGeoData[i].fY, fGeoData[i].fColor, &context, atlasGlyphCache, &glyphCache, sizeof(SkAtlasTextRenderer::SDFVertex));
        GrAtlasTextBlob::VertexRegenerator::Result result;
        do {
            result = regenerator.regenerate();
            context.recordDraw(result.fFirstVertex, result.fGlyphsRegenerated, target);
            if (!result.fFinished) {
                // Make spoce in the atlas so we can continue generating vertices.
                context.flush();
            }
        } while(result.fFinished);
    }
}
