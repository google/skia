/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrStencilAndCoverTextContext.h"
#include "GrAtlasTextContext.h"
#include "GrContext.h"
#include "GrDrawContext.h"
#include "GrPath.h"
#include "GrPathRange.h"
#include "GrResourceProvider.h"
#include "GrTextUtils.h"
#include "SkAutoKern.h"
#include "SkDraw.h"
#include "SkDrawProcs.h"
#include "SkGlyphCache.h"
#include "SkGrPriv.h"
#include "SkDrawFilter.h"
#include "SkPath.h"
#include "SkTextBlobRunIterator.h"
#include "SkTextMapStateProc.h"
#include "SkTextFormatParams.h"

#include "batches/GrDrawPathBatch.h"

template<typename Key, typename Val> static void delete_hash_map_entry(const Key&, Val* val) {
    SkASSERT(*val);
    delete *val;
}

template<typename T> static void delete_hash_table_entry(T* val) {
    SkASSERT(*val);
    delete *val;
}

GrStencilAndCoverTextContext::GrStencilAndCoverTextContext()
    : fFallbackTextContext(nullptr)
    , fCacheSize(0) {
}

GrStencilAndCoverTextContext*
GrStencilAndCoverTextContext::Create() {
    GrStencilAndCoverTextContext* textContext = new GrStencilAndCoverTextContext();
    textContext->fFallbackTextContext = GrAtlasTextContext::Create();

    return textContext;
}

GrStencilAndCoverTextContext::~GrStencilAndCoverTextContext() {
    delete fFallbackTextContext;
    fBlobIdCache.foreach(delete_hash_map_entry<uint32_t, TextBlob*>);
    fBlobKeyCache.foreach(delete_hash_table_entry<TextBlob*>);
}

bool GrStencilAndCoverTextContext::internalCanDraw(const SkPaint& skPaint) {
    if (skPaint.getRasterizer()) {
        return false;
    }
    if (skPaint.getMaskFilter()) {
        return false;
    }
    if (SkPathEffect* pe = skPaint.getPathEffect()) {
        if (pe->asADash(nullptr) != SkPathEffect::kDash_DashType) {
            return false;
        }
    }
    // No hairlines. They would require new paths with customized strokes for every new draw matrix.
    return SkPaint::kStroke_Style != skPaint.getStyle() || 0 != skPaint.getStrokeWidth();
}

void GrStencilAndCoverTextContext::drawText(GrContext* context, GrDrawContext* dc,
                                            const GrClip& clip, const GrPaint& paint,
                                            const SkPaint& skPaint, const SkMatrix& viewMatrix,
                                            const SkSurfaceProps& props,
                                            const char text[], size_t byteLength,
                                            SkScalar x, SkScalar y, const SkIRect& clipBounds) {
    if (context->abandoned()) {
        return;
    } else if (this->canDraw(skPaint, viewMatrix)) {
        if (skPaint.getTextSize() > 0) {
            TextRun run(skPaint);
            GrPipelineBuilder pipelineBuilder(paint);
            run.setText(text, byteLength, x, y);
            run.draw(context, dc, &pipelineBuilder, clip, paint.getColor(), viewMatrix, props, 0, 0,
                     clipBounds, fFallbackTextContext, skPaint);
        }
        return;
    } else if (fFallbackTextContext->canDraw(skPaint, viewMatrix, props,
                                             *context->caps()->shaderCaps())) {
        fFallbackTextContext->drawText(context, dc, clip, paint, skPaint, viewMatrix, props, text,
                                       byteLength, x, y, clipBounds);
        return;
    }

    // fall back to drawing as a path
    GrTextUtils::DrawTextAsPath(context, dc, clip, skPaint, viewMatrix, text, byteLength, x, y,
                                clipBounds);
}

