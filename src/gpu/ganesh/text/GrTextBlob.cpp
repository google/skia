/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/core/SkScalar.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/private/SkTemplates.h"
#include "include/private/chromium/GrSlug.h"
#include "include/private/chromium/SkChromeRemoteGlyphCache.h"
#include "src/core/SkEnumerate.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "src/gpu/ganesh/GrClip.h"
#include "src/gpu/ganesh/GrMeshDrawTarget.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrStyle.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/effects/GrDistanceFieldGeoProc.h"
#include "src/gpu/ganesh/geometry/GrStyledShape.h"

#include "src/gpu/ganesh/text/GrAtlasManager.h"
#include "src/gpu/ganesh/text/GrSDFTControl.h"
#include "src/gpu/ganesh/text/GrTextBlob.h"
#include "src/text/gpu/Glyph.h"
#include "src/text/gpu/GlyphVector.h"
#include "src/text/gpu/StrikeCache.h"
#include "src/text/gpu/SubRunAllocator.h"

#include "src/gpu/ganesh/GrBlurUtils.h"
#include "src/gpu/ganesh/ops/AtlasTextOp.h"
#include "src/gpu/ganesh/v1/Device_v1.h"
#include "src/gpu/ganesh/v1/SurfaceDrawContext_v1.h"

using AtlasTextOp = skgpu::v1::AtlasTextOp;
using MaskFormat = skgpu::MaskFormat;

using Glyph = sktext::gpu::Glyph;
using GlyphVector = sktext::gpu::GlyphVector;
using StrikeCache = sktext::gpu::StrikeCache;
using SubRunAllocator = sktext::gpu::SubRunAllocator;

// -- GPU Text -------------------------------------------------------------------------------------
// There are three broad types of SubRun implementations for drawing text using the GPU.
// GrTextBlob (runs with no postfix) - these runs support drawing for GrTextBlobs.
// GrSlug (Slug postfix) - these runs support drawing of GrSlugs.
//
// Naming conventions
//  * drawMatrix - the CTM from the canvas.
//  * drawOrigin - the x, y location of the drawTextBlob call.
//  * positionMatrix - this is the combination of the drawMatrix and the drawOrigin:
//        positionMatrix = drawMatrix * TranslationMatrix(drawOrigin.x, drawOrigin.y);
//
// Note:
//   In order to use GrSlugs, you need to set the fSupportBilerpFromGlyphAtlas on GrContextOptions.

enum GrSubRun::SubRunType : int {
    kBad = 0,  // Make this 0 to line up with errors from readInt.
    kDirectMask,
    kSDFT,
    kTransformMask,
    kPath,
    kDrawable,
    kSubRunTypeCount,
};

// -- GrSubRun -------------------------------------------------------------------------------------
GrSubRun::~GrSubRun() = default;
const GrBlobSubRun* GrSubRun::blobCast() const {
    SK_ABORT("This is not a subclass of GrBlobSubRun.");
}

namespace {

template <typename T>
bool pun_read(SkReadBuffer& buffer, T* dst) {
    return buffer.readPad32(dst, sizeof(T));
}

template <typename T>
void pun_write(SkWriteBuffer& buffer, const T& src) {
    buffer.writePad32(&src, sizeof(T));
}

// -- TransformedMaskVertexFiller ------------------------------------------------------------------
class TransformedMaskVertexFiller {
    struct PositionAndExtent;
public:
    TransformedMaskVertexFiller(MaskFormat maskFormat,
                                int dstPadding,
                                SkScalar strikeToSourceScale,
                                SkRect sourceBounds,
                                SkSpan<const PositionAndExtent> positionAndExtent);

    static TransformedMaskVertexFiller Make(MaskFormat maskType,
                int dstPadding,
                SkScalar strikeToSourceScale,
                const SkZip<SkGlyphVariant, SkPoint>& accepted,
                SubRunAllocator* alloc);

    static std::optional<TransformedMaskVertexFiller> MakeFromBuffer(
            SkReadBuffer& buffer, SubRunAllocator* alloc);
    int unflattenSize() const;
    void flatten(SkWriteBuffer& buffer) const;

    size_t vertexStride(const SkMatrix& matrix) const {
        if (fMaskType != MaskFormat::kARGB) {
            // For formats MaskFormat::kA565 and MaskFormat::kA8 where A8 include SDF.
            return matrix.hasPerspective() ? sizeof(Mask3DVertex) : sizeof(Mask2DVertex);
        } else {
            // For format MaskFormat::kARGB
            return matrix.hasPerspective() ? sizeof(ARGB3DVertex) : sizeof(ARGB2DVertex);
        }
    }

    SkRect deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const;

    void fillVertexData(int offset, int count,
                        SkSpan<const Glyph*> glyphs,
                        GrColor color,
                        const SkMatrix& positionMatrix,
                        SkIRect clip,
                        void* vertexBuffer) const;

    AtlasTextOp::MaskType opMaskType() const;
    MaskFormat grMaskType() const {return fMaskType;}
    int count() const { return SkCount(fPositionAndExtent); }

private:
    struct PositionAndExtent {
        const SkPoint pos;
        // The rectangle of the glyphs in strike space. But, for kDirectMask this also implies a
        // device space rect.
        skgpu::IRect16 rect;
    };

    struct AtlasPt {
        uint16_t u;
        uint16_t v;
    };

    // Normal text mask, SDFT, or color.
    struct Mask2DVertex {
        SkPoint devicePos;
        GrColor color;
        AtlasPt atlasPos;
    };

    struct ARGB2DVertex {
        ARGB2DVertex(SkPoint d, GrColor, AtlasPt a) : devicePos{d}, atlasPos{a} {}

        SkPoint devicePos;
        AtlasPt atlasPos;
    };

    // Perspective SDFT or SDFT forced to 3D or perspective color.
    struct Mask3DVertex {
        SkPoint3 devicePos;
        GrColor color;
        AtlasPt atlasPos;
    };

    struct ARGB3DVertex {
        ARGB3DVertex(SkPoint3 d, GrColor, AtlasPt a) : devicePos{d}, atlasPos{a} {}

        SkPoint3 devicePos;
        AtlasPt atlasPos;
    };

    std::array<SkScalar, 4> sourceRect(PositionAndExtent positionAndExtent) const;

    template<typename Quad, typename VertexData>
    void fill2D(SkZip<Quad, const Glyph*, const VertexData> quadData,
                GrColor color,
                const SkMatrix& matrix) const;

    template<typename Quad, typename VertexData>
    void fill3D(SkZip<Quad, const Glyph*, const VertexData> quadData,
                GrColor color,
                const SkMatrix& matrix) const;

    const MaskFormat fMaskType;
    const SkPoint fPaddingInset;
    const SkScalar fStrikeToSourceScale;
    const SkRect fSourceBounds;
    const SkSpan<const PositionAndExtent> fPositionAndExtent;
};

TransformedMaskVertexFiller::TransformedMaskVertexFiller(
    MaskFormat maskFormat,
    int dstPadding,
    SkScalar strikeToSourceScale,
    SkRect sourceBounds,
    SkSpan<const PositionAndExtent> positionAndExtent)
        : fMaskType{maskFormat}
        , fPaddingInset{SkPoint::Make(dstPadding, dstPadding)}
        , fStrikeToSourceScale{strikeToSourceScale}
        , fSourceBounds{sourceBounds}
        , fPositionAndExtent{positionAndExtent} {}

TransformedMaskVertexFiller TransformedMaskVertexFiller::Make(
        MaskFormat maskType,
        int dstPadding,
        SkScalar strikeToSourceScale,
        const SkZip<SkGlyphVariant, SkPoint>& accepted,
        SubRunAllocator* alloc) {
    SkRect sourceBounds = SkRectPriv::MakeLargestInverted();
    SkSpan<PositionAndExtent> positionAndExtent = alloc->makePODArray<PositionAndExtent>(
            accepted,
            [&](auto e) {
                auto [variant, pos] = e;
                const SkGlyph* skGlyph = variant;
                int16_t l = skGlyph->left(),
                        t = skGlyph->top(),
                        r = l + skGlyph->width(),
                        b = t + skGlyph->height();
                SkPoint lt = SkPoint::Make(l, t) * strikeToSourceScale + pos,
                        rb = SkPoint::Make(r, b) * strikeToSourceScale + pos;

                sourceBounds.joinPossiblyEmptyRect(
                        SkRect::MakeLTRB(lt.x(), lt.y(), rb.x(), rb.y()));
                return PositionAndExtent{pos, {l, t, r, b}};
            });
    return TransformedMaskVertexFiller{
            maskType, dstPadding, strikeToSourceScale, sourceBounds, positionAndExtent};
}

static bool check_glyph_count(SkReadBuffer& buffer, int glyphCount) {
    return 0 < glyphCount && static_cast<size_t>(glyphCount) < (buffer.available() / 4);
}

std::optional<TransformedMaskVertexFiller> TransformedMaskVertexFiller::MakeFromBuffer(
        SkReadBuffer& buffer, SubRunAllocator* alloc) {
    int checkingMaskType = buffer.readInt();
    if (!buffer.validate(0 <= checkingMaskType && checkingMaskType < skgpu::kMaskFormatCount)) {
        return {};
    }
    MaskFormat maskType = (MaskFormat)checkingMaskType;
    int dstPadding = buffer.readInt();
    if (!buffer.validate(0 <= dstPadding && dstPadding <= 2)) { return {}; }
    SkScalar strikeToSourceScale = buffer.readScalar();
    if (!buffer.validate(0 < strikeToSourceScale)) { return {}; }
    SkRect sourceBounds = buffer.readRect();

    int glyphCount = buffer.readInt();
    if (!buffer.validate(check_glyph_count(buffer, glyphCount))) { return {}; }
    PositionAndExtent* positionAndExtentStorage =
            alloc->makePODArray<PositionAndExtent>(glyphCount);
    for (int i = 0; i < glyphCount; ++i) {
        pun_read(buffer, &positionAndExtentStorage[i]);
    }
    SkSpan<PositionAndExtent> positionAndExtent(positionAndExtentStorage, glyphCount);

    return {TransformedMaskVertexFiller{
            maskType, dstPadding, strikeToSourceScale, sourceBounds, positionAndExtent}};
}

SkRect TransformedMaskVertexFiller::deviceRect(
        const SkMatrix& drawMatrix, SkPoint drawOrigin) const {
    SkRect outBounds = fSourceBounds;
    outBounds.offset(drawOrigin);
    return drawMatrix.mapRect(outBounds);
}

int TransformedMaskVertexFiller::unflattenSize() const {
    return fPositionAndExtent.size_bytes();
}

void TransformedMaskVertexFiller::flatten(SkWriteBuffer& buffer) const {
    buffer.writeInt(static_cast<int>(fMaskType));
    buffer.writeInt(SkScalarRoundToInt(fPaddingInset.x()));
    buffer.writeScalar(fStrikeToSourceScale);
    buffer.writeRect(fSourceBounds);
    buffer.writeInt(SkCount(fPositionAndExtent));
    for (auto posAndExt : fPositionAndExtent) {
        pun_write(buffer, posAndExt);
    }
}

void TransformedMaskVertexFiller::fillVertexData(int offset, int count,
                                                 SkSpan<const Glyph*> glyphs,
                                                 GrColor color,
                                                 const SkMatrix& positionMatrix,
                                                 SkIRect clip,
                                                 void* vertexBuffer) const {
    auto quadData = [&](auto dst) {
        return SkMakeZip(dst,
                         glyphs.subspan(offset, count),
                         fPositionAndExtent.subspan(offset, count));
    };

    if (!positionMatrix.hasPerspective()) {
        if (fMaskType == MaskFormat::kARGB) {
            using Quad = ARGB2DVertex[4];
            SkASSERT(sizeof(ARGB2DVertex) == this->vertexStride(positionMatrix));
            this->fill2D(quadData((Quad*) vertexBuffer), color, positionMatrix);
        } else {
            using Quad = Mask2DVertex[4];
            SkASSERT(sizeof(Mask2DVertex) == this->vertexStride(positionMatrix));
            this->fill2D(quadData((Quad*) vertexBuffer), color, positionMatrix);
        }
    } else {
        if (fMaskType == MaskFormat::kARGB) {
            using Quad = ARGB3DVertex[4];
            SkASSERT(sizeof(ARGB3DVertex) == this->vertexStride(positionMatrix));
            this->fill3D(quadData((Quad*) vertexBuffer), color, positionMatrix);
        } else {
            using Quad = Mask3DVertex[4];
            SkASSERT(sizeof(Mask3DVertex) == this->vertexStride(positionMatrix));
            this->fill3D(quadData((Quad*) vertexBuffer), color, positionMatrix);
        }
    }
}

std::array<SkScalar, 4>
TransformedMaskVertexFiller::sourceRect(PositionAndExtent positionAndExtent) const {
    auto[pos, rect] = positionAndExtent;
    auto[l, t, r, b] = rect;
    SkPoint LT = (SkPoint::Make(l, t) + fPaddingInset) * fStrikeToSourceScale + pos,
            RB = (SkPoint::Make(r, b) - fPaddingInset) * fStrikeToSourceScale + pos;
    return {LT.x(), LT.y(), RB.x(), RB.y()};
}

template<typename Quad, typename VertexData>
void TransformedMaskVertexFiller::fill2D(SkZip<Quad, const Glyph*, const VertexData> quadData,
                                         GrColor color,
                                         const SkMatrix& positionMatrix) const {
    for (auto[quad, glyph, positionAndExtent] : quadData) {
        auto [l, t, r, b] = this->sourceRect(positionAndExtent);
        SkPoint lt = positionMatrix.mapXY(l, t),
                lb = positionMatrix.mapXY(l, b),
                rt = positionMatrix.mapXY(r, t),
                rb = positionMatrix.mapXY(r, b);
        auto[al, at, ar, ab] = glyph->fAtlasLocator.getUVs();
        quad[0] = {lt, color, {al, at}};  // L,T
        quad[1] = {lb, color, {al, ab}};  // L,B
        quad[2] = {rt, color, {ar, at}};  // R,T
        quad[3] = {rb, color, {ar, ab}};  // R,B
    }
}

template<typename Quad, typename VertexData>
void TransformedMaskVertexFiller::fill3D(SkZip<Quad, const Glyph*, const VertexData> quadData,
                                         GrColor color,
                                         const SkMatrix& positionMatrix) const {
    auto mapXYZ = [&](SkScalar x, SkScalar y) {
        SkPoint pt{x, y};
        SkPoint3 result;
        positionMatrix.mapHomogeneousPoints(&result, &pt, 1);
        return result;
    };
    for (auto[quad, glyph, positionAndExtent] : quadData) {
        auto [l, t, r, b] = this->sourceRect(positionAndExtent);
        SkPoint3 lt = mapXYZ(l, t),
                 lb = mapXYZ(l, b),
                 rt = mapXYZ(r, t),
                 rb = mapXYZ(r, b);
        auto[al, at, ar, ab] = glyph->fAtlasLocator.getUVs();
        quad[0] = {lt, color, {al, at}};  // L,T
        quad[1] = {lb, color, {al, ab}};  // L,B
        quad[2] = {rt, color, {ar, at}};  // R,T
        quad[3] = {rb, color, {ar, ab}};  // R,B
    }
}

AtlasTextOp::MaskType TransformedMaskVertexFiller::opMaskType() const {
    switch (fMaskType) {
        case MaskFormat::kA8:   return AtlasTextOp::MaskType::kGrayscaleCoverage;
        case MaskFormat::kA565: return AtlasTextOp::MaskType::kLCDCoverage;
        case MaskFormat::kARGB: return AtlasTextOp::MaskType::kColorBitmap;
    }
    SkUNREACHABLE;
}

struct AtlasPt {
    uint16_t u;
    uint16_t v;
};

// Normal text mask, SDFT, or color.
struct Mask2DVertex {
    SkPoint devicePos;
    GrColor color;
    AtlasPt atlasPos;
};

struct ARGB2DVertex {
    ARGB2DVertex(SkPoint d, GrColor, AtlasPt a) : devicePos{d}, atlasPos{a} {}

