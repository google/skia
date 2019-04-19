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
#include "GrMemoryPool.h"
#include "SkAtlasTextContext.h"
#include "SkAtlasTextFont.h"
#include "SkAtlasTextRenderer.h"
#include "SkGlyphRunPainter.h"
#include "SkGr.h"
#include "SkInternalAtlasTextContext.h"
#include "ops/GrAtlasTextOp.h"
#include "text/GrTextContext.h"

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
static const SkSurfaceProps kProps(
        SkSurfaceProps::kUseDistanceFieldFonts_Flag, kUnknown_SkPixelGeometry);

//////////////////////////////////////////////////////////////////////////////

class SkInternalAtlasTextTarget : public GrTextTarget, public SkAtlasTextTarget {
public:
    SkInternalAtlasTextTarget(sk_sp<SkAtlasTextContext> context,
                              int width, int height,
                              void* handle)
            : GrTextTarget(width, height, kColorSpaceInfo)
            , SkAtlasTextTarget(std::move(context), width, height, handle)
            , fGlyphPainter(kProps, kColorSpaceInfo) {
        fOpMemoryPool = fContext->internal().grContext()->priv().refOpMemoryPool();
    }

    ~SkInternalAtlasTextTarget() override {
        this->deleteOps();
    }

    /** GrTextTarget overrides */

    void addDrawOp(const GrClip&, std::unique_ptr<GrAtlasTextOp> op) override;

    void drawShape(const GrClip&, const SkPaint&, const SkMatrix& viewMatrix,
                   const GrShape&) override {
        SkDebugf("Path glyph??");
    }

    void makeGrPaint(GrMaskFormat, const SkPaint& skPaint, const SkMatrix&,
                     GrPaint* grPaint) override {
        grPaint->setColor4f(skPaint.getColor4f().premul());
    }

    GrContext* getContext() override {
        return this->context()->internal().grContext();
    }

    SkGlyphRunListPainter* glyphPainter() override {
        return &fGlyphPainter;
    }

    /** SkAtlasTextTarget overrides */

    void drawText(const SkGlyphID[], const SkPoint[], int glyphCnt, uint32_t color,
                  const SkAtlasTextFont&) override;
    void flush() override;

private:
    void deleteOps();

    uint32_t fColor;
    using SkAtlasTextTarget::fWidth;
    using SkAtlasTextTarget::fHeight;
    SkTArray<std::unique_ptr<GrAtlasTextOp>, true> fOps;
    sk_sp<GrOpMemoryPool> fOpMemoryPool;
    SkGlyphRunListPainter fGlyphPainter;
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

    // The atlas text context does munging of the paint color. We store the client's color here
    // and then overwrite the generated op's color when addDrawOp() is called.
    fColor = color;

    SkSurfaceProps props(SkSurfaceProps::kUseDistanceFieldFonts_Flag, kUnknown_SkPixelGeometry);
    auto grContext = this->context()->internal().grContext();
    auto atlasTextContext = grContext->priv().drawingManager()->getTextContext();
    SkGlyphRunBuilder builder;
    builder.drawGlyphsWithPositions(paint, font.makeFont(),
                                    SkSpan<const SkGlyphID>{glyphs, SkTo<size_t>(glyphCnt)},
                                    positions);
    auto glyphRunList = builder.useGlyphRunList();
    if (!glyphRunList.empty()) {
        atlasTextContext->drawGlyphRunList(grContext, this, GrNoClip(), this->ctm(), props,
                                           glyphRunList);
    }
}

void SkInternalAtlasTextTarget::addDrawOp(const GrClip& clip, std::unique_ptr<GrAtlasTextOp> op) {
    SkASSERT(clip.quickContains(SkRect::MakeIWH(fWidth, fHeight)));
    // The SkAtlasTextRenderer currently only handles grayscale SDF glyphs.
    if (op->maskType() != GrAtlasTextOp::kGrayscaleDistanceField_MaskType) {
        return;
    }
    const GrCaps& caps = *this->context()->internal().grContext()->priv().caps();
    op->finalizeForTextTarget(fColor, caps);
    int n = SkTMin(kMaxBatchLookBack, fOps.count());
    for (int i = 0; i < n; ++i) {
        GrAtlasTextOp* other = fOps.fromBack(i).get();
        if (other->combineIfPossible(op.get(), caps) == GrOp::CombineResult::kMerged) {
            fOpMemoryPool->release(std::move(op));
            return;
        }
        if (GrRectsOverlap(op->bounds(), other->bounds())) {
            break;
        }
    }
    fOps.emplace_back(std::move(op));
}

void SkInternalAtlasTextTarget::deleteOps() {
    for (int i = 0; i < fOps.count(); ++i) {
        if (fOps[i]) {
            fOpMemoryPool->release(std::move(fOps[i]));
        }
    }
    fOps.reset();
}

void SkInternalAtlasTextTarget::flush() {
    for (int i = 0; i < fOps.count(); ++i) {
        fOps[i]->executeForTextTarget(this);
    }
    this->context()->internal().flush();
    this->deleteOps();
}

void GrAtlasTextOp::finalizeForTextTarget(uint32_t color, const GrCaps& caps) {
    // TODO4F: Odd handling of client colors among AtlasTextTarget and AtlasTextRenderer
    SkPMColor4f color4f = SkPMColor4f::FromBytes_RGBA(color);
    for (int i = 0; i < fGeoCount; ++i) {
        fGeoData[i].fColor = color4f;
    }
    // Atlas text doesn't use MSAA, so no need to handle a GrFSAAType.
    // Also, no need to support normalized F16 with manual clamp?
    this->finalize(caps, nullptr /* applied clip */, GrFSAAType::kNone, GrClampType::kAuto);
}

void GrAtlasTextOp::executeForTextTarget(SkAtlasTextTarget* target) {
    FlushInfo flushInfo;
    SkExclusiveStrikePtr autoGlyphCache;
    auto& context = target->context()->internal();
    auto glyphCache = context.grContext()->priv().getGrStrikeCache();
    auto atlasManager = context.grContext()->priv().getAtlasManager();
    auto resourceProvider = context.grContext()->priv().resourceProvider();

    unsigned int numProxies;
    if (!atlasManager->getProxies(kA8_GrMaskFormat, &numProxies)) {
        return;
    }

    for (int i = 0; i < fGeoCount; ++i) {
        // TODO4F: Preserve float colors
        GrTextBlob::VertexRegenerator regenerator(
                resourceProvider, fGeoData[i].fBlob, fGeoData[i].fRun, fGeoData[i].fSubRun,
                fGeoData[i].fViewMatrix, fGeoData[i].fX, fGeoData[i].fY,
                fGeoData[i].fColor.toBytes_RGBA(), &context, glyphCache, atlasManager,
                &autoGlyphCache);
        bool done = false;
        while (!done) {
            GrTextBlob::VertexRegenerator::Result result;
            if (!regenerator.regenerate(&result)) {
                break;
            }
            done = result.fFinished;

            context.recordDraw(result.fFirstVertex, result.fGlyphsRegenerated,
                               fGeoData[i].fViewMatrix, target->handle());
            if (!result.fFinished) {
                // Make space in the atlas so we can continue generating vertices.
                context.flush();
            }
        }
    }
}