void GrStencilAndCoverTextContext::drawPosText(GrContext* context, GrDrawContext* dc,
                                               const GrClip& clip,
                                               const GrPaint& paint,
                                               const SkPaint& skPaint,
                                               const SkMatrix& viewMatrix,
                                               const SkSurfaceProps& props,
                                               const char text[],
                                               size_t byteLength,
                                               const SkScalar pos[],
                                               int scalarsPerPosition,
                                               const SkPoint& offset,
                                               const SkIRect& clipBounds) {
    if (context->abandoned()) {
        return;
    } else if (this->canDraw(skPaint, viewMatrix)) {
        if (skPaint.getTextSize() > 0) {
            TextRun run(skPaint);
            GrPipelineBuilder pipelineBuilder(paint);
            run.setPosText(text, byteLength, pos, scalarsPerPosition, offset);
            run.draw(context, dc, &pipelineBuilder, clip, paint.getColor(), viewMatrix, props, 0, 0,
                     clipBounds, fFallbackTextContext, skPaint);
        }
        return;
    } else if (fFallbackTextContext->canDraw(skPaint, viewMatrix, props,
                                             *context->caps()->shaderCaps())) {
        fFallbackTextContext->drawPosText(context, dc, clip, paint, skPaint, viewMatrix, props,
                                          text, byteLength, pos,
                                          scalarsPerPosition, offset, clipBounds);
        return;
    }

    // fall back to drawing as a path
    GrTextUtils::DrawPosTextAsPath(context, dc, props, clip, skPaint, viewMatrix, text,
                                   byteLength, pos, scalarsPerPosition, offset, clipBounds);
}

void GrStencilAndCoverTextContext::uncachedDrawTextBlob(GrContext* context,
                                                        GrDrawContext* dc,
                                                        const GrClip& clip, const SkPaint& skPaint,
                                                        const SkMatrix& viewMatrix,
                                                        const SkSurfaceProps& props,
                                                        const SkTextBlob* blob,
                                                        SkScalar x, SkScalar y,
                                                        SkDrawFilter* drawFilter,
                                                        const SkIRect& clipBounds) {
    SkPaint runPaint = skPaint;

    SkTextBlobRunIterator it(blob);
    for (;!it.done(); it.next()) {
        size_t textLen = it.glyphCount() * sizeof(uint16_t);
        const SkPoint& offset = it.offset();

        // applyFontToPaint() always overwrites the exact same attributes,
        // so it is safe to not re-seed the paint for this reason.
        it.applyFontToPaint(&runPaint);

        if (drawFilter && !drawFilter->filter(&runPaint, SkDrawFilter::kText_Type)) {
            // A false return from filter() means we should abort the current draw.
            runPaint = skPaint;
            continue;
        }

        runPaint.setFlags(GrTextUtils::FilterTextFlags(props, runPaint));

        GrPaint grPaint;
        if (!SkPaintToGrPaint(context, runPaint, viewMatrix, dc->isGammaCorrect(), &grPaint)) {
            return;
        }

        switch (it.positioning()) {
            case SkTextBlob::kDefault_Positioning:
                this->drawText(context, dc, clip, grPaint, runPaint, viewMatrix, props,
                               (const char *)it.glyphs(),
                               textLen, x + offset.x(), y + offset.y(), clipBounds);
                break;
            case SkTextBlob::kHorizontal_Positioning:
                this->drawPosText(context, dc, clip, grPaint, runPaint, viewMatrix, props,
                                  (const char*)it.glyphs(),
                                  textLen, it.pos(), 1, SkPoint::Make(x, y + offset.y()),
                                  clipBounds);
                break;
            case SkTextBlob::kFull_Positioning:
                this->drawPosText(context, dc, clip, grPaint, runPaint, viewMatrix, props,
                                  (const char*)it.glyphs(),
                                  textLen, it.pos(), 2, SkPoint::Make(x, y), clipBounds);
                break;
        }

        if (drawFilter) {
            // A draw filter may change the paint arbitrarily, so we must re-seed in this case.
            runPaint = skPaint;
        }
    }
}