    SkPoint devicePos;
    AtlasPt atlasPos;
};

// Perspective SDFT or SDFT forced to 3D or perspective color.
struct Mask3DVertex {
    SkPoint3 devicePos;
    GrColor color;
    AtlasPt atlasPos;
};

struct ARGB3DVertex {
    ARGB3DVertex(SkPoint3 d, GrColor, AtlasPt a) : devicePos{d}, atlasPos{a} {}

    SkPoint3 devicePos;
    AtlasPt atlasPos;
};

AtlasTextOp::MaskType op_mask_type(MaskFormat maskFormat) {
    switch (maskFormat) {
        case MaskFormat::kA8:   return AtlasTextOp::MaskType::kGrayscaleCoverage;
        case MaskFormat::kA565: return AtlasTextOp::MaskType::kLCDCoverage;
        case MaskFormat::kARGB: return AtlasTextOp::MaskType::kColorBitmap;
    }
    SkUNREACHABLE;
}

SkMatrix position_matrix(const SkMatrix& drawMatrix, SkPoint drawOrigin) {
    SkMatrix position_matrix = drawMatrix;
    return position_matrix.preTranslate(drawOrigin.x(), drawOrigin.y());
}

SkPMColor4f calculate_colors(skgpu::SurfaceContext* sc,
                             const SkPaint& paint,
                             const SkMatrixProvider& matrix,
                             MaskFormat maskFormat,
                             GrPaint* grPaint) {
    GrRecordingContext* rContext = sc->recordingContext();
    const GrColorInfo& colorInfo = sc->colorInfo();
    if (maskFormat == MaskFormat::kARGB) {
        SkPaintToGrPaintReplaceShader(rContext, colorInfo, paint, matrix, nullptr, grPaint);
        float a = grPaint->getColor4f().fA;
        return {a, a, a, a};
    }
    SkPaintToGrPaint(rContext, colorInfo, paint, matrix, grPaint);
    return grPaint->getColor4f();
}

// Check for integer translate with the same 2x2 matrix.
// Returns the translation, and true if the change from initial matrix to the position matrix
// support using direct glyph masks.
std::tuple<bool, SkVector> can_use_direct(
        const SkMatrix& initialPositionMatrix, const SkMatrix& positionMatrix) {
    // The existing direct glyph info can be used if the initialPositionMatrix, and the
    // positionMatrix have the same 2x2, and the translation between them is integer.
    // Calculate the translation in source space to a translation in device space by mapping
    // (0, 0) through both the initial position matrix and the position matrix; take the difference.
    SkVector translation = positionMatrix.mapOrigin() - initialPositionMatrix.mapOrigin();
    return {initialPositionMatrix.getScaleX() == positionMatrix.getScaleX() &&
            initialPositionMatrix.getScaleY() == positionMatrix.getScaleY() &&
            initialPositionMatrix.getSkewX()  == positionMatrix.getSkewX()  &&
            initialPositionMatrix.getSkewY()  == positionMatrix.getSkewY()  &&
            SkScalarIsInt(translation.x()) && SkScalarIsInt(translation.y()),
            translation};
}

// -- PathOpSubmitter ------------------------------------------------------------------------------
// Shared code for submitting GPU ops for drawing glyphs as paths.
class PathOpSubmitter {
    union Variant;

public:
    PathOpSubmitter(bool isAntiAliased,
                    SkScalar strikeToSourceScale,
                    SkSpan<SkPoint> positions,
                    SkSpan<SkGlyphID> glyphIDs,
                    std::unique_ptr<SkPath[], SubRunAllocator::ArrayDestroyer> paths,
                    const SkDescriptor& descriptor);

    static PathOpSubmitter Make(const SkZip<SkGlyphVariant, SkPoint>& accepted,
                                bool isAntiAliased,
                                SkScalar strikeToSourceScale,
                                const SkDescriptor& descriptor,
                                SubRunAllocator* alloc);

    int unflattenSize() const;
    void flatten(SkWriteBuffer& buffer) const;
    static std::optional<PathOpSubmitter> MakeFromBuffer(SkReadBuffer& buffer,
                                                         SubRunAllocator* alloc,
                                                         const SkStrikeClient* client);

    void submitOps(SkCanvas*,
                   const GrClip* clip,
                   const SkMatrixProvider& viewMatrix,
                   SkPoint drawOrigin,
                   const SkPaint& paint,
                   skgpu::v1::SurfaceDrawContext* sdc) const;

private:
    const bool fIsAntiAliased;
    const SkScalar fStrikeToSourceScale;
    const SkSpan<const SkPoint> fPositions;
    const SkSpan<const SkGlyphID> fGlyphIDs;
    std::unique_ptr<SkPath[], SubRunAllocator::ArrayDestroyer> fPaths;
    const SkAutoDescriptor fDescriptor;
};

int PathOpSubmitter::unflattenSize() const {
    return fPositions.size_bytes() + fGlyphIDs.size_bytes() + SkCount(fGlyphIDs) * sizeof(SkPath);
}

void PathOpSubmitter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeInt(fIsAntiAliased);
    buffer.writeScalar(fStrikeToSourceScale);
    buffer.writeInt(SkCount(fPositions));
    for (auto pos : fPositions) {
        buffer.writePoint(pos);
    }
    for (SkGlyphID glyphID : fGlyphIDs) {
        buffer.writeInt(glyphID);
    }
    fDescriptor.getDesc()->flatten(buffer);
}

std::optional<PathOpSubmitter> PathOpSubmitter::MakeFromBuffer(SkReadBuffer& buffer,
                                                               SubRunAllocator* alloc,
                                                               const SkStrikeClient* client) {
    bool isAntiAlias = buffer.readInt();
    SkScalar strikeToSourceScale = buffer.readScalar();

    int glyphCount = buffer.readInt();
    if (!buffer.validate(check_glyph_count(buffer, glyphCount))) { return {}; }
    if (!buffer.validateCanReadN<SkPoint>(glyphCount)) { return {}; }
    SkPoint* positions = alloc->makePODArray<SkPoint>(glyphCount);
    for (int i = 0; i < glyphCount; ++i) {
        positions[i] = buffer.readPoint();
    }

    // Remember, we stored an int for glyph id.
    if (!buffer.validateCanReadN<int>(glyphCount)) { return {}; }
    SkGlyphID* glyphIDs = alloc->makePODArray<SkGlyphID>(glyphCount);
    for (int i = 0; i < glyphCount; ++i) {
        glyphIDs[i] = SkTo<SkGlyphID>(buffer.readInt());
    }

    auto descriptor = SkAutoDescriptor::MakeFromBuffer(buffer);
    if (!buffer.validate(descriptor.has_value())) { return {}; }

    // Translate the TypefaceID if this was transferred from the GPU process.
    if (client != nullptr) {
        if (!client->translateTypefaceID(&descriptor.value())) { return {}; }
    }

    auto strike = SkStrikeCache::GlobalStrikeCache()->findStrike(*descriptor->getDesc());
    if (!buffer.validate(strike != nullptr)) { return {}; }

    auto paths = alloc->makeUniqueArray<SkPath>(glyphCount);
    SkBulkGlyphMetricsAndPaths pathGetter{std::move(strike)};

    for (auto [i, glyphID] : SkMakeEnumerate(SkMakeSpan(glyphIDs, glyphCount))) {
        const SkPath* path = pathGetter.glyph(glyphID)->path();
        // There should never be missing paths in a sub run.
        if (path == nullptr) { return {}; }
        paths[i] = *path;
    }

    SkASSERT(buffer.isValid());
    return {PathOpSubmitter{isAntiAlias,
                            strikeToSourceScale,
                            SkMakeSpan(positions, glyphCount),
                            SkMakeSpan(glyphIDs, glyphCount),
                            std::move(paths),
                            *descriptor->getDesc()}};
}

PathOpSubmitter::PathOpSubmitter(
    bool isAntiAliased,
    SkScalar strikeToSourceScale,
    SkSpan<SkPoint> positions,
    SkSpan<SkGlyphID> glyphIDs,
    std::unique_ptr<SkPath[], SubRunAllocator::ArrayDestroyer> paths,
    const SkDescriptor& descriptor)
        : fIsAntiAliased{isAntiAliased}
        , fStrikeToSourceScale{strikeToSourceScale}
        , fPositions{positions}
        , fGlyphIDs{glyphIDs}
        , fPaths{std::move(paths)}
        , fDescriptor{descriptor} {
    SkASSERT(!fPositions.empty());
}

PathOpSubmitter PathOpSubmitter::Make(const SkZip<SkGlyphVariant, SkPoint>& accepted,
                                      bool isAntiAliased,
                                      SkScalar strikeToSourceScale,
                                      const SkDescriptor& descriptor,
                                      SubRunAllocator* alloc) {
    int glyphCount = SkCount(accepted);
    SkPoint* positions = alloc->makePODArray<SkPoint>(glyphCount);
    SkGlyphID* glyphIDs = alloc->makePODArray<SkGlyphID>(glyphCount);
    auto paths = alloc->makeUniqueArray<SkPath>(glyphCount);

    for (auto [i, variant, pos] : SkMakeEnumerate(accepted)) {
        positions[i] = pos;
        glyphIDs[i] = variant.glyph()->getGlyphID();
        paths[i] = *variant.glyph()->path();
    }

    return PathOpSubmitter{isAntiAliased,
                           strikeToSourceScale,
                           SkMakeSpan(positions, glyphCount),
                           SkMakeSpan(glyphIDs, glyphCount),
                           std::move(paths),
                           descriptor};
}

void PathOpSubmitter::submitOps(SkCanvas* canvas,
                                const GrClip* clip,
                                const SkMatrixProvider& viewMatrix,
                                SkPoint drawOrigin,
                                const SkPaint& paint,
                                skgpu::v1::SurfaceDrawContext* sdc) const {
    SkPaint runPaint{paint};
    runPaint.setAntiAlias(fIsAntiAliased);


    SkMaskFilterBase* maskFilter = as_MFB(runPaint.getMaskFilter());

    // Calculate the matrix that maps the path glyphs from their size in the strike to
    // the graphics source space.
    SkMatrix strikeToSource = SkMatrix::Scale(fStrikeToSourceScale, fStrikeToSourceScale);
    strikeToSource.postTranslate(drawOrigin.x(), drawOrigin.y());

    // If there are shaders, non-blur mask filters or styles, the path must be scaled into source
    // space independently of the CTM. This allows the CTM to be correct for the different effects.
    GrStyle style(runPaint);
    bool needsExactCTM = runPaint.getShader()
                         || style.applies()
                         || (maskFilter != nullptr && !maskFilter->asABlur(nullptr));
    if (!needsExactCTM) {
        SkMaskFilterBase::BlurRec blurRec;

        // If there is a blur mask filter, then sigma needs to be adjusted to account for the
        // scaling of fStrikeToSourceScale.
        if (maskFilter != nullptr && maskFilter->asABlur(&blurRec)) {
            runPaint.setMaskFilter(
                    SkMaskFilter::MakeBlur(blurRec.fStyle, blurRec.fSigma / fStrikeToSourceScale));
        }
        for (auto [path, pos] : SkMakeZip(fPaths.get(), fPositions)) {
            // Transform the glyph to source space.
            SkMatrix pathMatrix = strikeToSource;
            pathMatrix.postTranslate(pos.x(), pos.y());

            SkAutoCanvasRestore acr(canvas, true);
            canvas->concat(pathMatrix);
            canvas->drawPath(path, runPaint);
        }
    } else {
        // Transform the path to device because the deviceMatrix must be unchanged to
        // draw effect, filter or shader paths.
        for (auto [path, pos] : SkMakeZip(fPaths.get(), fPositions)) {
            // Transform the glyph to source space.
            SkMatrix pathMatrix = strikeToSource;
            pathMatrix.postTranslate(pos.x(), pos.y());

            SkPath deviceOutline;
            path.transform(pathMatrix, &deviceOutline);
            deviceOutline.setIsVolatile(true);
            canvas->drawPath(deviceOutline, runPaint);
        }
    }
}

// -- PathSubRun -----------------------------------------------------------------------------------
class PathSubRun final : public GrSubRun {
public:
    PathSubRun(PathOpSubmitter&& pathDrawing) : fPathDrawing(std::move(pathDrawing)) {}

    static GrSubRunOwner Make(const SkZip<SkGlyphVariant, SkPoint>& accepted,
                              bool isAntiAliased,
                              SkScalar strikeToSourceScale,
                              const SkDescriptor& descriptor,
                              SubRunAllocator* alloc) {
        return alloc->makeUnique<PathSubRun>(
                PathOpSubmitter::Make(
                        accepted, isAntiAliased, strikeToSourceScale, descriptor, alloc));
    }

