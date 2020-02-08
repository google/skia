/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/gpu/GrContext.h"
#include "include/private/SkTemplates.h"
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

#include <cstddef>
#include <new>

static SkMatrix make_inverse(const SkMatrix& matrix) {
    SkMatrix inverseMatrix;
    if (!matrix.invert(&inverseMatrix)) {
        inverseMatrix = SkMatrix::I();
    }
    return inverseMatrix;
}

// -- GrTextBlob::Key ------------------------------------------------------------------------------
GrTextBlob::Key::Key() { sk_bzero(this, sizeof(Key)); }

bool GrTextBlob::Key::operator==(const GrTextBlob::Key& other) const {
    return 0 == memcmp(this, &other, sizeof(Key));
}

// -- GrTextBlob::PathGlyph ------------------------------------------------------------------------
GrTextBlob::PathGlyph::PathGlyph(const SkPath& path, SkPoint origin)
        : fPath(path)
        , fOrigin(origin) {}

// -- GrTextBlob::SubRun ---------------------------------------------------------------------------
GrTextBlob::SubRun::SubRun(SubRunType type, GrTextBlob* textBlob, const SkStrikeSpec& strikeSpec,
                           GrMaskFormat format,
                           const SkSpan<GrGlyph*>& glyphs, const SkSpan<char>& vertexData,
                           sk_sp<GrTextStrike>&& grStrike)
        : fType{type}
        , fBlob{textBlob}
        , fMaskFormat{format}
        , fGlyphs{glyphs}
        , fVertexData{vertexData}
        , fStrikeSpec{strikeSpec}
        , fStrike{grStrike}
        , fCurrentColor{textBlob->fColor}
        , fCurrentOrigin{textBlob->fInitialOrigin}
        , fCurrentMatrix{textBlob->fInitialMatrix} {
    SkASSERT(type != kTransformedPath);
    textBlob->insertSubRun(this);
}

GrTextBlob::SubRun::SubRun(GrTextBlob* textBlob, const SkStrikeSpec& strikeSpec)
        : fType{kTransformedPath}
        , fBlob{textBlob}
        , fMaskFormat{kA8_GrMaskFormat}
        , fGlyphs{SkSpan<GrGlyph*>{}}
        , fVertexData{SkSpan<char>{}}
        , fStrikeSpec{strikeSpec}
        , fStrike{nullptr}
        , fCurrentColor{textBlob->fColor}
        , fPaths{} {
    textBlob->insertSubRun(this);
}

void GrTextBlob::SubRun::appendGlyphs(const SkZip<SkGlyphVariant, SkPoint>& drawables) {
    GrTextStrike* grStrike = fStrike.get();
    SkScalar strikeToSource = fStrikeSpec.strikeToSourceRatio();
    GrGlyph** glyphCursor = fGlyphs.data();
    char* vertexCursor = fVertexData.data();
    size_t vertexStride = this->vertexStride();
    // We always write the third position component used by SDFs. If it is unused it gets
    // overwritten. Similarly, we always write the color and the blob will later overwrite it
    // with texture coords if it is unused.
    size_t colorOffset = this->colorOffset();
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
        *reinterpret_cast<GrColor*>(vertexCursor + colorOffset) = fCurrentColor;
        vertexCursor += vertexStride;

        // V1
        *reinterpret_cast<SkPoint3*>(vertexCursor) = {dstRect.fLeft, dstRect.fBottom, 1.f};
        *reinterpret_cast<GrColor*>(vertexCursor + colorOffset) = fCurrentColor;
        vertexCursor += vertexStride;

        // V2
        *reinterpret_cast<SkPoint3*>(vertexCursor) = {dstRect.fRight, dstRect.fTop, 1.f};
        *reinterpret_cast<GrColor*>(vertexCursor + colorOffset) = fCurrentColor;
        vertexCursor += vertexStride;

        // V3
        *reinterpret_cast<SkPoint3*>(vertexCursor) = {dstRect.fRight, dstRect.fBottom, 1.f};
        *reinterpret_cast<GrColor*>(vertexCursor + colorOffset) = fCurrentColor;
        vertexCursor += vertexStride;

        *glyphCursor++ = grGlyph;
    }
}

