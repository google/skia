/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAtlasTextTarget.h"
#include "GrClip.h"
#include "GrContextPriv.h"
#include "GrDrawingManager.h"
#include "SkAtlasTextContext.h"
#include "SkAtlasTextFont.h"
#include "SkAtlasTextRenderer.h"
#include "SkGr.h"
#include "SkInternalAtlasTextContext.h"
#include "ops/GrAtlasTextOp.h"
#include "text/GrAtlasTextContext.h"

static constexpr int kMaxBatchLookBack = 10;

SkAtlasTextTarget::SkAtlasTextTarget(sk_sp<SkAtlasTextContext> context, int width, int height,
                                     void* handle)
        : fHandle(handle)
        , fContext(std::move(context))
        , fWidth(width)
        , fHeight(height)
        , fMatrixStack(sizeof(SkMatrix), 4)
        , fSaveCnt(0) {
    fMatrixStack.push_back();
    this->accessCTM()->reset();
}

SkAtlasTextTarget::~SkAtlasTextTarget() { fContext->renderer()->targetDeleted(fHandle); }

int SkAtlasTextTarget::save() {
    const auto& currCTM = this->ctm();
    *static_cast<SkMatrix*>(fMatrixStack.push_back()) = currCTM;
    return fSaveCnt++;
}

void SkAtlasTextTarget::restore() {
    if (fSaveCnt) {
        fMatrixStack.pop_back();
        fSaveCnt--;
    }
}

void SkAtlasTextTarget::restoreToCount(int count) {
    while (fSaveCnt > count) {
        this->restore();
    }
}

void SkAtlasTextTarget::translate(SkScalar dx, SkScalar dy) {
    this->accessCTM()->preTranslate(dx, dy);
}

void SkAtlasTextTarget::scale(SkScalar sx, SkScalar sy) { this->accessCTM()->preScale(sx, sy); }

void SkAtlasTextTarget::rotate(SkScalar degrees) { this->accessCTM()->preRotate(degrees); }

void SkAtlasTextTarget::rotate(SkScalar degrees, SkScalar px, SkScalar py) {
    this->accessCTM()->preRotate(degrees, px, py);
}

void SkAtlasTextTarget::skew(SkScalar sx, SkScalar sy) { this->accessCTM()->preSkew(sx, sy); }

void SkAtlasTextTarget::concat(const SkMatrix& matrix) { this->accessCTM()->preConcat(matrix); }

//////////////////////////////////////////////////////////////////////////////

static const GrColorSpaceInfo kColorSpaceInfo(nullptr, kRGBA_8888_GrPixelConfig);

//////////////////////////////////////////////////////////////////////////////

class SkInternalAtlasTextTarget : public GrTextUtils::Target, public SkAtlasTextTarget {
public:
    SkInternalAtlasTextTarget(sk_sp<SkAtlasTextContext> context, int width, int height,
                              void* handle)
            : GrTextUtils::Target(width, height, kColorSpaceInfo)
            , SkAtlasTextTarget(std::move(context), width, height, handle) {}

    /** GrTextUtils::Target overrides */

    void addDrawOp(const GrClip&, std::unique_ptr<GrAtlasTextOp> op) override;

    void drawPath(const GrClip&, const SkPath&, const SkPaint&, const SkMatrix& viewMatrix,
                  const SkMatrix* pathMatrix, const SkIRect& clipBounds) override {
        SkDebugf("Path glyph??");
    }

    void makeGrPaint(GrMaskFormat, const SkPaint& skPaint, const SkMatrix&,
                     GrPaint* grPaint) override {
        grPaint->setColor4f(SkColorToPremulGrColor4fLegacy(skPaint.getColor()));
    }

    /** SkAtlasTextTarget overrides */

    void drawText(const SkGlyphID[], const SkPoint[], int glyphCnt, uint32_t color,
                  const SkAtlasTextFont&) override;
    void flush() override;

private:
    uint32_t fColor;
    using SkAtlasTextTarget::fWidth;
    using SkAtlasTextTarget::fHeight;
    SkTArray<std::unique_ptr<GrAtlasTextOp>, true> fOps;
};

//////////////////////////////////////////////////////////////////////////////

std::unique_ptr<SkAtlasTextTarget> SkAtlasTextTarget::Make(sk_sp<SkAtlasTextContext> context,
                                                           int width, int height, void* handle) {
    return std::unique_ptr<SkAtlasTextTarget>(
            new SkInternalAtlasTextTarget(std::move(context), width, height, handle));
}