void GrStencilAndCoverTextContext::drawTextBlob(GrContext* context, GrDrawContext* dc,
                                                const GrClip& clip, const SkPaint& skPaint,
                                                const SkMatrix& viewMatrix,
                                                const SkSurfaceProps& props,
                                                const SkTextBlob* skBlob, SkScalar x, SkScalar y,
                                                SkDrawFilter* drawFilter,
                                                const SkIRect& clipBounds) {
    if (context->abandoned()) {
        return;
    }

    if (!this->internalCanDraw(skPaint)) {
        fFallbackTextContext->drawTextBlob(context, dc, clip, skPaint, viewMatrix, props, skBlob,
                                           x, y, drawFilter, clipBounds);
        return;
    }

    if (drawFilter || skPaint.getPathEffect()) {
        // This draw can't be cached.
        this->uncachedDrawTextBlob(context, dc, clip, skPaint, viewMatrix, props, skBlob, x, y,
                                   drawFilter, clipBounds);
        return;
    }

    GrPaint paint;
    if (!SkPaintToGrPaint(context, skPaint, viewMatrix, dc->isGammaCorrect(), &paint)) {
        return;
    }

    const TextBlob& blob = this->findOrCreateTextBlob(skBlob, skPaint);
    GrPipelineBuilder pipelineBuilder(paint);

    TextBlob::Iter iter(blob);
    for (TextRun* run = iter.get(); run; run = iter.next()) {
        run->draw(context, dc, &pipelineBuilder, clip, paint.getColor(), viewMatrix, props,  x, y,
                  clipBounds, fFallbackTextContext, skPaint);
        run->releaseGlyphCache();
    }
}

static inline int style_key_cnt(const GrStyle& style) {
    int cnt = GrStyle::KeySize(style, GrStyle::Apply::kPathEffectAndStrokeRec);
    // We should be able to make a key because we filtered out arbitrary path effects.
    SkASSERT(cnt > 0);
    return cnt;
}

static inline void write_style_key(uint32_t* dst, const GrStyle& style) {
    // Pass 1 for the scale since the GPU will apply the style not GrStyle::applyToPath().
    GrStyle::WriteKey(dst, style, GrStyle::Apply::kPathEffectAndStrokeRec, SK_Scalar1);
}

const GrStencilAndCoverTextContext::TextBlob&
GrStencilAndCoverTextContext::findOrCreateTextBlob(const SkTextBlob* skBlob,
                                                   const SkPaint& skPaint) {
    // The font-related parameters are baked into the text blob and will override this skPaint, so
    // the only remaining properties that can affect a TextBlob are the ones related to stroke.
    if (SkPaint::kFill_Style == skPaint.getStyle()) { // Fast path.
        if (TextBlob** found = fBlobIdCache.find(skBlob->uniqueID())) {
            fLRUList.remove(*found);
            fLRUList.addToTail(*found);
            return **found;
        }
        TextBlob* blob = new TextBlob(skBlob->uniqueID(), skBlob, skPaint);
        this->purgeToFit(*blob);
        fBlobIdCache.set(skBlob->uniqueID(), blob);
        fLRUList.addToTail(blob);
        fCacheSize += blob->cpuMemorySize();
        return *blob;
    } else {
        GrStyle style(skPaint);
        SkSTArray<4, uint32_t, true> key;
        key.reset(1 + style_key_cnt(style));
        key[0] = skBlob->uniqueID();
        write_style_key(&key[1], style);
        if (TextBlob** found = fBlobKeyCache.find(key)) {
            fLRUList.remove(*found);
            fLRUList.addToTail(*found);
            return **found;
        }
        TextBlob* blob = new TextBlob(key, skBlob, skPaint);
        this->purgeToFit(*blob);
        fBlobKeyCache.set(blob);
        fLRUList.addToTail(blob);
        fCacheSize += blob->cpuMemorySize();
        return *blob;
    }
}