void GrTextBlob::SubRun::resetBulkUseToken() { fBulkUseToken.reset(); }

GrDrawOpAtlas::BulkUseTokenUpdater* GrTextBlob::SubRun::bulkUseToken() { return &fBulkUseToken; }
void GrTextBlob::SubRun::setStrike(sk_sp<GrTextStrike> strike) { fStrike = std::move(strike); }
GrTextStrike* GrTextBlob::SubRun::strike() const { return fStrike.get(); }
GrMaskFormat GrTextBlob::SubRun::maskFormat() const { return fMaskFormat; }
size_t GrTextBlob::SubRun::vertexStride() const {
    return GetVertexStride(this->maskFormat(), this->hasW());
}
size_t GrTextBlob::SubRun::colorOffset() const {
    return this->hasW() ? offsetof(SDFT3DVertex, color) : offsetof(Mask2DVertex, color);
}

size_t GrTextBlob::SubRun::texCoordOffset() const {
    switch (fMaskFormat) {
        case kA8_GrMaskFormat:
            return this->hasW() ? offsetof(SDFT3DVertex, atlasPos)
                                : offsetof(Mask2DVertex, atlasPos);
        case kARGB_GrMaskFormat:
            return this->hasW() ? offsetof(ARGB3DVertex, atlasPos)
                                : offsetof(ARGB2DVertex, atlasPos);
        default:
            SkASSERT(!this->hasW());
            return offsetof(Mask2DVertex, atlasPos);
    }
}

char* GrTextBlob::SubRun::quadStart(size_t index) const {
    return SkTAddOffset<char>(fVertexData.data(), this->quadOffset(index));
}

size_t GrTextBlob::SubRun::quadOffset(size_t index) const {
    return index * kVerticesPerGlyph * this->vertexStride();
}

const SkRect& GrTextBlob::SubRun::vertexBounds() const { return fVertexBounds; }
void GrTextBlob::SubRun::joinGlyphBounds(const SkRect& glyphBounds) {
    fVertexBounds.joinNonEmptyArg(glyphBounds);
}

bool GrTextBlob::SubRun::drawAsDistanceFields() const { return fType == kTransformedSDFT; }

bool GrTextBlob::SubRun::drawAsPaths() const { return fType == kTransformedPath; }

bool GrTextBlob::SubRun::needsTransform() const {
    return fType == kTransformedPath
           || fType == kTransformedMask
           || fType == kTransformedSDFT;
}

bool GrTextBlob::SubRun::hasW() const {
    return fBlob->hasW(fType);
}

void GrTextBlob::SubRun::translateVerticesIfNeeded(
        const SkMatrix& drawMatrix, SkPoint drawOrigin) {
    SkVector translation;
    if (this->needsTransform()) {
        // If transform is needed, then the vertices are in source space, calculate the source
        // space translation.
        translation = drawOrigin - fCurrentOrigin;
    } else {
        // Calculate the translation in source space to a translation in device space. Calculate
        // the translation by mapping (0, 0) through both the current matrix, and the draw
        // matrix, and taking the difference.
        SkMatrix currentMatrix{fCurrentMatrix};
        currentMatrix.preTranslate(fCurrentOrigin.x(), fCurrentOrigin.y());
        SkPoint currentDeviceOrigin{0, 0};
        currentMatrix.mapPoints(&currentDeviceOrigin, 1);
        SkMatrix completeDrawMatrix{drawMatrix};
        completeDrawMatrix.preTranslate(drawOrigin.x(), drawOrigin.y());
        SkPoint drawDeviceOrigin{0, 0};
        completeDrawMatrix.mapPoints(&drawDeviceOrigin, 1);
        translation = drawDeviceOrigin - currentDeviceOrigin;
    }

    if (translation != SkPoint{0, 0}) {
        size_t vertexStride = this->vertexStride();
        for (size_t quad = 0; quad < fGlyphs.size(); quad++) {
            SkPoint* vertexCursor = reinterpret_cast<SkPoint*>(quadStart(quad));
            for (int i = 0; i < 4; ++i) {
                *vertexCursor += translation;
                vertexCursor = SkTAddOffset<SkPoint>(vertexCursor, vertexStride);
            }
        }
        fCurrentMatrix = drawMatrix;
        fCurrentOrigin = drawOrigin;
    }
}