    void draw(SkCanvas* canvas,
              const GrClip* clip,
              const SkMatrixProvider& viewMatrix,
              SkPoint drawOrigin,
              const SkPaint& paint,
              skgpu::v1::SurfaceDrawContext* sdc) const override {
        fPathDrawing.submitOps(canvas, clip, viewMatrix, drawOrigin, paint, sdc);
    }

    int unflattenSize() const override;

    bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const override {
        return true;
    }
    const GrAtlasSubRun* testingOnly_atlasSubRun() const override { return nullptr; }
    static GrSubRunOwner MakeFromBuffer(const GrTextReferenceFrame* referenceFrame,
                                        SkReadBuffer& buffer,
                                        SubRunAllocator* alloc,
                                        const SkStrikeClient* client);

protected:
    SubRunType subRunType() const override { return kPath; }
    void doFlatten(SkWriteBuffer& buffer) const override;

private:
    PathOpSubmitter fPathDrawing;
};

int PathSubRun::unflattenSize() const {
    return sizeof(PathSubRun) + fPathDrawing.unflattenSize();
}

void PathSubRun::doFlatten(SkWriteBuffer& buffer) const {
    fPathDrawing.flatten(buffer);
}

GrSubRunOwner PathSubRun::MakeFromBuffer(const GrTextReferenceFrame* referenceFrame,
                                         SkReadBuffer& buffer,
                                         SubRunAllocator* alloc,
                                         const SkStrikeClient* client) {
    auto pathOpSubmitter = PathOpSubmitter::MakeFromBuffer(buffer, alloc, client);
    if (!buffer.validate(pathOpSubmitter.has_value())) { return nullptr; }
    return alloc->makeUnique<PathSubRun>(std::move(*pathOpSubmitter));
}

// -- DrawableOpSubmitter --------------------------------------------------------------------------
// Shared code for submitting GPU ops for drawing glyphs as drawables.
class DrawableOpSubmitter {
public:
    DrawableOpSubmitter(SkScalar strikeToSourceScale,
                        SkSpan<SkPoint> positions,
                        SkSpan<SkGlyphID> glyphIDs,
                        std::unique_ptr<sk_sp<SkDrawable>[],
                                        SubRunAllocator::ArrayDestroyer> drawableData,
                        const SkDescriptor& descriptor);

    static DrawableOpSubmitter Make(const SkZip<SkGlyphVariant, SkPoint>& accepted,
                                    SkScalar strikeToSourceScale,
                                    const SkDescriptor& descriptor,
                                    SubRunAllocator* alloc);

    int unflattenSize() const;
    void flatten(SkWriteBuffer& buffer) const;
    static std::optional<DrawableOpSubmitter> MakeFromBuffer(SkReadBuffer& buffer,
                                                             SubRunAllocator* alloc,
                                                             const SkStrikeClient* client);

    void submitOps(SkCanvas*,
                   const GrClip* clip,
                   const SkMatrixProvider& viewMatrix,
                   SkPoint drawOrigin,
                   const SkPaint& paint,
                   skgpu::v1::SurfaceDrawContext* sdc) const;

private:
    const SkScalar fStrikeToSourceScale;
    const SkSpan<SkPoint> fPositions;
    const SkSpan<SkGlyphID> fGlyphIDs;
    std::unique_ptr<sk_sp<SkDrawable>[], SubRunAllocator::ArrayDestroyer> fDrawables;
    const SkAutoDescriptor fDescriptor;
};

int DrawableOpSubmitter::unflattenSize() const {
    return fPositions.size_bytes() +
           fGlyphIDs.size_bytes() +
           SkCount(fPositions) * sizeof(sk_sp<SkDrawable>);
}

void DrawableOpSubmitter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeScalar(fStrikeToSourceScale);
    buffer.writeInt(SkCount(fPositions));
    for (auto pos : fPositions) {
        buffer.writePoint(pos);
    }
    for (SkGlyphID glyphID : fGlyphIDs) {
        buffer.writeInt(glyphID);
    }
    fDescriptor.getDesc()->flatten(buffer);
}

std::optional<DrawableOpSubmitter> DrawableOpSubmitter::MakeFromBuffer(
        SkReadBuffer& buffer, SubRunAllocator* alloc, const SkStrikeClient* client) {
    SkScalar strikeToSourceScale = buffer.readScalar();

    int glyphCount = buffer.readInt();
    if (!buffer.validate(check_glyph_count(buffer, glyphCount))) { return {}; }
    if (!buffer.validateCanReadN<SkPoint>(glyphCount)) { return {}; }
    SkPoint* positions = alloc->makePODArray<SkPoint>(glyphCount);
    for (int i = 0; i < glyphCount; ++i) {
        positions[i] = buffer.readPoint();
    }

    // Remember, we stored an int for glyph id.
    if (!buffer.validateCanReadN<int>(glyphCount)) { return {}; }
    SkGlyphID* glyphIDs = alloc->makePODArray<SkGlyphID>(glyphCount);
    for (int i = 0; i < glyphCount; ++i) {
        glyphIDs[i] = SkTo<SkGlyphID>(buffer.readInt());
    }

    auto descriptor = SkAutoDescriptor::MakeFromBuffer(buffer);
    if (!buffer.validate(descriptor.has_value())) { return {}; }

    // Translate the TypefaceID if this was transferred from the GPU process.
    if (client != nullptr) {
        if (!client->translateTypefaceID(&descriptor.value())) { return {}; }
    }

    auto strike = SkStrikeCache::GlobalStrikeCache()->findStrike(*descriptor->getDesc());
    if (!buffer.validate(strike != nullptr)) { return {}; }

    auto drawables = alloc->makeUniqueArray<sk_sp<SkDrawable>>(glyphCount);
    SkBulkGlyphMetricsAndDrawables drawableGetter{std::move(strike)};
    auto glyphs = drawableGetter.glyphs(SkMakeSpan(glyphIDs, glyphCount));

    for (auto [i, glyph] : SkMakeEnumerate(glyphs)) {
        drawables[i] = sk_ref_sp(glyph->drawable());
    }

    SkASSERT(buffer.isValid());
    return {DrawableOpSubmitter{strikeToSourceScale,
                                SkMakeSpan(positions, glyphCount),
                                SkMakeSpan(glyphIDs, glyphCount),
                                std::move(drawables),
                                *descriptor->getDesc()}};
}

DrawableOpSubmitter::DrawableOpSubmitter(
        SkScalar strikeToSourceScale,
        SkSpan<SkPoint> positions,
        SkSpan<SkGlyphID> glyphIDs,
        std::unique_ptr<sk_sp<SkDrawable>[],
                        SubRunAllocator::ArrayDestroyer> drawables,
        const SkDescriptor& descriptor)
            : fStrikeToSourceScale{strikeToSourceScale}
            , fPositions{positions}
            , fGlyphIDs{glyphIDs}
            , fDrawables{std::move(drawables)}
            , fDescriptor{descriptor} {
    SkASSERT(!fPositions.empty());
}

DrawableOpSubmitter DrawableOpSubmitter::Make(const SkZip<SkGlyphVariant, SkPoint>& accepted,
                                              SkScalar strikeToSourceScale,
                                              const SkDescriptor& descriptor,
                                              SubRunAllocator* alloc) {
    int glyphCount = SkCount(accepted);
    SkPoint* positions = alloc->makePODArray<SkPoint>(glyphCount);
    SkGlyphID* glyphIDs = alloc->makePODArray<SkGlyphID>(glyphCount);
    auto drawables = alloc->makeUniqueArray<sk_sp<SkDrawable>>(glyphCount);
    for (auto [i, variant, pos] : SkMakeEnumerate(accepted)) {
        positions[i] = pos;
        glyphIDs[i] = variant.glyph()->getGlyphID();
        drawables[i] = sk_ref_sp(variant.glyph()->drawable());
    }

    return DrawableOpSubmitter{strikeToSourceScale,
                               SkMakeSpan(positions, glyphCount),
                               SkMakeSpan(glyphIDs, glyphCount),
                               std::move(drawables),
                               descriptor};
}

void DrawableOpSubmitter::submitOps(SkCanvas* canvas,
                                    const GrClip* clip,
                                    const SkMatrixProvider& viewMatrix,
                                    SkPoint drawOrigin,
                                    const SkPaint& paint,
                                    skgpu::v1::SurfaceDrawContext* sdc) const {
    // Calculate the matrix that maps the path glyphs from their size in the strike to
    // the graphics source space.
    SkMatrix strikeToSource = SkMatrix::Scale(fStrikeToSourceScale, fStrikeToSourceScale);
    strikeToSource.postTranslate(drawOrigin.x(), drawOrigin.y());

    // Transform the path to device because the deviceMatrix must be unchanged to
    // draw effect, filter or shader paths.
    for (auto [i, position] : SkMakeEnumerate(fPositions)) {
        const sk_sp<SkDrawable>& drawable = fDrawables[i];
        // Transform the glyph to source space.
        SkMatrix pathMatrix = strikeToSource;
        pathMatrix.postTranslate(position.x(), position.y());

        SkAutoCanvasRestore acr(canvas, false);
        SkRect drawableBounds = drawable->getBounds();
        pathMatrix.mapRect(&drawableBounds);
        canvas->saveLayer(&drawableBounds, &paint);
        drawable->draw(canvas, &pathMatrix);
    }
}

template <typename SubRun>
GrSubRunOwner make_drawable_sub_run(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                    SkScalar strikeToSourceScale,
                                    const SkDescriptor& descriptor,
                                    SubRunAllocator* alloc) {
    return alloc->makeUnique<SubRun>(
            DrawableOpSubmitter::Make(drawables, strikeToSourceScale, descriptor, alloc));
}

// -- DrawableSubRun -------------------------------------------------------------------------------
class DrawableSubRun : public GrSubRun {
public:
    DrawableSubRun(DrawableOpSubmitter&& drawingDrawing)
            : fDrawingDrawing(std::move(drawingDrawing)) {}

    static GrSubRunOwner MakeFromBuffer(const GrTextReferenceFrame* referenceFrame,
                                        SkReadBuffer& buffer,
                                        SubRunAllocator* alloc,
                                        const SkStrikeClient* client);

    void draw(SkCanvas* canvas,
              const GrClip* clip,
              const SkMatrixProvider& viewMatrix,
              SkPoint drawOrigin,
              const SkPaint& paint,
              skgpu::v1::SurfaceDrawContext* sdc) const override {
        fDrawingDrawing.submitOps(canvas, clip, viewMatrix, drawOrigin, paint, sdc);
    }

    int unflattenSize() const override;

    bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const override;

    const GrAtlasSubRun* testingOnly_atlasSubRun() const override;

protected:
    SubRunType subRunType() const override { return kDrawable; }
    void doFlatten(SkWriteBuffer& buffer) const override;

private:
    DrawableOpSubmitter fDrawingDrawing;
};

int DrawableSubRun::unflattenSize() const {
    return sizeof(DrawableSubRun) + fDrawingDrawing.unflattenSize();
}

void DrawableSubRun::doFlatten(SkWriteBuffer& buffer) const {
    fDrawingDrawing.flatten(buffer);
}

GrSubRunOwner DrawableSubRun::MakeFromBuffer(const GrTextReferenceFrame* referenceFrame,
                                                 SkReadBuffer& buffer,
                                                 SubRunAllocator* alloc,
                                                 const SkStrikeClient* client) {
    auto drawableOpSubmitter = DrawableOpSubmitter::MakeFromBuffer(buffer, alloc, client);
    if (!buffer.validate(drawableOpSubmitter.has_value())) { return nullptr; }
    return alloc->makeUnique<DrawableSubRun>(std::move(*drawableOpSubmitter));
}

bool DrawableSubRun::canReuse( const SkPaint& paint, const SkMatrix& positionMatrix) const {
    return true;
}

const GrAtlasSubRun* DrawableSubRun::testingOnly_atlasSubRun() const {
    return nullptr;
}

namespace {
enum ClipMethod {
    kClippedOut,
    kUnclipped,
    kGPUClipped,
    kGeometryClipped
};

std::tuple<ClipMethod, SkIRect>
calculate_clip(const GrClip* clip, SkRect deviceBounds, SkRect glyphBounds) {
    if (clip == nullptr && !deviceBounds.intersects(glyphBounds)) {
        return {kClippedOut, SkIRect::MakeEmpty()};
    } else if (clip != nullptr) {
        switch (auto result = clip->preApply(glyphBounds, GrAA::kNo); result.fEffect) {
            case GrClip::Effect::kClippedOut:
                return {kClippedOut, SkIRect::MakeEmpty()};
            case GrClip::Effect::kUnclipped:
                return {kUnclipped, SkIRect::MakeEmpty()};
            case GrClip::Effect::kClipped: {
                if (result.fIsRRect && result.fRRect.isRect()) {
                    SkRect r = result.fRRect.rect();
                    if (result.fAA == GrAA::kNo || GrClip::IsPixelAligned(r)) {
                        SkIRect clipRect = SkIRect::MakeEmpty();
                        // Clip geometrically during onPrepare using clipRect.
                        r.round(&clipRect);
                        if (clipRect.contains(glyphBounds)) {
                            // If fully within the clip, signal no clipping using the empty rect.
                            return {kUnclipped, SkIRect::MakeEmpty()};
                        }
                        // Use the clipRect to clip the geometry.
                        return {kGeometryClipped, clipRect};
                    }
                    // Partial pixel clipped at this point. Have the GPU handle it.
                }
            }
            break;
        }
    }
    return {kGPUClipped, SkIRect::MakeEmpty()};
}

using DevicePosition = skvx::Vec<2, int16_t>;

// The 99% case. No clip. Non-color only.
void direct_2D(SkZip<Mask2DVertex[4],
                     const Glyph*,
                     const DevicePosition> quadData,
               GrColor color,
               SkPoint originOffset) {
    for (auto[quad, glyph, leftTop] : quadData) {
        auto[al, at, ar, ab] = glyph->fAtlasLocator.getUVs();
        SkScalar dl = leftTop[0] + originOffset.x(),
                 dt = leftTop[1] + originOffset.y(),
                 dr = dl + (ar - al),
                 db = dt + (ab - at);

        quad[0] = {{dl, dt}, color, {al, at}};  // L,T
        quad[1] = {{dl, db}, color, {al, ab}};  // L,B
        quad[2] = {{dr, dt}, color, {ar, at}};  // R,T
        quad[3] = {{dr, db}, color, {ar, ab}};  // R,B
    }
}
}  // namespace

