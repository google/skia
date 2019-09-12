/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextBlob.h"
#include "GrBlurUtils.h"
#include "GrClip.h"
#include "GrContext.h"
#include "GrShape.h"
#include "GrStyle.h"
#include "GrTextTarget.h"
#include "SkColorFilter.h"
#include "SkMaskFilterBase.h"
#include "SkPaintPriv.h"
#include "ops/GrAtlasTextOp.h"

#include <new>

template <size_t N> static size_t sk_align(size_t s) {
    return ((s + (N-1)) / N) * N;
}

sk_sp<GrTextBlob> GrTextBlob::Make(int glyphCount,
                                   int runCount,
                                   GrColor color,
                                   GrStrikeCache* strikeCache) {
    // We allocate size for the GrTextBlob itself, plus size for the vertices array,
    // and size for the glyphIds array.
    size_t verticesCount = glyphCount * kVerticesPerGlyph * kMaxVASize;

    size_t blobStart = 0;
    size_t vertex = sk_align<alignof(char)>           (blobStart + sizeof(GrTextBlob) * 1);
    size_t glyphs = sk_align<alignof(GrGlyph*)>       (vertex + sizeof(char) * verticesCount);
    size_t   runs = sk_align<alignof(GrTextBlob::Run)>(glyphs + sizeof(GrGlyph*) * glyphCount);
    size_t   size =                                   (runs + sizeof(GrTextBlob::Run) * runCount);

    void* allocation = ::operator new (size);

    if (CACHE_SANITY_CHECK) {
        sk_bzero(allocation, size);
    }

    sk_sp<GrTextBlob> blob{new (allocation) GrTextBlob{strikeCache}};
    blob->fSize = size;

    // setup offsets for vertices / glyphs
    blob->fVertices = SkTAddOffset<char>(blob.get(), vertex);
    blob->fGlyphs = SkTAddOffset<GrGlyph*>(blob.get(), glyphs);
    blob->fRuns = SkTAddOffset<GrTextBlob::Run>(blob.get(), runs);

    // Initialize runs
    for (int i = 0; i < runCount; i++) {
        new (&blob->fRuns[i]) GrTextBlob::Run{blob.get(), color};
    }
    blob->fRunCountLimit = runCount;
    return blob;
}

void GrTextBlob::Run::setupFont(const SkStrikeSpec& strikeSpec) {
    fTypeface = sk_ref_sp(&strikeSpec.typeface());
    fPathEffect = sk_ref_sp(strikeSpec.effects().fPathEffect);
    fMaskFilter = sk_ref_sp(strikeSpec.effects().fMaskFilter);
    // if we have an override descriptor for the run, then we should use that
    SkAutoDescriptor* desc =
            fARGBFallbackDescriptor.get() ? fARGBFallbackDescriptor.get() : &fDescriptor;
    // Set up the descriptor for possible cache lookups during regen.
    desc->reset(strikeSpec.desc());
}

void GrTextBlob::Run::appendPathGlyph(const SkPath& path, SkPoint position,
                                      SkScalar scale, bool preTransformed) {
    fPathGlyphs.push_back(PathGlyph(path, position.x(), position.y(), scale, preTransformed));
}

