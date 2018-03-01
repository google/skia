/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAtlasTextBlob.h"
#include "GrBlurUtils.h"
#include "GrClip.h"
#include "GrContext.h"
#include "GrTextUtils.h"
#include "SkColorFilter.h"
#include "SkDrawFilter.h"
#include "SkGlyphCache.h"
#include "SkMaskFilterBase.h"
#include "SkTextBlobRunIterator.h"
#include "SkTextToPathIter.h"
#include "ops/GrAtlasTextOp.h"

sk_sp<GrAtlasTextBlob> GrAtlasTextBlob::Make(GrMemoryPool* pool, int glyphCount, int runCount) {
    // We allocate size for the GrAtlasTextBlob itself, plus size for the vertices array,
    // and size for the glyphIds array.
    size_t verticesCount = glyphCount * kVerticesPerGlyph * kMaxVASize;
    size_t size = sizeof(GrAtlasTextBlob) +
                  verticesCount +
                  glyphCount * sizeof(GrGlyph**) +
                  sizeof(GrAtlasTextBlob::Run) * runCount;

    void* allocation;
    if (pool) {
        allocation = pool->allocate(size);
    } else {
        allocation = ::operator new (size);
    }
    if (CACHE_SANITY_CHECK) {
        sk_bzero(allocation, size);
    }

    sk_sp<GrAtlasTextBlob> cacheBlob(new (allocation) GrAtlasTextBlob);
    cacheBlob->fSize = size;

    // setup offsets for vertices / glyphs
    cacheBlob->fVertices = sizeof(GrAtlasTextBlob) + reinterpret_cast<char*>(cacheBlob.get());
    cacheBlob->fGlyphs = reinterpret_cast<GrGlyph**>(cacheBlob->fVertices + verticesCount);
    cacheBlob->fRuns = reinterpret_cast<GrAtlasTextBlob::Run*>(cacheBlob->fGlyphs + glyphCount);

    // Initialize runs
    for (int i = 0; i < runCount; i++) {
        new (&cacheBlob->fRuns[i]) GrAtlasTextBlob::Run;
    }
    cacheBlob->fRunCount = runCount;
    cacheBlob->fPool = pool;
    return cacheBlob;
}

SkGlyphCache* GrAtlasTextBlob::setupCache(int runIndex,
                                          const SkSurfaceProps& props,
                                          SkScalerContextFlags scalerContextFlags,
                                          const SkPaint& skPaint,
                                          const SkMatrix* viewMatrix) {
    GrAtlasTextBlob::Run* run = &fRuns[runIndex];

    // if we have an override descriptor for the run, then we should use that
    SkAutoDescriptor* desc = run->fOverrideDescriptor.get() ? run->fOverrideDescriptor.get() :
                                                              &run->fDescriptor;
    SkScalerContextEffects effects;
    SkScalerContext::CreateDescriptorAndEffectsUsingPaint(
        skPaint, &props, scalerContextFlags, viewMatrix, desc, &effects);
    run->fTypeface.reset(SkSafeRef(skPaint.getTypeface()));
    run->fPathEffect = sk_ref_sp(effects.fPathEffect);
    run->fMaskFilter = sk_ref_sp(effects.fMaskFilter);
    return SkGlyphCache::DetachCache(run->fTypeface.get(), effects, desc->getDesc());
}

