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
#include "src/gpu/text/GrAtlasManager.h"
#include "src/gpu/text/GrTextBlob.h"
#include "src/gpu/text/GrTextTarget.h"

#include <new>

// -- GrTextBlob::SubRun ---------------------------------------------------------------------------
// Hold data to draw the different types of sub run. SubRuns are produced knowing all the
// glyphs that are included in them.
class GrTextBlob::SubRun {
public:
    // SubRun for masks
    SubRun(SubRunType type, GrTextBlob* textBlob, const SkStrikeSpec& strikeSpec,
           GrMaskFormat format, size_t glyphCount,
           sk_sp<GrTextStrike>&& grStrike);

    // SubRun for paths
    SubRun(GrTextBlob* textBlob, const SkStrikeSpec& strikeSpec, size_t glyphCount);

    void appendGlyphs(const SkZip<SkGlyphVariant, SkPoint>& drawables);

    // TODO when this object is more internal, drop the privacy
    void resetBulkUseToken() { fBulkUseToken.reset(); }
    GrDrawOpAtlas::BulkUseTokenUpdater* bulkUseToken() { return &fBulkUseToken; }
    void setStrike(sk_sp<GrTextStrike> strike) { fStrike = std::move(strike); }
    GrTextStrike* strike() const { return fStrike.get(); }
    sk_sp<GrTextStrike> refStrike() const { return fStrike; }

    void setAtlasGeneration(uint64_t atlasGeneration) { fAtlasGeneration = atlasGeneration;}
    uint64_t atlasGeneration() const { return fAtlasGeneration; }

    void setColor(GrColor color) { fColor = color; }
    GrColor color() const { return fColor; }

    GrMaskFormat maskFormat() const { return fMaskFormat; }

    const SkRect& vertexBounds() const { return fVertexBounds; }
    void joinGlyphBounds(const SkRect& glyphBounds) {
        fVertexBounds.joinNonEmptyArg(glyphBounds);
    }

    void init(const SkMatrix& viewMatrix, SkScalar x, SkScalar y) {
        fCurrentViewMatrix = viewMatrix;
        fX = x;
        fY = y;
    }

    // This function assumes the translation will be applied before it is called again
    void computeTranslation(const SkMatrix& viewMatrix, SkScalar x, SkScalar y,
                            SkScalar* transX, SkScalar* transY);

    bool drawAsDistanceFields() const { return fType == kTransformedSDFT; }
    bool drawAsPaths() const { return fType == kTransformedPath; }
    bool needsTransform() const {
        return fType == kTransformedPath
               || fType == kTransformedMask
               || fType == kTransformedSDFT;
    }
    bool hasW() const { return fBlob->hasW(fType); }
    size_t vertexStride() const {
        return GrTextBlob::GetVertexStride(fMaskFormat, this->hasW());
    }

    SkSpan<GrGlyph*> glyphs() {
        static_assert(alignof(SubRun) >= alignof(GrGlyph*));
        return SkSpan<GrGlyph*>{SkTAddOffset<GrGlyph*>(this, sizeof(SubRun)), fGlyphCount};
    }

    char* vertices() {
        // Make sure that vertices are properly aligned.
        static_assert(alignof(GrGlyph*) >= alignof(float));
        return (char*)this->glyphs().end();
    }

    // df properties
    void setUseLCDText(bool useLCDText) { fFlags.useLCDText = useLCDText; }
    bool hasUseLCDText() const { return fFlags.useLCDText; }
    void setAntiAliased(bool antiAliased) { fFlags.antiAliased = antiAliased; }
    bool isAntiAliased() const { return fFlags.antiAliased; }

    const SkStrikeSpec& strikeSpec() const { return fStrikeSpec; }

    SubRun* fNextSubRun{nullptr};
    const SubRunType fType;
    GrTextBlob* const fBlob;
    const GrMaskFormat fMaskFormat;
    const size_t fGlyphCount;
    const SkStrikeSpec fStrikeSpec;
    sk_sp<GrTextStrike> fStrike;
    struct {
        bool useLCDText:1;
        bool antiAliased:1;
    } fFlags{false, false};
    GrColor fColor;
    GrDrawOpAtlas::BulkUseTokenUpdater fBulkUseToken;
    SkRect fVertexBounds = SkRectPriv::MakeLargestInverted();
    uint64_t fAtlasGeneration{GrDrawOpAtlas::kInvalidAtlasGeneration};
    SkScalar fX;
    SkScalar fY;
    SkMatrix fCurrentViewMatrix;
    std::vector<PathGlyph> fPaths;
};  // SubRun