template <typename Rect>
auto ltbr(const Rect& r) {
    return std::make_tuple(r.left(), r.top(), r.right(), r.bottom());
}

// Handle any combination of BW or color and clip or no clip.
template<typename Quad, typename VertexData>
void generalized_direct_2D(SkZip<Quad, const Glyph*, const VertexData> quadData,
                           GrColor color,
                           SkPoint originOffset,
                           SkIRect* clip = nullptr) {
    for (auto[quad, glyph, leftTop] : quadData) {
        auto[al, at, ar, ab] = glyph->fAtlasLocator.getUVs();
        uint16_t w = ar - al,
                 h = ab - at;
        SkScalar l = (SkScalar)leftTop[0] + originOffset.x(),
                 t = (SkScalar)leftTop[1] + originOffset.y();
        if (clip == nullptr) {
            auto[dl, dt, dr, db] = SkRect::MakeLTRB(l, t, l + w, t + h);
            quad[0] = {{dl, dt}, color, {al, at}};  // L,T
            quad[1] = {{dl, db}, color, {al, ab}};  // L,B
            quad[2] = {{dr, dt}, color, {ar, at}};  // R,T
            quad[3] = {{dr, db}, color, {ar, ab}};  // R,B
        } else {
            SkIRect devIRect = SkIRect::MakeLTRB(l, t, l + w, t + h);
            SkScalar dl, dt, dr, db;
            if (!clip->containsNoEmptyCheck(devIRect)) {
                if (SkIRect clipped; clipped.intersect(devIRect, *clip)) {
                    al += clipped.left()   - devIRect.left();
                    at += clipped.top()    - devIRect.top();
                    ar += clipped.right()  - devIRect.right();
                    ab += clipped.bottom() - devIRect.bottom();
                    std::tie(dl, dt, dr, db) = ltbr(clipped);
                } else {
                    // TODO: omit generating any vertex data for fully clipped glyphs ?
                    std::tie(dl, dt, dr, db) = std::make_tuple(0, 0, 0, 0);
                    std::tie(al, at, ar, ab) = std::make_tuple(0, 0, 0, 0);
                }
            } else {
                std::tie(dl, dt, dr, db) = ltbr(devIRect);
            }
            quad[0] = {{dl, dt}, color, {al, at}};  // L,T
            quad[1] = {{dl, db}, color, {al, ab}};  // L,B
            quad[2] = {{dr, dt}, color, {ar, at}};  // R,T
            quad[3] = {{dr, db}, color, {ar, ab}};  // R,B
        }
    }
}

// -- DirectMaskSubRun -------------------------------------------------------------------------
class DirectMaskSubRun final : public GrSubRun, public GrAtlasSubRun {
public:
    DirectMaskSubRun(const GrTextReferenceFrame* referenceFrame,
                         MaskFormat format,
                         SkGlyphRect deviceBounds,
                         SkSpan<const DevicePosition> devicePositions,
                         GlyphVector&& glyphs,
                         bool glyphsOutOfBounds);

    static GrSubRunOwner Make(const GrTextReferenceFrame* referenceFrame,
                              const SkZip<SkGlyphVariant, SkPoint>& accepted,
                              sk_sp<SkStrike>&& strike,
                              MaskFormat format,
                              SubRunAllocator* alloc);

    static GrSubRunOwner MakeFromBuffer(const GrTextReferenceFrame* referenceFrame,
                                        SkReadBuffer& buffer,
                                        SubRunAllocator* alloc,
                                        const SkStrikeClient* client);

    void draw(SkCanvas*,
              const GrClip* clip,
              const SkMatrixProvider& viewMatrix,
              SkPoint drawOrigin,
              const SkPaint& paint,
              skgpu::v1::SurfaceDrawContext* sdc) const override;

    int unflattenSize() const override;

    size_t vertexStride(const SkMatrix& drawMatrix) const override;

    int glyphCount() const override;

    std::tuple<const GrClip*, GrOp::Owner>
    makeAtlasTextOp(const GrClip*,
                    const SkMatrixProvider& viewMatrix,
                    SkPoint,
                    const SkPaint&,
                    skgpu::v1::SurfaceDrawContext*) const override;

    void testingOnly_packedGlyphIDToGlyph(StrikeCache *cache) const override;

    std::tuple<bool, int>
    regenerateAtlas(int begin, int end, GrMeshDrawTarget*) const override;

    void fillVertexData(void* vertexDst, int offset, int count,
                        GrColor color,
                        const SkMatrix& drawMatrix, SkPoint drawOrigin,
                        SkIRect clip) const override;

    bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const override;

    const GrAtlasSubRun* testingOnly_atlasSubRun() const override;

protected:
    SubRunType subRunType() const override { return kDirectMask; }
    void doFlatten(SkWriteBuffer& buffer) const override;

private:
    // Return true if the positionMatrix represents an integer translation. Return the device
    // bounding box of all the glyphs. If the bounding box is empty, then something went singular
    // and this operation should be dropped.
    std::tuple<bool, SkRect> deviceRectAndCheckTransform(const SkMatrix& positionMatrix) const;

    const GrTextReferenceFrame* const fReferenceFrame;
    const MaskFormat fMaskFormat;

    // The vertex bounds in device space. The bounds are the joined rectangles of all the glyphs.
    const SkGlyphRect fGlyphDeviceBounds;
    const SkSpan<const DevicePosition> fLeftTopDevicePos;
    const bool fSomeGlyphsExcluded;

    // The regenerateAtlas method mutates fGlyphs. It should be called from onPrepare which must
    // be single threaded.
    mutable GlyphVector fGlyphs;
};

DirectMaskSubRun::DirectMaskSubRun(const GrTextReferenceFrame* referenceFrame,
                                   MaskFormat format,
                                   SkGlyphRect deviceBounds,
                                   SkSpan<const DevicePosition> devicePositions,
                                   GlyphVector&& glyphs,
                                   bool glyphsOutOfBounds)
        : fReferenceFrame{referenceFrame}
        , fMaskFormat{format}
        , fGlyphDeviceBounds{deviceBounds}
        , fLeftTopDevicePos{devicePositions}
        , fSomeGlyphsExcluded{glyphsOutOfBounds}
        , fGlyphs{std::move(glyphs)} { }

GrSubRunOwner DirectMaskSubRun::Make(const GrTextReferenceFrame* referenceFrame,
                                     const SkZip<SkGlyphVariant, SkPoint>& accepted,
                                     sk_sp<SkStrike>&& strike,
                                     MaskFormat format,
                                     SubRunAllocator* alloc) {
    auto glyphLeftTop = alloc->makePODArray<DevicePosition>(accepted.size());
    auto glyphIDs = alloc->makePODArray<GlyphVector::Variant>(accepted.size());

    // Because this is the direct case, the maximum width or height is the size that fits in the
    // atlas. This boundary is checked below to ensure that the call to SkGlyphRect below will
    // not overflow.
    constexpr SkScalar kMaxPos =
            std::numeric_limits<int16_t>::max() - SkStrikeCommon::kSkSideTooBigForAtlas;
    SkGlyphRect runBounds = skglyph::empty_rect();
    size_t goodPosCount = 0;
    for (auto [variant, pos] : accepted) {
        auto [x, y] = pos;
        // Ensure that the .offset() call below does not overflow. And, at this point none of the
        // rectangles are empty because they were culled before the run was created. Basically,
        // cull all the glyphs that can't appear on the screen.
        if (-kMaxPos < x && x < kMaxPos && -kMaxPos  < y && y < kMaxPos) {
            const SkGlyph* const skGlyph = variant;
            const SkGlyphRect deviceBounds =
                    skGlyph->glyphRect().offset(SkScalarRoundToInt(x), SkScalarRoundToInt(y));
            runBounds = skglyph::rect_union(runBounds, deviceBounds);
            glyphLeftTop[goodPosCount] = deviceBounds.leftTop();
            glyphIDs[goodPosCount].packedGlyphID = skGlyph->getPackedID();
            goodPosCount += 1;
        }
    }

    // Wow! no glyphs are in bounds and had non-empty bounds.
    if (goodPosCount == 0) {
        return nullptr;
    }

    // If some glyphs were excluded by the bounds, then this subrun can't be generally be used
    // for other draws. Mark the subrun as not general.
    bool glyphsExcluded = goodPosCount != accepted.size();
    SkSpan<const DevicePosition> leftTop{glyphLeftTop, goodPosCount};
    return alloc->makeUnique<DirectMaskSubRun>(
            referenceFrame, format, runBounds, leftTop,
            GlyphVector{std::move(strike), {glyphIDs, goodPosCount}},
            glyphsExcluded);
}

bool DirectMaskSubRun::canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const {
    auto [reuse, translation] =
            can_use_direct(fReferenceFrame->initialPositionMatrix(), positionMatrix);

    // If glyphs were excluded because of position bounds, then this subrun can only be reused if
    // there is no change in position.
    if (fSomeGlyphsExcluded) {
        return translation.x() == 0 && translation.y() == 0;
    }

    return reuse;
}

GrSubRunOwner DirectMaskSubRun::MakeFromBuffer(const GrTextReferenceFrame* referenceFrame,
                                               SkReadBuffer& buffer,
                                               SubRunAllocator* alloc,
                                               const SkStrikeClient* client) {
    MaskFormat maskType = (MaskFormat)buffer.readInt();
    SkGlyphRect runBounds;
    pun_read(buffer, &runBounds);

    int glyphCount = buffer.readInt();
    if (!buffer.validate(check_glyph_count(buffer, glyphCount))) { return nullptr; }
    if (!buffer.validateCanReadN<DevicePosition>(glyphCount)) { return nullptr; }
    DevicePosition* positionsData = alloc->makePODArray<DevicePosition>(glyphCount);
    for (int i = 0; i < glyphCount; ++i) {
        pun_read(buffer, &positionsData[i]);
    }
    SkSpan<DevicePosition> positions(positionsData, glyphCount);

    auto glyphVector = GlyphVector::MakeFromBuffer(buffer, client, alloc);
    if (!buffer.validate(glyphVector.has_value())) { return nullptr; }
    if (!buffer.validate(SkCount(glyphVector->glyphs()) == glyphCount)) { return nullptr; }
    SkASSERT(buffer.isValid());
    return alloc->makeUnique<DirectMaskSubRun>(
            referenceFrame, maskType, runBounds, positions, std::move(glyphVector.value()), false);
}

void DirectMaskSubRun::doFlatten(SkWriteBuffer& buffer) const {
    buffer.writeInt(static_cast<int>(fMaskFormat));
    pun_write(buffer, fGlyphDeviceBounds);
    int glyphCount = SkTo<int>(fLeftTopDevicePos.size());
    buffer.writeInt(glyphCount);
    for (auto pos : fLeftTopDevicePos) {
        pun_write(buffer, pos);
    }
    fGlyphs.flatten(buffer);
}

int DirectMaskSubRun::unflattenSize() const {
    return sizeof(DirectMaskSubRun) +
           fGlyphs.unflattenSize() +
           sizeof(DevicePosition) * fGlyphs.glyphs().size();
}

const GrAtlasSubRun* DirectMaskSubRun::testingOnly_atlasSubRun() const {
    return this;
}

size_t DirectMaskSubRun::vertexStride(const SkMatrix& positionMatrix) const {
    if (!positionMatrix.hasPerspective()) {
        if (fMaskFormat != MaskFormat::kARGB) {
            return sizeof(Mask2DVertex);
        } else {
            return sizeof(ARGB2DVertex);
        }
    } else {
        if (fMaskFormat != MaskFormat::kARGB) {
            return sizeof(Mask3DVertex);
        } else {
            return sizeof(ARGB3DVertex);
        }
    }
}

int DirectMaskSubRun::glyphCount() const {
    return SkCount(fGlyphs.glyphs());
}

void DirectMaskSubRun::draw(SkCanvas*,
                                const GrClip* clip,
                                const SkMatrixProvider& viewMatrix,
                                SkPoint drawOrigin,
                                const SkPaint& paint,
                                skgpu::v1::SurfaceDrawContext* sdc) const {
    auto[drawingClip, op] = this->makeAtlasTextOp(clip, viewMatrix, drawOrigin, paint, sdc);
    if (op != nullptr) {
        sdc->addDrawOp(drawingClip, std::move(op));
    }
}

std::tuple<const GrClip*, GrOp::Owner> DirectMaskSubRun::makeAtlasTextOp(const GrClip* clip,
                                      const SkMatrixProvider& viewMatrix,
                                      SkPoint drawOrigin,
                                      const SkPaint& paint,
                                      skgpu::v1::SurfaceDrawContext* sdc) const {
    SkASSERT(this->glyphCount() != 0);
    const SkMatrix& drawMatrix = viewMatrix.localToDevice();
    const SkMatrix& positionMatrix = position_matrix(drawMatrix, drawOrigin);

    auto [integerTranslate, subRunDeviceBounds] = this->deviceRectAndCheckTransform(positionMatrix);
    if (subRunDeviceBounds.isEmpty()) {
        return {nullptr, nullptr};
    }
    // Rect for optimized bounds clipping when doing an integer translate.
    SkIRect geometricClipRect = SkIRect::MakeEmpty();
    if (integerTranslate) {
        // We can clip geometrically using clipRect and ignore clip when an axis-aligned rectangular
        // non-AA clip is used. If clipRect is empty, and clip is nullptr, then there is no clipping
        // needed.
        const SkRect deviceBounds = SkRect::MakeWH(sdc->width(), sdc->height());
        auto [clipMethod, clipRect] = calculate_clip(clip, deviceBounds, subRunDeviceBounds);

        switch (clipMethod) {
            case kClippedOut:
                // Returning nullptr as op means skip this op.
                return {nullptr, nullptr};
            case kUnclipped:
            case kGeometryClipped:
                // GPU clip is not needed.
                clip = nullptr;
                break;
            case kGPUClipped:
                // Use th GPU clip; clipRect is ignored.
                break;
        }
        geometricClipRect = clipRect;

        if (!geometricClipRect.isEmpty()) { SkASSERT(clip == nullptr); }
    }

    GrPaint grPaint;
    const SkPMColor4f drawingColor =
            calculate_colors(sdc, paint, viewMatrix, fMaskFormat, &grPaint);

    auto geometry = AtlasTextOp::Geometry::MakeForBlob(*this,
                                                       drawMatrix,
                                                       drawOrigin,
                                                       geometricClipRect,
                                                       sk_ref_sp(fReferenceFrame),
                                                       drawingColor,
                                                       sdc->arenaAlloc());

    GrRecordingContext* const rContext = sdc->recordingContext();
    GrOp::Owner op = GrOp::Make<AtlasTextOp>(rContext,
                                             op_mask_type(fMaskFormat),
                                             !integerTranslate,
                                             this->glyphCount(),
                                             subRunDeviceBounds,
                                             geometry,
                                             std::move(grPaint));
    return {clip, std::move(op)};
}

