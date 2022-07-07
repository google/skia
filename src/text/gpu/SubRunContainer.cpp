/*
* Copyright 2022 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/text/gpu/SubRunContainer.h"

#include "include/core/SkScalar.h"
#include "include/private/chromium/SkChromeRemoteGlyphCache.h"
#include "src/core/SkDescriptor.h"
#include "src/core/SkDistanceFieldGen.h"
#include "src/core/SkGlyphBuffer.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkStrikeCache.h"
#include "src/gpu/AtlasTypes.h"
#include "src/text/GlyphRun.h"
#include "src/text/gpu/Glyph.h"
#include "src/text/gpu/GlyphVector.h"
#include "src/text/gpu/SubRunAllocator.h"

#if SK_SUPPORT_GPU  // Ganesh Support
#include "src/gpu/ganesh/GrClip.h"
#include "src/gpu/ganesh/GrStyle.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/ops/AtlasTextOp.h"
#include "src/gpu/ganesh/v1/SurfaceDrawContext_v1.h"
using AtlasTextOp = skgpu::v1::AtlasTextOp;
#endif  // SK_SUPPORT_GPU

#ifdef SK_GRAPHITE_ENABLED
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/Renderer.h"
#endif

#include <cinttypes>

namespace sktext::gpu {
// -- SubRunType -----------------------------------------------------------------------------------
enum SubRun::SubRunType : int {
    kBad = 0,  // Make this 0 to line up with errors from readInt.
    kDirectMask,
    kSDFT,
    kTransformMask,
    kPath,
    kDrawable,
    kSubRunTypeCount,
};
}  // namespace sktext::gpu

using MaskFormat = skgpu::MaskFormat;

using namespace sktext::gpu;

#if defined(SK_GRAPHITE_ENABLED)
using Device = skgpu::graphite::Device;
using DrawWriter = skgpu::graphite::DrawWriter;
using Rect = skgpu::graphite::Rect;
using Recorder = skgpu::graphite::Recorder;
using Renderer = skgpu::graphite::Renderer;
using Transform = skgpu::graphite::Transform;
#endif

namespace {
// Use the following in your args.gn to dump telemetry for diagnosing chrome Renderer/GPU
// differences.
// extra_cflags = ["-D", "SK_TRACE_GLYPH_RUN_PROCESS"]
#if defined(SK_TRACE_GLYPH_RUN_PROCESS)
static const constexpr bool kTrace = true;
#else
static const constexpr bool kTrace = false;
#endif

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
public:
    TransformedMaskVertexFiller(MaskFormat maskFormat,
                                SkScalar strikeToSourceScale,
                                SkRect sourceBounds,
                                SkSpan<const SkPoint> leftTop);

    static TransformedMaskVertexFiller Make(MaskFormat maskType,
                                            int strikePadding,
                                            SkScalar strikeToSourceScale,
                                            const SkZip<SkGlyphVariant, SkPoint>& accepted,
                                            SubRunAllocator* alloc);

    static std::optional<TransformedMaskVertexFiller> MakeFromBuffer(
            SkReadBuffer& buffer, SubRunAllocator* alloc);
    int unflattenSize() const;
    void flatten(SkWriteBuffer& buffer) const;

#if SK_SUPPORT_GPU
    size_t vertexStride(const SkMatrix& matrix) const {
        if (fMaskType != MaskFormat::kARGB) {
            // For formats MaskFormat::kA565 and MaskFormat::kA8 where A8 include SDF.
            return matrix.hasPerspective() ? sizeof(Mask3DVertex) : sizeof(Mask2DVertex);
        } else {
            // For format MaskFormat::kARGB
            return matrix.hasPerspective() ? sizeof(ARGB3DVertex) : sizeof(ARGB2DVertex);
        }
    }

    void fillVertexData(int offset, int count,
                        SkSpan<const Glyph*> glyphs,
                        GrColor color,
                        const SkMatrix& positionMatrix,
                        SkIRect clip,
                        void* vertexBuffer) const;

    AtlasTextOp::MaskType opMaskType() const;
#endif  // SK_SUPPORT_GPU
#if defined(SK_GRAPHITE_ENABLED)
    SkRect localRect() const { return fSourceBounds; }
#endif
    SkRect deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const;
    MaskFormat grMaskType() const {return fMaskType;}
    int count() const { return SkCount(fLeftTop); }

private:
    struct AtlasPt {
        uint16_t u;
        uint16_t v;
    };

#if SK_SUPPORT_GPU
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

    template<typename Quad, typename VertexData>
    void fill2D(SkZip<Quad, const Glyph*, const VertexData> quadData,
                GrColor color,
                const SkMatrix& matrix) const;

    template<typename Quad, typename VertexData>
    void fill3D(SkZip<Quad, const Glyph*, const VertexData> quadData,
                GrColor color,
                const SkMatrix& matrix) const;
#endif  // SK_SUPPORT_GPU

    const MaskFormat fMaskType;
    const SkScalar fStrikeToSourceScale;
    const SkRect fSourceBounds;
    const SkSpan<const SkPoint> fLeftTop;
};

TransformedMaskVertexFiller::TransformedMaskVertexFiller(
        MaskFormat maskFormat,
        SkScalar strikeToSourceScale,
        SkRect sourceBounds,
        SkSpan<const SkPoint> leftTop)
        : fMaskType{maskFormat}
        , fStrikeToSourceScale{strikeToSourceScale}
        , fSourceBounds{sourceBounds}
        , fLeftTop{leftTop} {}

TransformedMaskVertexFiller TransformedMaskVertexFiller::Make(
        MaskFormat maskType,
        int strikePadding,
        SkScalar strikeToSourceScale,
        const SkZip<SkGlyphVariant, SkPoint>& accepted,
        SubRunAllocator* alloc) {
    SkRect sourceBounds = SkRectPriv::MakeLargestInverted();
    SkSpan<SkPoint> leftTop = alloc->makePODArray<SkPoint>(
            accepted,
            [&](auto e) -> SkPoint {
                auto [variant, pos] = e;
                const SkGlyph* skGlyph = variant;

                // Make the glyphBounds and inset by any padding which may be included in the
                // strike mask.
                SkRect glyphBounds = skGlyph->rect();
                glyphBounds.inset(strikePadding, strikePadding);

                // Scale and position the glyph in source space.
                SkRect sourceGlyphBounds = SkRect::MakeXYWH(
                        glyphBounds.left()   * strikeToSourceScale + pos.x(),
                        glyphBounds.top()    * strikeToSourceScale + pos.y(),
                        glyphBounds.width()  * strikeToSourceScale,
                        glyphBounds.height() * strikeToSourceScale);

                sourceBounds.joinPossiblyEmptyRect(sourceGlyphBounds);
                return {sourceGlyphBounds.left(), sourceGlyphBounds.top()};
            });
    return TransformedMaskVertexFiller{maskType, strikeToSourceScale, sourceBounds, leftTop};
}

static bool check_glyph_count(SkReadBuffer& buffer, int glyphCount) {
    return 0 < glyphCount && static_cast<size_t>(glyphCount) < (buffer.available() / 4);
}

std::optional<TransformedMaskVertexFiller> TransformedMaskVertexFiller::MakeFromBuffer(
        SkReadBuffer& buffer, SubRunAllocator* alloc) {
    int checkingMaskType = buffer.readInt();
    if (!buffer.validate(0 <= checkingMaskType && checkingMaskType < skgpu::kMaskFormatCount)) {
        return std::nullopt;
    }
    MaskFormat maskType = (MaskFormat)checkingMaskType;
    SkScalar strikeToSourceScale = buffer.readScalar();
    if (!buffer.validate(0 < strikeToSourceScale)) { return std::nullopt; }
    SkRect sourceBounds = buffer.readRect();

    int glyphCount = buffer.readInt();
    if (!buffer.validate(check_glyph_count(buffer, glyphCount))) { return std::nullopt; }
    SkPoint* leftTopStorage =
            alloc->makePODArray<SkPoint>(glyphCount);
    for (int i = 0; i < glyphCount; ++i) {
        leftTopStorage[i] = buffer.readPoint();
    }
    SkSpan<SkPoint> topLeft(leftTopStorage, glyphCount);

    return {TransformedMaskVertexFiller{maskType, strikeToSourceScale, sourceBounds, topLeft}};
}

SkRect TransformedMaskVertexFiller::deviceRect(
        const SkMatrix& drawMatrix, SkPoint drawOrigin) const {
    SkRect outBounds = fSourceBounds;
    outBounds.offset(drawOrigin);
    return drawMatrix.mapRect(outBounds);
}

int TransformedMaskVertexFiller::unflattenSize() const {
    return fLeftTop.size_bytes();
}

void TransformedMaskVertexFiller::flatten(SkWriteBuffer& buffer) const {
    buffer.writeInt(static_cast<int>(fMaskType));
    buffer.writeScalar(fStrikeToSourceScale);
    buffer.writeRect(fSourceBounds);
    buffer.writeInt(SkCount(fLeftTop));
    for (SkPoint leftTop : fLeftTop) {
        buffer.writePoint(leftTop);
    }
}

#if SK_SUPPORT_GPU
void TransformedMaskVertexFiller::fillVertexData(int offset, int count,
                                                 SkSpan<const Glyph*> glyphs,
                                                 GrColor color,
                                                 const SkMatrix& positionMatrix,
                                                 SkIRect clip,
                                                 void* vertexBuffer) const {
    auto quadData = [&](auto dst) {
        return SkMakeZip(dst,
                         glyphs.subspan(offset, count),
                         fLeftTop.subspan(offset, count));
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

template<typename Quad, typename VertexData>
void TransformedMaskVertexFiller::fill2D(SkZip<Quad, const Glyph*, const VertexData> quadData,
                                         GrColor color,
                                         const SkMatrix& positionMatrix) const {
    for (auto[quad, glyph, leftTop] : quadData) {
        SkPoint widthHeight = SkPoint::Make(glyph->fAtlasLocator.width() * fStrikeToSourceScale,
                                            glyph->fAtlasLocator.height() * fStrikeToSourceScale);
        auto [l, t] = leftTop;
        auto [r, b] = leftTop + widthHeight;
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
    for (auto[quad, glyph, leftTop] : quadData) {
        SkPoint widthHeight = SkPoint::Make(glyph->fAtlasLocator.width() * fStrikeToSourceScale,
                                            glyph->fAtlasLocator.height() * fStrikeToSourceScale);
        auto [l, t] = leftTop;
        auto [r, b] = leftTop + widthHeight;
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
#endif  // SK_SUPPORT_GPU


struct AtlasPt {
    uint16_t u;
    uint16_t v;
};

#if SK_SUPPORT_GPU
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

SkMatrix position_matrix(const SkMatrix& drawMatrix, SkPoint drawOrigin) {
    SkMatrix position_matrix = drawMatrix;
    return position_matrix.preTranslate(drawOrigin.x(), drawOrigin.y());
}
#endif  // SK_SUPPORT_GPU

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
    void submitDraws(SkCanvas*,
                     SkPoint drawOrigin,
                     const SkPaint& paint) const;

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
    if (!buffer.validate(check_glyph_count(buffer, glyphCount))) { return std::nullopt; }
    if (!buffer.validateCanReadN<SkPoint>(glyphCount)) { return std::nullopt; }
    SkPoint* positions = alloc->makePODArray<SkPoint>(glyphCount);
    for (int i = 0; i < glyphCount; ++i) {
        positions[i] = buffer.readPoint();
    }

    // Remember, we stored an int for glyph id.
    if (!buffer.validateCanReadN<int>(glyphCount)) { return std::nullopt; }
    SkGlyphID* glyphIDs = alloc->makePODArray<SkGlyphID>(glyphCount);
    for (int i = 0; i < glyphCount; ++i) {
        glyphIDs[i] = SkTo<SkGlyphID>(buffer.readInt());
    }

    auto descriptor = SkAutoDescriptor::MakeFromBuffer(buffer);
    if (!buffer.validate(descriptor.has_value())) { return std::nullopt; }

    // Translate the TypefaceID if this was transferred from the GPU process.
    if (client != nullptr) {
        if (!client->translateTypefaceID(&descriptor.value())) { return std::nullopt; }
    }

    auto strike = SkStrikeCache::GlobalStrikeCache()->findStrike(*descriptor->getDesc());
    if (!buffer.validate(strike != nullptr)) { return std::nullopt; }

    auto paths = alloc->makeUniqueArray<SkPath>(glyphCount);
    SkBulkGlyphMetricsAndPaths pathGetter{std::move(strike)};

    for (auto [i, glyphID] : SkMakeEnumerate(SkSpan(glyphIDs, glyphCount))) {
        const SkPath* path = pathGetter.glyph(glyphID)->path();
        // There should never be missing paths in a sub run.
        if (path == nullptr) { return std::nullopt; }
        paths[i] = *path;
    }

    SkASSERT(buffer.isValid());
    return {PathOpSubmitter{isAntiAlias,
                            strikeToSourceScale,
                            SkSpan(positions, glyphCount),
                            SkSpan(glyphIDs, glyphCount),
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
                           SkSpan(positions, glyphCount),
                           SkSpan(glyphIDs, glyphCount),
                           std::move(paths),
                           descriptor};
}

void PathOpSubmitter::submitDraws(SkCanvas* canvas,
                                  SkPoint drawOrigin,
                                  const SkPaint& paint) const {
    SkPaint runPaint{paint};
    runPaint.setAntiAlias(fIsAntiAliased);

    SkMaskFilterBase* maskFilter = as_MFB(runPaint.getMaskFilter());

    // Calculate the matrix that maps the path glyphs from their size in the strike to
    // the graphics source space.
    SkMatrix strikeToSource = SkMatrix::Scale(fStrikeToSourceScale, fStrikeToSourceScale);
    strikeToSource.postTranslate(drawOrigin.x(), drawOrigin.y());

    // If there are shaders, non-blur mask filters or styles, the path must be scaled into source
    // space independently of the CTM. This allows the CTM to be correct for the different effects.
    SkStrokeRec style(runPaint);
    bool needsExactCTM = runPaint.getShader()
                         || runPaint.getPathEffect()
                         || (!style.isFillStyle() && !style.isHairlineStyle())
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
class PathSubRun final : public SubRun {
public:
    PathSubRun(PathOpSubmitter&& pathDrawing) : fPathDrawing(std::move(pathDrawing)) {}

    static SubRunOwner Make(const SkZip<SkGlyphVariant, SkPoint>& accepted,
                            bool isAntiAliased,
                            SkScalar strikeToSourceScale,
                            const SkDescriptor& descriptor,
                            SubRunAllocator* alloc) {
        return alloc->makeUnique<PathSubRun>(
                PathOpSubmitter::Make(
                        accepted, isAntiAliased, strikeToSourceScale, descriptor, alloc));
    }

#if SK_SUPPORT_GPU
    void draw(SkCanvas* canvas,
              const GrClip*,
              const SkMatrixProvider&,
              SkPoint drawOrigin,
              const SkPaint& paint,
              sk_sp<SkRefCnt>,
              skgpu::v1::SurfaceDrawContext*) const override {
        fPathDrawing.submitDraws(canvas, drawOrigin, paint);
    }
#endif  // SK_SUPPORT_GPU
#if defined(SK_GRAPHITE_ENABLED)
    void draw(SkCanvas* canvas,
              SkPoint drawOrigin,
              const SkPaint& paint,
              sk_sp<SkRefCnt> subRunStorage,
              Device* device) const override {
        fPathDrawing.submitDraws(canvas, drawOrigin, paint);
    }
#endif  // SK_GRAPHITE_ENABLED

    int unflattenSize() const override;

    bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const override {
        return true;
    }
    const AtlasSubRun* testingOnly_atlasSubRun() const override { return nullptr; }
    static SubRunOwner MakeFromBuffer(const SkMatrix& initialPositionMatrix,
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

SubRunOwner PathSubRun::MakeFromBuffer(const SkMatrix& initialPositionMatrix,
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
                        SkSpan<SkDrawable*> drawableData,
                        sk_sp<SkStrike>&& strike,
                        const SkDescriptor& descriptor);

    static DrawableOpSubmitter Make(const SkZip<SkGlyphVariant, SkPoint>& accepted,
                                    sk_sp<SkStrike>&& strike,
                                    SkScalar strikeToSourceScale,
                                    const SkDescriptor& descriptor,
                                    SubRunAllocator* alloc);

    int unflattenSize() const;
    void flatten(SkWriteBuffer& buffer) const;
    static std::optional<DrawableOpSubmitter> MakeFromBuffer(SkReadBuffer& buffer,
                                                             SubRunAllocator* alloc,
                                                             const SkStrikeClient* client);
#if SK_SUPPORT_GPU
    void submitOps(SkCanvas*,
                   const GrClip* clip,
                   const SkMatrixProvider& viewMatrix,
                   SkPoint drawOrigin,
                   const SkPaint& paint,
                   skgpu::v1::SurfaceDrawContext* sdc) const;
#endif  // SK_SUPPORT_GPU

private:
    const SkScalar fStrikeToSourceScale;
    const SkSpan<SkPoint> fPositions;
    const SkSpan<SkGlyphID> fGlyphIDs;
    const SkSpan<SkDrawable*> fDrawables;
    sk_sp<SkStrike> fStrike;  // Owns the fDrawables.
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
    if (!buffer.validate(check_glyph_count(buffer, glyphCount))) { return std::nullopt; }
    if (!buffer.validateCanReadN<SkPoint>(glyphCount)) { return std::nullopt; }
    SkPoint* positions = alloc->makePODArray<SkPoint>(glyphCount);
    for (int i = 0; i < glyphCount; ++i) {
        positions[i] = buffer.readPoint();
    }

    // Remember, we stored an int for glyph id.
    if (!buffer.validateCanReadN<int>(glyphCount)) { return std::nullopt; }
    SkGlyphID* glyphIDs = alloc->makePODArray<SkGlyphID>(glyphCount);
    for (int i = 0; i < glyphCount; ++i) {
        glyphIDs[i] = SkTo<SkGlyphID>(buffer.readInt());
    }

    auto descriptor = SkAutoDescriptor::MakeFromBuffer(buffer);
    if (!buffer.validate(descriptor.has_value())) { return std::nullopt; }

    // Translate the TypefaceID if this was transferred from the GPU process.
    if (client != nullptr) {
        if (!client->translateTypefaceID(&descriptor.value())) { return std::nullopt; }
    }

    auto strike = SkStrikeCache::GlobalStrikeCache()->findStrike(*descriptor->getDesc());
    if (!buffer.validate(strike != nullptr)) { return std::nullopt; }

    auto drawables = alloc->makePODArray<SkDrawable*>(glyphCount);
    SkBulkGlyphMetricsAndDrawables drawableGetter(sk_sp<SkStrike>{strike});
    auto glyphs = drawableGetter.glyphs(SkSpan(glyphIDs, glyphCount));

    for (auto [i, glyph] : SkMakeEnumerate(glyphs)) {
        drawables[i] = glyph->drawable();
    }

    SkASSERT(buffer.isValid());
    return {DrawableOpSubmitter{strikeToSourceScale,
                                SkSpan(positions, glyphCount),
                                SkSpan(glyphIDs, glyphCount),
                                SkSpan(drawables, glyphCount),
                                std::move(strike),
                                *descriptor->getDesc()}};
}

DrawableOpSubmitter::DrawableOpSubmitter(
        SkScalar strikeToSourceScale,
        SkSpan<SkPoint> positions,
        SkSpan<SkGlyphID> glyphIDs,
        SkSpan<SkDrawable*> drawables,
        sk_sp<SkStrike>&& strike,
        const SkDescriptor& descriptor)
        : fStrikeToSourceScale{strikeToSourceScale}
        , fPositions{positions}
        , fGlyphIDs{glyphIDs}
        , fDrawables{std::move(drawables)}
        , fStrike(std::move(strike))
        , fDescriptor{descriptor} {
    SkASSERT(!fPositions.empty());
}

DrawableOpSubmitter DrawableOpSubmitter::Make(const SkZip<SkGlyphVariant, SkPoint>& accepted,
                                              sk_sp<SkStrike>&& strike,
                                              SkScalar strikeToSourceScale,
                                              const SkDescriptor& descriptor,
                                              SubRunAllocator* alloc) {
    int glyphCount = SkCount(accepted);
    SkPoint* positions = alloc->makePODArray<SkPoint>(glyphCount);
    SkGlyphID* glyphIDs = alloc->makePODArray<SkGlyphID>(glyphCount);
    SkDrawable** drawables = alloc->makePODArray<SkDrawable*>(glyphCount);
    for (auto [i, variant, pos] : SkMakeEnumerate(accepted)) {
        positions[i] = pos;
        glyphIDs[i] = variant.glyph()->getGlyphID();
        drawables[i] = variant.glyph()->drawable();
    }

    return DrawableOpSubmitter{strikeToSourceScale,
                               SkSpan(positions, glyphCount),
                               SkSpan(glyphIDs, glyphCount),
                               SkSpan(drawables, glyphCount),
                               std::move(strike),
                               descriptor};
}

#if SK_SUPPORT_GPU
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
        SkDrawable* drawable = fDrawables[i];
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
#endif  // SK_SUPPORT_GPU

template <typename SubRunT>
SubRunOwner make_drawable_sub_run(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                  sk_sp<SkStrike>&& strike,
                                  SkScalar strikeToSourceScale,
                                  const SkDescriptor& descriptor,
                                  SubRunAllocator* alloc) {
    return alloc->makeUnique<SubRunT>(
            DrawableOpSubmitter::Make(drawables, std::move(strike),
                                      strikeToSourceScale, descriptor, alloc));
}

// -- DrawableSubRun -------------------------------------------------------------------------------
class DrawableSubRun : public SubRun {
public:
    DrawableSubRun(DrawableOpSubmitter&& drawingDrawing)
            : fDrawingDrawing(std::move(drawingDrawing)) {}

    static SubRunOwner MakeFromBuffer(const SkMatrix&,
                                      SkReadBuffer& buffer,
                                      SubRunAllocator* alloc,
                                      const SkStrikeClient* client);
#if SK_SUPPORT_GPU
    void draw(SkCanvas* canvas,
              const GrClip* clip,
              const SkMatrixProvider& viewMatrix,
              SkPoint drawOrigin,
              const SkPaint& paint,
              sk_sp<SkRefCnt> subRunStorage,
              skgpu::v1::SurfaceDrawContext* sdc) const override {
        fDrawingDrawing.submitOps(canvas, clip, viewMatrix, drawOrigin, paint, sdc);
    }
#endif  // SK_SUPPORT_GPU
#if defined(SK_GRAPHITE_ENABLED)
    void draw(SkCanvas* canvas,
              SkPoint drawOrigin,
              const SkPaint& paint,
              sk_sp<SkRefCnt> subRunStorage,
              Device* device) const override {
        // TODO
    }
#endif  // SK_SUPPORT_GPU

    int unflattenSize() const override;

    bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const override;

    const AtlasSubRun* testingOnly_atlasSubRun() const override;

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

SubRunOwner DrawableSubRun::MakeFromBuffer(const SkMatrix&,
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

const AtlasSubRun* DrawableSubRun::testingOnly_atlasSubRun() const {
    return nullptr;
}

using DevicePosition = skvx::Vec<2, int16_t>;

#if SK_SUPPORT_GPU
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
#endif  // SK_SUPPORT_GPU

// -- DirectMaskSubRun -------------------------------------------------------------------------
class DirectMaskSubRun final : public SubRun, public AtlasSubRun {
public:
    DirectMaskSubRun(MaskFormat format,
                     const SkMatrix& initialPositionMatrix,
                     SkGlyphRect deviceBounds,
                     SkSpan<const DevicePosition> devicePositions,
                     GlyphVector&& glyphs,
                     bool glyphsOutOfBounds);

    static SubRunOwner Make(const SkZip<SkGlyphVariant, SkPoint>& accepted,
                            const SkMatrix& initialPositionMatrix,
                            sk_sp<SkStrike>&& strike,
                            MaskFormat format,
                            SubRunAllocator* alloc);

    static SubRunOwner MakeFromBuffer(const SkMatrix& initialPositionMatrix,
                                      SkReadBuffer& buffer,
                                      SubRunAllocator* alloc,
                                      const SkStrikeClient* client);
#if SK_SUPPORT_GPU
    void draw(SkCanvas*,
              const GrClip* clip,
              const SkMatrixProvider& viewMatrix,
              SkPoint drawOrigin,
              const SkPaint& paint,
              sk_sp<SkRefCnt> subRunOwner,
              skgpu::v1::SurfaceDrawContext* sdc) const override;
#endif  // SK_SUPPORT_GPU

#ifdef SK_GRAPHITE_ENABLED
    void draw(SkCanvas*,
              SkPoint drawOrigin,
              const SkPaint&,
              sk_sp<SkRefCnt> subRunStorage,
              Device*) const override;
#endif

    int unflattenSize() const override;

    int glyphCount() const override;

    void testingOnly_packedGlyphIDToGlyph(StrikeCache *cache) const override;

#if SK_SUPPORT_GPU
    size_t vertexStride(const SkMatrix& drawMatrix) const override;

    std::tuple<const GrClip*, GrOp::Owner>
    makeAtlasTextOp(const GrClip*,
                    const SkMatrixProvider& viewMatrix,
                    SkPoint,
                    const SkPaint&,
                    sk_sp<SkRefCnt>&& subRunStorage,
                    skgpu::v1::SurfaceDrawContext*) const override;

    std::tuple<bool, int>
    regenerateAtlas(int begin, int end, GrMeshDrawTarget*) const override;

    void fillVertexData(void* vertexDst, int offset, int count,
                        GrColor color,
                        const SkMatrix& drawMatrix, SkPoint drawOrigin,
                        SkIRect clip) const override;
#endif  // SK_SUPPORT_GPU

#if defined(SK_GRAPHITE_ENABLED)
    std::tuple<bool, int>
    regenerateAtlas(int begin, int end, Recorder*) const override;

    std::tuple<Rect, Transform> boundsAndDeviceMatrix(const Transform&,
                                                      SkPoint drawOrigin) const override;

    const Renderer* renderer() const override { return &Renderer::TextDirect(fMaskFormat); }

    void fillVertexData(DrawWriter*,
                        int offset, int count,
                        SkColor color, SkScalar depth,
                        const skgpu::graphite::Transform& transform) const override;
#endif

    bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const override;

    const AtlasSubRun* testingOnly_atlasSubRun() const override;

protected:
    SubRunType subRunType() const override { return kDirectMask; }
    void doFlatten(SkWriteBuffer& buffer) const override;

private:
    // Return true if the positionMatrix represents an integer translation. Return the device
    // bounding box of all the glyphs. If the bounding box is empty, then something went singular
    // and this operation should be dropped.
    std::tuple<bool, SkRect> deviceRectAndCheckTransform(const SkMatrix& positionMatrix) const;

    const MaskFormat fMaskFormat;
    const SkMatrix& fInitialPositionMatrix;

    // The vertex bounds in device space. The bounds are the joined rectangles of all the glyphs.
    const SkGlyphRect fGlyphDeviceBounds;
    const SkSpan<const DevicePosition> fLeftTopDevicePos;
    const bool fSomeGlyphsExcluded;

    // The regenerateAtlas method mutates fGlyphs. It should be called from onPrepare which must
    // be single threaded.
    mutable GlyphVector fGlyphs;
};

DirectMaskSubRun::DirectMaskSubRun(MaskFormat format,
                                   const SkMatrix& initialPositionMatrix,
                                   SkGlyphRect deviceBounds,
                                   SkSpan<const DevicePosition> devicePositions,
                                   GlyphVector&& glyphs,
                                   bool glyphsOutOfBounds)
        : fMaskFormat{format}
        , fInitialPositionMatrix{initialPositionMatrix}
        , fGlyphDeviceBounds{deviceBounds}
        , fLeftTopDevicePos{devicePositions}
        , fSomeGlyphsExcluded{glyphsOutOfBounds}
        , fGlyphs{std::move(glyphs)} { }

SubRunOwner DirectMaskSubRun::Make(const SkZip<SkGlyphVariant, SkPoint>& accepted,
                                   const SkMatrix& initialPositionMatrix,
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
            format, initialPositionMatrix, runBounds, leftTop,
            GlyphVector{std::move(strike), {glyphIDs, goodPosCount}},
            glyphsExcluded);
}

bool DirectMaskSubRun::canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const {
    auto [reuse, translation] =
            can_use_direct(fInitialPositionMatrix, positionMatrix);

    // If glyphs were excluded because of position bounds, then this subrun can only be reused if
    // there is no change in position.
    if (fSomeGlyphsExcluded) {
        return translation.x() == 0 && translation.y() == 0;
    }

    return reuse;
}

SubRunOwner DirectMaskSubRun::MakeFromBuffer(const SkMatrix& initialPositionMatrix,
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
            maskType, initialPositionMatrix, runBounds, positions,
            std::move(glyphVector.value()), false);
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

const AtlasSubRun* DirectMaskSubRun::testingOnly_atlasSubRun() const {
    return this;
}

int DirectMaskSubRun::glyphCount() const {
    return SkCount(fGlyphs.glyphs());
}

#if SK_SUPPORT_GPU
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

void DirectMaskSubRun::draw(SkCanvas*,
                            const GrClip* clip,
                            const SkMatrixProvider& viewMatrix,
                            SkPoint drawOrigin,
                            const SkPaint& paint,
                            sk_sp<SkRefCnt> subRunStorage,
                            skgpu::v1::SurfaceDrawContext* sdc) const {
    auto[drawingClip, op] = this->makeAtlasTextOp(
            clip, viewMatrix, drawOrigin, paint, std::move(subRunStorage), sdc);
    if (op != nullptr) {
        sdc->addDrawOp(drawingClip, std::move(op));
    }
}

std::tuple<const GrClip*, GrOp::Owner> DirectMaskSubRun::makeAtlasTextOp(
        const GrClip* clip,
        const SkMatrixProvider& viewMatrix,
        SkPoint drawOrigin,
        const SkPaint& paint,
        sk_sp<SkRefCnt>&& subRunStorage,
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

    auto geometry = AtlasTextOp::Geometry::Make(*this,
                                                drawMatrix,
                                                drawOrigin,
                                                geometricClipRect,
                                                std::move(subRunStorage),
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
#endif  // SK_SUPPORT_GPU

#ifdef SK_GRAPHITE_ENABLED
void DirectMaskSubRun::draw(SkCanvas*,
                            SkPoint drawOrigin,
                            const SkPaint& paint,
                            sk_sp<SkRefCnt> subRunStorage,
                            Device* device) const {
    // TODO: see makeAtlasTextOp for Geometry set up
    device->drawAtlasSubRun(this, drawOrigin, paint, std::move(subRunStorage));
}
#endif

void DirectMaskSubRun::testingOnly_packedGlyphIDToGlyph(StrikeCache *cache) const {
    fGlyphs.packedGlyphIDToGlyph(cache);
}

#if SK_SUPPORT_GPU
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
            can_use_direct(fInitialPositionMatrix, positionMatrix);

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
    } else if (SkMatrix inverse; fInitialPositionMatrix.invert(&inverse)) {
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
#endif  // SK_SUPPORT_GPU

#if defined(SK_GRAPHITE_ENABLED)
std::tuple<bool, int> DirectMaskSubRun::regenerateAtlas(int begin, int end,
                                                        Recorder* recorder) const {
    return fGlyphs.regenerateAtlas(begin, end, fMaskFormat, 0, recorder);
}

std::tuple<Rect, Transform> DirectMaskSubRun::boundsAndDeviceMatrix(const Transform& localToDevice,
                                                                    SkPoint drawOrigin) const {
    // The baked-in matrix differs from the current localToDevice by a translation if the upper 2x2
    // remains the same, and there's no perspective. Since there's no projection, Z is irrelevant
    // so it's okay that fInitialPositionMatrix is an SkMatrix and has discarded the 3rd row/col,
    // and can ignore those values in localToDevice.
    const SkM44& positionMatrix = localToDevice.matrix();
    const bool compatibleMatrix = positionMatrix.rc(0,0) == fInitialPositionMatrix.rc(0,0) &&
                                  positionMatrix.rc(0,1) == fInitialPositionMatrix.rc(0,1) &&
                                  positionMatrix.rc(1,0) == fInitialPositionMatrix.rc(1,0) &&
                                  positionMatrix.rc(1,1) == fInitialPositionMatrix.rc(1,1) &&
                                  localToDevice.type() != Transform::Type::kProjection &&
                                  !fInitialPositionMatrix.hasPerspective();

    if (compatibleMatrix) {
        const SkV4 mappedOrigin = positionMatrix.map(drawOrigin.x(), drawOrigin.y(), 0.f, 1.f);
        const SkV2 offset = {mappedOrigin.x - fInitialPositionMatrix.getTranslateX(),
                             mappedOrigin.y - fInitialPositionMatrix.getTranslateY()};
        if (SkScalarIsInt(offset.x) && SkScalarIsInt(offset.y)) {
            // The offset is an integer (but make sure), which means the generated mask can be
            // accessed without changing how texels would be sampled.
            return {Rect(fGlyphDeviceBounds.rect()),
                    Transform(SkM44::Translate(SkScalarRoundToInt(offset.x),
                                               SkScalarRoundToInt(offset.y)))};
        }
    }

    // Otherwise compute the relative transformation from fInitialPositionMatrix to localToDevice,
    // with the drawOrigin applied. If fInitialPositionMatrix or the concatenation is not invertible
    // the returned Transform is marked invalid and the draw will be automatically dropped.
    return {Rect(fGlyphDeviceBounds.rect()),
            localToDevice.preTranslate(drawOrigin.x(), drawOrigin.y())
                         .concatInverse(SkM44(fInitialPositionMatrix))};
}

template<typename VertexData>
void direct_2D_mask_dw(DrawWriter* dw,
                       SkZip<const Glyph*, const VertexData> quadData,
                       SkColor color, SkScalar depth) {
    DrawWriter::Vertices verts{*dw};
    for (auto [glyph, leftTop]: quadData) {
        auto[al, at, ar, ab] = glyph->fAtlasLocator.getUVs();
        SkScalar dl = leftTop[0],
                 dt = leftTop[1],
                 dr = dl + (ar - al),
                 db = dt + (ab - at);
        // TODO: this should be drawn with indices but there doesn't seem to be a way to do that.
        // TODO: we should really use instances as well.
        verts.append(6) << SkPoint{dl, dt} << depth << color << AtlasPt{al, at}  // L,T
                        << SkPoint{dl, db} << depth << color << AtlasPt{al, ab}  // L,B
                        << SkPoint{dr, dt} << depth << color << AtlasPt{ar, at}  // R,T
                        << SkPoint{dl, db} << depth << color << AtlasPt{al, ab}  // L,B
                        << SkPoint{dr, db} << depth << color << AtlasPt{ar, ab}  // R,B
                        << SkPoint{dr, dt} << depth << color << AtlasPt{ar, at}; // R,T
    }
}

template<typename VertexData>
void direct_2D_rgba_dw(DrawWriter* dw,
                       SkZip<const Glyph*, const VertexData> quadData,
                       SkScalar depth) {
    DrawWriter::Vertices verts{*dw};
    for (auto [glyph, leftTop]: quadData) {
        auto[al, at, ar, ab] = glyph->fAtlasLocator.getUVs();
        SkScalar dl = leftTop[0],
                 dt = leftTop[1],
                 dr = dl + (ar - al),
                 db = dt + (ab - at);
        // TODO: this should be drawn with indices but there doesn't seem to be a way to do that.
        // TODO: we should really use instances as well.
        verts.append(6) << SkPoint{dl, dt} << depth << AtlasPt{al, at}  // L,T
                        << SkPoint{dl, db} << depth << AtlasPt{al, ab}  // L,B
                        << SkPoint{dr, dt} << depth << AtlasPt{ar, at}  // R,T
                        << SkPoint{dl, db} << depth << AtlasPt{al, ab}  // L,B
                        << SkPoint{dr, db} << depth << AtlasPt{ar, ab}  // R,B
                        << SkPoint{dr, dt} << depth << AtlasPt{ar, at}; // R,T
    }
}

template<typename VertexData>
void transformed_direct_2D_mask_dw(DrawWriter* dw,
                                   SkZip<const Glyph*, const VertexData> quadData,
                                   SkColor color, SkScalar depth,
                                   const Transform& transform) {
    // TODO
}

template<typename VertexData>
void transformed_direct_2D_rgba_dw(DrawWriter* dw,
                                   SkZip<const Glyph*, const VertexData> quadData,
                                   SkScalar depth,
                                   const Transform& transform) {
    // TODO
}

void DirectMaskSubRun::fillVertexData(DrawWriter* dw,
                                      int offset, int count,
                                      SkColor color, SkScalar depth,
                                      const skgpu::graphite::Transform& toDevice) const {
    auto quadData = [&]() {
        return SkMakeZip(fGlyphs.glyphs().subspan(offset, count),
                         fLeftTopDevicePos.subspan(offset, count));
    };

    bool noTransformNeeded = (toDevice.type() == Transform::Type::kIdentity);
    if (noTransformNeeded) {
        if (fMaskFormat != MaskFormat::kARGB) {
            direct_2D_mask_dw(dw, quadData(), color, depth);
        } else {
            direct_2D_rgba_dw(dw, quadData(), depth);
        }
    } else {
        if (toDevice.type() == Transform::Type::kProjection) {
            // TODO: handle float3 position data
        } else {
            if (fMaskFormat != MaskFormat::kARGB) {
                transformed_direct_2D_mask_dw(dw, quadData(), color, depth, toDevice);
            } else {
                transformed_direct_2D_rgba_dw(dw, quadData(), depth, toDevice);
            }
        }
    }
}
#endif

// true if only need to translate by integer amount, device rect.
std::tuple<bool, SkRect> DirectMaskSubRun::deviceRectAndCheckTransform(
        const SkMatrix& positionMatrix) const {
    const SkMatrix& initialMatrix = fInitialPositionMatrix;
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
    } else if (SkMatrix inverse; fInitialPositionMatrix.invert(&inverse)) {
        SkMatrix viewDifference = SkMatrix::Concat(positionMatrix, inverse);
        return {false, viewDifference.mapRect(fGlyphDeviceBounds.rect())};
    }

    // initialPositionMatrix is singular. Do nothing.
    return {false, SkRect::MakeEmpty()};
}

// -- TransformedMaskSubRun ------------------------------------------------------------------------
class TransformedMaskSubRun final : public SubRun, public AtlasSubRun {
public:
    TransformedMaskSubRun(const SkMatrix& initialPositionMatrix,
                          TransformedMaskVertexFiller&& vertexFiller,
                          GlyphVector&& glyphs);

    static SubRunOwner Make(const SkZip<SkGlyphVariant, SkPoint>& accepted,
                            const SkMatrix& initialPositionMatrix,
                            sk_sp<SkStrike>&& strike,
                            SkScalar strikeToSourceScale,
                            MaskFormat maskType,
                            SubRunAllocator* alloc);

    static SubRunOwner MakeFromBuffer(const SkMatrix& initialPositionMatrix,
                                      SkReadBuffer& buffer,
                                      SubRunAllocator* alloc,
                                      const SkStrikeClient* client);
#if SK_SUPPORT_GPU
    void draw(SkCanvas*,
              const GrClip*,
              const SkMatrixProvider& viewMatrix,
              SkPoint drawOrigin,
              const SkPaint& paint,
              sk_sp<SkRefCnt> subRunStorage,
              skgpu::v1::SurfaceDrawContext*) const override;

    std::tuple<const GrClip*, GrOp::Owner>
    makeAtlasTextOp(const GrClip*,
                    const SkMatrixProvider& viewMatrix,
                    SkPoint drawOrigin,
                    const SkPaint&,
                    sk_sp<SkRefCnt>&& subRunStorage,
                    skgpu::v1::SurfaceDrawContext*) const override;
#endif  // SK_SUPPORT_GPU
#if defined(SK_GRAPHITE_ENABLED)
    void draw(SkCanvas*,
              SkPoint drawOrigin,
              const SkPaint&,
              sk_sp<SkRefCnt> subRunStorage,
              Device*) const override;
#endif

    int unflattenSize() const override;

    bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const override;

    const AtlasSubRun* testingOnly_atlasSubRun() const override;

    void testingOnly_packedGlyphIDToGlyph(StrikeCache *cache) const override;

#if SK_SUPPORT_GPU
    std::tuple<bool, int> regenerateAtlas(int begin, int end, GrMeshDrawTarget*) const override;

    void fillVertexData(
            void* vertexDst, int offset, int count,
            GrColor color,
            const SkMatrix& drawMatrix, SkPoint drawOrigin,
            SkIRect clip) const override;

    size_t vertexStride(const SkMatrix& drawMatrix) const override;
#endif  // SK_SUPPORT_GPU
#if defined(SK_GRAPHITE_ENABLED)
    std::tuple<bool, int> regenerateAtlas(int begin, int end, Recorder*) const override;

    std::tuple<Rect, Transform> boundsAndDeviceMatrix(const Transform&,
                                                      SkPoint drawOrigin) const override;

    const Renderer* renderer() const override {
        return &Renderer::TextDirect(fVertexFiller.grMaskType());
    }

    void fillVertexData(DrawWriter*,
                        int offset, int count,
                        SkColor color, SkScalar depth,
                        const skgpu::graphite::Transform& transform) const override;
#endif

    int glyphCount() const override;

protected:
    SubRunType subRunType() const override { return kTransformMask; }
    void doFlatten(SkWriteBuffer& buffer) const override;

private:
    // The rectangle that surrounds all the glyph bounding boxes in device space.
    SkRect deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const;

    const SkMatrix& fInitialPositionMatrix;

    const TransformedMaskVertexFiller fVertexFiller;

    // The regenerateAtlas method mutates fGlyphs. It should be called from onPrepare which must
    // be single threaded.
    mutable GlyphVector fGlyphs;
};

TransformedMaskSubRun::TransformedMaskSubRun(const SkMatrix& initialPositionMatrix,
                                             TransformedMaskVertexFiller&& vertexFiller,
                                             GlyphVector&& glyphs)
        : fInitialPositionMatrix{initialPositionMatrix}
        , fVertexFiller{std::move(vertexFiller)}
        , fGlyphs{std::move(glyphs)} { }

SubRunOwner TransformedMaskSubRun::Make(const SkZip<SkGlyphVariant, SkPoint>& accepted,
                                        const SkMatrix& initialPositionMatrix,
                                        sk_sp<SkStrike>&& strike,
                                        SkScalar strikeToSourceScale,
                                        MaskFormat maskType,
                                        SubRunAllocator* alloc) {
    auto vertexFiller = TransformedMaskVertexFiller::Make(
            maskType, 0, strikeToSourceScale, accepted, alloc);

    auto glyphVector = GlyphVector::Make(std::move(strike), accepted.get<0>(), alloc);

    return alloc->makeUnique<TransformedMaskSubRun>(
            initialPositionMatrix, std::move(vertexFiller), std::move(glyphVector));
}

SubRunOwner TransformedMaskSubRun::MakeFromBuffer(const SkMatrix& initialPositionMatrix,
                                                  SkReadBuffer& buffer,
                                                  SubRunAllocator* alloc,
                                                  const SkStrikeClient* client) {
    auto vertexFiller = TransformedMaskVertexFiller::MakeFromBuffer(buffer, alloc);
    if (!buffer.validate(vertexFiller.has_value())) { return nullptr; }

    auto glyphVector = GlyphVector::MakeFromBuffer(buffer, client, alloc);
    if (!buffer.validate(glyphVector.has_value())) { return nullptr; }
    if (!buffer.validate(SkCount(glyphVector->glyphs()) == vertexFiller->count())) {
        return nullptr;
    }
    return alloc->makeUnique<TransformedMaskSubRun>(
            initialPositionMatrix, std::move(*vertexFiller), std::move(*glyphVector));
}

int TransformedMaskSubRun::unflattenSize() const {
    return sizeof(TransformedMaskSubRun) + fGlyphs.unflattenSize() + fVertexFiller.unflattenSize();
}

void TransformedMaskSubRun::doFlatten(SkWriteBuffer& buffer) const {
    fVertexFiller.flatten(buffer);
    fGlyphs.flatten(buffer);
}

#if SK_SUPPORT_GPU
void TransformedMaskSubRun::draw(SkCanvas*,
                                 const GrClip* clip,
                                 const SkMatrixProvider& viewMatrix,
                                 SkPoint drawOrigin,
                                 const SkPaint& paint,
                                 sk_sp<SkRefCnt> subRunStorage,
                                 skgpu::v1::SurfaceDrawContext* sdc) const {
    auto[drawingClip, op] = this->makeAtlasTextOp(
            clip, viewMatrix, drawOrigin, paint, std::move(subRunStorage), sdc);
    if (op != nullptr) {
        sdc->addDrawOp(drawingClip, std::move(op));
    }
}

std::tuple<const GrClip*, GrOp::Owner>
TransformedMaskSubRun::makeAtlasTextOp(const GrClip* clip,
                                       const SkMatrixProvider& viewMatrix,
                                       SkPoint drawOrigin,
                                       const SkPaint& paint,
                                       sk_sp<SkRefCnt>&& subRunStorage,
                                       skgpu::v1::SurfaceDrawContext* sdc) const {
    SkASSERT(this->glyphCount() != 0);

    const SkMatrix& drawMatrix = viewMatrix.localToDevice();

    GrPaint grPaint;
    SkPMColor4f drawingColor = calculate_colors(
            sdc, paint, viewMatrix, fVertexFiller.grMaskType(), &grPaint);

    auto geometry = AtlasTextOp::Geometry::Make(*this,
                                                drawMatrix,
                                                drawOrigin,
                                                SkIRect::MakeEmpty(),
                                                std::move(subRunStorage),
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
#endif  // SK_SUPPORT_GPU

#if defined(SK_GRAPHITE_ENABLED)
void TransformedMaskSubRun::draw(SkCanvas*,
                                 SkPoint drawOrigin,
                                 const SkPaint& paint,
                                 sk_sp<SkRefCnt> subRunStorage,
                                 Device* device) const {
    // TODO: see makeAtlasTextOp for Geometry set up
    device->drawAtlasSubRun(this, drawOrigin, paint, std::move(subRunStorage));
}
#endif
// If we are not scaling the cache entry to be larger, than a cache with smaller glyphs may be
// better.
bool TransformedMaskSubRun::canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const {
    if (fInitialPositionMatrix.getMaxScale() < 1) {
        return false;
    }
    return true;
}

void TransformedMaskSubRun::testingOnly_packedGlyphIDToGlyph(StrikeCache *cache) const {
    fGlyphs.packedGlyphIDToGlyph(cache);
}

#if SK_SUPPORT_GPU
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
#endif  // SK_SUPPORT_GPU

#if defined(SK_GRAPHITE_ENABLED)
std::tuple<bool, int> TransformedMaskSubRun::regenerateAtlas(int begin, int end,
                                                             Recorder* recorder) const {
    return fGlyphs.regenerateAtlas(begin, end, fVertexFiller.grMaskType(), 1, recorder);
}

std::tuple<Rect, Transform> TransformedMaskSubRun::boundsAndDeviceMatrix(
        const Transform& localToDevice, SkPoint drawOrigin) const {
    return {Rect(fVertexFiller.localRect()),
            localToDevice.preTranslate(drawOrigin.x(), drawOrigin.y())};
}

void TransformedMaskSubRun::fillVertexData(DrawWriter*,
                                           int offset, int count,
                                           SkColor color, SkScalar depth,
                                           const skgpu::graphite::Transform& transform) const {
    // TODO
}
#endif

int TransformedMaskSubRun::glyphCount() const {
    return SkCount(fGlyphs.glyphs());
}

SkRect TransformedMaskSubRun::deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const {
    return fVertexFiller.deviceRect(drawMatrix, drawOrigin);
}

const AtlasSubRun* TransformedMaskSubRun::testingOnly_atlasSubRun() const {
    return this;
}

// -- SDFTSubRun -----------------------------------------------------------------------------------
class SDFTSubRun final : public SubRun, public AtlasSubRun {
public:
    SDFTSubRun(bool useLCDText,
               bool antiAliased,
               const SDFTMatrixRange& matrixRange,
               TransformedMaskVertexFiller&& vertexFiller,
               GlyphVector&& glyphs);

    static SubRunOwner Make(const SkZip<SkGlyphVariant, SkPoint>& accepted,
                            const SkFont& runFont,
                            sk_sp<SkStrike>&& strike,
                            SkScalar strikeToSourceScale,
                            const SDFTMatrixRange& matrixRange,
                            SubRunAllocator* alloc);

    static SubRunOwner MakeFromBuffer(const SkMatrix& initialPositionMatrix,
                                      SkReadBuffer& buffer,
                                      SubRunAllocator* alloc,
                                      const SkStrikeClient* client);
#if SK_SUPPORT_GPU
    void draw(SkCanvas*,
              const GrClip*,
              const SkMatrixProvider& viewMatrix,
              SkPoint drawOrigin,
              const SkPaint&,
              sk_sp<SkRefCnt> subRunStorage,
              skgpu::v1::SurfaceDrawContext*) const override;

    std::tuple<const GrClip*, GrOp::Owner>
    makeAtlasTextOp(const GrClip*,
                    const SkMatrixProvider& viewMatrix,
                    SkPoint drawOrigin,
                    const SkPaint&,
                    sk_sp<SkRefCnt>&& subRunStorage,
                    skgpu::v1::SurfaceDrawContext*) const override;
#endif  // SK_SUPPORT_GPU
#if defined(SK_GRAPHITE_ENABLED)
    void draw(SkCanvas*,
              SkPoint drawOrigin,
              const SkPaint&,
              sk_sp<SkRefCnt> subRunStorage,
              Device*) const override;
#endif

    int unflattenSize() const override;

    bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const override;

    const AtlasSubRun* testingOnly_atlasSubRun() const override;

    void testingOnly_packedGlyphIDToGlyph(StrikeCache *cache) const override;

#if SK_SUPPORT_GPU
    std::tuple<bool, int> regenerateAtlas(int begin, int end, GrMeshDrawTarget*) const override;

    void fillVertexData(
            void* vertexDst, int offset, int count,
            GrColor color,
            const SkMatrix& drawMatrix, SkPoint drawOrigin,
            SkIRect clip) const override;

    size_t vertexStride(const SkMatrix& drawMatrix) const override;
#endif  // SK_SUPPORT_GPU
#if defined(SK_GRAPHITE_ENABLED)
    std::tuple<bool, int> regenerateAtlas(int begin, int end, Recorder*) const override;

    std::tuple<Rect, Transform> boundsAndDeviceMatrix(const Transform&,
                                                      SkPoint drawOrigin) const override;

    const Renderer* renderer() const override { return &Renderer::TextSDF(fUseLCDText); }

    void fillVertexData(DrawWriter*,
                        int offset, int count,
                        SkColor color, SkScalar depth,
                        const skgpu::graphite::Transform& transform) const override;
#endif

    int glyphCount() const override;

protected:
    SubRunType subRunType() const override { return kSDFT; }
    void doFlatten(SkWriteBuffer& buffer) const override;

private:
    // The rectangle that surrounds all the glyph bounding boxes in device space.
    SkRect deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const;

    const bool fUseLCDText;
    const bool fAntiAliased;
    const sktext::gpu::SDFTMatrixRange fMatrixRange;

    const TransformedMaskVertexFiller fVertexFiller;

    // The regenerateAtlas method mutates fGlyphs. It should be called from onPrepare which must
    // be single threaded.
    mutable GlyphVector fGlyphs;
};

SDFTSubRun::SDFTSubRun(bool useLCDText,
                       bool antiAliased,
                       const SDFTMatrixRange& matrixRange,
                       TransformedMaskVertexFiller&& vertexFiller,
                       GlyphVector&& glyphs)
        : fUseLCDText{useLCDText}
        , fAntiAliased{antiAliased}
        , fMatrixRange{matrixRange}
        , fVertexFiller{std::move(vertexFiller)}
        , fGlyphs{std::move(glyphs)} { }

bool has_some_antialiasing(const SkFont& font ) {
    SkFont::Edging edging = font.getEdging();
    return edging == SkFont::Edging::kAntiAlias
           || edging == SkFont::Edging::kSubpixelAntiAlias;
}

SubRunOwner SDFTSubRun::Make(const SkZip<SkGlyphVariant, SkPoint>& accepted,
                             const SkFont& runFont,
                             sk_sp<SkStrike>&& strike,
                             SkScalar strikeToSourceScale,
                             const SDFTMatrixRange& matrixRange,
                             SubRunAllocator* alloc) {
    auto vertexFiller = TransformedMaskVertexFiller::Make(
            MaskFormat::kA8,
            SK_DistanceFieldInset,
            strikeToSourceScale,
            accepted,
            alloc);

    auto glyphVector = GlyphVector::Make(std::move(strike), accepted.get<0>(), alloc);

    return alloc->makeUnique<SDFTSubRun>(
            runFont.getEdging() == SkFont::Edging::kSubpixelAntiAlias,
            has_some_antialiasing(runFont),
            matrixRange,
            std::move(vertexFiller),
            std::move(glyphVector));
}

SubRunOwner SDFTSubRun::MakeFromBuffer(const SkMatrix&,
                                       SkReadBuffer& buffer,
                                       SubRunAllocator* alloc,
                                       const SkStrikeClient* client) {
    int useLCD = buffer.readInt();
    int isAntiAliased = buffer.readInt();
    SDFTMatrixRange matrixRange = SDFTMatrixRange::MakeFromBuffer(buffer);
    auto vertexFiller = TransformedMaskVertexFiller::MakeFromBuffer(buffer, alloc);
    if (!buffer.validate(vertexFiller.has_value())) { return nullptr; }
    auto glyphVector = GlyphVector::MakeFromBuffer(buffer, client, alloc);
    if (!buffer.validate(glyphVector.has_value())) { return nullptr; }
    if (!buffer.validate(SkCount(glyphVector->glyphs()) == vertexFiller->count())) {
        return nullptr;
    }
    return alloc->makeUnique<SDFTSubRun>(useLCD,
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

#if SK_SUPPORT_GPU
void SDFTSubRun::draw(SkCanvas*,
                      const GrClip* clip,
                      const SkMatrixProvider& viewMatrix,
                      SkPoint drawOrigin,
                      const SkPaint& paint,
                      sk_sp<SkRefCnt> subRunStorage,
                      skgpu::v1::SurfaceDrawContext* sdc) const {
    auto[drawingClip, op] = this->makeAtlasTextOp(
            clip, viewMatrix, drawOrigin, paint, std::move(subRunStorage), sdc);
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
                            sk_sp<SkRefCnt>&& subRunStorage,
                            skgpu::v1::SurfaceDrawContext* sdc) const {
    SkASSERT(this->glyphCount() != 0);

    const SkMatrix& drawMatrix = viewMatrix.localToDevice();

    GrPaint grPaint;
    SkPMColor4f drawingColor = calculate_colors(sdc, paint, viewMatrix, MaskFormat::kA8,
                                                &grPaint);

    auto [maskType, DFGPFlags, useGammaCorrectDistanceTable] =
            calculate_sdf_parameters(*sdc, drawMatrix, fUseLCDText, fAntiAliased);

    auto geometry = AtlasTextOp::Geometry::Make(*this,
                                                drawMatrix,
                                                drawOrigin,
                                                SkIRect::MakeEmpty(),
                                                std::move(subRunStorage),
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
#endif  // SK_SUPPORT_GPU

#ifdef SK_GRAPHITE_ENABLED
void SDFTSubRun::draw(SkCanvas*,
                      SkPoint drawOrigin,
                      const SkPaint& paint,
                      sk_sp<SkRefCnt> subRunStorage,
                      Device* device) const {
    // TODO: see makeAtlasTextOp for Geometry set up
    device->drawAtlasSubRun(this, drawOrigin, paint, std::move(subRunStorage));
}
#endif

bool SDFTSubRun::canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const {
    return fMatrixRange.matrixInRange(positionMatrix);
}

void SDFTSubRun::testingOnly_packedGlyphIDToGlyph(StrikeCache *cache) const {
    fGlyphs.packedGlyphIDToGlyph(cache);
}

#if SK_SUPPORT_GPU
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
#endif  // SK_SUPPORT_GPU

#if defined(SK_GRAPHITE_ENABLED)
std::tuple<bool, int>
SDFTSubRun::regenerateAtlas(int begin, int end, Recorder *recorder) const {
    return fGlyphs.regenerateAtlas(begin, end, MaskFormat::kA8, SK_DistanceFieldInset,
                                   recorder);
}

std::tuple<Rect, Transform> SDFTSubRun::boundsAndDeviceMatrix(const Transform& localToDevice,
                                                              SkPoint drawOrigin) const {
    return {Rect(fVertexFiller.localRect()),
            localToDevice.preTranslate(drawOrigin.x(), drawOrigin.y())};
}

void SDFTSubRun::fillVertexData(DrawWriter*,
                                int offset, int count,
                                SkColor color, SkScalar depth,
                                const skgpu::graphite::Transform& transform) const {
    // TODO
}
#endif

int SDFTSubRun::glyphCount() const {
    return fVertexFiller.count();
}

SkRect SDFTSubRun::deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const {
    return fVertexFiller.deviceRect(drawMatrix, drawOrigin);
}

const AtlasSubRun* SDFTSubRun::testingOnly_atlasSubRun() const {
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

namespace sktext::gpu {
// -- SubRun -------------------------------------------------------------------------------------
SubRun::~SubRun() = default;
void SubRun::flatten(SkWriteBuffer& buffer) const {
    buffer.writeInt(this->subRunType());
    this->doFlatten(buffer);
}

SubRunOwner SubRun::MakeFromBuffer(const SkMatrix& initialPositionMatrix,
                                   SkReadBuffer& buffer,
                                   SubRunAllocator* alloc,
                                   const SkStrikeClient* client) {
    using Maker = SubRunOwner (*)(const SkMatrix&,
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
    return maker(initialPositionMatrix, buffer, alloc, client);
}

// -- SubRunContainer ------------------------------------------------------------------------------
SubRunContainer::SubRunContainer(const SkMatrix& initialPositionMatrix)
        : fInitialPositionMatrix{initialPositionMatrix} {}

void SubRunContainer::flattenAllocSizeHint(SkWriteBuffer& buffer) const {
    int unflattenSizeHint = 0;
    for (auto& subrun : fSubRuns) {
        unflattenSizeHint += subrun.unflattenSize();
    }
    buffer.writeInt(unflattenSizeHint);
}

int SubRunContainer::AllocSizeHintFromBuffer(SkReadBuffer& buffer) {
    int subRunsSizeHint = buffer.readInt();

    // Since the hint doesn't affect correctness, if it looks fishy just pick a reasonable
    // value.
    if (subRunsSizeHint < 0 || (1 << 16) < subRunsSizeHint) {
        subRunsSizeHint = 128;
    }
    return subRunsSizeHint;
}

void SubRunContainer::flattenRuns(SkWriteBuffer& buffer) const {
    buffer.writeMatrix(fInitialPositionMatrix);
    int subRunCount = 0;
    for ([[maybe_unused]] auto& subRun : fSubRuns) {
        subRunCount += 1;
    }
    buffer.writeInt(subRunCount);
    for (auto& subRun : fSubRuns) {
        subRun.flatten(buffer);
    }
}

SubRunContainerOwner SubRunContainer::MakeFromBufferInAlloc(SkReadBuffer& buffer,
                                                            const SkStrikeClient* client,
                                                            SubRunAllocator* alloc) {
    SkMatrix positionMatrix;
    buffer.readMatrix(&positionMatrix);
    if (!buffer.isValid()) { return nullptr; }
    SubRunContainerOwner container = alloc->makeUnique<SubRunContainer>(positionMatrix);

    int subRunCount = buffer.readInt();
    SkASSERT(subRunCount > 0);
    if (!buffer.validate(subRunCount > 0)) { return nullptr; }
    for (int i = 0; i < subRunCount; ++i) {
        auto subRunOwner = SubRun::MakeFromBuffer(
                container->initialPosition(), buffer, alloc, client);
        if (!buffer.validate(subRunOwner != nullptr)) { return nullptr; }
        if (subRunOwner != nullptr) {
            container->fSubRuns.append(std::move(subRunOwner));
        }
    }
    return container;
}

size_t SubRunContainer::EstimateAllocSize(const GlyphRunList& glyphRunList) {
    // The difference in alignment from the per-glyph data to the SubRun;
    constexpr size_t alignDiff = alignof(DirectMaskSubRun) - alignof(DevicePosition);
    constexpr size_t vertexDataToSubRunPadding = alignDiff > 0 ? alignDiff : 0;
    size_t totalGlyphCount = glyphRunList.totalGlyphCount();
    // This is optimized for DirectMaskSubRun which is by far the most common case.
    return totalGlyphCount * sizeof(DevicePosition)
           + GlyphVector::GlyphVectorSize(totalGlyphCount)
           + glyphRunList.runCount() * (sizeof(DirectMaskSubRun) + vertexDataToSubRunPadding)
           + sizeof(SubRunContainer);
}

std::tuple<bool, SubRunContainerOwner> SubRunContainer::MakeInAlloc(
        const GlyphRunList& glyphRunList,
        const SkMatrix& positionMatrix,
        const SkPaint& runPaint,
        SkStrikeDeviceInfo strikeDeviceInfo,
        SkStrikeForGPUCacheInterface* strikeCache,
        SubRunAllocator* alloc,
        SubRunCreationBehavior creationBehavior,
        const char* tag) {
    SkASSERT(alloc != nullptr);
    [[maybe_unused]] SkString msg;
    if constexpr (kTrace) {
        const uint64_t uniqueID = glyphRunList.uniqueID();
        msg.appendf("\nStart glyph run processing");
        if (tag != nullptr) {
            msg.appendf(" for %s ", tag);
            if (uniqueID != SK_InvalidUniqueID) {
                msg.appendf(" uniqueID: %" PRIu64, uniqueID);
            }
        }
        msg.appendf("\n   matrix\n");
        msg.appendf("   %7.3g %7.3g %7.3g\n   %7.3g %7.3g %7.3g\n",
                    positionMatrix[0], positionMatrix[1], positionMatrix[2],
                    positionMatrix[3], positionMatrix[4], positionMatrix[5]);
    }

    SubRunContainerOwner container = alloc->makeUnique<SubRunContainer>(positionMatrix);
    SkASSERT(strikeDeviceInfo.fSDFTControl != nullptr);
    if (strikeDeviceInfo.fSDFTControl == nullptr) {
        // Return empty container.
        return {true, std::move(container)};
    }

    const SkSurfaceProps deviceProps = strikeDeviceInfo.fSurfaceProps;
    const SkScalerContextFlags scalerContextFlags = strikeDeviceInfo.fScalerContextFlags;
    const sktext::gpu::SDFTControl SDFTControl = *strikeDeviceInfo.fSDFTControl;

    auto bufferScope = SkSubRunBuffers::EnsureBuffers(glyphRunList);
    auto [accepted, rejected] = bufferScope.buffers();
    bool someGlyphExcluded = false;
    for (auto& glyphRun : glyphRunList) {
        rejected->setSource(glyphRun.source());
        const SkFont& runFont = glyphRun.font();

        // Only consider using direct or SDFT drawing if not drawing hairlines and not perspective.
        if ((runPaint.getStyle() != SkPaint::kStroke_Style || runPaint.getStrokeWidth() != 0)
            && !positionMatrix.hasPerspective()) {
            SkScalar approximateDeviceTextSize =
                    SkFontPriv::ApproximateTransformedTextSize(runFont, positionMatrix);

            if (SDFTControl.isSDFT(approximateDeviceTextSize, runPaint)) {
                // Process SDFT - This should be the .009% case.
                const auto& [strikeSpec, strikeToSourceScale, matrixRange] =
                        SkStrikeSpec::MakeSDFT(
                                runFont, runPaint, deviceProps, positionMatrix, SDFTControl);

                if constexpr(kTrace) {
                    msg.appendf("  SDFT case:\n%s", strikeSpec.dump().c_str());
                }

                if (!SkScalarNearlyZero(strikeToSourceScale)) {
                    SkScopedStrikeForGPU strike = strikeSpec.findOrCreateScopedStrike(strikeCache);

                    accepted->startSource(rejected->source());
                    if constexpr (kTrace) {
                        msg.appendf("    glyphs:(x,y):\n      %s\n", accepted->dumpInput().c_str());
                    }
                    strike->prepareForSDFTDrawing(accepted, rejected);
                    rejected->flipRejectsToSource();

                    if (creationBehavior == kAddSubRuns && !accepted->empty()) {
                        container->fSubRuns.append(SDFTSubRun::Make(
                                accepted->accepted(),
                                runFont,
                                strike->getUnderlyingStrike(),
                                strikeToSourceScale,
                                matrixRange, alloc));
                    }
                }
            }

            if (!rejected->source().empty() && !SDFTControl.forcePaths()) {
                // Process masks including ARGB - this should be the 99.99% case.
                // This will handle medium size emoji that are sharing the run with SDFT drawn text.
                // If things are too big they will be passed along to the drawing of last resort
                // below.
                SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(
                        runFont, runPaint, deviceProps, scalerContextFlags, positionMatrix);

                if constexpr (kTrace) {
                    msg.appendf("  Mask case:\n%s", strikeSpec.dump().c_str());
                }

                SkScopedStrikeForGPU strike = strikeSpec.findOrCreateScopedStrike(strikeCache);

                accepted->startDevicePositioning(
                        rejected->source(), positionMatrix, strike->roundingSpec());
                if constexpr (kTrace) {
                    msg.appendf("    glyphs:(x,y):\n      %s\n", accepted->dumpInput().c_str());
                }
                strike->prepareForMaskDrawing(accepted, rejected);
                rejected->flipRejectsToSource();

                if (creationBehavior == kAddSubRuns && !accepted->empty()) {
                    auto addGlyphsWithSameFormat =
                            [&](const SkZip<SkGlyphVariant, SkPoint>& acceptedGlyphsAndLocations,
                                MaskFormat format,
                                sk_sp<SkStrike>&& runStrike) {
                                SubRunOwner subRun =
                                        DirectMaskSubRun::Make(acceptedGlyphsAndLocations,
                                                               container->initialPosition(),
                                                               std::move(runStrike),
                                                               format,
                                                               alloc);
                                if (subRun != nullptr) {
                                    container->fSubRuns.append(std::move(subRun));
                                } else {
                                    someGlyphExcluded |= true;
                                }
                            };
                    add_multi_mask_format(addGlyphsWithSameFormat,
                                          accepted->accepted(),
                                          strike->getUnderlyingStrike());
                }
            }
        }

        if (!rejected->source().empty()) {
            // Drawable case - handle big things with that have a drawable.
            auto [strikeSpec, strikeToSourceScale] =
                    SkStrikeSpec::MakePath(runFont, runPaint, deviceProps, scalerContextFlags);

            if constexpr (kTrace) {
                msg.appendf("  Drawable case:\n%s", strikeSpec.dump().c_str());
            }

            if (!SkScalarNearlyZero(strikeToSourceScale)) {
                SkScopedStrikeForGPU strike = strikeSpec.findOrCreateScopedStrike(strikeCache);

                accepted->startSource(rejected->source());
                if constexpr (kTrace) {
                    msg.appendf("    glyphs:(x,y):\n      %s\n", accepted->dumpInput().c_str());
                }
                strike->prepareForDrawableDrawing(accepted, rejected);
                rejected->flipRejectsToSource();

                if (container && !accepted->empty()) {
                    container->fSubRuns.append(
                            make_drawable_sub_run<DrawableSubRun>(accepted->accepted(),
                                                                  strike->getUnderlyingStrike(),
                                                                  strikeToSourceScale,
                                                                  strikeSpec.descriptor(),
                                                                  alloc));
                }
            }
        }
        if (!rejected->source().empty()) {
            // Path case - handle big things without color and that have a path.
            auto [strikeSpec, strikeToSourceScale] =
                    SkStrikeSpec::MakePath(runFont, runPaint, deviceProps, scalerContextFlags);

            if constexpr (kTrace) {
                msg.appendf("  Path case:\n%s", strikeSpec.dump().c_str());
            }

            if (!SkScalarNearlyZero(strikeToSourceScale)) {
                SkScopedStrikeForGPU strike = strikeSpec.findOrCreateScopedStrike(strikeCache);

                accepted->startSource(rejected->source());
                if constexpr (kTrace) {
                    msg.appendf("    glyphs:(x,y):\n      %s\n", accepted->dumpInput().c_str());
                }
                strike->prepareForPathDrawing(accepted, rejected);
                rejected->flipRejectsToSource();

                if (creationBehavior == kAddSubRuns && !accepted->empty()) {
                    container->fSubRuns.append(PathSubRun::Make(accepted->accepted(),
                                                                has_some_antialiasing(runFont),
                                                                strikeToSourceScale,
                                                                strikeSpec.descriptor(),
                                                                alloc));
                }
            }
        }

        if (!rejected->source().empty()) {
            // Drawing of last resort - Scale masks that fit in the atlas to the screen using
            // bilerp.

            const SkMatrix* gaugingMatrix = &positionMatrix;
            if (positionMatrix.hasPerspective()) {
                // The Scaler can't take perspective matrices, so use I as our best guess.
                gaugingMatrix = &SkMatrix::I();
            }

            // Remember, this will be an integer. Reduce to make a one pixel border for the
            // bilerp padding.
            static const constexpr SkScalar kMaxBilerpAtlasDimension =
                    SkStrikeCommon::kSkSideTooBigForAtlas - 2;

            // Get the raw glyph IDs to simulate device drawing to figure the maximum device
            // dimension.
            const SkSpan<const SkGlyphID> glyphs = rejected->source().get<0>();

            // It could be that this font produces glyphs that won't fit in the atlas. Find the
            // largest glyph dimension for the glyph run.
            SkStrikeSpec gaugingStrikeSpec = SkStrikeSpec::MakeTransformMask(
                    runFont, runPaint, deviceProps, scalerContextFlags, *gaugingMatrix);
            SkScopedStrikeForGPU gaugingStrike =
                    gaugingStrikeSpec.findOrCreateScopedStrike(strikeCache);

            // Remember, this will be an integer.
            const SkScalar originalMaxGlyphDimension =
                    gaugingStrike->findMaximumGlyphDimension(glyphs);

            if (originalMaxGlyphDimension == 0) {
                // Nothing to draw here. Skip this SubRun.
                continue;
            }

            SkScalar strikeToSourceScale = 1;
            SkFont reducedFont = runFont;
            if (originalMaxGlyphDimension > kMaxBilerpAtlasDimension) {
                // For glyphs that won't fit in the atlas, calculating the amount to reduce the
                // size can't be done using simple ratios. This is because the SkScaler forces
                // glyph width and height to integer amounts causing the ratio to be too high if
                // it rounds up. If it does round up, you need to try again to find the maximum
                // glyph dimensions and scale factor.
                SkScalar maxGlyphDimension = originalMaxGlyphDimension;
                SkScalar reductionFactor = kMaxBilerpAtlasDimension / maxGlyphDimension;

                // Find a new reducedFont that produces a maximum glyph dimension that is
                // <= kMaxBilerpAtlasDimension.
                do {
                    // Make a smaller font that will hopefully fit in the atlas.
                    reducedFont.setSize(runFont.getSize() * reductionFactor);

                    // Create a strike to calculate the new reduced maximum glyph dimension.
                    SkStrikeSpec reducingStrikeSpec = SkStrikeSpec::MakeTransformMask(
                            reducedFont, runPaint, deviceProps, scalerContextFlags, *gaugingMatrix);
                    SkScopedStrikeForGPU reducingStrike =
                            reducingStrikeSpec.findOrCreateScopedStrike(strikeCache);

                    // Remember, this will be an integer.
                    maxGlyphDimension = reducingStrike->findMaximumGlyphDimension(glyphs);
                    if (maxGlyphDimension == 0) {
                        // Avoid the divide by zero below.
                        goto skipSubRun;
                    }

                    // The largest reduction factor allowed for each iteration. Smaller reduction
                    // factors reduce the font size faster.
                    static constexpr SkScalar kMaximumReduction =
                            1.0f - 1.0f / kMaxBilerpAtlasDimension;

                    // Make the reduction smaller by at least kMaximumReduction in case the
                    // maximum glyph dimension is too big.
                    reductionFactor *=
                            std::min(kMaximumReduction, kMaxBilerpAtlasDimension/maxGlyphDimension);
                } while (maxGlyphDimension > kMaxBilerpAtlasDimension);

                // Calculate the factor to make the maximum dimension of the reduced font be the
                // same size as the maximum dimension from the original runFont.
                strikeToSourceScale = originalMaxGlyphDimension / maxGlyphDimension;
            }

            if (!SkScalarNearlyZero(strikeToSourceScale)) {
                SkStrikeSpec strikeSpec = SkStrikeSpec::MakeTransformMask(
                        reducedFont, runPaint, deviceProps, scalerContextFlags, *gaugingMatrix);
                if constexpr (kTrace) {
                    msg.appendf("Transformed case:\n%s", strikeSpec.dump().c_str());
                }
                SkScopedStrikeForGPU strike = strikeSpec.findOrCreateScopedStrike(strikeCache);

                accepted->startSource(rejected->source());
                if constexpr (kTrace) {
                    msg.appendf("glyphs:(x,y):\n      %s\n", accepted->dumpInput().c_str());
                }
                strike->prepareForMaskDrawing(accepted, rejected);
                rejected->flipRejectsToSource();
                SkASSERT(rejected->source().empty());

                if (creationBehavior == kAddSubRuns && !accepted->empty()) {
                    auto addGlyphsWithSameFormat =
                            [&](const SkZip<SkGlyphVariant, SkPoint>& acceptedGlyphsAndLocations,
                                MaskFormat format,
                                sk_sp<SkStrike>&& runStrike) {
                                SubRunOwner subRun =
                                        TransformedMaskSubRun::Make(acceptedGlyphsAndLocations,
                                                                    container->initialPosition(),
                                                                    std::move(runStrike),
                                                                    strikeToSourceScale,
                                                                    format,
                                                                    alloc);
                                if (subRun != nullptr) {
                                    container->fSubRuns.append(std::move(subRun));
                                } else {
                                    someGlyphExcluded |= true;
                                }
                            };
                    add_multi_mask_format(addGlyphsWithSameFormat,
                                          accepted->accepted(),
                                          strike->getUnderlyingStrike());
                }
            }
        }

    skipSubRun:
        ;
    }

    if constexpr (kTrace) {
        msg.appendf("End glyph run processing");
        if (tag != nullptr) {
            msg.appendf(" for %s ", tag);
        }
        SkDebugf("%s\n", msg.c_str());
    }
    return {someGlyphExcluded, std::move(container)};
}

#if SK_SUPPORT_GPU
void SubRunContainer::draw(SkCanvas* canvas,
                           const GrClip* clip,
                           const SkMatrixProvider& viewMatrix,
                           SkPoint drawOrigin,
                           const SkPaint& paint,
                           const SkRefCnt* subRunStorage,
                           skgpu::v1::SurfaceDrawContext* sdc) const {
    for (auto& subRun : fSubRuns) {
        subRun.draw(canvas, clip, viewMatrix, drawOrigin, paint, sk_ref_sp(subRunStorage), sdc);
    }
}
#endif  // SK_SUPPORT_GPU

#if defined(SK_GRAPHITE_ENABLED)
void SubRunContainer::draw(SkCanvas* canvas,
                           SkPoint drawOrigin,
                           const SkPaint& paint,
                           const SkRefCnt* subRunStorage,
                           skgpu::graphite::Device* device) const {
    for (auto& subRun : fSubRuns) {
        subRun.draw(canvas, drawOrigin, paint, sk_ref_sp(subRunStorage), device);
    }
}
#endif

bool SubRunContainer::canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const {
    for (const SubRun& subRun : fSubRuns) {
        if (!subRun.canReuse(paint, positionMatrix)) {
            return false;
        }
    }
    return true;
}

}  // namespace sktext::gpu