GrTextBlob::SubRun::SubRun(
        SubRunType type, GrTextBlob* textBlob, const SkStrikeSpec& strikeSpec,
        GrMaskFormat format, size_t glyphCount,
        sk_sp<GrTextStrike>&& grStrike)
        : fType{type}
        , fBlob{textBlob}
        , fMaskFormat{format}
        , fGlyphCount{glyphCount}
        , fStrikeSpec{strikeSpec}
        , fStrike{grStrike}
        , fColor{textBlob->fColor}
        , fX{textBlob->fInitialOrigin.x()}
        , fY{textBlob->fInitialOrigin.y()}
        , fCurrentViewMatrix{textBlob->fInitialViewMatrix} {
    SkASSERT(type != kTransformedPath);
    fBlob->insertSubRun(this);
}

GrTextBlob::SubRun::SubRun(
        GrTextBlob* textBlob, const SkStrikeSpec& strikeSpec, size_t glyphCount)
        : fType{kTransformedPath}
        , fBlob{textBlob}
        , fMaskFormat{kA8_GrMaskFormat}
        , fGlyphCount{glyphCount}
        , fStrikeSpec{strikeSpec}
        , fStrike{nullptr}
        , fColor{textBlob->fColor}
        , fX{textBlob->fInitialOrigin.x()}
        , fY{textBlob->fInitialOrigin.y()}
        , fPaths{} {
    fBlob->insertSubRun(this);
}

