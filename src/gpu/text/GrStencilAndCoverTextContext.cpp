/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrStencilAndCoverTextContext.h"
#include "GrAtlasTextContext.h"
#include "GrContext.h"
#include "GrPath.h"
#include "GrPathRange.h"
#include "GrPipelineBuilder.h"
#include "GrRenderTargetContext.h"
#include "GrResourceProvider.h"
#include "GrTextUtils.h"
#include "SkAutoKern.h"
#include "SkDraw.h"
#include "SkDrawFilter.h"
#include "SkDrawProcs.h"
#include "SkGlyphCache.h"
#include "SkGr.h"
#include "SkPath.h"
#include "SkTextBlobRunIterator.h"
#include "SkTextFormatParams.h"
#include "SkTextMapStateProc.h"

#include "ops/GrDrawPathOp.h"

template<typename Key, typename Val> static void delete_hash_map_entry(const Key&, Val* val) {
    SkASSERT(*val);
    delete *val;
}

template<typename T> static void delete_hash_table_entry(T* val) {
    SkASSERT(*val);
    delete *val;
}

GrStencilAndCoverTextContext::GrStencilAndCoverTextContext(GrAtlasTextContext* fallbackTextContext)
    : fFallbackTextContext(fallbackTextContext)
    , fCacheSize(0) {
}

GrStencilAndCoverTextContext*
GrStencilAndCoverTextContext::Create(GrAtlasTextContext* fallbackTextContext) {
    return new GrStencilAndCoverTextContext(fallbackTextContext);;
}