void DirectMaskSubRun::testingOnly_packedGlyphIDToGlyph(StrikeCache *cache) const {
    fGlyphs.packedGlyphIDToGlyph(cache);
}

std::tuple<bool, int> DirectMaskSubRun::regenerateAtlas(int begin, int end,
                                                        GrMeshDrawTarget* target) const {
    return fGlyphs.regenerateAtlas(begin, end, fMaskFormat, 0, target);
}

template<typename Quad, typename VertexData>
void transformed_direct_2D(SkZip<Quad, const Glyph*, const VertexData> quadData,
                           GrColor color,
                           const SkMatrix& matrix) {
    for (auto[quad, glyph, leftTop] : quadData) {
        auto[al, at, ar, ab] = glyph->fAtlasLocator.getUVs();
        SkScalar dl = leftTop[0],
                 dt = leftTop[1],
                 dr = dl + (ar - al),
                 db = dt + (ab - at);
        SkPoint lt = matrix.mapXY(dl, dt),
                lb = matrix.mapXY(dl, db),
                rt = matrix.mapXY(dr, dt),
                rb = matrix.mapXY(dr, db);
        quad[0] = {lt, color, {al, at}};  // L,T
        quad[1] = {lb, color, {al, ab}};  // L,B
        quad[2] = {rt, color, {ar, at}};  // R,T
        quad[3] = {rb, color, {ar, ab}};  // R,B
    }
}

template<typename Quad, typename VertexData>
void transformed_direct_3D(SkZip<Quad, const Glyph*, const VertexData> quadData,
                           GrColor color,
                           const SkMatrix& matrix) {
    auto mapXYZ = [&](SkScalar x, SkScalar y) {
        SkPoint pt{x, y};
        SkPoint3 result;
        matrix.mapHomogeneousPoints(&result, &pt, 1);
        return result;
    };
    for (auto[quad, glyph, leftTop] : quadData) {
        auto[al, at, ar, ab] = glyph->fAtlasLocator.getUVs();
        SkScalar dl = leftTop[0],
                 dt = leftTop[1],
                 dr = dl + (ar - al),
                 db = dt + (ab - at);
        SkPoint3 lt = mapXYZ(dl, dt),
                 lb = mapXYZ(dl, db),
                 rt = mapXYZ(dr, dt),
                 rb = mapXYZ(dr, db);
        quad[0] = {lt, color, {al, at}};  // L,T
        quad[1] = {lb, color, {al, ab}};  // L,B
        quad[2] = {rt, color, {ar, at}};  // R,T
        quad[3] = {rb, color, {ar, ab}};  // R,B
    }
}

void DirectMaskSubRun::fillVertexData(void* vertexDst, int offset, int count,
                                          GrColor color,
                                          const SkMatrix& drawMatrix, SkPoint drawOrigin,
                                          SkIRect clip) const {
    auto quadData = [&](auto dst) {
        return SkMakeZip(dst,
                         fGlyphs.glyphs().subspan(offset, count),
                         fLeftTopDevicePos.subspan(offset, count));
    };

    const SkMatrix positionMatrix = position_matrix(drawMatrix, drawOrigin);
    auto [noTransformNeeded, originOffset] =
            can_use_direct(fReferenceFrame->initialPositionMatrix(), positionMatrix);

    if (noTransformNeeded) {
        if (clip.isEmpty()) {
            if (fMaskFormat != MaskFormat::kARGB) {
                using Quad = Mask2DVertex[4];
                SkASSERT(sizeof(Mask2DVertex) == this->vertexStride(SkMatrix::I()));
                direct_2D(quadData((Quad*)vertexDst), color, originOffset);
            } else {
                using Quad = ARGB2DVertex[4];
                SkASSERT(sizeof(ARGB2DVertex) == this->vertexStride(SkMatrix::I()));
                generalized_direct_2D(quadData((Quad*)vertexDst), color, originOffset);
            }
        } else {
            if (fMaskFormat != MaskFormat::kARGB) {
                using Quad = Mask2DVertex[4];
                SkASSERT(sizeof(Mask2DVertex) == this->vertexStride(SkMatrix::I()));
                generalized_direct_2D(quadData((Quad*)vertexDst), color, originOffset, &clip);
            } else {
                using Quad = ARGB2DVertex[4];
                SkASSERT(sizeof(ARGB2DVertex) == this->vertexStride(SkMatrix::I()));
                generalized_direct_2D(quadData((Quad*)vertexDst), color, originOffset, &clip);
            }
        }
    } else if (SkMatrix inverse; fReferenceFrame->initialPositionMatrix().invert(&inverse)) {
        SkMatrix viewDifference = SkMatrix::Concat(positionMatrix, inverse);
        if (!viewDifference.hasPerspective()) {
            if (fMaskFormat != MaskFormat::kARGB) {
                using Quad = Mask2DVertex[4];
                SkASSERT(sizeof(Mask2DVertex) == this->vertexStride(positionMatrix));
                transformed_direct_2D(quadData((Quad*)vertexDst), color, viewDifference);
            } else {
                using Quad = ARGB2DVertex[4];
                SkASSERT(sizeof(ARGB2DVertex) == this->vertexStride(positionMatrix));
                transformed_direct_2D(quadData((Quad*)vertexDst), color, viewDifference);
            }
        } else {
            if (fMaskFormat != MaskFormat::kARGB) {
                using Quad = Mask3DVertex[4];
                SkASSERT(sizeof(Mask3DVertex) == this->vertexStride(positionMatrix));
                transformed_direct_3D(quadData((Quad*)vertexDst), color, viewDifference);
            } else {
                using Quad = ARGB3DVertex[4];
                SkASSERT(sizeof(ARGB3DVertex) == this->vertexStride(positionMatrix));
                transformed_direct_3D(quadData((Quad*)vertexDst), color, viewDifference);
            }
        }
    }
}

// true if only need to translate by integer amount, device rect.
std::tuple<bool, SkRect> DirectMaskSubRun::deviceRectAndCheckTransform(
            const SkMatrix& positionMatrix) const {
    const SkMatrix& initialMatrix = fReferenceFrame->initialPositionMatrix();
    const SkPoint offset = positionMatrix.mapOrigin() - initialMatrix.mapOrigin();

    const bool compatibleMatrix = positionMatrix[0] == initialMatrix[0] &&
                                  positionMatrix[1] == initialMatrix[1] &&
                                  positionMatrix[3] == initialMatrix[3] &&
                                  positionMatrix[4] == initialMatrix[4] &&
                                  !positionMatrix.hasPerspective() &&
                                  !initialMatrix.hasPerspective();

    if (compatibleMatrix && SkScalarIsInt(offset.x()) && SkScalarIsInt(offset.y())) {
        // Handle the integer offset case.
        // The offset should be integer, but make sure.
        SkIVector iOffset = {SkScalarRoundToInt(offset.x()), SkScalarRoundToInt(offset.y())};

        SkIRect outBounds = fGlyphDeviceBounds.iRect();
        return {true, SkRect::Make(outBounds.makeOffset(iOffset))};
    } else if (SkMatrix inverse; fReferenceFrame->initialPositionMatrix().invert(&inverse)) {
        SkMatrix viewDifference = SkMatrix::Concat(positionMatrix, inverse);
        return {false, viewDifference.mapRect(fGlyphDeviceBounds.rect())};
    }

    // initialPositionMatrix is singular. Do nothing.
    return {false, SkRect::MakeEmpty()};
}

// -- TransformedMaskSubRun ------------------------------------------------------------------------
class TransformedMaskSubRun final : public GrSubRun, public GrAtlasSubRun {
public:
    TransformedMaskSubRun(const GrTextReferenceFrame* referenceFrame,
                          TransformedMaskVertexFiller&& vertexFiller,
                          GlyphVector&& glyphs);

    static GrSubRunOwner Make(const GrTextReferenceFrame* referenceFrame,
                              const SkZip<SkGlyphVariant, SkPoint>& accepted,
                              sk_sp<SkStrike>&& strike,
                              SkScalar strikeToSourceScale,
                              MaskFormat maskType,
                              SubRunAllocator* alloc);

    static GrSubRunOwner MakeFromBuffer(const GrTextReferenceFrame* referenceFrame,
                                        SkReadBuffer& buffer,
                                        SubRunAllocator* alloc,
                                        const SkStrikeClient* client);

    void draw(SkCanvas*,
              const GrClip*,
              const SkMatrixProvider& viewMatrix,
              SkPoint drawOrigin,
              const SkPaint& paint,
              skgpu::v1::SurfaceDrawContext*) const override;

    std::tuple<const GrClip*, GrOp::Owner>
    makeAtlasTextOp(const GrClip*,
                    const SkMatrixProvider& viewMatrix,
                    SkPoint drawOrigin,
                    const SkPaint&,
                    skgpu::v1::SurfaceDrawContext*) const override;

    int unflattenSize() const override;

    bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const override;

    const GrAtlasSubRun* testingOnly_atlasSubRun() const override;

    void testingOnly_packedGlyphIDToGlyph(StrikeCache *cache) const override;

    std::tuple<bool, int> regenerateAtlas(int begin, int end, GrMeshDrawTarget*) const override;

    void fillVertexData(
            void* vertexDst, int offset, int count,
            GrColor color,
            const SkMatrix& drawMatrix, SkPoint drawOrigin,
            SkIRect clip) const override;

    size_t vertexStride(const SkMatrix& drawMatrix) const override;
    int glyphCount() const override;

protected:
    SubRunType subRunType() const override { return kTransformMask; }
    void doFlatten(SkWriteBuffer& buffer) const override;

private:
    // The rectangle that surrounds all the glyph bounding boxes in device space.
    SkRect deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const;

    const GrTextReferenceFrame* const fReferenceFrame;

    const TransformedMaskVertexFiller fVertexFiller;

    // The regenerateAtlas method mutates fGlyphs. It should be called from onPrepare which must
    // be single threaded.
    mutable GlyphVector fGlyphs;
};

TransformedMaskSubRun::TransformedMaskSubRun(const GrTextReferenceFrame* referenceFrame,
                                             TransformedMaskVertexFiller&& vertexFiller,
                                             GlyphVector&& glyphs)
        : fReferenceFrame{referenceFrame}
        , fVertexFiller{std::move(vertexFiller)}
        , fGlyphs{std::move(glyphs)} { }

GrSubRunOwner TransformedMaskSubRun::Make(const GrTextReferenceFrame* referenceFrame,
                                          const SkZip<SkGlyphVariant, SkPoint>& accepted,
                                          sk_sp<SkStrike>&& strike,
                                          SkScalar strikeToSourceScale,
                                          MaskFormat maskType,
                                          SubRunAllocator* alloc) {
    auto vertexFiller = TransformedMaskVertexFiller::Make(
            maskType, 0, strikeToSourceScale, accepted, alloc);

    auto glyphVector = GlyphVector::Make(std::move(strike), accepted.get<0>(), alloc);

    return alloc->makeUnique<TransformedMaskSubRun>(
            referenceFrame, std::move(vertexFiller), std::move(glyphVector));
}

GrSubRunOwner TransformedMaskSubRun::MakeFromBuffer(const GrTextReferenceFrame* referenceFrame,
                                                    SkReadBuffer& buffer,
                                                    SubRunAllocator* alloc,
                                                    const SkStrikeClient* client) {
    auto vertexFiller = TransformedMaskVertexFiller::MakeFromBuffer(buffer, alloc);
    if (!buffer.validate(vertexFiller.has_value())) { return {}; }

    auto glyphVector = GlyphVector::MakeFromBuffer(buffer, client, alloc);
    if (!buffer.validate(glyphVector.has_value())) { return {}; }
    if (!buffer.validate(SkCount(glyphVector->glyphs()) == vertexFiller->count())) { return {}; }
    return alloc->makeUnique<TransformedMaskSubRun>(
            referenceFrame, std::move(*vertexFiller), std::move(*glyphVector));
}

int TransformedMaskSubRun::unflattenSize() const {
    return sizeof(TransformedMaskSubRun) + fGlyphs.unflattenSize() + fVertexFiller.unflattenSize();
}

void TransformedMaskSubRun::doFlatten(SkWriteBuffer& buffer) const {
    fVertexFiller.flatten(buffer);
    fGlyphs.flatten(buffer);
}

void TransformedMaskSubRun::draw(SkCanvas*,
                                 const GrClip* clip,
                                 const SkMatrixProvider& viewMatrix,
                                 SkPoint drawOrigin,
                                 const SkPaint& paint,
                                 skgpu::v1::SurfaceDrawContext* sdc) const {
    auto[drawingClip, op] = this->makeAtlasTextOp(clip, viewMatrix, drawOrigin, paint, sdc);
    if (op != nullptr) {
        sdc->addDrawOp(drawingClip, std::move(op));
    }
}

std::tuple<const GrClip*, GrOp::Owner>
TransformedMaskSubRun::makeAtlasTextOp(const GrClip* clip,
                                       const SkMatrixProvider& viewMatrix,
                                       SkPoint drawOrigin,
                                       const SkPaint& paint,
                                       skgpu::v1::SurfaceDrawContext* sdc) const {
    SkASSERT(this->glyphCount() != 0);

    const SkMatrix& drawMatrix = viewMatrix.localToDevice();

    GrPaint grPaint;
    SkPMColor4f drawingColor = calculate_colors(
            sdc, paint, viewMatrix, fVertexFiller.grMaskType(), &grPaint);

    auto geometry = AtlasTextOp::Geometry::MakeForBlob(*this,
                                                       drawMatrix,
                                                       drawOrigin,
                                                       SkIRect::MakeEmpty(),
                                                       sk_ref_sp(fReferenceFrame),
                                                       drawingColor,
                                                       sdc->arenaAlloc());

    GrRecordingContext* const rContext = sdc->recordingContext();
    GrOp::Owner op = GrOp::Make<AtlasTextOp>(rContext,
                                             fVertexFiller.opMaskType(),
                                             true,
                                             this->glyphCount(),
                                             this->deviceRect(drawMatrix, drawOrigin),
                                             geometry,
                                             std::move(grPaint));
    return {clip, std::move(op)};
}