void GrTextBlob::SubRun::appendGlyphs(const SkZip<SkGlyphVariant, SkPoint>& drawables) {
    GrTextStrike* grStrike = fStrike.get();
    SkScalar strikeToSource = fStrikeSpec.strikeToSourceRatio();
    GrGlyph** glyphCursor = this->glyphs().data();
    char* vertexCursor = this->vertices();
    bool hasW = this->hasW();
    GrColor color = this->color();
    // glyphs drawn in perspective must always have a w coord.
    SkASSERT(hasW || !fBlob->fInitialViewMatrix.hasPerspective());
    size_t vertexStride = GetVertexStride(fMaskFormat, hasW);
    // We always write the third position component used by SDFs. If it is unused it gets
    // overwritten. Similarly, we always write the color and the blob will later overwrite it
    // with texture coords if it is unused.
    size_t colorOffset = hasW ? sizeof(SkPoint3) : sizeof(SkPoint);
    SkASSERT(fGlyphCount == drawables.size());
    for (auto [variant, pos] : drawables) {
        SkGlyph* skGlyph = variant;
        GrGlyph* grGlyph = grStrike->getGlyph(*skGlyph);
        // Only floor the device coordinates.
        SkRect dstRect;
        if (!this->needsTransform()) {
            pos = {SkScalarFloorToScalar(pos.x()), SkScalarFloorToScalar(pos.y())};
            dstRect = grGlyph->destRect(pos);
        } else {
            dstRect = grGlyph->destRect(pos, strikeToSource);
        }

        this->joinGlyphBounds(dstRect);

        // V0
        *reinterpret_cast<SkPoint3*>(vertexCursor) = {dstRect.fLeft, dstRect.fTop, 1.f};
        *reinterpret_cast<GrColor*>(vertexCursor + colorOffset) = color;
        vertexCursor += vertexStride;

        // V1
        *reinterpret_cast<SkPoint3*>(vertexCursor) = {dstRect.fLeft, dstRect.fBottom, 1.f};
        *reinterpret_cast<GrColor*>(vertexCursor + colorOffset) = color;
        vertexCursor += vertexStride;

        // V2
        *reinterpret_cast<SkPoint3*>(vertexCursor) = {dstRect.fRight, dstRect.fTop, 1.f};
        *reinterpret_cast<GrColor*>(vertexCursor + colorOffset) = color;
        vertexCursor += vertexStride;

        // V3
        *reinterpret_cast<SkPoint3*>(vertexCursor) = {dstRect.fRight, dstRect.fBottom, 1.f};
        *reinterpret_cast<GrColor*>(vertexCursor + colorOffset) = color;
        vertexCursor += vertexStride;

        *glyphCursor++ = grGlyph;
    }
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

// -- GrTextBlob -----------------------------------------------------------------------------------
template <size_t N> static size_t sk_align(size_t s) {
    return ((s + (N-1)) / N) * N;
}

void GrTextBlob::insertSubRun(GrTextBlob::SubRun* subRun) {
    if (fFirstSubRun == nullptr) {
        fFirstSubRun = subRun;
        fLastSubRun = subRun;
    } else {
        fLastSubRun->fNextSubRun = subRun;
        fLastSubRun = subRun;
    }
}

sk_sp<GrTextBlob> GrTextBlob::Make(int glyphCount,
                                   int runCount,
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
    size_t subRuns = sk_align<4>       (glyphs +  sizeof(GrGlyph*) * glyphCount);
    size_t   size =                             (subRuns + sizeof(SubRun) * runCount);

    void* allocation = ::operator new (size);

    if (CACHE_SANITY_CHECK) {
        sk_bzero(allocation, size);
    }

    size_t allocSize = size - sizeof(GrTextBlob);

    sk_sp<GrTextBlob> blob{new (allocation) GrTextBlob{
        allocSize, strikeCache, viewMatrix, origin, color, forceWForDistanceFields}};

    return blob;
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

void GrTextBlob::flush(GrTextTarget* target, const SkSurfaceProps& props,
                       const GrDistanceFieldAdjustTable* distanceAdjustTable,
                       const SkPaint& paint, const SkPMColor4f& filteredColor, const GrClip& clip,
                       const SkMatrix& viewMatrix, SkScalar x, SkScalar y) {

    for (SubRun* subRun = fFirstSubRun; subRun != nullptr; subRun = subRun->fNextSubRun) {
        if (subRun->drawAsPaths()) {
            SkPaint runPaint{paint};
            runPaint.setAntiAlias(subRun->isAntiAliased());
            // If there are shaders, blurs or styles, the path must be scaled into source
            // space independently of the CTM. This allows the CTM to be correct for the
            // different effects.
            GrStyle style(runPaint);

            bool scalePath = runPaint.getShader()
                             || style.applies()
                             || runPaint.getMaskFilter();

            // The origin for the blob may have changed, so figure out the delta.
            SkVector originShift = SkPoint{x, y} - fInitialOrigin;

            for (const auto& pathGlyph : subRun->fPaths) {
                SkMatrix ctm{viewMatrix};
                SkMatrix pathMatrix = SkMatrix::MakeScale(
                        subRun->fStrikeSpec.strikeToSourceRatio());
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
            int glyphCount = subRun->fGlyphCount;
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
            if (!subRun->drawAsDistanceFields() && !subRun->needsTransform() &&
                clip.isRRect(rtBounds, &clipRRect, &aa) &&
                clipRRect.isRect() && GrAA::kNo == aa) {
                skipClip = true;
                // We only need to do clipping work if the subrun isn't contained by the clip
                SkRect subRunBounds;
                this->computeSubRunBounds(&subRunBounds, *subRun, viewMatrix, x, y, false);
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
                auto op = this->makeOp(*subRun, glyphCount, viewMatrix, x, y,
                                       clipRect, paint, filteredColor, props, distanceAdjustTable,
                                       target);
                if (op) {
                    if (skipClip) {
                        target->addDrawOp(GrNoClip(), std::move(op));
                    } else {
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
    GrTextBlob::SubRun& info = *fFirstSubRun;
    SkIRect emptyRect = SkIRect::MakeEmpty();
    return this->makeOp(info, glyphCount, viewMatrix, x, y, emptyRect,
                        paint, filteredColor, props, distanceAdjustTable, target);
}

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

void GrTextBlob::addSingleMaskFormat(
        SubRunType type,
        const SkZip<SkGlyphVariant, SkPoint>& drawables,
        const SkStrikeSpec& strikeSpec,
        GrMaskFormat format) {
    this->makeSubRun(type, drawables, strikeSpec, format);
}

void GrTextBlob::addMultiMaskFormat(
        SubRunType type,
        const SkZip<SkGlyphVariant, SkPoint>& drawables,
        const SkStrikeSpec& strikeSpec) {
    this->setHasBitmap();
    if (drawables.empty()) { return; }

    auto glyphSpan = drawables.get<0>();
    SkGlyph* glyph = glyphSpan[0];
    GrMaskFormat format = GrGlyph::FormatFromSkGlyph(glyph->maskFormat());
    size_t startIndex = 0;
    for (size_t i = 1; i < drawables.size(); i++) {
        glyph = glyphSpan[i];
        GrMaskFormat nextFormat = GrGlyph::FormatFromSkGlyph(glyph->maskFormat());
        if (format != nextFormat) {
            auto sameFormat = drawables.subspan(startIndex, i - startIndex);
            this->addSingleMaskFormat(type, sameFormat, strikeSpec, format);
            format = nextFormat;
            startIndex = i;
        }
    }
    auto sameFormat = drawables.last(drawables.size() - startIndex);
    this->addSingleMaskFormat(type, sameFormat, strikeSpec, format);
}

void GrTextBlob::addSDFT(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                         const SkStrikeSpec& strikeSpec,
                         const SkFont& runFont,
                         SkScalar minScale,
                         SkScalar maxScale) {
    this->setHasDistanceField();
    this->setMinAndMaxScale(minScale, maxScale);

    SubRun* subRun = this->makeSubRun(kTransformedSDFT, drawables, strikeSpec, kA8_GrMaskFormat);
    subRun->setUseLCDText(runFont.getEdging() == SkFont::Edging::kSubpixelAntiAlias);
    subRun->setAntiAliased(runFont.hasSomeAntiAliasing());
}

void GrTextBlob::processDeviceMasks(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                    const SkStrikeSpec& strikeSpec) {
    this->addMultiMaskFormat(kDirectMask, drawables, strikeSpec);
}

void GrTextBlob::processSourcePaths(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                    const SkFont& runFont,
                                    const SkStrikeSpec& strikeSpec) {
    this->setHasBitmap();

    // No extra memory needed.
    SubRun* subRun = fAlloc.make<SubRun>(this, strikeSpec, drawables.size());
    subRun->setAntiAliased(runFont.hasSomeAntiAliasing());
    for (auto [variant, pos] : drawables) {
        subRun->fPaths.emplace_back(*variant.path(), pos);
    }
}

void GrTextBlob::processSourceSDFT(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                   const SkStrikeSpec& strikeSpec,
                                   const SkFont& runFont,
                                   SkScalar minScale,
                                   SkScalar maxScale) {
    this->addSDFT(drawables, strikeSpec, runFont, minScale, maxScale);
}

void GrTextBlob::processSourceMasks(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                    const SkStrikeSpec& strikeSpec) {
    this->addMultiMaskFormat(kTransformedMask, drawables, strikeSpec);
}


class GrTextBlob::SubRun* GrTextBlob::makeSubRun(SubRunType type,
                                                 const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                                 const SkStrikeSpec& strikeSpec,
                                                 GrMaskFormat format) {


    sk_sp<GrTextStrike> grStrike = strikeSpec.findOrCreateGrStrike(fStrikeCache);

    size_t quadSize = GrTextBlob::GetVertexStride(format, this->hasW(type)) * kVerticesPerGlyph;

    size_t bytes =
            sizeof(SubRun) + drawables.size() * sizeof(GrGlyph*) + drawables.size() * quadSize;

    void* subRunStorage = fAlloc.makeBytesAlignedTo(bytes, alignof(SubRun));

    new (subRunStorage) SubRun{
        type, this, strikeSpec, format, drawables.size(), std::move(grStrike)};

    fLastSubRun->appendGlyphs(drawables);

    return fLastSubRun;
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

static SkMatrix make_inverse(const SkMatrix& matrix) {
    SkMatrix inverseMatrix;
    if (!matrix.invert(&inverseMatrix)) {
        inverseMatrix = SkMatrix::I();
    }
    return inverseMatrix;
}

GrTextBlob::GrTextBlob(size_t arenaSize,
                       GrStrikeCache* strikeCache,
                       const SkMatrix& viewMatrix,
                       SkPoint origin,
                       GrColor color,
                       bool forceWForDistanceFields)
        : fSize{arenaSize}
        , fStrikeCache{strikeCache}
        , fInitialViewMatrix{viewMatrix}
        , fInitialViewMatrixInverse{make_inverse(viewMatrix)}
        , fInitialOrigin{origin}
        , fForceWForDistanceFields{forceWForDistanceFields}
        , fColor{color}
        , fAlloc{SkTAddOffset<char>(this, sizeof(GrTextBlob)), arenaSize, arenaSize} { }


enum RegenMask {
    kNoRegen    = 0x0,
    kRegenPos   = 0x1,
    kRegenCol   = 0x2,
    kRegenTex   = 0x4,
    kRegenGlyph = 0x8,
};

////////////////////////////////////////////////////////////////////////////////////////////////////

static void regen_positions(char* vertex, size_t vertexStride, SkScalar transX, SkScalar transY) {
    SkPoint* point = reinterpret_cast<SkPoint*>(vertex);
    for (int i = 0; i < 4; ++i) {
        point->fX += transX;
        point->fY += transY;
        point = SkTAddOffset<SkPoint>(point, vertexStride);
    }
}

static void regen_colors(char* vertex, size_t vertexStride, GrColor color) {
    // This is a bit wonky, but sometimes we have LCD text, in which case we won't have color
    // vertices, hence vertexStride - sizeof(SkIPoint16)
    size_t colorOffset = vertexStride - sizeof(SkIPoint16) - sizeof(GrColor);
    GrColor* vcolor = reinterpret_cast<GrColor*>(vertex + colorOffset);
    for (int i = 0; i < 4; ++i) {
        *vcolor = color;
        vcolor = SkTAddOffset<GrColor>(vcolor, vertexStride);
    }
}

static void regen_texcoords(char* vertex, size_t vertexStride, const GrGlyph* glyph,
                            bool useDistanceFields) {
    // This is a bit wonky, but sometimes we have LCD text, in which case we won't have color
    // vertices, hence vertexStride - sizeof(SkIPoint16)
    size_t texCoordOffset = vertexStride - sizeof(SkIPoint16);

    uint16_t u0, v0, u1, v1;
    SkASSERT(glyph);
    int width = glyph->fBounds.width();
    int height = glyph->fBounds.height();

    if (useDistanceFields) {
        u0 = glyph->fAtlasLocation.fX + SK_DistanceFieldInset;
        v0 = glyph->fAtlasLocation.fY + SK_DistanceFieldInset;
        u1 = u0 + width - 2 * SK_DistanceFieldInset;
        v1 = v0 + height - 2 * SK_DistanceFieldInset;
    } else {
        u0 = glyph->fAtlasLocation.fX;
        v0 = glyph->fAtlasLocation.fY;
        u1 = u0 + width;
        v1 = v0 + height;
    }
    // We pack the 2bit page index in the low bit of the u and v texture coords
    uint32_t pageIndex = glyph->pageIndex();
    SkASSERT(pageIndex < 4);
    uint16_t uBit = (pageIndex >> 1) & 0x1;
    uint16_t vBit = pageIndex & 0x1;
    u0 <<= 1;
    u0 |= uBit;
    v0 <<= 1;
    v0 |= vBit;
    u1 <<= 1;
    u1 |= uBit;
    v1 <<= 1;
    v1 |= vBit;

    uint16_t* textureCoords = reinterpret_cast<uint16_t*>(vertex + texCoordOffset);
    textureCoords[0] = u0;
    textureCoords[1] = v0;
    textureCoords = SkTAddOffset<uint16_t>(textureCoords, vertexStride);
    textureCoords[0] = u0;
    textureCoords[1] = v1;
    textureCoords = SkTAddOffset<uint16_t>(textureCoords, vertexStride);
    textureCoords[0] = u1;
    textureCoords[1] = v0;
    textureCoords = SkTAddOffset<uint16_t>(textureCoords, vertexStride);
    textureCoords[0] = u1;
    textureCoords[1] = v1;

#ifdef DISPLAY_PAGE_INDEX
    // Enable this to visualize the page from which each glyph is being drawn.
    // Green Red Magenta Cyan -> 0 1 2 3; Black -> error
    GrColor hackColor;
    switch (pageIndex) {
        case 0:
            hackColor = GrColorPackRGBA(0, 255, 0, 255);
            break;
        case 1:
            hackColor = GrColorPackRGBA(255, 0, 0, 255);;
            break;
        case 2:
            hackColor = GrColorPackRGBA(255, 0, 255, 255);
            break;
        case 3:
            hackColor = GrColorPackRGBA(0, 255, 255, 255);
            break;
        default:
            hackColor = GrColorPackRGBA(0, 0, 0, 255);
            break;
    }
    regen_colors(vertex, vertexStride, hackColor);
#endif
}

GrTextBlob::VertexRegenerator::VertexRegenerator(GrResourceProvider* resourceProvider,
                                                 GrTextBlob* blob,
                                                 SubRun* subRun,
                                                 const SkMatrix& viewMatrix, SkScalar x, SkScalar y,
                                                 GrColor color,
                                                 GrDeferredUploadTarget* uploadTarget,
                                                 GrStrikeCache* grStrikeCache,
                                                 GrAtlasManager* fullAtlasManager)
        : fResourceProvider(resourceProvider)
        , fViewMatrix(viewMatrix)
        , fBlob(blob)
        , fUploadTarget(uploadTarget)
        , fGrStrikeCache(grStrikeCache)
        , fFullAtlasManager(fullAtlasManager)
        , fSubRun(subRun)
        , fColor(color) {
    // Compute translation if any
    fSubRun->computeTranslation(fViewMatrix, x, y, &fTransX, &fTransY);

    // Because the GrStrikeCache may evict the strike a blob depends on using for
    // generating its texture coords, we have to track whether or not the strike has
    // been abandoned.  If it hasn't been abandoned, then we can use the GrGlyph*s as is
    // otherwise we have to get the new strike, and use that to get the correct glyphs.
    // Because we do not have the packed ids, and thus can't look up our glyphs in the
    // new strike, we instead keep our ref to the old strike and use the packed ids from
    // it.  These ids will still be valid as long as we hold the ref.  When we are done
    // updating our cache of the GrGlyph*s, we drop our ref on the old strike
    if (fSubRun->strike()->isAbandoned()) {
        fRegenFlags |= kRegenGlyph;
        fRegenFlags |= kRegenTex;
    }
    if (kARGB_GrMaskFormat != fSubRun->maskFormat() && fSubRun->color() != color) {
        fRegenFlags |= kRegenCol;
    }
    if (0.f != fTransX || 0.f != fTransY) {
        fRegenFlags |= kRegenPos;
    }
}

bool GrTextBlob::VertexRegenerator::doRegen(GrTextBlob::VertexRegenerator::Result* result,
                                            bool regenPos, bool regenCol, bool regenTexCoords,
                                            bool regenGlyphs) {
    SkASSERT(!regenGlyphs || regenTexCoords);
    if (regenTexCoords) {
        fSubRun->resetBulkUseToken();

        const SkStrikeSpec& strikeSpec = fSubRun->strikeSpec();

        if (!fMetricsAndImages.isValid()
            || fMetricsAndImages->descriptor() != strikeSpec.descriptor()) {
            fMetricsAndImages.init(strikeSpec);
        }

        if (regenGlyphs) {
            // Take the glyphs from the old strike, and translate them a new strike.
            sk_sp<GrTextStrike> newStrike = strikeSpec.findOrCreateGrStrike(fGrStrikeCache);

            // Start this batch at the start of the subRun plus any glyphs that were previously
            // processed.
            auto glyphs = fSubRun->glyphs();

            // Convert old glyphs to newStrike.
            for (auto& glyph : glyphs) {
                SkPackedGlyphID id = glyph->fPackedID;
                glyph = newStrike->getGlyph(id, fMetricsAndImages.get());
                SkASSERT(id == glyph->fPackedID);
            }

            fSubRun->setStrike(newStrike);
        }
    }

    sk_sp<GrTextStrike> grStrike = fSubRun->refStrike();
    bool hasW = fSubRun->hasW();
    auto vertexStride = GetVertexStride(fSubRun->maskFormat(), hasW);
    char* currVertex = fSubRun->vertices();
    result->fFirstVertex = currVertex;
    auto glyphs = fSubRun->glyphs();

    for (size_t glyphIdx = fCurrGlyph; glyphIdx < fSubRun->fGlyphCount; glyphIdx++) {
        GrGlyph* glyph = nullptr;
        if (regenTexCoords) {
            glyph = glyphs[glyphIdx];
            SkASSERT(glyph && glyph->fMaskFormat == fSubRun->maskFormat());

            if (!fFullAtlasManager->hasGlyph(glyph)) {
                GrDrawOpAtlas::ErrorCode code;
                code = grStrike->addGlyphToAtlas(fResourceProvider, fUploadTarget, fGrStrikeCache,
                                                 fFullAtlasManager, glyph,
                                                 fMetricsAndImages.get(), fSubRun->maskFormat(),
                                                 fSubRun->needsTransform());
                if (GrDrawOpAtlas::ErrorCode::kError == code) {
                    // Something horrible has happened - drop the op
                    return false;
                }
                else if (GrDrawOpAtlas::ErrorCode::kTryAgain == code) {
                    fBrokenRun = glyphIdx > 0;
                    result->fFinished = false;
                    return true;
                }
            }
            auto tokenTracker = fUploadTarget->tokenTracker();
            fFullAtlasManager->addGlyphToBulkAndSetUseToken(fSubRun->bulkUseToken(), glyph,
                                                            tokenTracker->nextDrawToken());
        }

        if (regenPos) {
            regen_positions(currVertex, vertexStride, fTransX, fTransY);
        }
        if (regenCol) {
            regen_colors(currVertex, vertexStride, fColor);
        }
        if (regenTexCoords) {
            regen_texcoords(currVertex, vertexStride, glyph, fSubRun->drawAsDistanceFields());
        }

        currVertex += vertexStride * GrAtlasTextOp::kVerticesPerGlyph;
        ++result->fGlyphsRegenerated;
        ++fCurrGlyph;
    }

    // We may have changed the color so update it here
    fSubRun->setColor(fColor);
    if (regenTexCoords) {
        fSubRun->setAtlasGeneration(fBrokenRun
                                    ? GrDrawOpAtlas::kInvalidAtlasGeneration
                                    : fFullAtlasManager->atlasGeneration(fSubRun->maskFormat()));
    } else {
        // For the non-texCoords case we need to ensure that we update the associated use tokens
        fFullAtlasManager->setUseTokenBulk(*fSubRun->bulkUseToken(),
                                           fUploadTarget->tokenTracker()->nextDrawToken(),
                                           fSubRun->maskFormat());
    }
    return true;
}

bool GrTextBlob::VertexRegenerator::regenerate(GrTextBlob::VertexRegenerator::Result* result) {
    uint64_t currentAtlasGen = fFullAtlasManager->atlasGeneration(fSubRun->maskFormat());
    // If regenerate() is called multiple times then the atlas gen may have changed. So we check
    // this each time.
    if (fSubRun->atlasGeneration() != currentAtlasGen) {
        fRegenFlags |= kRegenTex;
    }

    if (fRegenFlags) {
        return this->doRegen(result,
                             fRegenFlags & kRegenPos,
                             fRegenFlags & kRegenCol,
                             fRegenFlags & kRegenTex,
                             fRegenFlags & kRegenGlyph);
    } else {
        bool hasW = fSubRun->hasW();
        auto vertexStride = GetVertexStride(fSubRun->maskFormat(), hasW);
        result->fFinished = true;
        result->fGlyphsRegenerated = fSubRun->fGlyphCount - fCurrGlyph;
        result->fFirstVertex = fSubRun->vertices() + fCurrGlyph * kVerticesPerGlyph * vertexStride;
        fCurrGlyph = fSubRun->fGlyphCount;

        // set use tokens for all of the glyphs in our subrun.  This is only valid if we
        // have a valid atlas generation
        fFullAtlasManager->setUseTokenBulk(*fSubRun->bulkUseToken(),
                                           fUploadTarget->tokenTracker()->nextDrawToken(),
                                           fSubRun->maskFormat());
        return true;
    }
    SK_ABORT("Should not get here");
}

