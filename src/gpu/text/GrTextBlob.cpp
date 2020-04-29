/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/gpu/GrContext.h"
#include "include/private/SkTemplates.h"
#include "src/codec/SkMasks.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkPaintPriv.h"
#include "src/gpu/GrBlurUtils.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/geometry/GrStyledShape.h"
#include "src/gpu/ops/GrAtlasTextOp.h"
#include "src/gpu/text/GrAtlasManager.h"
#include "src/gpu/text/GrStrikeCache.h"
#include "src/gpu/text/GrTextBlob.h"
#include "src/gpu/text/GrTextTarget.h"

#include <cstddef>
#include <new>


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
                           GrMaskFormat format, const SkSpan<PackedGlyphIDorGrGlyph>& glyphs,
                           const SkSpan<char>& vertexData)
        : fType{type}
        , fBlob{textBlob}
        , fMaskFormat{format}
        , fGlyphs{glyphs}
        , fVertexData{vertexData}
        , fStrikeSpec{strikeSpec}
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
        , fGlyphs{SkSpan<PackedGlyphIDorGrGlyph>{}}
        , fVertexData{SkSpan<char>{}}
        , fStrikeSpec{strikeSpec}
        , fCurrentColor{textBlob->fColor}
        , fPaths{} {
    textBlob->insertSubRun(this);
}

static SkRect dest_rect(const SkGlyph& g, SkPoint origin) {
    return SkRect::MakeXYWH(
            SkIntToScalar(g.left()) + origin.x(),
            SkIntToScalar(g.top())  + origin.y(),
            SkIntToScalar(g.width()),
            SkIntToScalar(g.height()));
}

static bool is_SDF(const SkGlyph& skGlyph) {
    return skGlyph.maskFormat() == SkMask::kSDF_Format;
}

static SkRect dest_rect(const SkGlyph& g, SkPoint origin, SkScalar textScale) {
    if (!is_SDF(g)) {
        return SkRect::MakeXYWH(
                SkIntToScalar(g.left())   * textScale + origin.x(),
                SkIntToScalar(g.top())    * textScale + origin.y(),
                SkIntToScalar(g.width())  * textScale,
                SkIntToScalar(g.height()) * textScale);
    } else {
        return SkRect::MakeXYWH(
                (SkIntToScalar(g.left()) + SK_DistanceFieldInset) * textScale + origin.x(),
                (SkIntToScalar(g.top())  + SK_DistanceFieldInset) * textScale + origin.y(),
                (SkIntToScalar(g.width())  - 2 * SK_DistanceFieldInset) * textScale,
                (SkIntToScalar(g.height()) - 2 * SK_DistanceFieldInset) * textScale);
    }
}