void GrTextBlob::SubRun::updateVerticesColorIfNeeded(GrColor newColor) {
    if (this->maskFormat() != kARGB_GrMaskFormat && fCurrentColor != newColor) {
        size_t vertexStride = this->vertexStride();
        size_t colorOffset = this->colorOffset();
        for (size_t quad = 0; quad < fGlyphs.size(); quad++) {
            GrColor* colorCursor = SkTAddOffset<GrColor>(quadStart(quad), colorOffset);
            for (int i = 0; i < 4; ++i) {
                *colorCursor = newColor;
                colorCursor = SkTAddOffset<GrColor>(colorCursor, vertexStride);
            }
        }
        this->fCurrentColor = newColor;
    }
}

void GrTextBlob::SubRun::updateTexCoords(int begin, int end) {
    const size_t vertexStride = this->vertexStride();
    const size_t texCoordOffset = this->texCoordOffset();
    char* vertex = this->quadStart(begin);
    uint16_t* textureCoords = reinterpret_cast<uint16_t*>(vertex + texCoordOffset);
    for (int i = begin; i < end; i++) {
        GrGlyph* glyph = this->fGlyphs[i];
        SkASSERT(glyph != nullptr);

        int width = glyph->fBounds.width();
        int height = glyph->fBounds.height();
        uint16_t u0, v0, u1, v1;
        if (this->drawAsDistanceFields()) {
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
        std::tie(u0, v0) = GrDrawOpAtlas::PackIndexInTexCoords(u0, v0, pageIndex);
        std::tie(u1, v1) = GrDrawOpAtlas::PackIndexInTexCoords(u1, v1, pageIndex);

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
        textureCoords = SkTAddOffset<uint16_t>(textureCoords, vertexStride);
    }
}


void GrTextBlob::SubRun::setUseLCDText(bool useLCDText) { fFlags.useLCDText = useLCDText; }
bool GrTextBlob::SubRun::hasUseLCDText() const { return fFlags.useLCDText; }
void GrTextBlob::SubRun::setAntiAliased(bool antiAliased) { fFlags.antiAliased = antiAliased; }
bool GrTextBlob::SubRun::isAntiAliased() const { return fFlags.antiAliased; }
const SkStrikeSpec& GrTextBlob::SubRun::strikeSpec() const { return fStrikeSpec; }

// -- GrTextBlob -----------------------------------------------------------------------------------
void GrTextBlob::operator delete(void* p) { ::operator delete(p); }
void* GrTextBlob::operator new(size_t) { SK_ABORT("All blobs are created by placement new."); }
void* GrTextBlob::operator new(size_t, void* p) { return p; }

GrTextBlob::~GrTextBlob() = default;

sk_sp<GrTextBlob> GrTextBlob::Make(const SkGlyphRunList& glyphRunList,
                                   GrStrikeCache* strikeCache,
                                   const SkMatrix& drawMatrix,
                                   GrColor color,
                                   bool forceWForDistanceFields) {

    static_assert(sizeof(ARGB2DVertex) <= sizeof(Mask2DVertex));
    static_assert(alignof(ARGB2DVertex) <= alignof(Mask2DVertex));
    size_t quadSize = sizeof(Mask2DVertex) * kVerticesPerGlyph;
    if (drawMatrix.hasPerspective() || forceWForDistanceFields) {
        static_assert(sizeof(ARGB3DVertex) <= sizeof(SDFT3DVertex));
        static_assert(alignof(ARGB3DVertex) <= alignof(SDFT3DVertex));
        quadSize = sizeof(SDFT3DVertex) * kVerticesPerGlyph;
    }

    // We can use the alignment of SDFT3DVertex as a proxy for all Vertex alignments.
    static_assert(alignof(SDFT3DVertex) >= alignof(Mask2DVertex));
    // Assume there is no padding needed between glyph pointers and vertices.
    static_assert(alignof(GrGlyph*) >= alignof(SDFT3DVertex));

    // In the arena, the layout is GrGlyph*... | SDFT3DVertex... | SubRun, so there is no padding
    // between GrGlyph* and SDFT3DVertex, but padding is needed between the Mask2DVertex array
    // and the SubRun.
    size_t vertexToSubRunPadding = alignof(SDFT3DVertex) - alignof(SubRun);
    size_t arenaSize =
            sizeof(GrGlyph*) * glyphRunList.totalGlyphCount()
          + quadSize * glyphRunList.totalGlyphCount()
          + glyphRunList.runCount() * (sizeof(SubRun) + vertexToSubRunPadding);

    size_t allocationSize = sizeof(GrTextBlob) + arenaSize;

    void* allocation = ::operator new (allocationSize);

    SkColor initialLuminance = SkPaintPriv::ComputeLuminanceColor(glyphRunList.paint());
    sk_sp<GrTextBlob> blob{new (allocation) GrTextBlob{
            arenaSize, strikeCache, drawMatrix, glyphRunList.origin(),
            color, initialLuminance, forceWForDistanceFields}};

    return blob;
}

void GrTextBlob::setupKey(const GrTextBlob::Key& key, const SkMaskFilterBase::BlurRec& blurRec,
                          const SkPaint& paint) {
    fKey = key;
    if (key.fHasBlur) {
        fBlurRec = blurRec;
    }
    if (key.fStyle != SkPaint::kFill_Style) {
        fStrokeInfo.fFrameWidth = paint.getStrokeWidth();
        fStrokeInfo.fMiterLimit = paint.getStrokeMiter();
        fStrokeInfo.fJoin = paint.getStrokeJoin();
    }
}
const GrTextBlob::Key& GrTextBlob::GetKey(const GrTextBlob& blob) { return blob.fKey; }
uint32_t GrTextBlob::Hash(const GrTextBlob::Key& key) { return SkOpts::hash(&key, sizeof(Key)); }

bool GrTextBlob::hasDistanceField() const {
    return SkToBool(fTextType & kHasDistanceField_TextType);
}
bool GrTextBlob::hasBitmap() const { return SkToBool(fTextType & kHasBitmap_TextType); }
bool GrTextBlob::hasPerspective() const { return fInitialMatrix.hasPerspective(); }

void GrTextBlob::setHasDistanceField() { fTextType |= kHasDistanceField_TextType; }
void GrTextBlob::setHasBitmap() { fTextType |= kHasBitmap_TextType; }
void GrTextBlob::setMinAndMaxScale(SkScalar scaledMin, SkScalar scaledMax) {
    // we init fMaxMinScale and fMinMaxScale in the constructor
    fMaxMinScale = std::max(scaledMin, fMaxMinScale);
    fMinMaxScale = std::min(scaledMax, fMinMaxScale);
}

size_t GrTextBlob::GetVertexStride(GrMaskFormat maskFormat, bool hasWCoord) {
    switch (maskFormat) {
        case kA8_GrMaskFormat:
            return hasWCoord ? sizeof(SDFT3DVertex) : sizeof(Mask2DVertex);
        case kARGB_GrMaskFormat:
            return hasWCoord ? sizeof(ARGB3DVertex) : sizeof(ARGB2DVertex);
        default:
            SkASSERT(!hasWCoord);
            return sizeof(Mask2DVertex);
    }
}

bool GrTextBlob::mustRegenerate(const SkPaint& paint, bool anyRunHasSubpixelPosition,
                                const SkMaskFilterBase::BlurRec& blurRec,
                                const SkMatrix& drawMatrix, SkPoint drawOrigin) {
    // If we have LCD text then our canonical color will be set to transparent, in this case we have
    // to regenerate the blob on any color change
    // We use the grPaint to get any color filter effects
    if (fKey.fCanonicalColor == SK_ColorTRANSPARENT &&
        fInitialLuminance != SkPaintPriv::ComputeLuminanceColor(paint)) {
        return true;
    }

    if (fInitialMatrix.hasPerspective() != drawMatrix.hasPerspective()) {
        return true;
    }

    /** This could be relaxed for blobs with only distance field glyphs. */
    if (fInitialMatrix.hasPerspective() && !SkMatrixPriv::CheapEqual(fInitialMatrix, drawMatrix)) {
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
        return !(SkMatrixPriv::CheapEqual(fInitialMatrix, drawMatrix) &&
                 drawOrigin == fInitialOrigin);
    }

    if (this->hasBitmap()) {
        if (fInitialMatrix.getScaleX() != drawMatrix.getScaleX() ||
            fInitialMatrix.getScaleY() != drawMatrix.getScaleY() ||
            fInitialMatrix.getSkewX() != drawMatrix.getSkewX() ||
            fInitialMatrix.getSkewY() != drawMatrix.getSkewY()) {
            return true;
        }

        // TODO(herb): this is not needed for full pixel glyph choice, but is needed to adjust
        //  the quads properly. Devise a system that regenerates the quads from original data
        //  using the transform to allow this to be used in general.

        // We can update the positions in the text blob without regenerating the whole
        // blob, but only for integer translations.
        // Calculate the translation in source space to a translation in device space by mapping
        // (0, 0) through both the initial matrix and the draw matrix; take the difference.
        SkMatrix initialMatrix{fInitialMatrix};
        initialMatrix.preTranslate(fInitialOrigin.x(), fInitialOrigin.y());
        SkPoint initialDeviceOrigin{0, 0};
        initialMatrix.mapPoints(&initialDeviceOrigin, 1);
        SkMatrix completeDrawMatrix{drawMatrix};
        completeDrawMatrix.preTranslate(drawOrigin.x(), drawOrigin.y());
        SkPoint drawDeviceOrigin{0, 0};
        completeDrawMatrix.mapPoints(&drawDeviceOrigin, 1);
        SkPoint translation = drawDeviceOrigin - initialDeviceOrigin;

        if (!SkScalarIsInt(translation.x()) || !SkScalarIsInt(translation.y())) {
            return true;
        }
    } else if (this->hasDistanceField()) {
        // A scale outside of [blob.fMaxMinScale, blob.fMinMaxScale] would result in a different
        // distance field being generated, so we have to regenerate in those cases
        SkScalar newMaxScale = drawMatrix.getMaxScale();
        SkScalar oldMaxScale = fInitialMatrix.getMaxScale();
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

void GrTextBlob::flush(GrTextTarget* target, const SkSurfaceProps& props,
                       const GrDistanceFieldAdjustTable* distanceAdjustTable,
                       const SkPaint& paint, const SkPMColor4f& filteredColor, const GrClip& clip,
                       const SkMatrix& drawMatrix, SkPoint drawOrigin) {

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


            for (const auto& pathGlyph : subRun->fPaths) {
                SkMatrix ctm{drawMatrix};
                ctm.preTranslate(drawOrigin.x(), drawOrigin.y());
                SkMatrix pathMatrix = SkMatrix::MakeScale(
                        subRun->fStrikeSpec.strikeToSourceRatio());
                pathMatrix.postTranslate(pathGlyph.fOrigin.x(), pathGlyph.fOrigin.y());

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
            int glyphCount = subRun->fGlyphs.size();
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
                this->computeSubRunBounds(
                        &subRunBounds, *subRun, drawMatrix, drawOrigin, false);
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
                auto op = this->makeOp(*subRun, glyphCount, drawMatrix, drawOrigin,
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

void GrTextBlob::computeSubRunBounds(SkRect* outBounds, const SubRun& subRun,
                                     const SkMatrix& drawMatrix, SkPoint drawOrigin,
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
        outBounds->offset(drawOrigin - fInitialOrigin);
        drawMatrix.mapRect(outBounds);
    } else {
        // Bitmap text is fully positioned on the CPU, and offset by an (X,Y) translate in
        // device space.
        SkMatrix boundsMatrix = fInitialMatrixInverse;

        boundsMatrix.postTranslate(-fInitialOrigin.x(), -fInitialOrigin.y());

        boundsMatrix.postTranslate(drawOrigin.x(), drawOrigin.y());

        boundsMatrix.postConcat(drawMatrix);
        boundsMatrix.mapRect(outBounds);

        // Due to floating point numerical inaccuracies, we have to round out here
        outBounds->roundOut(outBounds);
    }
}

const GrTextBlob::Key& GrTextBlob::key() const { return fKey; }
size_t GrTextBlob::size() const { return fSize; }

std::unique_ptr<GrDrawOp> GrTextBlob::test_makeOp(
        int glyphCount, const SkMatrix& drawMatrix,
        SkPoint drawOrigin, const SkPaint& paint, const SkPMColor4f& filteredColor,
        const SkSurfaceProps& props, const GrDistanceFieldAdjustTable* distanceAdjustTable,
        GrTextTarget* target) {
    SubRun* info = fFirstSubRun;
    SkIRect emptyRect = SkIRect::MakeEmpty();
    return this->makeOp(*info, glyphCount, drawMatrix, drawOrigin, emptyRect,
                        paint, filteredColor, props, distanceAdjustTable, target);
}

bool GrTextBlob::hasW(GrTextBlob::SubRunType type) const {
    if (type == kTransformedSDFT) {
        return this->hasPerspective() || fForceWForDistanceFields;
    } else if (type == kTransformedMask || type == kTransformedPath) {
        return this->hasPerspective();
    }

    // The viewMatrix is implicitly SkMatrix::I when drawing kDirectMask, because it is not
    // used.
    return false;
}

GrTextBlob::SubRun* GrTextBlob::makeSubRun(SubRunType type,
                                           const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                           const SkStrikeSpec& strikeSpec,
                                           GrMaskFormat format) {
    SkSpan<GrGlyph*> glyphs{fAlloc.makeArrayDefault<GrGlyph*>(drawables.size()), drawables.size()};
    bool hasW = this->hasW(type);

    SkASSERT(!fInitialMatrix.hasPerspective() || hasW);

    size_t vertexDataSize = drawables.size() * GetVertexStride(format, hasW) * kVerticesPerGlyph;
    SkSpan<char> vertexData{fAlloc.makeArrayDefault<char>(vertexDataSize), vertexDataSize};

    sk_sp<GrTextStrike> grStrike = strikeSpec.findOrCreateGrStrike(fStrikeCache);

    SubRun* subRun = fAlloc.make<SubRun>(
            type, this, strikeSpec, format, glyphs, vertexData, std::move(grStrike));

    subRun->appendGlyphs(drawables);

    return subRun;
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

GrTextBlob::GrTextBlob(size_t allocSize,
                       GrStrikeCache* strikeCache,
                       const SkMatrix& drawMatrix,
                       SkPoint origin,
                       GrColor color,
                       SkColor initialLuminance,
                       bool forceWForDistanceFields)
        : fSize{allocSize}
        , fStrikeCache{strikeCache}
        , fInitialMatrix{drawMatrix}
        , fInitialMatrixInverse{make_inverse(drawMatrix)}
        , fInitialOrigin{origin}
        , fForceWForDistanceFields{forceWForDistanceFields}
        , fColor{color}
        , fInitialLuminance{initialLuminance}
        , fAlloc{SkTAddOffset<char>(this, sizeof(GrTextBlob)), allocSize, allocSize/2} { }

void GrTextBlob::insertSubRun(SubRun* subRun) {
    if (fFirstSubRun == nullptr) {
        fFirstSubRun = subRun;
        fLastSubRun = subRun;
    } else {
        fLastSubRun->fNextSubRun = subRun;
        fLastSubRun = subRun;
    }
}

std::unique_ptr<GrAtlasTextOp> GrTextBlob::makeOp(
        SubRun& info, int glyphCount,
        const SkMatrix& drawMatrix, SkPoint drawOrigin, const SkIRect& clipRect,
        const SkPaint& paint, const SkPMColor4f& filteredColor, const SkSurfaceProps& props,
        const GrDistanceFieldAdjustTable* distanceAdjustTable, GrTextTarget* target) {
    GrMaskFormat format = info.maskFormat();

    GrPaint grPaint;
    target->makeGrPaint(info.maskFormat(), paint, drawMatrix, &grPaint);
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
    geometry.fDrawMatrix = drawMatrix;
    geometry.fClipRect = clipRect;
    geometry.fBlob = SkRef(this);
    geometry.fSubRunPtr = &info;
    geometry.fColor = info.maskFormat() == kARGB_GrMaskFormat ? SK_PMColor4fWHITE : filteredColor;
    geometry.fDrawOrigin = drawOrigin;
    op->init();
    return op;
}

void GrTextBlob::processDeviceMasks(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                    const SkStrikeSpec& strikeSpec) {
    this->addMultiMaskFormat(kDirectMask, drawables, strikeSpec);
}

void GrTextBlob::processSourcePaths(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                    const SkFont& runFont,
                                    const SkStrikeSpec& strikeSpec) {
    this->setHasBitmap();
    SubRun* subRun = fAlloc.make<SubRun>(this, strikeSpec);
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

// -- GrTextBlob::VertexRegenerator ----------------------------------------------------------------
GrTextBlob::VertexRegenerator::VertexRegenerator(GrResourceProvider* resourceProvider,
                                                 GrTextBlob::SubRun* subRun,
                                                 GrDeferredUploadTarget* uploadTarget,
                                                 GrAtlasManager* fullAtlasManager)
        : fResourceProvider(resourceProvider)
        , fUploadTarget(uploadTarget)
        , fFullAtlasManager(fullAtlasManager)
        , fSubRun(subRun) { }

std::tuple<bool, int> GrTextBlob::VertexRegenerator::updateTextureCoordinates(
        const int begin, const int end) {

    const SkStrikeSpec& strikeSpec = fSubRun->strikeSpec();

    if (!fMetricsAndImages.isValid()
            || fMetricsAndImages->descriptor() != strikeSpec.descriptor()) {
        fMetricsAndImages.init(strikeSpec);
    }

    // Update the atlas information in the GrStrike.
    auto code = GrDrawOpAtlas::ErrorCode::kSucceeded;
    GrTextStrike* grStrike = fSubRun->strike();
    auto tokenTracker = fUploadTarget->tokenTracker();
    int i = begin;
    for (; i < end; i++) {
        GrGlyph* grGlyph = fSubRun->fGlyphs[i];
        SkASSERT(grGlyph && grGlyph->fMaskFormat == fSubRun->maskFormat());

        if (!fFullAtlasManager->hasGlyph(grGlyph)) {
            const SkGlyph& skGlyph = *fMetricsAndImages->glyph(grGlyph->fPackedID);
            if (skGlyph.image() == nullptr) { return {false, 0}; }
            code = grStrike->addGlyphToAtlas(skGlyph,
                    fSubRun->maskFormat(),
                    fSubRun->needsTransform(),
                    fResourceProvider, fUploadTarget, fFullAtlasManager, grGlyph);
            if (code != GrDrawOpAtlas::ErrorCode::kSucceeded) {
                break;
            }
        }
        fFullAtlasManager->addGlyphToBulkAndSetUseToken(
                fSubRun->bulkUseToken(), grGlyph, tokenTracker->nextDrawToken());
    }
    int glyphsPlacedInAtlas = i - begin;

    // Update the quads with the new atlas coordinates.
    fSubRun->updateTexCoords(begin, begin + glyphsPlacedInAtlas);

    return {code != GrDrawOpAtlas::ErrorCode::kError, glyphsPlacedInAtlas};
}

std::tuple<bool, int> GrTextBlob::VertexRegenerator::regenerate(int begin, int end) {
    uint64_t currentAtlasGen = fFullAtlasManager->atlasGeneration(fSubRun->maskFormat());

    if (fSubRun->fAtlasGeneration != currentAtlasGen) {
        // Calculate the texture coordinates for the vertexes during first use (fAtlasGeneration
        // is set to kInvalidAtlasGeneration) or the atlas has changed in subsequent calls..
        fSubRun->resetBulkUseToken();
        auto [success, glyphsPlacedInAtlas] = this->updateTextureCoordinates(begin, end);

        // Update atlas generation if there are no more glyphs to put in the atlas.
        if (success && begin + glyphsPlacedInAtlas == (int)fSubRun->fGlyphs.size()) {
            // Need to get the freshest value of the atlas' generation because
            // updateTextureCoordinates may have changed it.
            fSubRun->fAtlasGeneration = fFullAtlasManager->atlasGeneration(fSubRun->maskFormat());
        }
        return {success, glyphsPlacedInAtlas};
    } else {
        // The atlas hasn't changed, so our texture coordinates are still valid.
        if (end == (int)fSubRun->fGlyphs.size()) {
            // The atlas hasn't changed and the texture coordinates are all still valid. Update
            // all the plots used to the new use token.
            fFullAtlasManager->setUseTokenBulk(*fSubRun->bulkUseToken(),
                                               fUploadTarget->tokenTracker()->nextDrawToken(),
                                               fSubRun->maskFormat());
        }
        return {true, end - begin};
    }
}