//////////////////////////////////////////////////////////////////////////////

void SkInternalAtlasTextTarget::drawText(const SkGlyphID glyphs[], const SkPoint positions[],
                                         int glyphCnt, uint32_t color,
                                         const SkAtlasTextFont& font) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTypeface(font.refTypeface());
    paint.setTextSize(font.size());
    paint.setStyle(SkPaint::kFill_Style);
    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);

    // The atlas text context does munging of the paint color. We store the client's color here
    // and then overwrite the generated op's color when addDrawOp() is called.
    fColor = color;

    SkSurfaceProps props(SkSurfaceProps::kUseDistanceFieldFonts_Flag, kUnknown_SkPixelGeometry);
    auto* grContext = this->context()->internal().grContext();
    auto bounds = SkIRect::MakeWH(fWidth, fHeight);
    auto atlasTextContext = grContext->contextPriv().drawingManager()->getAtlasTextContext();
    size_t byteLength = sizeof(SkGlyphID) * glyphCnt;
    const SkScalar* pos = &positions->fX;
    atlasTextContext->drawPosText(grContext, this, GrNoClip(), paint, this->ctm(), props,
                                  (const char*)glyphs, byteLength, pos, 2, {0, 0}, bounds);
}

void SkInternalAtlasTextTarget::addDrawOp(const GrClip& clip, std::unique_ptr<GrAtlasTextOp> op) {
    SkASSERT(clip.quickContains(SkRect::MakeIWH(fWidth, fHeight)));
    // The SkAtlasTextRenderer currently only handles grayscale SDF glyphs.
    if (op->maskType() != GrAtlasTextOp::kGrayscaleDistanceField_MaskType) {
        return;
    }
    const GrCaps& caps = *this->context()->internal().grContext()->caps();
    op->finalizeForTextTarget(fColor, caps);
    int n = SkTMin(kMaxBatchLookBack, fOps.count());
    for (int i = 0; i < n; ++i) {
        GrAtlasTextOp* other = fOps.fromBack(i).get();
        if (other->combineIfPossible(op.get(), caps)) {
            return;
        }
        if (GrRectsOverlap(op->bounds(), other->bounds())) {
            break;
        }
    }
    op->visitProxies([](GrSurfaceProxy*) {});
    fOps.emplace_back(std::move(op));
}

void SkInternalAtlasTextTarget::flush() {
    for (int i = 0; i < fOps.count(); ++i) {
        fOps[i]->executeForTextTarget(this);
    }
    this->context()->internal().flush();
    fOps.reset();
}

void GrAtlasTextOp::finalizeForTextTarget(uint32_t color, const GrCaps& caps) {
    for (int i = 0; i < fGeoCount; ++i) {
        fGeoData[i].fColor = color;
    }
    this->finalize(caps, nullptr /* applied clip */, GrPixelConfigIsClamped::kNo);
}

void GrAtlasTextOp::executeForTextTarget(SkAtlasTextTarget* target) {
    FlushInfo flushInfo;
    SkAutoGlyphCache autoGlyphCache;
    auto& context = target->context()->internal();
    auto glyphCache = context.grContext()->contextPriv().getGlyphCache();
    auto fullAtlasManager = context.grContext()->contextPriv().getFullAtlasManager();
    auto resourceProvider = context.grContext()->contextPriv().resourceProvider();

    for (int i = 0; i < fGeoCount; ++i) {
        GrAtlasTextBlob::VertexRegenerator regenerator(
                resourceProvider, fGeoData[i].fBlob, fGeoData[i].fRun, fGeoData[i].fSubRun,
                fGeoData[i].fViewMatrix, fGeoData[i].fX, fGeoData[i].fY, fGeoData[i].fColor,
                &context, glyphCache, fullAtlasManager, &autoGlyphCache);
        GrAtlasTextBlob::VertexRegenerator::Result result;
        do {
            result = regenerator.regenerate();
            context.recordDraw(result.fFirstVertex, result.fGlyphsRegenerated,
                               fGeoData[i].fViewMatrix, target->handle());
            if (!result.fFinished) {
                // Make space in the atlas so we can continue generating vertices.
                context.flush();
            }
        } while (!result.fFinished);
    }
}