// If we are not scaling the cache entry to be larger, than a cache with smaller glyphs may be
// better.
bool TransformedMaskSubRun::canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const {
    if (fReferenceFrame->initialPositionMatrix().getMaxScale() < 1) {
        return false;
    }
    return true;
}

void TransformedMaskSubRun::testingOnly_packedGlyphIDToGlyph(StrikeCache *cache) const {
    fGlyphs.packedGlyphIDToGlyph(cache);
}

std::tuple<bool, int> TransformedMaskSubRun::regenerateAtlas(int begin, int end,
                                                             GrMeshDrawTarget* target) const {
    return fGlyphs.regenerateAtlas(begin, end, fVertexFiller.grMaskType(), 1, target);
}

void TransformedMaskSubRun::fillVertexData(void* vertexDst, int offset, int count,
                                           GrColor color,
                                           const SkMatrix& drawMatrix, SkPoint drawOrigin,
                                           SkIRect clip) const {
    const SkMatrix positionMatrix = position_matrix(drawMatrix, drawOrigin);
    fVertexFiller.fillVertexData(offset, count,
                                 fGlyphs.glyphs(),
                                 color,
                                 positionMatrix,
                                 clip,
                                 vertexDst);
}

size_t TransformedMaskSubRun::vertexStride(const SkMatrix& drawMatrix) const {
    return fVertexFiller.vertexStride(drawMatrix);
}

int TransformedMaskSubRun::glyphCount() const {
    return SkCount(fGlyphs.glyphs());
}

SkRect TransformedMaskSubRun::deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const {
    return fVertexFiller.deviceRect(drawMatrix, drawOrigin);
}

const GrAtlasSubRun* TransformedMaskSubRun::testingOnly_atlasSubRun() const {
    return this;
}

// -- SDFTSubRun -----------------------------------------------------------------------------------
class SDFTSubRun final : public GrSubRun, public GrAtlasSubRun {
public:
    SDFTSubRun(const GrTextReferenceFrame* referenceFrame,
               bool useLCDText,
               bool antiAliased,
               const GrSDFTMatrixRange& matrixRange,
               TransformedMaskVertexFiller&& vertexFiller,
               GlyphVector&& glyphs);

    static GrSubRunOwner Make(const GrTextReferenceFrame* referenceFrame,
                              const SkZip<SkGlyphVariant, SkPoint>& accepted,
                              const SkFont& runFont,
                              sk_sp<SkStrike>&& strike,
                              SkScalar strikeToSourceScale,
                              const GrSDFTMatrixRange& matrixRange,
                              SubRunAllocator* alloc);

    static GrSubRunOwner MakeFromBuffer(const GrTextReferenceFrame* referenceFrame,
                                        SkReadBuffer& buffer,
                                        SubRunAllocator* alloc,
                                        const SkStrikeClient* client);

    void draw(SkCanvas*,
              const GrClip*,
              const SkMatrixProvider& viewMatrix,
              SkPoint drawOrigin,
              const SkPaint&,
              skgpu::v1::SurfaceDrawContext*) const override;

    std::tuple<const GrClip*, GrOp::Owner>
    makeAtlasTextOp(const GrClip*,
                    const SkMatrixProvider& viewMatrix,
                    SkPoint drawOrigin,
                    const SkPaint&,
                    skgpu::v1::SurfaceDrawContext*) const override;

    int unflattenSize() const override;

    bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const override;

    const GrAtlasSubRun* testingOnly_atlasSubRun() const override;

    void testingOnly_packedGlyphIDToGlyph(StrikeCache *cache) const override;

    std::tuple<bool, int> regenerateAtlas(int begin, int end, GrMeshDrawTarget*) const override;

    void fillVertexData(
            void* vertexDst, int offset, int count,
            GrColor color,
            const SkMatrix& drawMatrix, SkPoint drawOrigin,
            SkIRect clip) const override;

    size_t vertexStride(const SkMatrix& drawMatrix) const override;
    int glyphCount() const override;

protected:
    SubRunType subRunType() const override { return kSDFT; }
    void doFlatten(SkWriteBuffer& buffer) const override;

private:
    // The rectangle that surrounds all the glyph bounding boxes in device space.
    SkRect deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const;

    const GrTextReferenceFrame* const fReferenceFrame;
    const bool fUseLCDText;
    const bool fAntiAliased;
    const GrSDFTMatrixRange fMatrixRange;

    const TransformedMaskVertexFiller fVertexFiller;

    // The regenerateAtlas method mutates fGlyphs. It should be called from onPrepare which must
    // be single threaded.
    mutable GlyphVector fGlyphs;
};

SDFTSubRun::SDFTSubRun(const GrTextReferenceFrame* referenceFrame,
                       bool useLCDText,
                       bool antiAliased,
                       const GrSDFTMatrixRange& matrixRange,
                       TransformedMaskVertexFiller&& vertexFiller,
                       GlyphVector&& glyphs)
        : fReferenceFrame{referenceFrame}
        , fUseLCDText{useLCDText}
        , fAntiAliased{antiAliased}
        , fMatrixRange{matrixRange}
        , fVertexFiller{std::move(vertexFiller)}
        , fGlyphs{std::move(glyphs)} {}

bool has_some_antialiasing(const SkFont& font ) {
    SkFont::Edging edging = font.getEdging();
    return edging == SkFont::Edging::kAntiAlias
           || edging == SkFont::Edging::kSubpixelAntiAlias;
}

GrSubRunOwner SDFTSubRun::Make(const GrTextReferenceFrame* referenceFrame,
                               const SkZip<SkGlyphVariant, SkPoint>& accepted,
                               const SkFont& runFont,
                               sk_sp<SkStrike>&& strike,
                               SkScalar strikeToSourceScale,
                               const GrSDFTMatrixRange& matrixRange,
                               SubRunAllocator* alloc) {
    auto vertexFiller = TransformedMaskVertexFiller::Make(
            MaskFormat::kA8,
            SK_DistanceFieldInset,
            strikeToSourceScale,
            accepted,
            alloc);

    auto glyphVector = GlyphVector::Make(std::move(strike), accepted.get<0>(), alloc);

    return alloc->makeUnique<SDFTSubRun>(
            referenceFrame,
            runFont.getEdging() == SkFont::Edging::kSubpixelAntiAlias,
            has_some_antialiasing(runFont),
            matrixRange,
            std::move(vertexFiller),
            std::move(glyphVector));
}

GrSubRunOwner SDFTSubRun::MakeFromBuffer(const GrTextReferenceFrame* referenceFrame,
                                         SkReadBuffer& buffer,
                                         SubRunAllocator* alloc,
                                         const SkStrikeClient* client) {
    int useLCD = buffer.readInt();
    int isAntiAliased = buffer.readInt();
    GrSDFTMatrixRange matrixRange = GrSDFTMatrixRange::MakeFromBuffer(buffer);
    auto vertexFiller = TransformedMaskVertexFiller::MakeFromBuffer(buffer, alloc);
    if (!buffer.validate(vertexFiller.has_value())) { return {}; }
    auto glyphVector = GlyphVector::MakeFromBuffer(buffer, client, alloc);
    if (!buffer.validate(glyphVector.has_value())) { return {}; }
    if (!buffer.validate(SkCount(glyphVector->glyphs()) == vertexFiller->count())) { return {}; }
    return alloc->makeUnique<SDFTSubRun>(referenceFrame,
                                         useLCD,
                                         isAntiAliased,
                                         matrixRange,
                                         std::move(*vertexFiller),
                                         std::move(*glyphVector));
}

int SDFTSubRun::unflattenSize() const {
    return sizeof(SDFTSubRun) + fGlyphs.unflattenSize() + fVertexFiller.unflattenSize();
}

void SDFTSubRun::doFlatten(SkWriteBuffer& buffer) const {
    buffer.writeInt(fUseLCDText);
    buffer.writeInt(fAntiAliased);
    fMatrixRange.flatten(buffer);
    fVertexFiller.flatten(buffer);
    fGlyphs.flatten(buffer);
}

void SDFTSubRun::draw(SkCanvas*,
                      const GrClip* clip,
                      const SkMatrixProvider& viewMatrix,
                      SkPoint drawOrigin,
                      const SkPaint& paint,
                      skgpu::v1::SurfaceDrawContext* sdc) const {
    auto[drawingClip, op] = this->makeAtlasTextOp(clip, viewMatrix, drawOrigin, paint, sdc);
    if (op != nullptr) {
        sdc->addDrawOp(drawingClip, std::move(op));
    }
}

static std::tuple<AtlasTextOp::MaskType, uint32_t, bool> calculate_sdf_parameters(
        const skgpu::v1::SurfaceDrawContext& sdc,
        const SkMatrix& drawMatrix,
        bool useLCDText,
        bool isAntiAliased) {
    const GrColorInfo& colorInfo = sdc.colorInfo();
    const SkSurfaceProps& props = sdc.surfaceProps();
    bool isBGR = SkPixelGeometryIsBGR(props.pixelGeometry());
    bool isLCD = useLCDText && SkPixelGeometryIsH(props.pixelGeometry());
    using MT = AtlasTextOp::MaskType;
    MT maskType = !isAntiAliased ? MT::kAliasedDistanceField
                                 : isLCD ? (isBGR ? MT::kLCDBGRDistanceField
                                                  : MT::kLCDDistanceField)
                                         : MT::kGrayscaleDistanceField;

    bool useGammaCorrectDistanceTable = colorInfo.isLinearlyBlended();
    uint32_t DFGPFlags = drawMatrix.isSimilarity() ? kSimilarity_DistanceFieldEffectFlag : 0;
    DFGPFlags |= drawMatrix.isScaleTranslate() ? kScaleOnly_DistanceFieldEffectFlag : 0;
    DFGPFlags |= useGammaCorrectDistanceTable ? kGammaCorrect_DistanceFieldEffectFlag : 0;
    DFGPFlags |= MT::kAliasedDistanceField == maskType ? kAliased_DistanceFieldEffectFlag : 0;

    if (isLCD) {
        DFGPFlags |= kUseLCD_DistanceFieldEffectFlag;
        DFGPFlags |= MT::kLCDBGRDistanceField == maskType ? kBGR_DistanceFieldEffectFlag : 0;
    }
    return {maskType, DFGPFlags, useGammaCorrectDistanceTable};
}

std::tuple<const GrClip*, GrOp::Owner >
SDFTSubRun::makeAtlasTextOp(const GrClip* clip,
                            const SkMatrixProvider& viewMatrix,
                            SkPoint drawOrigin,
                            const SkPaint& paint,
                            skgpu::v1::SurfaceDrawContext* sdc) const {
    SkASSERT(this->glyphCount() != 0);

    const SkMatrix& drawMatrix = viewMatrix.localToDevice();

    GrPaint grPaint;
    SkPMColor4f drawingColor = calculate_colors(sdc, paint, viewMatrix, MaskFormat::kA8,
                                                &grPaint);

    auto [maskType, DFGPFlags, useGammaCorrectDistanceTable] =
        calculate_sdf_parameters(*sdc, drawMatrix, fUseLCDText, fAntiAliased);

    auto geometry = AtlasTextOp::Geometry::MakeForBlob(*this,
                                                       drawMatrix,
                                                       drawOrigin,
                                                       SkIRect::MakeEmpty(),
                                                       sk_ref_sp(fReferenceFrame),
                                                       drawingColor,
                                                       sdc->arenaAlloc());

    GrRecordingContext* const rContext = sdc->recordingContext();
    GrOp::Owner op = GrOp::Make<AtlasTextOp>(rContext,
                                             maskType,
                                             true,
                                             this->glyphCount(),
                                             this->deviceRect(drawMatrix, drawOrigin),
                                             SkPaintPriv::ComputeLuminanceColor(paint),
                                             useGammaCorrectDistanceTable,
                                             DFGPFlags,
                                             geometry,
                                             std::move(grPaint));

    return {clip, std::move(op)};
}

bool SDFTSubRun::canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const {
    return fMatrixRange.matrixInRange(positionMatrix);
}

void SDFTSubRun::testingOnly_packedGlyphIDToGlyph(StrikeCache *cache) const {
    fGlyphs.packedGlyphIDToGlyph(cache);
}

std::tuple<bool, int>
SDFTSubRun::regenerateAtlas(int begin, int end, GrMeshDrawTarget *target) const {
    return fGlyphs.regenerateAtlas(begin, end, MaskFormat::kA8, SK_DistanceFieldInset,
                                   target);
}

size_t SDFTSubRun::vertexStride(const SkMatrix& drawMatrix) const {
    return sizeof(Mask2DVertex);
}

void SDFTSubRun::fillVertexData(
        void *vertexDst, int offset, int count,
        GrColor color,
        const SkMatrix& drawMatrix, SkPoint drawOrigin,
        SkIRect clip) const {
    const SkMatrix positionMatrix = position_matrix(drawMatrix, drawOrigin);

    fVertexFiller.fillVertexData(offset, count,
                                 fGlyphs.glyphs(),
                                 color,
                                 positionMatrix,
                                 clip,
                                 vertexDst);
}

int SDFTSubRun::glyphCount() const {
    return fVertexFiller.count();
}

SkRect SDFTSubRun::deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const {
    return fVertexFiller.deviceRect(drawMatrix, drawOrigin);
}

const GrAtlasSubRun* SDFTSubRun::testingOnly_atlasSubRun() const {
    return this;
}

template<typename AddSingleMaskFormat>
void add_multi_mask_format(
        AddSingleMaskFormat addSingleMaskFormat,
        const SkZip<SkGlyphVariant, SkPoint>& accepted,
        sk_sp<SkStrike>&& strike) {
    if (accepted.empty()) { return; }

    auto glyphSpan = accepted.get<0>();
    const SkGlyph* glyph = glyphSpan[0];
    MaskFormat format = Glyph::FormatFromSkGlyph(glyph->maskFormat());
    size_t startIndex = 0;
    for (size_t i = 1; i < accepted.size(); i++) {
        glyph = glyphSpan[i];
        MaskFormat nextFormat = Glyph::FormatFromSkGlyph(glyph->maskFormat());
        if (format != nextFormat) {
            auto glyphsWithSameFormat = accepted.subspan(startIndex, i - startIndex);
            // Take a ref on the strike. This should rarely happen.
            addSingleMaskFormat(glyphsWithSameFormat, format, sk_sp<SkStrike>(strike));
            format = nextFormat;
            startIndex = i;
        }
    }
    auto glyphsWithSameFormat = accepted.last(accepted.size() - startIndex);
    addSingleMaskFormat(glyphsWithSameFormat, format, std::move(strike));
}

}  // namespace