bool GrTextBlob::mustRegenerate(const SkPaint& paint, bool anyRunHasSubpixelPosition,
                                const SkMaskFilterBase::BlurRec& blurRec,
                                const SkMatrix& viewMatrix, SkScalar x, SkScalar y) {
    // If we have LCD text then our canonical color will be set to transparent, in this case we have
    // to regenerate the blob on any color change
    // We use the grPaint to get any color filter effects
    if (fKey.fCanonicalColor == SK_ColorTRANSPARENT &&
        fLuminanceColor != SkPaintPriv::ComputeLuminanceColor(paint)) {
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
        (fBlurRec.fSigma != blurRec.fSigma || fBlurRec.fStyle != blurRec.fStyle)) {
        return true;
    }

    // Similarly, we only cache one version for each style
    if (fKey.fStyle != SkPaint::kFill_Style &&
        (fStrokeInfo.fFrameWidth != paint.getStrokeWidth() ||
         fStrokeInfo.fMiterLimit != paint.getStrokeMiter() ||
         fStrokeInfo.fJoin != paint.getStrokeJoin())) {
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

        // TODO(herb): this is not needed for full pixel glyph choice, but is needed to adjust
        //  the quads properly. Devise a system that regenerates the quads from original data
        //  using the transform to allow this to be used in general.

        // We can update the positions in the text blob without regenerating the whole
        // blob, but only for integer translations.
        // This cool bit of math will determine the necessary translation to apply to the
        // already generated vertex coordinates to move them to the correct position.
        // Figure out the translation in view space given a translation in source space.
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

inline std::unique_ptr<GrAtlasTextOp> GrTextBlob::makeOp(
        const SubRun& info, int glyphCount, uint16_t run, uint16_t subRun,
        const SkMatrix& viewMatrix, SkScalar x, SkScalar y, const SkIRect& clipRect,
        const SkPaint& paint, const SkPMColor4f& filteredColor, const SkSurfaceProps& props,
        const GrDistanceFieldAdjustTable* distanceAdjustTable, GrTextTarget* target) {
    GrMaskFormat format = info.maskFormat();

    GrPaint grPaint;
    target->makeGrPaint(info.maskFormat(), paint, viewMatrix, &grPaint);
    std::unique_ptr<GrAtlasTextOp> op;
    if (info.drawAsDistanceFields()) {
        // TODO: Can we be even smarter based on the dest transfer function?
        op = GrAtlasTextOp::MakeDistanceField(
                target->getContext(), std::move(grPaint), glyphCount, distanceAdjustTable,
                target->colorSpaceInfo().isLinearlyBlended(),
                SkPaintPriv::ComputeLuminanceColor(paint),
                props, info.isAntiAliased(), info.hasUseLCDText());
    } else {
        op = GrAtlasTextOp::MakeBitmap(target->getContext(), std::move(grPaint), format, glyphCount,
                                       info.needsTransform());
    }
    GrAtlasTextOp::Geometry& geometry = op->geometry();
    geometry.fViewMatrix = viewMatrix;
    geometry.fClipRect = clipRect;
    geometry.fBlob = SkRef(this);
    geometry.fRun = run;
    geometry.fSubRun = subRun;
    geometry.fColor = info.maskFormat() == kARGB_GrMaskFormat ? SK_PMColor4fWHITE : filteredColor;
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

void GrTextBlob::flush(GrTextTarget* target, const SkSurfaceProps& props,
                       const GrDistanceFieldAdjustTable* distanceAdjustTable,
                       const SkPaint& paint, const SkPMColor4f& filteredColor, const GrClip& clip,
                       const SkMatrix& viewMatrix, SkScalar x, SkScalar y) {

    // GrTextBlob::makeOp only takes uint16_t values for run and subRun indices.
    // Encountering something larger than this is highly unlikely, so we'll just not draw it.
    int lastRun = SkTMin(fRunCountLimit, (1 << 16)) - 1;
    // For each run in the GrTextBlob we're going to churn through all the glyphs.
    // Each run is broken into a path part and a Mask / DFT / ARGB part.
    for (int runIndex = 0; runIndex <= lastRun; runIndex++) {

        Run& run = fRuns[runIndex];

        // first flush any path glyphs
        if (run.fPathGlyphs.count()) {
            SkPaint runPaint{paint};
            runPaint.setAntiAlias(run.fAntiAlias);

            for (int i = 0; i < run.fPathGlyphs.count(); i++) {
                GrTextBlob::Run::PathGlyph& pathGlyph = run.fPathGlyphs[i];

                SkMatrix ctm;
                const SkPath* path = &pathGlyph.fPath;

                // TmpPath must be in the same scope as GrShape shape below.
                SkTLazy<SkPath> tmpPath;

                // The glyph positions and glyph outlines are either in device space or in source
                // space based on fPreTransformed.
                if (!pathGlyph.fPreTransformed) {
                    // Positions and outlines are in source space.

                    ctm = viewMatrix;

                    SkMatrix pathMatrix = SkMatrix::MakeScale(pathGlyph.fScale, pathGlyph.fScale);

                    // The origin for the blob may have changed, so figure out the delta.
                    SkVector originShift = SkPoint{x, y} - SkPoint{fInitialX, fInitialY};

                    // Shift the original glyph location in source space to the position of the new
                    // blob.
                    pathMatrix.postTranslate(originShift.x() + pathGlyph.fX,
                                             originShift.y() + pathGlyph.fY);

                    // If there are shaders, blurs or styles, the path must be scaled into source
                    // space independently of the CTM. This allows the CTM to be correct for the
                    // different effects.
                    GrStyle style(runPaint);
                    bool scalePath = runPaint.getShader()
                                     || style.applies()
                                     || runPaint.getMaskFilter();
                    if (!scalePath) {
                        // Scale can be applied to CTM -- no effects.

                        ctm.preConcat(pathMatrix);
                    } else {
                        // Scale the outline into source space.

                        // Transform the path form the normalized outline to source space. This
                        // way the CTM will remain the same so it can be used by the effects.
                        SkPath* sourceOutline = tmpPath.init();
                        path->transform(pathMatrix, sourceOutline);
                        sourceOutline->setIsVolatile(true);
                        path = sourceOutline;
                    }


                } else {
                    // Positions and outlines are in device space.

                    SkPoint originalOrigin = {fInitialX, fInitialY};
                    fInitialViewMatrix.mapPoints(&originalOrigin, 1);

                    SkPoint newOrigin = {x, y};
                    viewMatrix.mapPoints(&newOrigin, 1);

                    // The origin shift in device space.
                    SkPoint originShift = newOrigin - originalOrigin;

                    // Shift the original glyph location in device space to the position of the
                    // new blob.
                    ctm = SkMatrix::MakeTrans(originShift.x() + pathGlyph.fX,
                                              originShift.y() + pathGlyph.fY);
                }

                // TODO: we are losing the mutability of the path here
                GrShape shape(*path, paint);

                target->drawShape(clip, runPaint, ctm, shape);
            }
        }

        // then flush each subrun, if any
        if (!run.fInitialized) {
            continue;
        }

        int lastSubRun = SkTMin(run.fSubRunInfo.count(), 1 << 16) - 1;
        for (int subRun = 0; subRun <= lastSubRun; subRun++) {
            const SubRun& info = run.fSubRunInfo[subRun];
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
            // We can clip geometrically if we're not using SDFs or transformed glyphs,
            // and we have an axis-aligned rectangular non-AA clip
            if (!info.drawAsDistanceFields() && !info.needsTransform() &&
                clip.isRRect(rtBounds, &clipRRect, &aa) &&
                clipRRect.isRect() && GrAA::kNo == aa) {
                skipClip = true;
                // We only need to do clipping work if the subrun isn't contained by the clip
                SkRect subRunBounds;
                this->computeSubRunBounds(&subRunBounds, runIndex, subRun, viewMatrix, x, y,
                                          false);
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
                                       clipRect, paint, filteredColor, props, distanceAdjustTable,
                                       target);
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

std::unique_ptr<GrDrawOp> GrTextBlob::test_makeOp(
        int glyphCount, uint16_t run, uint16_t subRun, const SkMatrix& viewMatrix,
        SkScalar x, SkScalar y, const SkPaint& paint, const SkPMColor4f& filteredColor,
        const SkSurfaceProps& props, const GrDistanceFieldAdjustTable* distanceAdjustTable,
        GrTextTarget* target) {
    const GrTextBlob::SubRun& info = fRuns[run].fSubRunInfo[subRun];
    SkIRect emptyRect = SkIRect::MakeEmpty();
    return this->makeOp(info, glyphCount, run, subRun, viewMatrix, x, y, emptyRect,
                        paint, filteredColor, props, distanceAdjustTable, target);
}

void GrTextBlob::AssertEqual(const GrTextBlob& l, const GrTextBlob& r) {
    SkASSERT_RELEASE(l.fSize == r.fSize);

    SkASSERT_RELEASE(l.fBlurRec.fSigma == r.fBlurRec.fSigma);
    SkASSERT_RELEASE(l.fBlurRec.fStyle == r.fBlurRec.fStyle);

    SkASSERT_RELEASE(l.fStrokeInfo.fFrameWidth == r.fStrokeInfo.fFrameWidth);
    SkASSERT_RELEASE(l.fStrokeInfo.fMiterLimit == r.fStrokeInfo.fMiterLimit);
    SkASSERT_RELEASE(l.fStrokeInfo.fJoin == r.fStrokeInfo.fJoin);

    SkASSERT_RELEASE(l.fKey == r.fKey);
    //SkASSERT_RELEASE(l.fPaintColor == r.fPaintColor); // Colors might not actually be identical
    SkASSERT_RELEASE(l.fMaxMinScale == r.fMaxMinScale);
    SkASSERT_RELEASE(l.fMinMaxScale == r.fMinMaxScale);
    SkASSERT_RELEASE(l.fTextType == r.fTextType);

    SkASSERT_RELEASE(l.fRunCountLimit == r.fRunCountLimit);
    for (int i = 0; i < l.fRunCountLimit; i++) {
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

        if (lRun.fARGBFallbackDescriptor.get()) {
            SkASSERT_RELEASE(lRun.fARGBFallbackDescriptor->getDesc());
            SkASSERT_RELEASE(rRun.fARGBFallbackDescriptor.get() && rRun.fARGBFallbackDescriptor->getDesc());
            SkASSERT_RELEASE(*lRun.fARGBFallbackDescriptor->getDesc() ==
                             *rRun.fARGBFallbackDescriptor->getDesc());
        } else {
            SkASSERT_RELEASE(!rRun.fARGBFallbackDescriptor.get());
        }

        // color can be changed
        //SkASSERT(lRun.fColor == rRun.fColor);
        SkASSERT_RELEASE(lRun.fInitialized == rRun.fInitialized);

        SkASSERT_RELEASE(lRun.fSubRunInfo.count() == rRun.fSubRunInfo.count());
        for(int j = 0; j < lRun.fSubRunInfo.count(); j++) {
            const SubRun& lSubRun = lRun.fSubRunInfo[j];
            const SubRun& rSubRun = rRun.fSubRunInfo[j];

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

void GrTextBlob::SubRun::computeTranslation(const SkMatrix& viewMatrix,
                                                SkScalar x, SkScalar y, SkScalar* transX,
                                                SkScalar* transY) {
    // Don't use the matrix to translate on distance field for fallback subruns.
    calculate_translation(!this->drawAsDistanceFields() && !this->isFallback(), viewMatrix,
            x, y, fCurrentViewMatrix, fX, fY, transX, transY);
    fCurrentViewMatrix = viewMatrix;
    fX = x;
    fY = y;
}
