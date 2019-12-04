/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/gpu/GrContext.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkPaintPriv.h"
#include "src/gpu/GrBlurUtils.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/geometry/GrShape.h"
#include "src/gpu/ops/GrAtlasTextOp.h"
#include "src/gpu/text/GrTextBlob.h"
#include "src/gpu/text/GrTextTarget.h"

#include <new>

template <size_t N> static size_t sk_align(size_t s) {
    return ((s + (N-1)) / N) * N;
}

sk_sp<GrTextBlob> GrTextBlob::Make(int glyphCount,
                                   GrStrikeCache* strikeCache,
                                   const SkMatrix& viewMatrix,
                                   SkPoint origin,
                                   GrColor color,
                                   bool forceWForDistanceFields) {
    // Default to no perspective. Implies one of the following vertex formats: kColorTextVASize,
    // kGrayTextVASize, kLCDTextVASize.
    static_assert(kColorTextVASize <= kGrayTextVASize && kLCDTextVASize <= kGrayTextVASize);
    size_t quadSize = kVerticesPerGlyph * kGrayTextVASize;
    if (viewMatrix.hasPerspective() || forceWForDistanceFields) {
        // Perspective. Implies one of the following vertex formats: kColorTextPerspectiveVASize,
        // kGrayTextDFPerspectiveVASize.
        static_assert(kColorTextPerspectiveVASize <= kGrayTextDFPerspectiveVASize);
        quadSize = kVerticesPerGlyph * kGrayTextDFPerspectiveVASize;
    }

    // We allocate size for the GrTextBlob itself, plus size for the vertices array,
    // and size for the glyphIds array.
    size_t verticesCount = glyphCount * quadSize;

    size_t blobStart = 0;
    size_t vertex = sk_align<alignof(char)>     (blobStart + sizeof(GrTextBlob) * 1);
    size_t glyphs = sk_align<alignof(GrGlyph*)> (vertex + sizeof(char) * verticesCount);
    size_t   size =                             (glyphs + sizeof(GrGlyph*) * glyphCount);

    void* allocation = ::operator new (size);

    if (CACHE_SANITY_CHECK) {
        sk_bzero(allocation, size);
    }

    sk_sp<GrTextBlob> blob{new (allocation) GrTextBlob{
        size, strikeCache, viewMatrix, origin, color, forceWForDistanceFields}};

    // setup offsets for vertices / glyphs
    blob->fVertices = SkTAddOffset<char>(blob.get(), vertex);
    blob->fGlyphs   = SkTAddOffset<GrGlyph*>(blob.get(), glyphs);

    return blob;
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
        // Identical view matrices and we can reuse in all cases
        return !(fInitialViewMatrix.cheapEqualTo(viewMatrix) && SkPoint{x, y} == fInitialOrigin);
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
                          viewMatrix.getScaleX() * (x - fInitialOrigin.x()) +
                          viewMatrix.getSkewX() * (y - fInitialOrigin.y()) -
                          fInitialViewMatrix.getTranslateX();
        SkScalar transY = viewMatrix.getTranslateY() +
                          viewMatrix.getSkewY() * (x - fInitialOrigin.x()) +
                          viewMatrix.getScaleY() * (y - fInitialOrigin.y()) -
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
        SubRun& info, int glyphCount,
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
                target->colorInfo().isLinearlyBlended(), SkPaintPriv::ComputeLuminanceColor(paint),
                props, info.isAntiAliased(), info.hasUseLCDText());
    } else {
        op = GrAtlasTextOp::MakeBitmap(target->getContext(), std::move(grPaint), format, glyphCount,
                                       info.needsTransform());
    }
    GrAtlasTextOp::Geometry& geometry = op->geometry();
    geometry.fViewMatrix = viewMatrix;
    geometry.fClipRect = clipRect;
    geometry.fBlob = SkRef(this);
    geometry.fSubRunPtr = &info;
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

    for (auto& subRun : fSubRuns) {
        if (subRun.drawAsPaths()) {
            SkPaint runPaint{paint};
            runPaint.setAntiAlias(subRun.isAntiAliased());
            // If there are shaders, blurs or styles, the path must be scaled into source
            // space independently of the CTM. This allows the CTM to be correct for the
            // different effects.
            GrStyle style(runPaint);

            bool scalePath = runPaint.getShader()
                             || style.applies()
                             || runPaint.getMaskFilter();

            // The origin for the blob may have changed, so figure out the delta.
            SkVector originShift = SkPoint{x, y} - fInitialOrigin;

            for (const auto& pathGlyph : subRun.fPaths) {
                SkMatrix ctm{viewMatrix};
                SkMatrix pathMatrix = SkMatrix::MakeScale(subRun.fStrikeSpec.strikeToSourceRatio());
                // Shift the original glyph location in source space to the position of the new
                // blob.
                pathMatrix.postTranslate(originShift.x() + pathGlyph.fOrigin.x(),
                                         originShift.y() + pathGlyph.fOrigin.y());

                // TmpPath must be in the same scope as GrShape shape below.
                SkTLazy<SkPath> tmpPath;
                const SkPath* path = &pathGlyph.fPath;
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

                // TODO: we are losing the mutability of the path here
                GrShape shape(*path, paint);
                target->drawShape(clip, runPaint, ctm, shape);
            }
        } else {
            int glyphCount = subRun.glyphCount();
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
            if (!subRun.drawAsDistanceFields() && !subRun.needsTransform() &&
                clip.isRRect(rtBounds, &clipRRect, &aa) &&
                clipRRect.isRect() && GrAA::kNo == aa) {
                skipClip = true;
                // We only need to do clipping work if the subrun isn't contained by the clip
                SkRect subRunBounds;
                this->computeSubRunBounds(&subRunBounds, subRun, viewMatrix, x, y, false);
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
                auto op = this->makeOp(subRun, glyphCount, viewMatrix, x, y,
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
        int glyphCount, const SkMatrix& viewMatrix,
        SkScalar x, SkScalar y, const SkPaint& paint, const SkPMColor4f& filteredColor,
        const SkSurfaceProps& props, const GrDistanceFieldAdjustTable* distanceAdjustTable,
        GrTextTarget* target) {
    GrTextBlob::SubRun& info = fSubRuns[0];
    SkIRect emptyRect = SkIRect::MakeEmpty();
    return this->makeOp(info, glyphCount, viewMatrix, x, y, emptyRect,
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

    for(auto t : SkMakeZip(l.fSubRuns, r.fSubRuns)) {
        const SubRun& lSubRun = std::get<0>(t);
        const SubRun& rSubRun = std::get<1>(t);
        SkASSERT(lSubRun.drawAsPaths() == rSubRun.drawAsPaths());
        if (!lSubRun.drawAsPaths()) {

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
            SkASSERT_RELEASE(lSubRun.glyphStartIndex() == rSubRun.glyphStartIndex());
            SkASSERT_RELEASE(lSubRun.maskFormat() == rSubRun.maskFormat());
            SkASSERT_RELEASE(lSubRun.drawAsDistanceFields() == rSubRun.drawAsDistanceFields());
            SkASSERT_RELEASE(lSubRun.hasUseLCDText() == rSubRun.hasUseLCDText());
        } else {
            SkASSERT_RELEASE(lSubRun.fPaths.size() == rSubRun.fPaths.size());
            for(auto p : SkMakeZip(lSubRun.fPaths, rSubRun.fPaths)) {
                const PathGlyph& lPath = std::get<0>(p);
                const PathGlyph& rPath = std::get<1>(p);
                SkASSERT_RELEASE(lPath.fPath == rPath.fPath);
                // We can't assert that these have the same translations
            }
        }
    }
}

static SkMatrix make_inverse(const SkMatrix& matrix) {
    SkMatrix inverseMatrix;
    if (!matrix.invert(&inverseMatrix)) {
        inverseMatrix = SkMatrix::I();
    }
    return inverseMatrix;
}

GrTextBlob::GrTextBlob(size_t size,
                       GrStrikeCache* strikeCache,
                       const SkMatrix& viewMatrix,
                       SkPoint origin,
                       GrColor color,
                       bool forceWForDistanceFields)
        : fSize{size}
        , fStrikeCache{strikeCache}
        , fInitialViewMatrix{viewMatrix}
        , fInitialViewMatrixInverse{make_inverse(viewMatrix)}
        , fInitialOrigin{origin}
        , fForceWForDistanceFields{forceWForDistanceFields}
        , fColor{color} { }

void GrTextBlob::computeSubRunBounds(SkRect* outBounds, const GrTextBlob::SubRun& subRun,
                                     const SkMatrix& viewMatrix, SkScalar x, SkScalar y,
                                     bool needsGlyphTransform) {
    // We don't yet position distance field text on the cpu, so we have to map the vertex bounds
    // into device space.
    // We handle vertex bounds differently for distance field text and bitmap text because
    // the vertex bounds of bitmap text are in device space.  If we are flushing multiple runs
    // from one blob then we are going to pay the price here of mapping the rect for each run.
    *outBounds = subRun.vertexBounds();
    if (needsGlyphTransform) {
        // Distance field text is positioned with the (X,Y) as part of the glyph position,
        // and currently the view matrix is applied on the GPU
        outBounds->offset(SkPoint{x, y} - fInitialOrigin);
        viewMatrix.mapRect(outBounds);
    } else {
        // Bitmap text is fully positioned on the CPU, and offset by an (X,Y) translate in
        // device space.
        SkMatrix boundsMatrix = fInitialViewMatrixInverse;

        boundsMatrix.postTranslate(-fInitialOrigin.x(), -fInitialOrigin.y());

        boundsMatrix.postTranslate(x, y);

        boundsMatrix.postConcat(viewMatrix);
        boundsMatrix.mapRect(outBounds);

        // Due to floating point numerical inaccuracies, we have to round out here
        outBounds->roundOut(outBounds);
    }
}

void GrTextBlob::SubRun::computeTranslation(const SkMatrix& viewMatrix,
                                                SkScalar x, SkScalar y, SkScalar* transX,
                                                SkScalar* transY) {
    // Don't use the matrix to translate on distance field for fallback subruns.
    calculate_translation(!this->drawAsDistanceFields() && !this->needsTransform(), viewMatrix,
            x, y, fCurrentViewMatrix, fX, fY, transX, transY);
    fCurrentViewMatrix = viewMatrix;
    fX = x;
    fY = y;
}