// -- GrTextBlob::Key ------------------------------------------------------------------------------

static SkColor compute_canonical_color(const SkPaint& paint, bool lcd) {
    SkColor canonicalColor = SkPaintPriv::ComputeLuminanceColor(paint);
    if (lcd) {
        // This is the correct computation for canonicalColor, but there are tons of cases where LCD
        // can be modified. For now we just regenerate if any run in a textblob has LCD.
        // TODO figure out where all of these modifications are and see if we can incorporate that
        //      logic at a higher level *OR* use sRGB
        //canonicalColor = SkMaskGamma::CanonicalColor(canonicalColor);

        // TODO we want to figure out a way to be able to use the canonical color on LCD text,
        // see the note above.  We pick a placeholder value for LCD text to ensure we always match
        // the same key
        return SK_ColorTRANSPARENT;
    } else {
        // A8, though can have mixed BMP text but it shouldn't matter because BMP text won't have
        // gamma corrected masks anyways, nor color
        U8CPU lum = SkComputeLuminance(SkColorGetR(canonicalColor),
                                       SkColorGetG(canonicalColor),
                                       SkColorGetB(canonicalColor));
        // reduce to our finite number of bits
        canonicalColor = SkMaskGamma::CanonicalColor(SkColorSetRGB(lum, lum, lum));
    }
    return canonicalColor;
}

auto GrTextBlob::Key::Make(const SkGlyphRunList& glyphRunList,
                           const SkPaint& paint,
                           const SkSurfaceProps& surfaceProps,
                           const GrColorInfo& colorInfo,
                           const SkMatrix& drawMatrix,
                           const GrSDFTControl& control) -> std::tuple<bool, Key> {
    SkMaskFilterBase::BlurRec blurRec;
    // It might be worth caching these things, but its not clear at this time
    // TODO for animated mask filters, this will fill up our cache.  We need a safeguard here
    const SkMaskFilter* maskFilter = paint.getMaskFilter();
    bool canCache = glyphRunList.canCache() &&
                    !(paint.getPathEffect() ||
                        (maskFilter && !as_MFB(maskFilter)->asABlur(&blurRec)));

    // If we're doing linear blending, then we can disable the gamma hacks.
    // Otherwise, leave them on. In either case, we still want the contrast boost:
    // TODO: Can we be even smarter about mask gamma based on the dest transfer function?
    SkScalerContextFlags scalerContextFlags = colorInfo.isLinearlyBlended()
                                              ? SkScalerContextFlags::kBoostContrast
                                              : SkScalerContextFlags::kFakeGammaAndBoostContrast;

    GrTextBlob::Key key;
    if (canCache) {
        bool hasLCD = glyphRunList.anyRunsLCD();

        // We canonicalize all non-lcd draws to use kUnknown_SkPixelGeometry
        SkPixelGeometry pixelGeometry =
                hasLCD ? surfaceProps.pixelGeometry() : kUnknown_SkPixelGeometry;

        GrColor canonicalColor = compute_canonical_color(paint, hasLCD);

        key.fPixelGeometry = pixelGeometry;
        key.fUniqueID = glyphRunList.uniqueID();
        key.fStyle = paint.getStyle();
        if (key.fStyle != SkPaint::kFill_Style) {
            key.fFrameWidth = paint.getStrokeWidth();
            key.fMiterLimit = paint.getStrokeMiter();
            key.fJoin = paint.getStrokeJoin();
        }
        key.fHasBlur = maskFilter != nullptr;
        if (key.fHasBlur) {
            key.fBlurRec = blurRec;
        }
        key.fCanonicalColor = canonicalColor;
        key.fScalerContextFlags = SkTo<uint32_t>(scalerContextFlags);

        // Do any runs use direct drawing types?.
        key.fHasSomeDirectSubRuns = false;
        for (auto& run : glyphRunList) {
            SkScalar approximateDeviceTextSize =
                    SkFontPriv::ApproximateTransformedTextSize(run.font(), drawMatrix);
            key.fHasSomeDirectSubRuns |= control.isDirect(approximateDeviceTextSize, paint);
        }

        if (key.fHasSomeDirectSubRuns) {
            // Store the fractional offset of the position. We know that the matrix can't be
            // perspective at this point.
            SkPoint mappedOrigin = drawMatrix.mapOrigin();
            key.fPositionMatrix = drawMatrix;
            key.fPositionMatrix.setTranslateX(
                    mappedOrigin.x() - SkScalarFloorToScalar(mappedOrigin.x()));
            key.fPositionMatrix.setTranslateY(
                    mappedOrigin.y() - SkScalarFloorToScalar(mappedOrigin.y()));
        } else {
            // For path and SDFT, the matrix doesn't matter.
            key.fPositionMatrix = SkMatrix::I();
        }
    }

    return {canCache, key};
}

bool GrTextBlob::Key::operator==(const GrTextBlob::Key& that) const {
    if (fUniqueID != that.fUniqueID) { return false; }
    if (fCanonicalColor != that.fCanonicalColor) { return false; }
    if (fStyle != that.fStyle) { return false; }
    if (fStyle != SkPaint::kFill_Style) {
        if (fFrameWidth != that.fFrameWidth ||
            fMiterLimit != that.fMiterLimit ||
            fJoin != that.fJoin) {
            return false;
        }
    }
    if (fPixelGeometry != that.fPixelGeometry) { return false; }
    if (fHasBlur != that.fHasBlur) { return false; }
    if (fHasBlur) {
        if (fBlurRec.fStyle != that.fBlurRec.fStyle || fBlurRec.fSigma != that.fBlurRec.fSigma) {
            return false;
        }
    }
    if (fScalerContextFlags != that.fScalerContextFlags) { return false; }

    // Just punt on perspective.
    if (fPositionMatrix.hasPerspective()) {
        return false;
    }

    if (fHasSomeDirectSubRuns != that.fHasSomeDirectSubRuns) {
        return false;
    }

    if (fHasSomeDirectSubRuns) {
        auto [compatible, _] = can_use_direct(fPositionMatrix, that.fPositionMatrix);
        return compatible;
    }

    return true;
}

// -- GrTextBlob -----------------------------------------------------------------------------------
void GrTextBlob::operator delete(void* p) { ::operator delete(p); }
void* GrTextBlob::operator new(size_t) { SK_ABORT("All blobs are created by placement new."); }
void* GrTextBlob::operator new(size_t, void* p) { return p; }

GrTextBlob::~GrTextBlob() = default;

sk_sp<GrTextBlob> GrTextBlob::Make(const SkGlyphRunList& glyphRunList,
                                   const SkPaint& paint,
                                   const SkMatrix& positionMatrix,
                                   const GrSDFTControl& control,
                                   SkGlyphRunListPainter* painter) {
    // The difference in alignment from the per-glyph data to the SubRun;
    constexpr size_t alignDiff = alignof(DirectMaskSubRun) - alignof(DevicePosition);
    constexpr size_t vertexDataToSubRunPadding = alignDiff > 0 ? alignDiff : 0;
    size_t totalGlyphCount = glyphRunList.totalGlyphCount();

    // The neededForSubRun is optimized for DirectMaskSubRun which is by far the most common case.
    size_t subRunSizeHint =
            totalGlyphCount * sizeof(DevicePosition)
            + GlyphVector::GlyphVectorSize(totalGlyphCount)
            + glyphRunList.runCount() * (sizeof(DirectMaskSubRun) + vertexDataToSubRunPadding);

    auto [initializer, totalMemoryAllocated, alloc] =
            SubRunAllocator::AllocateClassMemoryAndArena<GrTextBlob>(subRunSizeHint);
    SkColor initialLuminance = SkPaintPriv::ComputeLuminanceColor(paint);
    sk_sp<GrTextBlob> blob = sk_sp<GrTextBlob>(initializer.initialize(
            std::move(alloc), totalMemoryAllocated, positionMatrix, initialLuminance));

    const uint64_t uniqueID = glyphRunList.uniqueID();
    for (auto& glyphRun : glyphRunList) {
        painter->processGlyphRun(blob.get(),
                                 glyphRun,
                                 positionMatrix,
                                 paint,
                                 control,
                                 "GrTextBlob",
                                 uniqueID);
    }

    return blob;
}

void GrTextBlob::addKey(const Key& key) {
    fKey = key;
}

bool GrTextBlob::hasPerspective() const { return fInitialPositionMatrix.hasPerspective(); }

bool GrTextBlob::canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const {
    // A singular matrix will create a GrTextBlob with no SubRuns, but unknown glyphs can
    // also cause empty runs. If there are no subRuns or some glyphs were excluded or perspective,
    // then regenerate when the matrices don't match.
    if ((fSubRunList.isEmpty() || fSomeGlyphsExcluded || hasPerspective()) &&
        fInitialPositionMatrix != positionMatrix)
    {
        return false;
    }

    // If we have LCD text then our canonical color will be set to transparent, in this case we have
    // to regenerate the blob on any color change
    // We use the grPaint to get any color filter effects
    if (fKey.fCanonicalColor == SK_ColorTRANSPARENT &&
        fInitialLuminance != SkPaintPriv::ComputeLuminanceColor(paint)) {
        return false;
    }

    for (const GrSubRun& subRun : fSubRunList) {
        if (!subRun.canReuse(paint, positionMatrix)) {
            return false;
        }
    }

    return true;
}

const GrTextBlob::Key& GrTextBlob::key() const { return fKey; }

void GrTextBlob::draw(SkCanvas* canvas,
                      const GrClip* clip,
                      const SkMatrixProvider& viewMatrix,
                      SkPoint drawOrigin,
                      const SkPaint& paint,
                      skgpu::v1::SurfaceDrawContext* sdc) {
    for (const GrSubRun& subRun : fSubRunList) {
        subRun.draw(canvas, clip, viewMatrix, drawOrigin, paint, sdc);
    }
}

const GrAtlasSubRun* GrTextBlob::testingOnlyFirstSubRun() const {
    if (fSubRunList.isEmpty()) {
        return nullptr;
    }

    return fSubRunList.front().testingOnly_atlasSubRun();
}

GrTextBlob::GrTextBlob(SubRunAllocator&& alloc,
                       int totalMemorySize,
                       const SkMatrix& positionMatrix,
                       SkColor initialLuminance)
        : fAlloc{std::move(alloc)}
        , fSize(totalMemorySize)
        , fInitialPositionMatrix{positionMatrix}
        , fInitialLuminance{initialLuminance} { }

void GrTextBlob::processDeviceMasks(
        const SkZip<SkGlyphVariant, SkPoint>& accepted, sk_sp<SkStrike>&& strike) {
    SkASSERT(strike != nullptr);
    auto addGlyphsWithSameFormat = [&] (const SkZip<SkGlyphVariant, SkPoint>& accepted,
                                        MaskFormat format,
                                        sk_sp<SkStrike>&& runStrike) {
        GrSubRunOwner subRun = DirectMaskSubRun::Make(
                this, accepted, std::move(runStrike), format, &fAlloc);
        if (subRun != nullptr) {
            fSubRunList.append(std::move(subRun));
        } else {
            fSomeGlyphsExcluded = true;
        }
    };
    add_multi_mask_format(addGlyphsWithSameFormat, accepted, std::move(strike));
}

void GrTextBlob::processSourcePaths(const SkZip<SkGlyphVariant, SkPoint>& accepted,
                                    const SkFont& runFont,
                                    const SkDescriptor& descriptor,
                                    SkScalar strikeToSourceScale) {
    fSubRunList.append(PathSubRun::Make(
        accepted, has_some_antialiasing(runFont), strikeToSourceScale, descriptor, &fAlloc));
}

void GrTextBlob::processSourceDrawables(const SkZip<SkGlyphVariant, SkPoint>& accepted,
                                        const SkFont& runFont,
                                        const SkDescriptor& descriptor,
                                        SkScalar strikeToSourceScale) {
    fSubRunList.append(make_drawable_sub_run<DrawableSubRun>(
            accepted, strikeToSourceScale, descriptor, &fAlloc));
}

void GrTextBlob::processSourceSDFT(const SkZip<SkGlyphVariant, SkPoint>& accepted,
                                   sk_sp<SkStrike>&& strike,
                                   SkScalar strikeToSourceScale,
                                   const SkFont& runFont,
                                   const GrSDFTMatrixRange& matrixRange) {
    fSubRunList.append(SDFTSubRun::Make(
            this, accepted, runFont, std::move(strike), strikeToSourceScale, matrixRange, &fAlloc));
}

void GrTextBlob::processSourceMasks(const SkZip<SkGlyphVariant, SkPoint>& accepted,
                                    sk_sp<SkStrike>&& strike,
                                    SkScalar strikeToSourceScale) {
    auto addGlyphsWithSameFormat = [&] (const SkZip<SkGlyphVariant, SkPoint>& accepted,
                                        MaskFormat format,
                                        sk_sp<SkStrike>&& runStrike) {
        GrSubRunOwner subRun = TransformedMaskSubRun::Make(
                this, accepted, std::move(runStrike), strikeToSourceScale, format, &fAlloc);
        if (subRun != nullptr) {
            fSubRunList.append(std::move(subRun));
        } else {
            fSomeGlyphsExcluded = true;
        }
    };
    add_multi_mask_format(addGlyphsWithSameFormat, accepted, std::move(strike));
}

namespace {
// -- Slug -----------------------------------------------------------------------------------------
class Slug final : public GrSlug, public SkGlyphRunPainterInterface {
public:
    Slug(SubRunAllocator&& alloc,
         SkRect sourceBounds,
         const SkPaint& paint,
         const SkMatrix& positionMatrix,
         SkPoint origin);
    ~Slug() override = default;

    static sk_sp<Slug> Make(const SkMatrixProvider& viewMatrix,
                            const SkGlyphRunList& glyphRunList,
                            const SkPaint& initialPaint,
                            const SkPaint& drawingPaint,
                            const GrSDFTControl& control,
                            SkGlyphRunListPainter* painter);
    static sk_sp<GrSlug> MakeFromBuffer(SkReadBuffer& buffer,
                                        const SkStrikeClient* client);