void GrAtlasTextBlob::appendGlyph(int runIndex,
                                  const SkRect& positions,
                                  GrColor color,
                                  sk_sp<GrTextStrike> strike,
                                  GrGlyph* glyph,
                                  SkGlyphCache* cache, const SkGlyph& skGlyph,
                                  SkScalar x, SkScalar y, SkScalar scale, bool preTransformed) {
    if (positions.isEmpty()) {
        return;
    }

    // If the glyph is too large we fall back to paths
    if (glyph->fTooLargeForAtlas) {
        if (nullptr == glyph->fPath) {
            const SkPath* glyphPath = cache->findPath(skGlyph);
            if (!glyphPath) {
                return;
            }

            glyph->fPath = new SkPath(*glyphPath);
        }
        this->appendPathGlyph(runIndex, *glyph->fPath, x, y, scale, preTransformed);
        return;
    }

    Run& run = fRuns[runIndex];
    GrMaskFormat format = glyph->fMaskFormat;

    Run::SubRunInfo* subRun = &run.fSubRunInfo.back();
    if (run.fInitialized && subRun->maskFormat() != format) {
        subRun = &run.push_back();
        subRun->setStrike(std::move(strike));
    } else if (!run.fInitialized) {
        subRun->setStrike(std::move(strike));
    }

    run.fInitialized = true;

    bool hasW = subRun->hasWCoord();
    // DF glyphs drawn in perspective must always have a w coord.
    SkASSERT(hasW || !subRun->drawAsDistanceFields() || !fInitialViewMatrix.hasPerspective());
    // Non-DF glyphs should never have a w coord.
    SkASSERT(!hasW || subRun->drawAsDistanceFields());

    size_t vertexStride = GetVertexStride(format, hasW);

    subRun->setMaskFormat(format);

    subRun->joinGlyphBounds(positions);
    subRun->setColor(color);

    intptr_t vertex = reinterpret_cast<intptr_t>(this->fVertices + subRun->vertexEndIndex());

    // We always write the third position component used by SDFs. If it is unused it gets
    // overwritten. Similarly, we always write the color and the blob will later overwrite it
    // with texture coords if it is unused.
    size_t colorOffset = hasW ? sizeof(SkPoint3) : sizeof(SkPoint);
    // V0
    *reinterpret_cast<SkPoint3*>(vertex) = {positions.fLeft, positions.fTop, 1.f};
    *reinterpret_cast<GrColor*>(vertex + colorOffset) = color;
    vertex += vertexStride;

    // V1
    *reinterpret_cast<SkPoint3*>(vertex) = {positions.fLeft, positions.fBottom, 1.f};
    *reinterpret_cast<GrColor*>(vertex + colorOffset) = color;
    vertex += vertexStride;

    // V2
    *reinterpret_cast<SkPoint3*>(vertex) = {positions.fRight, positions.fTop, 1.f};
    *reinterpret_cast<GrColor*>(vertex + colorOffset) = color;
    vertex += vertexStride;

    // V3
    *reinterpret_cast<SkPoint3*>(vertex) = {positions.fRight, positions.fBottom, 1.f};
    *reinterpret_cast<GrColor*>(vertex + colorOffset) = color;

    subRun->appendVertices(vertexStride);
    fGlyphs[subRun->glyphEndIndex()] = glyph;
    subRun->glyphAppended();
}

void GrAtlasTextBlob::appendPathGlyph(int runIndex, const SkPath& path, SkScalar x, SkScalar y,
                                      SkScalar scale, bool preTransformed) {
    Run& run = fRuns[runIndex];
    run.fPathGlyphs.push_back(GrAtlasTextBlob::Run::PathGlyph(path, x, y, scale, preTransformed));
}