void GrStencilAndCoverTextContext::purgeToFit(const TextBlob& blob) {
    static const size_t maxCacheSize = 4 * 1024 * 1024; // Allow up to 4 MB for caching text blobs.

    size_t maxSizeForNewBlob = maxCacheSize - blob.cpuMemorySize();
    while (fCacheSize && fCacheSize > maxSizeForNewBlob) {
        TextBlob* lru = fLRUList.head();
        if (1 == lru->key().count()) {
            // 1-length keys are unterstood to be the blob id.
            fBlobIdCache.remove(lru->key()[0]);
        } else {
            fBlobKeyCache.remove(lru->key());
        }
        fLRUList.remove(lru);
        fCacheSize -= lru->cpuMemorySize();
        delete lru;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void GrStencilAndCoverTextContext::TextBlob::init(const SkTextBlob* skBlob,
                                                  const SkPaint& skPaint) {
    fCpuMemorySize = sizeof(TextBlob);
    SkPaint runPaint(skPaint);
    for (SkTextBlobRunIterator iter(skBlob); !iter.done(); iter.next()) {
        iter.applyFontToPaint(&runPaint); // No need to re-seed the paint.
        if (runPaint.getTextSize() <= 0) {
            continue;
        }
        TextRun* run = this->addToTail(runPaint);

        const char* text = reinterpret_cast<const char*>(iter.glyphs());
        size_t byteLength = sizeof(uint16_t) * iter.glyphCount();
        const SkPoint& runOffset = iter.offset();

        switch (iter.positioning()) {
            case SkTextBlob::kDefault_Positioning:
                run->setText(text, byteLength, runOffset.fX, runOffset.fY);
                break;
            case SkTextBlob::kHorizontal_Positioning:
                run->setPosText(text, byteLength, iter.pos(), 1, SkPoint::Make(0, runOffset.fY));
                break;
            case SkTextBlob::kFull_Positioning:
                run->setPosText(text, byteLength, iter.pos(), 2, SkPoint::Make(0, 0));
                break;
        }

        fCpuMemorySize += run->computeSizeInCache();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class GrStencilAndCoverTextContext::FallbackBlobBuilder {
public:
    FallbackBlobBuilder() : fBuffIdx(0), fCount(0) {}

    bool isInitialized() const { return fBuilder != nullptr; }

    void init(const SkPaint& font, SkScalar textRatio);

    void appendGlyph(uint16_t glyphId, const SkPoint& pos);

    const SkTextBlob* buildIfNeeded(int* count);

private:
    enum { kWriteBufferSize = 1024 };

    void flush();

    SkAutoTDelete<SkTextBlobBuilder>   fBuilder;
    SkPaint                            fFont;
    int                                fBuffIdx;
    int                                fCount;
    uint16_t                           fGlyphIds[kWriteBufferSize];
    SkPoint                            fPositions[kWriteBufferSize];
};

////////////////////////////////////////////////////////////////////////////////////////////////////

GrStencilAndCoverTextContext::TextRun::TextRun(const SkPaint& fontAndStroke)
    : fStyle(fontAndStroke),
      fFont(fontAndStroke),
      fTotalGlyphCount(0),
      fFallbackGlyphCount(0),
      fDetachedGlyphCache(nullptr),
      fLastDrawnGlyphsID(SK_InvalidUniqueID) {
    SkASSERT(fFont.getTextSize() > 0);
    SkASSERT(!fStyle.hasNonDashPathEffect()); // Arbitrary path effects not supported.
    SkASSERT(!fStyle.isSimpleHairline()); // Hairlines are not supported.

    // Setting to "fill" ensures that no strokes get baked into font outlines. (We use the GPU path
    // rendering API for stroking).
    fFont.setStyle(SkPaint::kFill_Style);

    if (fFont.isFakeBoldText() && fStyle.isSimpleFill()) {
        const SkStrokeRec& stroke = fStyle.strokeRec();
        // Instead of letting fake bold get baked into the glyph outlines, do it with GPU stroke.
        SkScalar fakeBoldScale = SkScalarInterpFunc(fFont.getTextSize(),
                                                    kStdFakeBoldInterpKeys,
                                                    kStdFakeBoldInterpValues,
                                                    kStdFakeBoldInterpLength);
        SkScalar extra = SkScalarMul(fFont.getTextSize(), fakeBoldScale);

        SkStrokeRec strokeRec(SkStrokeRec::kFill_InitStyle);
        strokeRec.setStrokeStyle(stroke.needToApply() ? stroke.getWidth() + extra : extra,
                                 true /*strokeAndFill*/);
        fStyle = GrStyle(strokeRec, fStyle.pathEffect());
        fFont.setFakeBoldText(false);
    }

    if (!fFont.getPathEffect() && !fStyle.isDashed()) {
        const SkStrokeRec& stroke = fStyle.strokeRec();
        // We can draw the glyphs from canonically sized paths.
        fTextRatio = fFont.getTextSize() / SkPaint::kCanonicalTextSizeForPaths;
        fTextInverseRatio = SkPaint::kCanonicalTextSizeForPaths / fFont.getTextSize();

        // Compensate for the glyphs being scaled by fTextRatio.
        if (!fStyle.isSimpleFill()) {
            SkStrokeRec strokeRec(SkStrokeRec::kFill_InitStyle);
            strokeRec.setStrokeStyle(stroke.getWidth() / fTextRatio,
                                     SkStrokeRec::kStrokeAndFill_Style == stroke.getStyle());
            fStyle = GrStyle(strokeRec, fStyle.pathEffect());
        }

        fFont.setLinearText(true);
        fFont.setLCDRenderText(false);
        fFont.setAutohinted(false);
        fFont.setHinting(SkPaint::kNo_Hinting);
        fFont.setSubpixelText(true);
        fFont.setTextSize(SkIntToScalar(SkPaint::kCanonicalTextSizeForPaths));

        fUsingRawGlyphPaths = SK_Scalar1 == fFont.getTextScaleX() &&
                              0 == fFont.getTextSkewX() &&
                              !fFont.isFakeBoldText() &&
                              !fFont.isVerticalText();
    } else {
        fTextRatio = fTextInverseRatio = 1.0f;
        fUsingRawGlyphPaths = false;
    }

    // Generate the key that will be used to cache the GPU glyph path objects.
    if (fUsingRawGlyphPaths && fStyle.isSimpleFill()) {
        static const GrUniqueKey::Domain kRawFillPathGlyphDomain = GrUniqueKey::GenerateDomain();

        const SkTypeface* typeface = fFont.getTypeface();
        GrUniqueKey::Builder builder(&fGlyphPathsKey, kRawFillPathGlyphDomain, 1);
        reinterpret_cast<uint32_t&>(builder[0]) = typeface ? typeface->uniqueID() : 0;
    } else {
        static const GrUniqueKey::Domain kPathGlyphDomain = GrUniqueKey::GenerateDomain();

        int styleDataCount = GrStyle::KeySize(fStyle, GrStyle::Apply::kPathEffectAndStrokeRec);
        // Key should be valid since we opted out of drawing arbitrary path effects.
        SkASSERT(styleDataCount >= 0);
        if (fUsingRawGlyphPaths) {
            const SkTypeface* typeface = fFont.getTypeface();
            GrUniqueKey::Builder builder(&fGlyphPathsKey, kPathGlyphDomain, 2 + styleDataCount);
            reinterpret_cast<uint32_t&>(builder[0]) = typeface ? typeface->uniqueID() : 0;
            reinterpret_cast<uint32_t&>(builder[1]) = styleDataCount;
            if (styleDataCount) {
                write_style_key(&builder[2], fStyle);
            }
        } else {
            SkGlyphCache* glyphCache = this->getGlyphCache();
            const SkTypeface* typeface = glyphCache->getScalerContext()->getTypeface();
            const SkDescriptor* desc = &glyphCache->getDescriptor();
            int descDataCount = (desc->getLength() + 3) / 4;
            GrUniqueKey::Builder builder(&fGlyphPathsKey, kPathGlyphDomain,
                                         2 + styleDataCount + descDataCount);
            reinterpret_cast<uint32_t&>(builder[0]) = typeface ? typeface->uniqueID() : 0;
            reinterpret_cast<uint32_t&>(builder[1]) = styleDataCount | (descDataCount << 16);
            if (styleDataCount) {
                write_style_key(&builder[2], fStyle);
            }
            memcpy(&builder[2 + styleDataCount], desc, desc->getLength());
        }
    }
}

GrStencilAndCoverTextContext::TextRun::~TextRun() {
    this->releaseGlyphCache();
}

void GrStencilAndCoverTextContext::TextRun::setText(const char text[], size_t byteLength,
                                                    SkScalar x, SkScalar y) {
    SkASSERT(byteLength == 0 || text != nullptr);

    SkGlyphCache* glyphCache = this->getGlyphCache();
    SkPaint::GlyphCacheProc glyphCacheProc = fFont.getGlyphCacheProc(true);

    fTotalGlyphCount = fFont.countText(text, byteLength);
    fInstanceData.reset(InstanceData::Alloc(GrPathRendering::kTranslate_PathTransformType,
                                            fTotalGlyphCount));

    const char* stop = text + byteLength;

    // Measure first if needed.
    if (fFont.getTextAlign() != SkPaint::kLeft_Align) {
        SkScalar   stopX = 0;
        SkScalar   stopY = 0;

        const char* textPtr = text;
        while (textPtr < stop) {
            // We don't need x, y here, since all subpixel variants will have the
            // same advance.
            const SkGlyph& glyph = glyphCacheProc(glyphCache, &textPtr);

            stopX += SkFloatToScalar(glyph.fAdvanceX);
            stopY += SkFloatToScalar(glyph.fAdvanceY);
        }
        SkASSERT(textPtr == stop);

        SkScalar alignX = stopX * fTextRatio;
        SkScalar alignY = stopY * fTextRatio;

        if (fFont.getTextAlign() == SkPaint::kCenter_Align) {
            alignX = SkScalarHalf(alignX);
            alignY = SkScalarHalf(alignY);
        }

        x -= alignX;
        y -= alignY;
    }

    SkAutoKern autokern;

    FallbackBlobBuilder fallback;
    while (text < stop) {
        const SkGlyph& glyph = glyphCacheProc(glyphCache, &text);
        x += autokern.adjust(glyph) * fTextRatio;
        if (glyph.fWidth) {
            this->appendGlyph(glyph, SkPoint::Make(x, y), &fallback);
        }

        x += SkFloatToScalar(glyph.fAdvanceX) * fTextRatio;
        y += SkFloatToScalar(glyph.fAdvanceY) * fTextRatio;
    }

    fFallbackTextBlob.reset(fallback.buildIfNeeded(&fFallbackGlyphCount));
}

void GrStencilAndCoverTextContext::TextRun::setPosText(const char text[], size_t byteLength,
                                                       const SkScalar pos[], int scalarsPerPosition,
                                                       const SkPoint& offset) {
    SkASSERT(byteLength == 0 || text != nullptr);
    SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);

    SkGlyphCache* glyphCache = this->getGlyphCache();
    SkPaint::GlyphCacheProc glyphCacheProc = fFont.getGlyphCacheProc(true);

    fTotalGlyphCount = fFont.countText(text, byteLength);
    fInstanceData.reset(InstanceData::Alloc(GrPathRendering::kTranslate_PathTransformType,
                                            fTotalGlyphCount));

    const char* stop = text + byteLength;

    SkTextMapStateProc tmsProc(SkMatrix::I(), offset, scalarsPerPosition);
    SkTextAlignProc alignProc(fFont.getTextAlign());
    FallbackBlobBuilder fallback;
    while (text < stop) {
        const SkGlyph& glyph = glyphCacheProc(glyphCache, &text);
        if (glyph.fWidth) {
            SkPoint tmsLoc;
            tmsProc(pos, &tmsLoc);
            SkPoint loc;
            alignProc(tmsLoc, glyph, &loc);

            this->appendGlyph(glyph, loc, &fallback);
        }
        pos += scalarsPerPosition;
    }

    fFallbackTextBlob.reset(fallback.buildIfNeeded(&fFallbackGlyphCount));
}

GrPathRange* GrStencilAndCoverTextContext::TextRun::createGlyphs(GrContext* ctx) const {
    GrPathRange* glyphs = static_cast<GrPathRange*>(
            ctx->resourceProvider()->findAndRefResourceByUniqueKey(fGlyphPathsKey));
    if (nullptr == glyphs) {
        if (fUsingRawGlyphPaths) {
            SkScalerContextEffects noeffects;
            glyphs = ctx->resourceProvider()->createGlyphs(fFont.getTypeface(), noeffects,
                                                           nullptr, fStyle);
        } else {
            SkGlyphCache* cache = this->getGlyphCache();
            glyphs = ctx->resourceProvider()->createGlyphs(cache->getScalerContext()->getTypeface(),
                                                           cache->getScalerContext()->getEffects(),
                                                           &cache->getDescriptor(),
                                                           fStyle);
        }
        ctx->resourceProvider()->assignUniqueKeyToResource(fGlyphPathsKey, glyphs);
    }
    return glyphs;
}

inline void GrStencilAndCoverTextContext::TextRun::appendGlyph(const SkGlyph& glyph,
                                                               const SkPoint& pos,
                                                               FallbackBlobBuilder* fallback) {
    // Stick the glyphs we can't draw into the fallback text blob.
    if (SkMask::kARGB32_Format == glyph.fMaskFormat) {
        if (!fallback->isInitialized()) {
            fallback->init(fFont, fTextRatio);
        }
        fallback->appendGlyph(glyph.getGlyphID(), pos);
    } else {
        fInstanceData->append(glyph.getGlyphID(), fTextInverseRatio * pos.x(),
                              fTextInverseRatio * pos.y());
    }
}

void GrStencilAndCoverTextContext::TextRun::draw(GrContext* ctx,
                                                 GrDrawContext* drawContext,
                                                 GrPipelineBuilder* pipelineBuilder,
                                                 const GrClip& clip,
                                                 GrColor color,
                                                 const SkMatrix& viewMatrix,
                                                 const SkSurfaceProps& props,
                                                 SkScalar x, SkScalar y,
                                                 const SkIRect& clipBounds,
                                                 GrAtlasTextContext* fallbackTextContext,
                                                 const SkPaint& originalSkPaint) const {
    SkASSERT(fInstanceData);
    SkASSERT(drawContext->isStencilBufferMultisampled() || !fFont.isAntiAlias());

    if (fInstanceData->count()) {
        pipelineBuilder->setState(GrPipelineBuilder::kHWAntialias_Flag, fFont.isAntiAlias());

        static constexpr GrUserStencilSettings kCoverPass(
            GrUserStencilSettings::StaticInit<
                0x0000,
                GrUserStencilTest::kNotEqual, // Stencil pass accounts for clip.
                0xffff,
                GrUserStencilOp::kZero,
                GrUserStencilOp::kKeep,
                0xffff>()
        );

        pipelineBuilder->setUserStencil(&kCoverPass);

        SkAutoTUnref<GrPathRange> glyphs(this->createGlyphs(ctx));
        if (fLastDrawnGlyphsID != glyphs->getUniqueID()) {
            // Either this is the first draw or the glyphs object was purged since last draw.
            glyphs->loadPathsIfNeeded(fInstanceData->indices(), fInstanceData->count());
            fLastDrawnGlyphsID = glyphs->getUniqueID();
        }

        // Don't compute a bounding box. For dst copy texture, we'll opt instead for it to just copy
        // the entire dst. Realistically this is a moot point, because any context that supports
        // NV_path_rendering will also support NV_blend_equation_advanced.
        // For clipping we'll just skip any optimizations based on the bounds. This does, however,
        // hurt batching.
        const SkRect bounds = SkRect::MakeIWH(drawContext->width(), drawContext->height());

        SkAutoTUnref<GrDrawBatch> batch(
            GrDrawPathRangeBatch::Create(viewMatrix, fTextRatio, fTextInverseRatio * x,
                                         fTextInverseRatio * y, color,
                                         GrPathRendering::kWinding_FillType, glyphs, fInstanceData,
                                         bounds));

        drawContext->drawBatch(*pipelineBuilder, clip, batch);
    }

    if (fFallbackTextBlob) {
        SkPaint fallbackSkPaint(originalSkPaint);
        fStyle.strokeRec().applyToPaint(&fallbackSkPaint);
        if (!fStyle.isSimpleFill()) {
            fallbackSkPaint.setStrokeWidth(fStyle.strokeRec().getWidth() * fTextRatio);
        }

        fallbackTextContext->drawTextBlob(ctx, drawContext, clip, fallbackSkPaint, viewMatrix,
                                          props, fFallbackTextBlob, x, y, nullptr, clipBounds);
    }
}

SkGlyphCache* GrStencilAndCoverTextContext::TextRun::getGlyphCache() const {
    if (!fDetachedGlyphCache) {
        fDetachedGlyphCache = fFont.detachCache(nullptr, SkPaint::kNone_ScalerContextFlags,
                                                nullptr);
    }
    return fDetachedGlyphCache;
}


void GrStencilAndCoverTextContext::TextRun::releaseGlyphCache() const {
    if (fDetachedGlyphCache) {
        SkGlyphCache::AttachCache(fDetachedGlyphCache);
        fDetachedGlyphCache = nullptr;
    }
}

size_t GrStencilAndCoverTextContext::TextRun::computeSizeInCache() const {
    size_t size = sizeof(TextRun) + fGlyphPathsKey.size();
    // The instance data always reserves enough space for every glyph.
    size += (fTotalGlyphCount + fFallbackGlyphCount) * (sizeof(uint16_t) + 2 * sizeof(float));
    if (fInstanceData) {
        size += sizeof(InstanceData);
    }
    if (fFallbackTextBlob) {
        size += sizeof(SkTextBlob);
    }
    return size;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void GrStencilAndCoverTextContext::FallbackBlobBuilder::init(const SkPaint& font,
                                                             SkScalar textRatio) {
    SkASSERT(!this->isInitialized());
    fBuilder.reset(new SkTextBlobBuilder);
    fFont = font;
    fFont.setTextAlign(SkPaint::kLeft_Align); // The glyph positions will already account for align.
    fFont.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
    // No need for subpixel positioning with bitmap glyphs. TODO: revisit if non-bitmap color glyphs
    // show up and https://code.google.com/p/skia/issues/detail?id=4408 gets resolved.
    fFont.setSubpixelText(false);
    fFont.setTextSize(fFont.getTextSize() * textRatio);
    fBuffIdx = 0;
}

void GrStencilAndCoverTextContext::FallbackBlobBuilder::appendGlyph(uint16_t glyphId,
                                                                    const SkPoint& pos) {
    SkASSERT(this->isInitialized());
    if (fBuffIdx >= kWriteBufferSize) {
        this->flush();
    }
    fGlyphIds[fBuffIdx] = glyphId;
    fPositions[fBuffIdx] = pos;
    fBuffIdx++;
    fCount++;
}

void GrStencilAndCoverTextContext::FallbackBlobBuilder::flush() {
    SkASSERT(this->isInitialized());
    SkASSERT(fBuffIdx <= kWriteBufferSize);
    if (!fBuffIdx) {
        return;
    }
    // This will automatically merge with previous runs since we use the same font.
    const SkTextBlobBuilder::RunBuffer& buff = fBuilder->allocRunPos(fFont, fBuffIdx);
    memcpy(buff.glyphs, fGlyphIds, fBuffIdx * sizeof(uint16_t));
    memcpy(buff.pos, fPositions[0].asScalars(), fBuffIdx * 2 * sizeof(SkScalar));
    fBuffIdx = 0;
}

const SkTextBlob* GrStencilAndCoverTextContext::FallbackBlobBuilder::buildIfNeeded(int *count) {
    *count = fCount;
    if (fCount) {
        this->flush();
        return fBuilder->build();
    }
    return nullptr;
}