    void surfaceDraw(SkCanvas*,
                     const GrClip* clip,
                     const SkMatrixProvider& viewMatrix,
                     const SkPaint& paint,
                     skgpu::v1::SurfaceDrawContext* sdc) const;

    void doFlatten(SkWriteBuffer& buffer) const override;
    SkRect sourceBounds() const override { return fSourceBounds; }
    const SkPaint& initialPaint() const override { return fInitialPaint; }

    // SkGlyphRunPainterInterface
    void processDeviceMasks(
            const SkZip<SkGlyphVariant, SkPoint>& accepted, sk_sp<SkStrike>&& strike) override;
    void processSourceMasks(
            const SkZip<SkGlyphVariant, SkPoint>& accepted, sk_sp<SkStrike>&& strike,
            SkScalar strikeToSourceScale) override;
    void processSourcePaths(
            const SkZip<SkGlyphVariant, SkPoint>& accepted, const SkFont& runFont,
            const SkDescriptor& descriptor, SkScalar strikeToSourceScale) override;
    void processSourceDrawables(
            const SkZip<SkGlyphVariant, SkPoint>& drawables, const SkFont& runFont,
            const SkDescriptor& descriptor,
            SkScalar strikeToSourceScale) override;
    void processSourceSDFT(
            const SkZip<SkGlyphVariant, SkPoint>& accepted, sk_sp<SkStrike>&& strike,
            SkScalar strikeToSourceScale, const SkFont& runFont,
            const GrSDFTMatrixRange& matrixRange) override;

    const SkMatrix& initialPositionMatrix() const override { return fInitialPositionMatrix; }
    SkPoint origin() const { return fOrigin; }

    // Change memory management to handle the data after Slug, but in the same allocation
    // of memory. Only allow placement new.
    void operator delete(void* p) { ::operator delete(p); }
    void* operator new(size_t) { SK_ABORT("All slugs are created by placement new."); }
    void* operator new(size_t, void* p) { return p; }

    std::tuple<int, int> subRunCountAndUnflattenSizeHint() const {
        int unflattenSizeHint = 0;
        int subRunCount = 0;
        for (auto& subrun : fSubRuns) {
            subRunCount += 1;
            unflattenSizeHint += subrun.unflattenSize();
        }
        return {subRunCount, unflattenSizeHint};
    }

private:
    // The allocator must come first because it needs to be destroyed last. Other fields of this
    // structure may have pointers into it.
    SubRunAllocator fAlloc;
    const SkRect fSourceBounds;
    const SkPaint fInitialPaint;
    const SkMatrix fInitialPositionMatrix;
    const SkPoint fOrigin;
    GrSubRunList fSubRuns;
};

Slug::Slug(SubRunAllocator&& alloc,
           SkRect sourceBounds,
           const SkPaint& initialPaint,
           const SkMatrix& positionMatrix,
           SkPoint origin)
    : fAlloc {std::move(alloc)}
    , fSourceBounds{sourceBounds}
    , fInitialPaint{initialPaint}
    , fInitialPositionMatrix{positionMatrix}
    , fOrigin{origin} {}

void Slug::surfaceDraw(SkCanvas* canvas, const GrClip* clip, const SkMatrixProvider& viewMatrix,
                       const SkPaint& drawingPaint, skgpu::v1::SurfaceDrawContext* sdc) const {
    for (const GrSubRun& subRun : fSubRuns) {
        subRun.draw(canvas, clip, viewMatrix, fOrigin, drawingPaint, sdc);
    }
}

void Slug::doFlatten(SkWriteBuffer& buffer) const {
    buffer.writeRect(fSourceBounds);
    SkPaintPriv::Flatten(fInitialPaint, buffer);
    buffer.writeMatrix(fInitialPositionMatrix);
    buffer.writePoint(fOrigin);
    auto [subRunCount, subRunsUnflattenSizeHint] = this->subRunCountAndUnflattenSizeHint();
    buffer.writeInt(subRunCount);
    buffer.writeInt(subRunsUnflattenSizeHint);
    for (auto& subRun : fSubRuns) {
        subRun.flatten(buffer);
    }
}

sk_sp<GrSlug> Slug::MakeFromBuffer(SkReadBuffer& buffer, const SkStrikeClient* client) {
    SkRect sourceBounds = buffer.readRect();
    SkASSERT(!sourceBounds.isEmpty());
    if (!buffer.validate(!sourceBounds.isEmpty())) { return nullptr; }

    SkPaint paint = buffer.readPaint();
    SkMatrix positionMatrix;
    buffer.readMatrix(&positionMatrix);
    SkPoint origin = buffer.readPoint();
    int subRunCount = buffer.readInt();
    SkASSERT(subRunCount > 0);
    if (!buffer.validate(subRunCount > 0)) { return nullptr; }
    int subRunsSizeHint = buffer.readInt();

    // Since the hint doesn't affect performance, then if it looks fishy just pick a reasonable
    // value.
    if (subRunsSizeHint < 0 || (1 << 16) < subRunsSizeHint) {
        subRunsSizeHint = 128;
    }

    auto [initializer, _, alloc] =
            SubRunAllocator::AllocateClassMemoryAndArena<Slug>(subRunsSizeHint);

    sk_sp<Slug> slug = sk_sp<Slug>(
            initializer.initialize(std::move(alloc), sourceBounds, paint, positionMatrix, origin));

    for (int i = 0; i < subRunCount; ++i) {
        auto subRun = GrSubRun::MakeFromBuffer(slug.get(), buffer, &slug->fAlloc, client);
        if (!buffer.validate(subRun != nullptr)) { return nullptr; }
        if (subRun != nullptr) {
            slug->fSubRuns.append(std::move(subRun));
        }
    }

    // Something went wrong while reading.
    SkASSERT(buffer.isValid());
    if (!buffer.isValid()) { return nullptr;}

    return std::move(slug);
}

void Slug::processDeviceMasks(
        const SkZip<SkGlyphVariant, SkPoint>& accepted, sk_sp<SkStrike>&& strike) {
    auto addGlyphsWithSameFormat = [&] (const SkZip<SkGlyphVariant, SkPoint>& accepted,
                                        MaskFormat format,
                                        sk_sp<SkStrike>&& runStrike) {
        GrSubRunOwner subRun = DirectMaskSubRun::Make(
                this, accepted, std::move(runStrike), format, &fAlloc);
        if (subRun != nullptr) {
            fSubRuns.append(std::move(subRun));
        }
    };

    add_multi_mask_format(addGlyphsWithSameFormat, accepted, std::move(strike));
}

sk_sp<Slug> Slug::Make(const SkMatrixProvider& viewMatrix,
                       const SkGlyphRunList& glyphRunList,
                       const SkPaint& initialPaint,
                       const SkPaint& drawingPaint,
                       const GrSDFTControl& control,
                       SkGlyphRunListPainter* painter) {
    // The difference in alignment from the per-glyph data to the SubRun;
    constexpr size_t alignDiff = alignof(DirectMaskSubRun) - alignof(DevicePosition);
    constexpr size_t vertexDataToSubRunPadding = alignDiff > 0 ? alignDiff : 0;
    size_t totalGlyphCount = glyphRunList.totalGlyphCount();
    // The bytesNeededForSubRun is optimized for DirectMaskSubRun which is by far the most
    // common case.
    size_t subRunSizeHint =
            totalGlyphCount * sizeof(DevicePosition)
            + GlyphVector::GlyphVectorSize(totalGlyphCount)
            + glyphRunList.runCount() * (sizeof(DirectMaskSubRun) + vertexDataToSubRunPadding);

    auto [initializer, _, alloc] =
            SubRunAllocator::AllocateClassMemoryAndArena<Slug>(subRunSizeHint);

    const SkMatrix positionMatrix =
            position_matrix(viewMatrix.localToDevice(), glyphRunList.origin());

    sk_sp<Slug> slug = sk_sp<Slug>(initializer.initialize(
            std::move(alloc), glyphRunList.sourceBounds(), initialPaint, positionMatrix,
            glyphRunList.origin()));

    const uint64_t uniqueID = glyphRunList.uniqueID();
    for (auto& glyphRun : glyphRunList) {
        painter->processGlyphRun(slug.get(),
                                 glyphRun,
                                 positionMatrix,
                                 drawingPaint,
                                 control,
                                 "Make Slug",
                                 uniqueID);
    }

    // There is nothing to draw here. This is particularly a problem with RSX form blobs where a
    // single space becomes a run with no glyphs.
    if (slug->fSubRuns.isEmpty()) { return nullptr; }

    return slug;
}

void Slug::processSourcePaths(const SkZip<SkGlyphVariant,
                              SkPoint>& accepted,
                              const SkFont& runFont,
                              const SkDescriptor& descriptor,
                              SkScalar strikeToSourceScale) {
    fSubRuns.append(PathSubRun::Make(accepted,
                                     has_some_antialiasing(runFont),
                                     strikeToSourceScale,
                                     descriptor,
                                     &fAlloc));
}

void Slug::processSourceDrawables(const SkZip<SkGlyphVariant, SkPoint>& accepted,
                                  const SkFont& runFont,
                                  const SkDescriptor& descriptor,
                                  SkScalar strikeToSourceScale) {
    fSubRuns.append(make_drawable_sub_run<DrawableSubRun>(
            accepted, strikeToSourceScale, descriptor, &fAlloc));
}

void Slug::processSourceSDFT(const SkZip<SkGlyphVariant, SkPoint>& accepted,
                                   sk_sp<SkStrike>&& strike,
                                   SkScalar strikeToSourceScale,
                                   const SkFont& runFont,
                                   const GrSDFTMatrixRange& matrixRange) {
    fSubRuns.append(SDFTSubRun::Make(
        this, accepted, runFont, std::move(strike), strikeToSourceScale, matrixRange, &fAlloc));
}

void Slug::processSourceMasks(const SkZip<SkGlyphVariant, SkPoint>& accepted,
                              sk_sp<SkStrike>&& strike,
                              SkScalar strikeToSourceScale) {

    auto addGlyphsWithSameFormat = [&] (const SkZip<SkGlyphVariant, SkPoint>& accepted,
                                        MaskFormat format,
                                        sk_sp<SkStrike>&& runStrike) {
        GrSubRunOwner subRun = TransformedMaskSubRun::Make(
                this, accepted, std::move(runStrike), strikeToSourceScale, format, &fAlloc);
        if (subRun != nullptr) {
            fSubRuns.append(std::move(subRun));
        }
    };

    add_multi_mask_format(addGlyphsWithSameFormat, accepted, std::move(strike));
}
}  // namespace

namespace skgpu::v1 {
sk_sp<GrSlug>
Device::convertGlyphRunListToSlug(const SkGlyphRunList& glyphRunList,
                                  const SkPaint& initialPaint,
                                  const SkPaint& drawingPaint) {
    auto recordingContextPriv = this->recordingContext()->priv();
    const GrSDFTControl control = recordingContextPriv.getSDFTControl(
            this->surfaceProps().isUseDeviceIndependentFonts());

    return Slug::Make(this->asMatrixProvider(),
                      glyphRunList,
                      initialPaint,
                      drawingPaint,
                      control,
                      fSurfaceDrawContext->glyphRunPainter());
}

void Device::drawSlug(SkCanvas* canvas, const GrSlug* grSlug, const SkPaint& drawingPaint) {
    const Slug* slug = static_cast<const Slug*>(grSlug);
    auto matrixProvider = this->asMatrixProvider();
#if defined(SK_DEBUG)
    if (!fContext->priv().options().fSupportBilerpFromGlyphAtlas) {
        // We can draw a slug if the atlas has padding or if the creation matrix and the
        // drawing matrix are the same. If they are the same, then the Slug will use the direct
        // drawing code and not use bi-lerp.
        SkMatrix slugMatrix = slug->initialPositionMatrix();
        SkMatrix positionMatrix = matrixProvider.localToDevice();
        positionMatrix.preTranslate(slug->origin().x(), slug->origin().y());
        SkASSERT(slugMatrix == positionMatrix);
    }
#endif
    slug->surfaceDraw(
            canvas, this->clip(), matrixProvider, drawingPaint, fSurfaceDrawContext.get());
}

sk_sp<GrSlug> MakeSlug(const SkMatrixProvider& drawMatrix,
                       const SkGlyphRunList& glyphRunList,
                       const SkPaint& initialPaint,
                       const SkPaint& drawingPaint,
                       const GrSDFTControl& control,
                       SkGlyphRunListPainter* painter) {
    return Slug::Make(drawMatrix, glyphRunList, initialPaint, drawingPaint, control, painter);
}
}  // namespace skgpu::v1

// -- GrSubRun -------------------------------------------------------------------------------------
void GrSubRun::flatten(SkWriteBuffer& buffer) const {
    buffer.writeInt(this->subRunType());
    this->doFlatten(buffer);
}

GrSubRunOwner GrSubRun::MakeFromBuffer(const GrTextReferenceFrame* referenceFrame,
                                       SkReadBuffer& buffer,
                                       SubRunAllocator* alloc,
                                       const SkStrikeClient* client) {
    using Maker = GrSubRunOwner (*)(const GrTextReferenceFrame*,
                                    SkReadBuffer&,
                                    SubRunAllocator*,
                                    const SkStrikeClient*);

    static Maker makers[kSubRunTypeCount] = {
            nullptr,                                             // 0 index is bad.
            DirectMaskSubRun::MakeFromBuffer,
            SDFTSubRun::MakeFromBuffer,
            TransformedMaskSubRun::MakeFromBuffer,
            PathSubRun::MakeFromBuffer,
            DrawableSubRun::MakeFromBuffer,
    };
    int subRunTypeInt = buffer.readInt();
    SkASSERT(kBad < subRunTypeInt && subRunTypeInt < kSubRunTypeCount);
    if (!buffer.validate(kBad < subRunTypeInt && subRunTypeInt < kSubRunTypeCount)) {
        return nullptr;
    }
    auto maker = makers[subRunTypeInt];
    if (!buffer.validate(maker != nullptr)) { return nullptr; }
    return maker(referenceFrame, buffer, alloc, client);
}

sk_sp<GrSlug> SkMakeSlugFromBuffer(SkReadBuffer& buffer, const SkStrikeClient* client) {
    return Slug::MakeFromBuffer(buffer, client);
}