GrStencilAndCoverTextContext::~GrStencilAndCoverTextContext() {
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

void GrStencilAndCoverTextContext::drawText(GrContext* context, GrRenderTargetContext* rtc,
                                            const GrClip& clip, const SkPaint& skPaint,
                                            const SkMatrix& viewMatrix, const SkSurfaceProps& props,
                                            const char text[], size_t byteLength, SkScalar x,
                                            SkScalar y, const SkIRect& clipBounds) {
    if (context->abandoned()) {
        return;
    } else if (this->canDraw(skPaint, viewMatrix)) {
        if (skPaint.getTextSize() > 0) {
            TextRun run(skPaint);
            run.setText(text, byteLength, x, y);
            run.draw(context, rtc, clip, viewMatrix, props, 0, 0, clipBounds, fFallbackTextContext,
                     skPaint);
        }
        return;
    } else if (fFallbackTextContext->canDraw(skPaint, viewMatrix, props,
                                             *context->caps()->shaderCaps())) {
        fFallbackTextContext->drawText(context, rtc, clip, skPaint, viewMatrix, props, text,
                                       byteLength, x, y, clipBounds);
        return;
    }

    // fall back to drawing as a path
    GrTextUtils::DrawTextAsPath(context, rtc, clip, skPaint, viewMatrix, text, byteLength, x, y,
                                clipBounds);
}

void GrStencilAndCoverTextContext::drawPosText(GrContext* context, GrRenderTargetContext* rtc,
                                               const GrClip& clip, const SkPaint& skPaint,
                                               const SkMatrix& viewMatrix,
                                               const SkSurfaceProps& props, const char text[],
                                               size_t byteLength, const SkScalar pos[],
                                               int scalarsPerPosition, const SkPoint& offset,
                                               const SkIRect& clipBounds) {
    if (context->abandoned()) {
        return;
    } else if (this->canDraw(skPaint, viewMatrix)) {
        if (skPaint.getTextSize() > 0) {
            TextRun run(skPaint);
            run.setPosText(text, byteLength, pos, scalarsPerPosition, offset);
            run.draw(context, rtc, clip, viewMatrix, props, 0, 0, clipBounds, fFallbackTextContext,
                     skPaint);
        }
        return;
    } else if (fFallbackTextContext->canDraw(skPaint, viewMatrix, props,
                                             *context->caps()->shaderCaps())) {
        fFallbackTextContext->drawPosText(context, rtc, clip, skPaint, viewMatrix, props, text,
                                          byteLength, pos, scalarsPerPosition, offset, clipBounds);
        return;
    }

    // fall back to drawing as a path
    GrTextUtils::DrawPosTextAsPath(context, rtc, props, clip, skPaint, viewMatrix, text,
                                   byteLength, pos, scalarsPerPosition, offset, clipBounds);
}

void GrStencilAndCoverTextContext::uncachedDrawTextBlob(GrContext* context,
                                                        GrRenderTargetContext* rtc,
                                                        const GrClip& clip,
                                                        const SkPaint& skPaint,
                                                        const SkMatrix& viewMatrix,
                                                        const SkSurfaceProps& props,
                                                        const SkTextBlob* blob,
                                                        SkScalar x, SkScalar y,
                                                        SkDrawFilter* drawFilter,
                                                        const SkIRect& clipBounds) {
    GrTextUtils::Paint paint(&skPaint);
    GrTextUtils::RunPaint runPaint(&paint, drawFilter, props);
    SkTextBlobRunIterator it(blob);
    for (;!it.done(); it.next()) {
        if (!runPaint.modifyForRun(it)) {
            continue;
        }
        size_t textLen = it.glyphCount() * sizeof(uint16_t);
        const SkPoint& offset = it.offset();

        switch (it.positioning()) {
            case SkTextBlob::kDefault_Positioning:
                this->drawText(context, rtc, clip, runPaint, viewMatrix, props,
                               (const char*)it.glyphs(), textLen, x + offset.x(), y + offset.y(),
                               clipBounds);
                break;
            case SkTextBlob::kHorizontal_Positioning:
                this->drawPosText(context, rtc, clip, runPaint, viewMatrix, props,
                                  (const char*)it.glyphs(), textLen, it.pos(), 1,
                                  SkPoint::Make(x, y + offset.y()), clipBounds);
                break;
            case SkTextBlob::kFull_Positioning:
                this->drawPosText(context, rtc, clip, runPaint, viewMatrix, props,
                                  (const char*)it.glyphs(), textLen, it.pos(), 2,
                                  SkPoint::Make(x, y), clipBounds);
                break;
        }
    }
}

void GrStencilAndCoverTextContext::drawTextBlob(GrContext* context, GrRenderTargetContext* rtc,
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
        fFallbackTextContext->drawTextBlob(context, rtc, clip, skPaint, viewMatrix, props, skBlob,
                                           x, y, drawFilter, clipBounds);
        return;
    }

    if (drawFilter || skPaint.getPathEffect()) {
        // This draw can't be cached.
        this->uncachedDrawTextBlob(context, rtc, clip, skPaint, viewMatrix, props, skBlob, x, y,
                                   drawFilter, clipBounds);
        return;
    }

    const TextBlob& blob = this->findOrCreateTextBlob(skBlob, skPaint);

    TextBlob::Iter iter(blob);
    for (TextRun *run = iter.get(), *nextRun; run; run = nextRun) {
        nextRun = iter.next();
        run->draw(context, rtc, clip, viewMatrix, props, x, y, clipBounds, fFallbackTextContext,
                  skPaint);
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

    sk_sp<SkTextBlob> makeIfNeeded(int* count);

private:
    enum { kWriteBufferSize = 1024 };

    void flush();

    std::unique_ptr<SkTextBlobBuilder> fBuilder;
    SkPaint                            fFont;
    int                                fBuffIdx;
    int                                fCount;
    uint16_t                           fGlyphIds[kWriteBufferSize];
    SkPoint                            fPositions[kWriteBufferSize];
};

////////////////////////////////////////////////////////////////////////////////////////////////////

GrStencilAndCoverTextContext::TextRun::TextRun(const SkPaint& fontAndStroke)
    : fStyle(fontAndStroke)
    , fFont(fontAndStroke)
    , fTotalGlyphCount(0)
    , fFallbackGlyphCount(0)
    , fDetachedGlyphCache(nullptr)
    , fLastDrawnGlyphsID(SK_InvalidUniqueID) {
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
        SkScalar extra = fFont.getTextSize() * fakeBoldScale;

        SkStrokeRec strokeRec(SkStrokeRec::kFill_InitStyle);
        strokeRec.setStrokeStyle(stroke.needToApply() ? stroke.getWidth() + extra : extra,
                                 true /*strokeAndFill*/);
        fStyle = GrStyle(strokeRec, fStyle.refPathEffect());
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
            fStyle = GrStyle(strokeRec, fStyle.refPathEffect());
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
    SkPaint::GlyphCacheProc glyphCacheProc = SkPaint::GetGlyphCacheProc(fFont.getTextEncoding(),
                                                                        fFont.isDevKernText(),
                                                                        true);

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

    fFallbackTextBlob = fallback.makeIfNeeded(&fFallbackGlyphCount);
}

void GrStencilAndCoverTextContext::TextRun::setPosText(const char text[], size_t byteLength,
                                                       const SkScalar pos[], int scalarsPerPosition,
                                                       const SkPoint& offset) {
    SkASSERT(byteLength == 0 || text != nullptr);
    SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);

    SkGlyphCache* glyphCache = this->getGlyphCache();
    SkPaint::GlyphCacheProc glyphCacheProc = SkPaint::GetGlyphCacheProc(fFont.getTextEncoding(),
                                                                        fFont.isDevKernText(),
                                                                        true);

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

    fFallbackTextBlob = fallback.makeIfNeeded(&fFallbackGlyphCount);
}