bool GrAtlasTextBlob::mustRegenerate(const GrTextUtils::Paint& paint,
                                     const SkMaskFilterBase::BlurRec& blurRec,
                                     const SkMatrix& viewMatrix, SkScalar x, SkScalar y) {
    // If we have LCD text then our canonical color will be set to transparent, in this case we have
    // to regenerate the blob on any color change
    // We use the grPaint to get any color filter effects
    if (fKey.fCanonicalColor == SK_ColorTRANSPARENT &&
        fLuminanceColor != paint.luminanceColor()) {
        return true;
    }

    if (fInitialViewMatrix.hasPerspective() != viewMatrix.hasPerspective()) {
        return true;
    }

    /** This could be relaxed for blobs with only distance field glyphs. */
    if (fInitialViewMatrix.hasPerspective() && !fInitialViewMatrix.cheapEqualTo(viewMatrix)) {
        return true;
    }

    // We only cache one masked version
    if (fKey.fHasBlur &&
        (fBlurRec.fSigma != blurRec.fSigma ||
         fBlurRec.fStyle != blurRec.fStyle ||
         fBlurRec.fQuality != blurRec.fQuality)) {
        return true;
    }

    // Similarly, we only cache one version for each style
    if (fKey.fStyle != SkPaint::kFill_Style &&
        (fStrokeInfo.fFrameWidth != paint.skPaint().getStrokeWidth() ||
         fStrokeInfo.fMiterLimit != paint.skPaint().getStrokeMiter() ||
         fStrokeInfo.fJoin != paint.skPaint().getStrokeJoin())) {
        return true;
    }

    // Mixed blobs must be regenerated.  We could probably figure out a way to do integer scrolls
    // for mixed blobs if this becomes an issue.
    if (this->hasBitmap() && this->hasDistanceField()) {
        // Identical viewmatrices and we can reuse in all cases
        if (fInitialViewMatrix.cheapEqualTo(viewMatrix) && x == fInitialX && y == fInitialY) {
            return false;
        }
        return true;
    }

    if (this->hasBitmap()) {
        if (fInitialViewMatrix.getScaleX() != viewMatrix.getScaleX() ||
            fInitialViewMatrix.getScaleY() != viewMatrix.getScaleY() ||
            fInitialViewMatrix.getSkewX() != viewMatrix.getSkewX() ||
            fInitialViewMatrix.getSkewY() != viewMatrix.getSkewY()) {
            return true;
        }

        // We can update the positions in the cachedtextblobs without regenerating the whole blob,
        // but only for integer translations.
        // This cool bit of math will determine the necessary translation to apply to the already
        // generated vertex coordinates to move them to the correct position
        SkScalar transX = viewMatrix.getTranslateX() +
                          viewMatrix.getScaleX() * (x - fInitialX) +
                          viewMatrix.getSkewX() * (y - fInitialY) -
                          fInitialViewMatrix.getTranslateX();
        SkScalar transY = viewMatrix.getTranslateY() +
                          viewMatrix.getSkewY() * (x - fInitialX) +
                          viewMatrix.getScaleY() * (y - fInitialY) -
                          fInitialViewMatrix.getTranslateY();
        if (!SkScalarIsInt(transX) || !SkScalarIsInt(transY)) {
            return true;
        }
    } else if (this->hasDistanceField()) {
        // A scale outside of [blob.fMaxMinScale, blob.fMinMaxScale] would result in a different
        // distance field being generated, so we have to regenerate in those cases
        SkScalar newMaxScale = viewMatrix.getMaxScale();
        SkScalar oldMaxScale = fInitialViewMatrix.getMaxScale();
        SkScalar scaleAdjust = newMaxScale / oldMaxScale;
        if (scaleAdjust < fMaxMinScale || scaleAdjust > fMinMaxScale) {
            return true;
        }
    }

    // It is possible that a blob has neither distanceField nor bitmaptext.  This is in the case
    // when all of the runs inside the blob are drawn as paths.  In this case, we always regenerate
    // the blob anyways at flush time, so no need to regenerate explicitly
    return false;
}

inline std::unique_ptr<GrAtlasTextOp> GrAtlasTextBlob::makeOp(
        const Run::SubRunInfo& info, int glyphCount, uint16_t run, uint16_t subRun,
        const SkMatrix& viewMatrix, SkScalar x, SkScalar y, const SkIRect& clipRect,
        const GrTextUtils::Paint& paint, const SkSurfaceProps& props,
        const GrDistanceFieldAdjustTable* distanceAdjustTable,
        GrRestrictedAtlasManager* restrictedAtlasManager, GrTextUtils::Target* target) {
    GrMaskFormat format = info.maskFormat();

    GrPaint grPaint;
    target->makeGrPaint(info.maskFormat(), paint, viewMatrix, &grPaint);
    std::unique_ptr<GrAtlasTextOp> op;
    if (info.drawAsDistanceFields()) {
        bool useBGR = SkPixelGeometryIsBGR(props.pixelGeometry());
        op = GrAtlasTextOp::MakeDistanceField(
                std::move(grPaint), glyphCount, restrictedAtlasManager, distanceAdjustTable,
                target->colorSpaceInfo().isGammaCorrect(), paint.luminanceColor(),
                info.hasUseLCDText(), useBGR, info.isAntiAliased());
    } else {
        op = GrAtlasTextOp::MakeBitmap(std::move(grPaint), format,
                                       glyphCount, restrictedAtlasManager);
    }
    GrAtlasTextOp::Geometry& geometry = op->geometry();
    geometry.fViewMatrix = viewMatrix;
    geometry.fClipRect = clipRect;
    geometry.fBlob = SkRef(this);
    geometry.fRun = run;
    geometry.fSubRun = subRun;
    geometry.fColor =
            info.maskFormat() == kARGB_GrMaskFormat ? GrColor_WHITE : paint.filteredPremulColor();
    geometry.fX = x;
    geometry.fY = y;
    op->init();
    return op;
}

