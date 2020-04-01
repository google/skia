/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/atlastext/SkAtlasTextTarget.h"

#include "include/atlastext/SkAtlasTextContext.h"
#include "include/atlastext/SkAtlasTextFont.h"
#include "include/atlastext/SkAtlasTextRenderer.h"
#include "src/atlastext/SkInternalAtlasTextContext.h"
#include "src/core/SkGlyphRunPainter.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/ops/GrAtlasTextOp.h"
#include "src/gpu/text/GrTextContext.h"

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

static const GrColorInfo kColorInfo(GrColorType::kRGBA_8888, kPremul_SkAlphaType, nullptr);
static const SkSurfaceProps kProps(
        SkSurfaceProps::kUseDistanceFieldFonts_Flag, kUnknown_SkPixelGeometry);

//////////////////////////////////////////////////////////////////////////////

class SkInternalAtlasTextTarget : public GrTextTarget, public SkAtlasTextTarget {
public:
    SkInternalAtlasTextTarget(sk_sp<SkAtlasTextContext> context, int width, int height,
                              void* handle)
            : GrTextTarget(width, height, kColorInfo)
            , SkAtlasTextTarget(std::move(context), width, height, handle)
            , fGlyphPainter(kProps, kColorInfo) {}

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

    GrRecordingContext::Arenas arenas() {
        return fContext->internal().grContext()->GrRecordingContext::priv().arenas();
    }

    uint32_t fColor;
    using SkAtlasTextTarget::fWidth;
    using SkAtlasTextTarget::fHeight;
    SkTArray<std::unique_ptr<GrAtlasTextOp>, true> fOps;
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
    int n = std::min(kMaxBatchLookBack, fOps.count());

    GrRecordingContext::Arenas arenas = this->arenas();
    for (int i = 0; i < n; ++i) {
        GrAtlasTextOp* other = fOps.fromBack(i).get();
        if (other->combineIfPossible(op.get(), &arenas, caps) == GrOp::CombineResult::kMerged) {
            arenas.opMemoryPool()->release(std::move(op));
            return;
        }
        if (GrRectsOverlap(op->bounds(), other->bounds())) {
            break;
        }
    }
    fOps.emplace_back(std::move(op));
}

void SkInternalAtlasTextTarget::deleteOps() {
    GrOpMemoryPool* pool = this->arenas().opMemoryPool();
    for (int i = 0; i < fOps.count(); ++i) {
        if (fOps[i]) {
            pool->release(std::move(fOps[i]));
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
    // Atlas text doesn't use MSAA, so no need to handle mixed samples.
    // Also, no need to support normalized F16 with manual clamp?
    this->finalize(caps, nullptr /* applied clip */, false /* mixed samples */, GrClampType::kAuto);
}

void GrAtlasTextOp::executeForTextTarget(SkAtlasTextTarget* target) {
    auto& context = target->context()->internal();
    auto atlasManager = context.grContext()->priv().getAtlasManager();
    auto resourceProvider = context.grContext()->priv().resourceProvider();

    unsigned int numProxies;
    if (!atlasManager->getViews(kA8_GrMaskFormat, &numProxies)) {
        return;
    }

    for (int i = 0; i < fGeoCount; ++i) {
        auto subRun = fGeoData[i].fSubRunPtr;
        // TODO4F: Preserve float colors
        subRun->updateVerticesColorIfNeeded(fGeoData[i].fColor.toBytes_RGBA());
        subRun->translateVerticesIfNeeded(fGeoData[i].fDrawMatrix, fGeoData[i].fDrawOrigin);
        GrTextBlob::VertexRegenerator regenerator(
                resourceProvider, fGeoData[i].fSubRunPtr, &context, atlasManager);
        int subRunEnd = subRun->fGlyphs.count();
        for (int subRunIndex = 0; subRunIndex < subRunEnd;) {
            auto [ok, glyphsRegenerated] = regenerator.regenerate(subRunIndex, subRunEnd);
            if (!ok) {
                break;
            }

            context.recordDraw(subRun->quadStart(subRunIndex), glyphsRegenerated,
                               fGeoData[i].fDrawMatrix, target->handle());
            subRunIndex += glyphsRegenerated;
            if (subRunIndex != subRunEnd) {
                // Make space in the atlas so we can continue generating vertices.
                context.flush();
            }
        }
    }
}