GrPathRange* GrStencilAndCoverTextContext::TextRun::createGlyphs(
                                                    GrResourceProvider* resourceProvider) const {
    GrPathRange* glyphs = static_cast<GrPathRange*>(
            resourceProvider->findAndRefResourceByUniqueKey(fGlyphPathsKey));
    if (nullptr == glyphs) {
        if (fUsingRawGlyphPaths) {
            SkScalerContextEffects noeffects;
            glyphs = resourceProvider->createGlyphs(fFont.getTypeface(), noeffects,
                                                    nullptr, fStyle);
        } else {
            SkGlyphCache* cache = this->getGlyphCache();
            glyphs = resourceProvider->createGlyphs(cache->getScalerContext()->getTypeface(),
                                                    cache->getScalerContext()->getEffects(),
                                                    &cache->getDescriptor(),
                                                    fStyle);
        }
        resourceProvider->assignUniqueKeyToResource(fGlyphPathsKey, glyphs);
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
                                                 GrRenderTargetContext* renderTargetContext,
                                                 const GrClip& clip, const SkMatrix& viewMatrix,
                                                 const SkSurfaceProps& props, SkScalar x,
                                                 SkScalar y, const SkIRect& clipBounds,
                                                 GrAtlasTextContext* fallbackTextContext,
                                                 const SkPaint& originalSkPaint) const {
    SkASSERT(fInstanceData);

    if (fInstanceData->count()) {
        static constexpr GrUserStencilSettings kCoverPass(
            GrUserStencilSettings::StaticInit<
                0x0000,
                GrUserStencilTest::kNotEqual, // Stencil pass accounts for clip.
                0xffff,
                GrUserStencilOp::kZero,
                GrUserStencilOp::kKeep,
                0xffff>()
        );

        sk_sp<GrPathRange> glyphs(this->createGlyphs(ctx->resourceProvider()));
        if (fLastDrawnGlyphsID != glyphs->uniqueID()) {
            // Either this is the first draw or the glyphs object was purged since last draw.
            glyphs->loadPathsIfNeeded(fInstanceData->indices(), fInstanceData->count());
            fLastDrawnGlyphsID = glyphs->uniqueID();
        }

        GrPaint grPaint;
        if (!SkPaintToGrPaint(ctx, renderTargetContext, originalSkPaint, viewMatrix, &grPaint)) {
            return;
        }

        // Don't compute a bounding box. For dst copy texture, we'll opt instead for it to just copy
        // the entire dst. Realistically this is a moot point, because any context that supports
        // NV_path_rendering will also support NV_blend_equation_advanced.
        // For clipping we'll just skip any optimizations based on the bounds. This does, however,
        // hurt GrOp combining.
        const SkRect bounds = SkRect::MakeIWH(renderTargetContext->width(),
                                              renderTargetContext->height());

        // The run's "font" overrides the anti-aliasing of the passed in SkPaint!
        GrAAType aaType;
        if (this->aa() == GrAA::kYes) {
            SkASSERT(renderTargetContext->isStencilBufferMultisampled());
            aaType = renderTargetContext->isUnifiedMultisampled() ? GrAAType::kMSAA
                                                                  : GrAAType::kMixedSamples;
        } else {
            aaType = GrAAType::kNone;
        }

        std::unique_ptr<GrDrawOp> op = GrDrawPathRangeOp::Make(
                viewMatrix, fTextRatio, fTextInverseRatio * x, fTextInverseRatio * y,
                std::move(grPaint), GrPathRendering::kWinding_FillType, aaType, glyphs.get(),
                fInstanceData.get(), bounds);

        renderTargetContext->addDrawOp(clip, std::move(op));
    }

    if (fFallbackTextBlob) {
        SkPaint fallbackSkPaint(originalSkPaint);
        fStyle.strokeRec().applyToPaint(&fallbackSkPaint);
        if (!fStyle.isSimpleFill()) {
            fallbackSkPaint.setStrokeWidth(fStyle.strokeRec().getWidth() * fTextRatio);
        }

        fallbackTextContext->drawTextBlob(ctx, renderTargetContext, clip, fallbackSkPaint,
                                          viewMatrix, props, fFallbackTextBlob.get(), x, y, nullptr,
                                          clipBounds);
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

sk_sp<SkTextBlob> GrStencilAndCoverTextContext::FallbackBlobBuilder::makeIfNeeded(int *count) {
    *count = fCount;
    if (fCount) {
        this->flush();
        return fBuilder->make();
    }
    return nullptr;
}