static void calculate_translation(bool applyVM,
                                  const SkMatrix& newViewMatrix, SkScalar newX, SkScalar newY,
                                  const SkMatrix& currentViewMatrix, SkScalar currentX,
                                  SkScalar currentY, SkScalar* transX, SkScalar* transY) {
    if (applyVM) {
        *transX = newViewMatrix.getTranslateX() +
                  newViewMatrix.getScaleX() * (newX - currentX) +
                  newViewMatrix.getSkewX() * (newY - currentY) -
                  currentViewMatrix.getTranslateX();

        *transY = newViewMatrix.getTranslateY() +
                  newViewMatrix.getSkewY() * (newX - currentX) +
                  newViewMatrix.getScaleY() * (newY - currentY) -
                  currentViewMatrix.getTranslateY();
    } else {
        *transX = newX - currentX;
        *transY = newY - currentY;
    }
}

void GrAtlasTextBlob::flush(GrRestrictedAtlasManager* restrictedAtlasManager,
                            GrTextUtils::Target* target, const SkSurfaceProps& props,
                            const GrDistanceFieldAdjustTable* distanceAdjustTable,
                            const GrTextUtils::Paint& paint, const GrClip& clip,
                            const SkMatrix& viewMatrix, const SkIRect& clipBounds,
                            SkScalar x, SkScalar y) {

    // GrAtlasTextBlob::makeOp only takes uint16_t values for run and subRun indices.
    // Encountering something larger than this is highly unlikely, so we'll just not draw it.
    int lastRun = SkTMin(fRunCount, (1 << 16)) - 1;
    GrTextUtils::RunPaint runPaint(&paint, nullptr, props);
    for (int runIndex = 0; runIndex <= lastRun; runIndex++) {
        Run& run = fRuns[runIndex];

        // first flush any path glyphs
        if (run.fPathGlyphs.count()) {
            SkScalar transX, transY;
            uint16_t paintFlags = run.fPaintFlags;
            if (!runPaint.modifyForRun(
                [paintFlags](SkPaint* p) {
                    p->setFlags((p->getFlags() & ~Run::kPaintFlagsMask) | paintFlags);
                })) {
                continue;
            }
            for (int i = 0; i < run.fPathGlyphs.count(); i++) {
                GrAtlasTextBlob::Run::PathGlyph& pathGlyph = run.fPathGlyphs[i];
                calculate_translation(pathGlyph.fPreTransformed, viewMatrix, x, y,
                                      fInitialViewMatrix, fInitialX, fInitialY, &transX, &transY);
                const SkMatrix& ctm = pathGlyph.fPreTransformed ? SkMatrix::I() : viewMatrix;
                SkMatrix pathMatrix;
                pathMatrix.setScale(pathGlyph.fScale, pathGlyph.fScale);
                pathMatrix.postTranslate(pathGlyph.fX + transX, pathGlyph.fY + transY);
                target->drawPath(clip, pathGlyph.fPath, runPaint, ctm, &pathMatrix, clipBounds);
            }
        }

        // then flush each subrun, if any
        if (!run.fInitialized) {
            continue;
        }
        int lastSubRun = SkTMin(run.fSubRunInfo.count(), 1 << 16) - 1;
        for (int subRun = 0; subRun <= lastSubRun; subRun++) {
            const Run::SubRunInfo& info = run.fSubRunInfo[subRun];
            int glyphCount = info.glyphCount();
            if (0 == glyphCount) {
                continue;
            }

            bool skipClip = false;
            bool submitOp = true;
            SkIRect clipRect = SkIRect::MakeEmpty();
            SkRect rtBounds = SkRect::MakeWH(target->width(), target->height());
            SkRRect clipRRect;
            GrAA aa;
            // We can clip geometrically if we're not using SDFs,
            // and we have an axis-aligned rectangular non-AA clip
            if (!info.drawAsDistanceFields() && clip.isRRect(rtBounds, &clipRRect, &aa) &&
                clipRRect.isRect() && GrAA::kNo == aa) {
                skipClip = true;
                // We only need to do clipping work if the subrun isn't contained by the clip
                SkRect subRunBounds;
                this->computeSubRunBounds(&subRunBounds, runIndex, subRun, viewMatrix, x, y);
                if (!clipRRect.getBounds().contains(subRunBounds)) {
                    // If the subrun is completely outside, don't add an op for it
                    if (!clipRRect.getBounds().intersects(subRunBounds)) {
                        submitOp = false;
                    }
                    else {
                        clipRRect.getBounds().round(&clipRect);
                    }
                }
            }

            if (submitOp) {
                auto op = this->makeOp(info, glyphCount, runIndex, subRun, viewMatrix, x, y,
                                       clipRect, std::move(paint), props, distanceAdjustTable,
                                       restrictedAtlasManager, target);
                if (op) {
                    if (skipClip) {
                        target->addDrawOp(GrNoClip(), std::move(op));
                    }
                    else {
                        target->addDrawOp(clip, std::move(op));
                    }
                }
            }
        }

    }
}