void GrTextBlob::SubRun::appendGlyphs(const SkZip<SkGlyphVariant, SkPoint>& drawables) {
    SkScalar strikeToSource = fStrikeSpec.strikeToSourceRatio();
    SkASSERT(!this->isPrepared());
    PackedGlyphIDorGrGlyph* packedIDCursor = fGlyphs.data();
    char* vertexCursor = fVertexData.data();
    size_t vertexStride = this->vertexStride();
    // We always write the third position component used by SDFs. If it is unused it gets
    // overwritten. Similarly, we always write the color and the blob will later overwrite it
    // with texture coords if it is unused.
    size_t colorOffset = this->colorOffset();
    for (auto [variant, pos] : drawables) {
        SkGlyph* skGlyph = variant;
        // Only floor the device coordinates.
        SkRect dstRect;
        if (!this->needsTransform()) {
            pos = {SkScalarFloorToScalar(pos.x()), SkScalarFloorToScalar(pos.y())};
            dstRect = dest_rect(*skGlyph, pos);
        } else {
            dstRect = dest_rect(*skGlyph, pos, strikeToSource);
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

        packedIDCursor->fPackedGlyphID = skGlyph->getPackedID();
        packedIDCursor++;
    }

    // Use the negative initial origin to make the fVertexBounds {0, 0} based.
    SkPoint pt = fBlob->fInitialOrigin;
    if (!this->needsTransform()) {
        // If the box is in device space, then transform the source space origin to device space.
        pt = fBlob->fInitialMatrix.mapXY(pt.x(), pt.y());
    }
    fVertexBounds.offset(-pt);
}

void GrTextBlob::SubRun::resetBulkUseToken() { fBulkUseToken.reset(); }

GrDrawOpAtlas::BulkUseTokenUpdater* GrTextBlob::SubRun::bulkUseToken() { return &fBulkUseToken; }
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

void GrTextBlob::SubRun::joinGlyphBounds(const SkRect& glyphBounds) {
    fVertexBounds.joinNonEmptyArg(glyphBounds);
}

bool GrTextBlob::SubRun::drawAsDistanceFields() const { return fType == kTransformedSDFT; }

bool GrTextBlob::SubRun::drawAsPaths() const { return fType == kTransformedPath; }

bool GrTextBlob::SubRun::needsTransform() const {
    return fType == kTransformedPath ||
           fType == kTransformedMask ||
           fType == kTransformedSDFT;
}

bool GrTextBlob::SubRun::needsPadding() const {
    return fType == kTransformedPath || fType == kTransformedMask;
}

bool GrTextBlob::SubRun::hasW() const {
    return fBlob->hasW(fType);
}

void GrTextBlob::SubRun::prepareGrGlyphs(GrStrikeCache* strikeCache) {
    if (fStrike) {
        return;
    }

    fStrike = fStrikeSpec.findOrCreateGrStrike(strikeCache);

    for (auto& tmp : fGlyphs) {
        tmp.fGrGlyph = fStrike->getGlyph(tmp.fPackedGlyphID);
    }
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
    SkASSERT(this->isPrepared());

    const size_t vertexStride = this->vertexStride();
    const size_t texCoordOffset = this->texCoordOffset();
    char* vertex = this->quadStart(begin);
    uint16_t* textureCoords = reinterpret_cast<uint16_t*>(vertex + texCoordOffset);
    for (int i = begin; i < end; i++) {
        GrGlyph* glyph = this->fGlyphs[i].fGrGlyph;
        SkASSERT(glyph != nullptr);

        int pad = this->drawAsDistanceFields() ? SK_DistanceFieldInset
                                               : (this->needsPadding() ? 1 : 0);
        std::array<uint16_t, 4> uvs = glyph->fAtlasLocator.getUVs(pad);

        textureCoords[0] = uvs[0];
        textureCoords[1] = uvs[1];
        textureCoords = SkTAddOffset<uint16_t>(textureCoords, vertexStride);
        textureCoords[0] = uvs[0];
        textureCoords[1] = uvs[3];
        textureCoords = SkTAddOffset<uint16_t>(textureCoords, vertexStride);
        textureCoords[0] = uvs[2];
        textureCoords[1] = uvs[1];
        textureCoords = SkTAddOffset<uint16_t>(textureCoords, vertexStride);
        textureCoords[0] = uvs[2];
        textureCoords[1] = uvs[3];
        textureCoords = SkTAddOffset<uint16_t>(textureCoords, vertexStride);
    }
}

SkRect GrTextBlob::SubRun::deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const {
    SkRect outBounds = fVertexBounds;
    if (this->needsTransform()) {
        // if the glyph needs transformation offset the by the new origin, and map to device space.
        outBounds.offset(drawOrigin);
        outBounds = drawMatrix.mapRect(outBounds);
    } else {
        SkPoint offset = drawMatrix.mapXY(drawOrigin.x(), drawOrigin.y());
        // The vertex bounds are already {0, 0} based, so just add the new origin offset.
        outBounds.offset(offset);

        // Due to floating point numerical inaccuracies, we have to round out here
        outBounds.roundOut();
    }
    return outBounds;
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
            arenaSize, drawMatrix, glyphRunList.origin(),
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

void GrTextBlob::addOp(GrTextTarget* target,
                       const SkSurfaceProps& props,
                       const SkPaint& paint,
                       const SkPMColor4f& filteredColor,
                       const GrClip& clip,
                       const SkMatrixProvider& deviceMatrix,
                       SkPoint drawOrigin) {
    for (SubRun* subRun = fFirstSubRun; subRun != nullptr; subRun = subRun->fNextSubRun) {
        if (subRun->drawAsPaths()) {
            SkPaint runPaint{paint};
            runPaint.setAntiAlias(subRun->isAntiAliased());
            // If there are shaders, blurs or styles, the path must be scaled into source
            // space independently of the CTM. This allows the CTM to be correct for the
            // different effects.
            GrStyle style(runPaint);

            bool needsExactCTM = runPaint.getShader()
                                 || style.applies()
                                 || runPaint.getMaskFilter();

            // Calculate the matrix that maps the path glyphs from their size in the strike to
            // the graphics source space.
            SkMatrix strikeToSource = SkMatrix::MakeScale(
                    subRun->fStrikeSpec.strikeToSourceRatio());
            strikeToSource.postTranslate(drawOrigin.x(), drawOrigin.y());
            if (!needsExactCTM) {
                for (const auto& pathPos : subRun->fPaths) {
                    const SkPath& path = pathPos.fPath;
                    const SkPoint pos = pathPos.fOrigin;                    // Transform the glyph to source space.
                    SkMatrix pathMatrix = strikeToSource;
                    pathMatrix.postTranslate(pos.x(), pos.y());
                    SkPreConcatMatrixProvider strikeToDevice(deviceMatrix, pathMatrix);

                    GrStyledShape shape(path, paint);
                    target->drawShape(clip, runPaint, strikeToDevice, shape);
                }
            } else {
                // Transform the path to device because the deviceMatrix must be unchanged to
                // draw effect, filter or shader paths.
                for (const auto& pathPos : subRun->fPaths) {
                    const SkPath& path = pathPos.fPath;
                    const SkPoint pos = pathPos.fOrigin;
                    // Transform the glyph to source space.
                    SkMatrix pathMatrix = strikeToSource;
                    pathMatrix.postTranslate(pos.x(), pos.y());

                    SkPath deviceOutline;
                    path.transform(pathMatrix, &deviceOutline);
                    deviceOutline.setIsVolatile(true);
                    GrStyledShape shape(deviceOutline, paint);
                    target->drawShape(clip, runPaint, deviceMatrix, shape);
                }
            }
        } else {
            int glyphCount = subRun->fGlyphs.count();
            if (0 == glyphCount) {
                continue;
            }

            bool skipClip = false;
            SkIRect clipRect = SkIRect::MakeEmpty();
            SkRect rtBounds = SkRect::MakeWH(target->width(), target->height());
            SkRRect clipRRect;
            GrAA aa;
            // We can clip geometrically if we're not using SDFs or transformed glyphs,
            // and we have an axis-aligned rectangular non-AA clip
            if (!subRun->drawAsDistanceFields() &&
                !subRun->needsTransform() &&
                clip.isRRect(rtBounds, &clipRRect, &aa) &&
                clipRRect.isRect() && GrAA::kNo == aa) {
                skipClip = true;
                // We only need to do clipping work if the subrun isn't contained by the clip
                SkRect subRunBounds = subRun->deviceRect(deviceMatrix.localToDevice(), drawOrigin);
                if (!clipRRect.getBounds().contains(subRunBounds)) {
                    // If the subrun is completely outside, don't add an op for it
                    if (!clipRRect.getBounds().intersects(subRunBounds)) {
                        continue;
                    } else {
                        clipRRect.getBounds().round(&clipRect);
                    }
                }
            }

            auto op = this->makeOp(*subRun, deviceMatrix, drawOrigin, clipRect,
                                   paint, filteredColor, props, target);
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

const GrTextBlob::Key& GrTextBlob::key() const { return fKey; }
size_t GrTextBlob::size() const { return fSize; }

std::unique_ptr<GrDrawOp> GrTextBlob::test_makeOp(const SkMatrixProvider& matrixProvider,
                                                  SkPoint drawOrigin,
                                                  const SkPaint& paint,
                                                  const SkPMColor4f& filteredColor,
                                                  const SkSurfaceProps& props,
                                                  GrTextTarget* target) {
    SubRun* info = fFirstSubRun;
    SkIRect emptyRect = SkIRect::MakeEmpty();
    return this->makeOp(*info, matrixProvider, drawOrigin, emptyRect, paint,
                        filteredColor, props, target);
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
    SkSpan<SubRun::PackedGlyphIDorGrGlyph> glyphs{
       fAlloc.makeArrayDefault<SubRun::PackedGlyphIDorGrGlyph>(drawables.size()), drawables.size()};
    bool hasW = this->hasW(type);

    SkASSERT(!fInitialMatrix.hasPerspective() || hasW);

    size_t vertexDataSize = drawables.size() * GetVertexStride(format, hasW) * kVerticesPerGlyph;
    SkSpan<char> vertexData{fAlloc.makeArrayDefault<char>(vertexDataSize), vertexDataSize};

    SubRun* subRun = fAlloc.make<SubRun>(type, this, strikeSpec, format, glyphs, vertexData);

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
                       const SkMatrix& drawMatrix,
                       SkPoint origin,
                       GrColor color,
                       SkColor initialLuminance,
                       bool forceWForDistanceFields)
        : fSize{allocSize}
        , fInitialMatrix{drawMatrix}
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

std::unique_ptr<GrAtlasTextOp> GrTextBlob::makeOp(SubRun& info,
                                                  const SkMatrixProvider& matrixProvider,
                                                  SkPoint drawOrigin,
                                                  const SkIRect& clipRect,
                                                  const SkPaint& paint,
                                                  const SkPMColor4f& filteredColor,
                                                  const SkSurfaceProps& props,
                                                  GrTextTarget* target) {
    GrPaint grPaint;
    target->makeGrPaint(info.maskFormat(), paint, matrixProvider, &grPaint);
    if (info.drawAsDistanceFields()) {
        // TODO: Can we be even smarter based on the dest transfer function?
        return GrAtlasTextOp::MakeDistanceField(target->getContext(),
                                                std::move(grPaint),
                                                &info,
                                                matrixProvider.localToDevice(),
                                                drawOrigin,
                                                clipRect,
                                                filteredColor,
                                                target->colorInfo().isLinearlyBlended(),
                                                SkPaintPriv::ComputeLuminanceColor(paint),
                                                props);
    } else {
        return GrAtlasTextOp::MakeBitmap(target->getContext(),
                                         std::move(grPaint),
                                         &info,
                                         matrixProvider.localToDevice(),
                                         drawOrigin,
                                         clipRect,
                                         filteredColor);
    }
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

// -- Adding a mask to an atlas ----------------------------------------------------------------

// expands each bit in a bitmask to 0 or ~0 of type INT_TYPE. Used to expand a BW glyph mask to
// A8, RGB565, or RGBA8888.
template <typename INT_TYPE>
static void expand_bits(INT_TYPE* dst,
                        const uint8_t* src,
                        int width,
                        int height,
                        int dstRowBytes,
                        int srcRowBytes) {
    for (int i = 0; i < height; ++i) {
        int rowWritesLeft = width;
        const uint8_t* s = src;
        INT_TYPE* d = dst;
        while (rowWritesLeft > 0) {
            unsigned mask = *s++;
            for (int i = 7; i >= 0 && rowWritesLeft; --i, --rowWritesLeft) {
                *d++ = (mask & (1 << i)) ? (INT_TYPE)(~0UL) : 0;
            }
        }
        dst = reinterpret_cast<INT_TYPE*>(reinterpret_cast<intptr_t>(dst) + dstRowBytes);
        src += srcRowBytes;
    }
}

static void get_packed_glyph_image(
        const SkGlyph& glyph, int dstRB, GrMaskFormat expectedMaskFormat, void* dst) {
    const int width = glyph.width();
    const int height = glyph.height();
    const void* src = glyph.image();
    SkASSERT(src != nullptr);

    GrMaskFormat grMaskFormat = GrGlyph::FormatFromSkGlyph(glyph.maskFormat());
    if (grMaskFormat == expectedMaskFormat) {
        int srcRB = glyph.rowBytes();
        // Notice this comparison is with the glyphs raw mask format, and not its GrMaskFormat.
        if (glyph.maskFormat() != SkMask::kBW_Format) {
            if (srcRB != dstRB) {
                const int bbp = GrMaskFormatBytesPerPixel(expectedMaskFormat);
                for (int y = 0; y < height; y++) {
                    memcpy(dst, src, width * bbp);
                    src = (const char*) src + srcRB;
                    dst = (char*) dst + dstRB;
                }
            } else {
                memcpy(dst, src, dstRB * height);
            }
        } else {
            // Handle 8-bit format by expanding the mask to the expected format.
            const uint8_t* bits = reinterpret_cast<const uint8_t*>(src);
            switch (expectedMaskFormat) {
                case kA8_GrMaskFormat: {
                    uint8_t* bytes = reinterpret_cast<uint8_t*>(dst);
                    expand_bits(bytes, bits, width, height, dstRB, srcRB);
                    break;
                }
                case kA565_GrMaskFormat: {
                    uint16_t* rgb565 = reinterpret_cast<uint16_t*>(dst);
                    expand_bits(rgb565, bits, width, height, dstRB, srcRB);
                    break;
                }
                default:
                    SK_ABORT("Invalid GrMaskFormat");
            }
        }
    } else if (grMaskFormat == kA565_GrMaskFormat && expectedMaskFormat == kARGB_GrMaskFormat) {
        // Convert if the glyph uses a 565 mask format since it is using LCD text rendering
        // but the expected format is 8888 (will happen on macOS with Metal since that
        // combination does not support 565).
        static constexpr SkMasks masks{
                {0b1111'1000'0000'0000, 11, 5},  // Red
                {0b0000'0111'1110'0000,  5, 6},  // Green
                {0b0000'0000'0001'1111,  0, 5},  // Blue
                {0, 0, 0}                        // Alpha
        };
        const int a565Bpp = GrMaskFormatBytesPerPixel(kA565_GrMaskFormat);
        const int argbBpp = GrMaskFormatBytesPerPixel(kARGB_GrMaskFormat);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                uint16_t color565 = 0;
                memcpy(&color565, src, a565Bpp);
                uint32_t colorRGBA = GrColorPackRGBA(masks.getRed(color565),
                                                     masks.getGreen(color565),
                                                     masks.getBlue(color565),
                                                     0xFF);
                memcpy(dst, &colorRGBA, argbBpp);
                src = (char*)src + a565Bpp;
                dst = (char*)dst + argbBpp;
            }
        }
    } else {
        // crbug:510931
        // Retrieving the image from the cache can actually change the mask format. This case is
        // very uncommon so for now we just draw a clear box for these glyphs.
        const int bpp = GrMaskFormatBytesPerPixel(expectedMaskFormat);
        for (int y = 0; y < height; y++) {
            sk_bzero(dst, width * bpp);
            dst = (char*)dst + dstRB;
        }
    }
}

// returns true if glyph successfully added to texture atlas, false otherwise.  If the glyph's
// mask format has changed, then add_glyph_to_atlas will draw a clear box.  This will almost never
// happen.
// TODO we can handle some of these cases if we really want to, but the long term solution is to
// get the actual glyph image itself when we get the glyph metrics.
static GrDrawOpAtlas::ErrorCode add_glyph_to_atlas(const SkGlyph& skGlyph,
                                                   GrMaskFormat expectedMaskFormat,
                                                   bool needsPadding,
                                                   GrResourceProvider* resourceProvider,
                                                   GrDeferredUploadTarget* target,
                                                   GrAtlasManager* fullAtlasManager,
                                                   GrGlyph* grGlyph) {
    SkASSERT(grGlyph != nullptr);
    SkASSERT(skGlyph.image() != nullptr);

    expectedMaskFormat = fullAtlasManager->resolveMaskFormat(expectedMaskFormat);
    int bytesPerPixel = GrMaskFormatBytesPerPixel(expectedMaskFormat);

    SkDEBUGCODE(bool isSDFGlyph = skGlyph.maskFormat() == SkMask::kSDF_Format;)
    SkASSERT(!needsPadding || !isSDFGlyph);

    // Add 1 pixel padding around grGlyph if needed.
    const int width = needsPadding ? skGlyph.width() + 2 : skGlyph.width();
    const int height = needsPadding ? skGlyph.height() + 2 : skGlyph.height();
    int rowBytes = width * bytesPerPixel;
    size_t size = height * rowBytes;

    // Temporary storage for normalizing grGlyph image.
    SkAutoSMalloc<1024> storage(size);
    void* dataPtr = storage.get();
    if (needsPadding) {
        sk_bzero(dataPtr, size);
        // Advance in one row and one column.
        dataPtr = (char*)(dataPtr) + rowBytes + bytesPerPixel;
    }

    get_packed_glyph_image(skGlyph, rowBytes, expectedMaskFormat, dataPtr);

    return fullAtlasManager->addToAtlas(resourceProvider, target, expectedMaskFormat, width, height,
                                        storage.get(), &grGlyph->fAtlasLocator);
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

    SkASSERT(fSubRun->isPrepared());
    const SkStrikeSpec& strikeSpec = fSubRun->strikeSpec();

    if (!fMetricsAndImages.isValid() ||
            fMetricsAndImages->descriptor() != strikeSpec.descriptor()) {
        fMetricsAndImages.init(strikeSpec);
    }

    // Update the atlas information in the GrStrike.
    auto code = GrDrawOpAtlas::ErrorCode::kSucceeded;
    auto tokenTracker = fUploadTarget->tokenTracker();
    int i = begin;
    for (; i < end; i++) {
        GrGlyph* grGlyph = fSubRun->fGlyphs[i].fGrGlyph;
        SkASSERT(grGlyph);

        if (!fFullAtlasManager->hasGlyph(fSubRun->maskFormat(), grGlyph)) {
            const SkGlyph& skGlyph = *fMetricsAndImages->glyph(grGlyph->fPackedID);
            if (skGlyph.image() == nullptr) {
                return {false, 0};
            }
            code = add_glyph_to_atlas(skGlyph, fSubRun->maskFormat(),
                                      fSubRun->needsPadding(), fResourceProvider,
                                      fUploadTarget, fFullAtlasManager, grGlyph);
            if (code != GrDrawOpAtlas::ErrorCode::kSucceeded) {
                break;
            }
        }
        fFullAtlasManager->addGlyphToBulkAndSetUseToken(
                fSubRun->bulkUseToken(), fSubRun->maskFormat(), grGlyph,
                tokenTracker->nextDrawToken());
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
        if (success && begin + glyphsPlacedInAtlas == fSubRun->fGlyphs.count()) {
            // Need to get the freshest value of the atlas' generation because
            // updateTextureCoordinates may have changed it.
            fSubRun->fAtlasGeneration = fFullAtlasManager->atlasGeneration(fSubRun->maskFormat());
        }
        return {success, glyphsPlacedInAtlas};
    } else {
        // The atlas hasn't changed, so our texture coordinates are still valid.
        if (end == fSubRun->fGlyphs.count()) {
            // The atlas hasn't changed and the texture coordinates are all still valid. Update
            // all the plots used to the new use token.
            fFullAtlasManager->setUseTokenBulk(*fSubRun->bulkUseToken(),
                                               fUploadTarget->tokenTracker()->nextDrawToken(),
                                               fSubRun->maskFormat());
        }
        return {true, end - begin};
    }
}