std::unique_ptr<GrDrawOp> GrAtlasTextBlob::test_makeOp(
        int glyphCount, uint16_t run, uint16_t subRun, const SkMatrix& viewMatrix,
        SkScalar x, SkScalar y, const GrTextUtils::Paint& paint, const SkSurfaceProps& props,
        const GrDistanceFieldAdjustTable* distanceAdjustTable,
        GrRestrictedAtlasManager* restrictedAtlasManager, GrTextUtils::Target* target) {
    const GrAtlasTextBlob::Run::SubRunInfo& info = fRuns[run].fSubRunInfo[subRun];
    SkIRect emptyRect = SkIRect::MakeEmpty();
    return this->makeOp(info, glyphCount, run, subRun, viewMatrix, x, y, emptyRect, paint, props,
                        distanceAdjustTable, restrictedAtlasManager, target);
}

void GrAtlasTextBlob::AssertEqual(const GrAtlasTextBlob& l, const GrAtlasTextBlob& r) {
    SkASSERT_RELEASE(l.fSize == r.fSize);
    SkASSERT_RELEASE(l.fPool == r.fPool);

    SkASSERT_RELEASE(l.fBlurRec.fSigma == r.fBlurRec.fSigma);
    SkASSERT_RELEASE(l.fBlurRec.fStyle == r.fBlurRec.fStyle);
    SkASSERT_RELEASE(l.fBlurRec.fQuality == r.fBlurRec.fQuality);

    SkASSERT_RELEASE(l.fStrokeInfo.fFrameWidth == r.fStrokeInfo.fFrameWidth);
    SkASSERT_RELEASE(l.fStrokeInfo.fMiterLimit == r.fStrokeInfo.fMiterLimit);
    SkASSERT_RELEASE(l.fStrokeInfo.fJoin == r.fStrokeInfo.fJoin);

    SkASSERT_RELEASE(l.fKey == r.fKey);
    //SkASSERT_RELEASE(l.fPaintColor == r.fPaintColor); // Colors might not actually be identical
    SkASSERT_RELEASE(l.fMaxMinScale == r.fMaxMinScale);
    SkASSERT_RELEASE(l.fMinMaxScale == r.fMinMaxScale);
    SkASSERT_RELEASE(l.fTextType == r.fTextType);

    SkASSERT_RELEASE(l.fRunCount == r.fRunCount);
    for (int i = 0; i < l.fRunCount; i++) {
        const Run& lRun = l.fRuns[i];
        const Run& rRun = r.fRuns[i];

        if (lRun.fTypeface.get()) {
            SkASSERT_RELEASE(rRun.fTypeface.get());
            SkASSERT_RELEASE(SkTypeface::Equal(lRun.fTypeface.get(), rRun.fTypeface.get()));
        } else {
            SkASSERT_RELEASE(!rRun.fTypeface.get());
        }


        SkASSERT_RELEASE(lRun.fDescriptor.getDesc());
        SkASSERT_RELEASE(rRun.fDescriptor.getDesc());
        SkASSERT_RELEASE(*lRun.fDescriptor.getDesc() == *rRun.fDescriptor.getDesc());

        if (lRun.fOverrideDescriptor.get()) {
            SkASSERT_RELEASE(lRun.fOverrideDescriptor->getDesc());
            SkASSERT_RELEASE(rRun.fOverrideDescriptor.get() && rRun.fOverrideDescriptor->getDesc());
            SkASSERT_RELEASE(*lRun.fOverrideDescriptor->getDesc() ==
                             *rRun.fOverrideDescriptor->getDesc());
        } else {
            SkASSERT_RELEASE(!rRun.fOverrideDescriptor.get());
        }

        // color can be changed
        //SkASSERT(lRun.fColor == rRun.fColor);
        SkASSERT_RELEASE(lRun.fInitialized == rRun.fInitialized);

        SkASSERT_RELEASE(lRun.fSubRunInfo.count() == rRun.fSubRunInfo.count());
        for(int j = 0; j < lRun.fSubRunInfo.count(); j++) {
            const Run::SubRunInfo& lSubRun = lRun.fSubRunInfo[j];
            const Run::SubRunInfo& rSubRun = rRun.fSubRunInfo[j];

            // TODO we can do this check, but we have to apply the VM to the old vertex bounds
            //SkASSERT_RELEASE(lSubRun.vertexBounds() == rSubRun.vertexBounds());

            if (lSubRun.strike()) {
                SkASSERT_RELEASE(rSubRun.strike());
                SkASSERT_RELEASE(GrTextStrike::GetKey(*lSubRun.strike()) ==
                                 GrTextStrike::GetKey(*rSubRun.strike()));

            } else {
                SkASSERT_RELEASE(!rSubRun.strike());
            }

            SkASSERT_RELEASE(lSubRun.vertexStartIndex() == rSubRun.vertexStartIndex());
            SkASSERT_RELEASE(lSubRun.vertexEndIndex() == rSubRun.vertexEndIndex());
            SkASSERT_RELEASE(lSubRun.glyphStartIndex() == rSubRun.glyphStartIndex());
            SkASSERT_RELEASE(lSubRun.glyphEndIndex() == rSubRun.glyphEndIndex());
            SkASSERT_RELEASE(lSubRun.maskFormat() == rSubRun.maskFormat());
            SkASSERT_RELEASE(lSubRun.drawAsDistanceFields() == rSubRun.drawAsDistanceFields());
            SkASSERT_RELEASE(lSubRun.hasUseLCDText() == rSubRun.hasUseLCDText());
        }

        SkASSERT_RELEASE(lRun.fPathGlyphs.count() == rRun.fPathGlyphs.count());
        for (int i = 0; i < lRun.fPathGlyphs.count(); i++) {
            const Run::PathGlyph& lPathGlyph = lRun.fPathGlyphs[i];
            const Run::PathGlyph& rPathGlyph = rRun.fPathGlyphs[i];

            SkASSERT_RELEASE(lPathGlyph.fPath == rPathGlyph.fPath);
            // We can't assert that these have the same translations
        }
    }
}

void GrAtlasTextBlob::Run::SubRunInfo::computeTranslation(const SkMatrix& viewMatrix,
                                                          SkScalar x, SkScalar y, SkScalar* transX,
                                                          SkScalar* transY) {
    calculate_translation(!this->drawAsDistanceFields(), viewMatrix, x, y,
                          fCurrentViewMatrix, fX, fY, transX, transY);
    fCurrentViewMatrix = viewMatrix;
    fX = x;
    fY = y;
}
