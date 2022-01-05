/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/private/SkTemplates.h"
#include "include/private/chromium/GrSlug.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrGlyph.h"
#include "src/gpu/GrMeshDrawTarget.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrDistanceFieldGeoProc.h"
#include "src/gpu/geometry/GrStyledShape.h"
#include "src/gpu/text/GrAtlasManager.h"
#include "src/gpu/text/GrStrikeCache.h"
#include "src/gpu/text/GrTextBlob.h"

#include "src/gpu/GrBlurUtils.h"
#include "src/gpu/ops/AtlasTextOp.h"
#include "src/gpu/v1/Device_v1.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"

// Note:
//   In order to use GrSlugs, you need to set the fSupportBilerpFromGlyphAtlas on GrContextOptions.

// Naming conventions
//  * drawMatrix - the CTM from the canvas.
//  * drawOrigin - the x, y location of the drawTextBlob call.
//  * positionMatrix - this is the combination of the drawMatrix and the drawOrigin:
//        positionMatrix = drawMatrix * TranslationMatrix(drawOrigin.x, drawOrigin.y);

using AtlasTextOp = skgpu::v1::AtlasTextOp;

namespace {
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

AtlasTextOp::MaskType op_mask_type(GrMaskFormat grMaskFormat) {
    switch (grMaskFormat) {
        case kA8_GrMaskFormat:   return AtlasTextOp::MaskType::kGrayscaleCoverage;
        case kA565_GrMaskFormat: return AtlasTextOp::MaskType::kLCDCoverage;
        case kARGB_GrMaskFormat: return AtlasTextOp::MaskType::kColorBitmap;
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
                             GrMaskFormat grMaskFormat,
                             GrPaint* grPaint) {
    GrRecordingContext* rContext = sc->recordingContext();
    const GrColorInfo& colorInfo = sc->colorInfo();
    if (grMaskFormat == kARGB_GrMaskFormat) {
        SkPaintToGrPaintReplaceShader(rContext, colorInfo, paint, matrix, nullptr, grPaint);
        float a = grPaint->getColor4f().fA;
        return {a, a, a, a};
    }
    SkPaintToGrPaint(rContext, colorInfo, paint, matrix, grPaint);
    return grPaint->getColor4f();
}

template<typename Quad, typename VertexData>
void fill_transformed_vertices_2D(SkZip<Quad, const GrGlyph*, const VertexData> quadData,
                                  SkScalar dstPadding,
                                  SkScalar strikeToSource,
                                  GrColor color,
                                  const SkMatrix& matrix) {
    SkPoint inset = {dstPadding, dstPadding};
    for (auto[quad, glyph, vertexData] : quadData) {
        auto[pos, rect] = vertexData;
        auto[l, t, r, b] = rect;
        SkPoint sLT = (SkPoint::Make(l, t) + inset) * strikeToSource + pos,
                sRB = (SkPoint::Make(r, b) - inset) * strikeToSource + pos;
        SkPoint lt = matrix.mapXY(sLT.x(), sLT.y()),
                lb = matrix.mapXY(sLT.x(), sRB.y()),
                rt = matrix.mapXY(sRB.x(), sLT.y()),
                rb = matrix.mapXY(sRB.x(), sRB.y());
        auto[al, at, ar, ab] = glyph->fAtlasLocator.getUVs();
        quad[0] = {lt, color, {al, at}};  // L,T
        quad[1] = {lb, color, {al, ab}};  // L,B
        quad[2] = {rt, color, {ar, at}};  // R,T
        quad[3] = {rb, color, {ar, ab}};  // R,B
    }
}

template<typename Quad, typename VertexData>
void fill_transformed_vertices_3D(SkZip<Quad, const GrGlyph*, const VertexData> quadData,
                                  SkScalar dstPadding,
                                  SkScalar strikeToSource,
                                  GrColor color,
                                  const SkMatrix& positionMatrix) {
    SkPoint inset = {dstPadding, dstPadding};
    auto mapXYZ = [&](SkScalar x, SkScalar y) {
        SkPoint pt{x, y};
        SkPoint3 result;
        positionMatrix.mapHomogeneousPoints(&result, &pt, 1);
        return result;
    };
    for (auto[quad, glyph, vertexData] : quadData) {
        auto[pos, rect] = vertexData;
        auto [l, t, r, b] = rect;
        SkPoint sLT = (SkPoint::Make(l, t) + inset) * strikeToSource + pos,
                sRB = (SkPoint::Make(r, b) - inset) * strikeToSource + pos;
        SkPoint3 lt = mapXYZ(sLT.x(), sLT.y()),
                 lb = mapXYZ(sLT.x(), sRB.y()),
                 rt = mapXYZ(sRB.x(), sLT.y()),
                 rb = mapXYZ(sRB.x(), sRB.y());
        auto[al, at, ar, ab] = glyph->fAtlasLocator.getUVs();
        quad[0] = {lt, color, {al, at}};  // L,T
        quad[1] = {lb, color, {al, ab}};  // L,B
        quad[2] = {rt, color, {ar, at}};  // R,T
        quad[3] = {rb, color, {ar, ab}};  // R,B
    }
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

// -- PathGlyph ------------------------------------------------------------------------------------
// The encoding of a specific path at a specific source location.
struct PathGlyph {
    PathGlyph(const SkPath& path, SkPoint origin)
        : fPath(path)
        , fOrigin(origin) {}
    SkPath fPath;
    SkPoint fOrigin;
};

// -- PathSubRun -----------------------------------------------------------------------------------
class PathSubRun final : public GrSubRun {
public:
    PathSubRun(bool isAntiAliased,
               SkScalar strikeToSourceScale,
               SkSpan<PathGlyph> paths,
               std::unique_ptr<PathGlyph[], GrSubRunAllocator::ArrayDestroyer> pathData);

    void draw(const GrClip*,
              const SkMatrixProvider& viewMatrix,
              SkPoint drawOrigin,
              const SkPaint& paint,
              skgpu::v1::SurfaceDrawContext*) const override;

    bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const override;

    GrAtlasSubRun* testingOnly_atlasSubRun() override;

    static GrSubRunOwner Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                              bool isAntiAliased,
                              SkScalar strikeToSourceScale,
                              GrSubRunAllocator* alloc);

private:
    const bool fIsAntiAliased;
    const SkScalar fStrikeToSourceScale;
    const SkSpan<const PathGlyph> fPaths;
    const std::unique_ptr<PathGlyph[], GrSubRunAllocator::ArrayDestroyer> fPathData;
};

PathSubRun::PathSubRun(bool isAntiAliased,
                       SkScalar strikeToSourceScale,
                       SkSpan<PathGlyph> paths,
                       std::unique_ptr<PathGlyph[], GrSubRunAllocator::ArrayDestroyer> pathData)
    : fIsAntiAliased{isAntiAliased}
    , fStrikeToSourceScale{strikeToSourceScale}
    , fPaths{paths}
    , fPathData{std::move(pathData)} { }

void PathSubRun::draw(const GrClip* clip,
                      const SkMatrixProvider& viewMatrix,
                      SkPoint drawOrigin,
                      const SkPaint& paint,
                      skgpu::v1::SurfaceDrawContext* sdc) const {
    SkASSERT(!fPaths.empty());
    SkPaint runPaint{paint};
    runPaint.setAntiAlias(fIsAntiAliased);
    // If there are shaders, blurs or styles, the path must be scaled into source
    // space independently of the CTM. This allows the CTM to be correct for the
    // different effects.
    GrStyle style(runPaint);

    bool needsExactCTM = runPaint.getShader()
                         || style.applies()
                         || runPaint.getMaskFilter();

    // Calculate the matrix that maps the path glyphs from their size in the strike to
    // the graphics source space.
    SkMatrix strikeToSource = SkMatrix::Scale(fStrikeToSourceScale, fStrikeToSourceScale);
    strikeToSource.postTranslate(drawOrigin.x(), drawOrigin.y());
    if (!needsExactCTM) {
        for (const auto& pathPos : fPaths) {
            const SkPath& path = pathPos.fPath;
            const SkPoint pos = pathPos.fOrigin;  // Transform the glyph to source space.
            SkMatrix pathMatrix = strikeToSource;
            pathMatrix.postTranslate(pos.x(), pos.y());
            SkPreConcatMatrixProvider strikeToDevice(viewMatrix, pathMatrix);

            GrStyledShape shape(path, paint);
            GrBlurUtils::drawShapeWithMaskFilter(
                    sdc->recordingContext(), sdc, clip, runPaint, strikeToDevice, shape);
        }
    } else {
        // Transform the path to device because the deviceMatrix must be unchanged to
        // draw effect, filter or shader paths.
        for (const auto& pathPos : fPaths) {
            const SkPath& path = pathPos.fPath;
            const SkPoint pos = pathPos.fOrigin;
            // Transform the glyph to source space.
            SkMatrix pathMatrix = strikeToSource;
            pathMatrix.postTranslate(pos.x(), pos.y());

            SkPath deviceOutline;
            path.transform(pathMatrix, &deviceOutline);
            deviceOutline.setIsVolatile(true);
            GrStyledShape shape(deviceOutline, paint);
            GrBlurUtils::drawShapeWithMaskFilter(sdc->recordingContext(), sdc, clip, runPaint,
                                                 viewMatrix, shape);
        }
    }
}

bool PathSubRun::canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const {
    return true;
}

GrSubRunOwner PathSubRun::Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                               bool isAntiAliased,
                               SkScalar strikeToSourceScale,
                               GrSubRunAllocator* alloc) {
    auto pathData = alloc->makeUniqueArray<PathGlyph>(
            drawables.size(),
            [&](int i){
                auto [variant, pos] = drawables[i];
                return PathGlyph{*variant.path(), pos};
            });
    SkSpan<PathGlyph> paths{pathData.get(), drawables.size()};

    return alloc->makeUnique<PathSubRun>(
            isAntiAliased, strikeToSourceScale, paths, std::move(pathData));
}

GrAtlasSubRun* PathSubRun::testingOnly_atlasSubRun() {
    return nullptr;
};

// -- GlyphVector ----------------------------------------------------------------------------------
// GlyphVector provides a way to delay the lookup of GrGlyphs until the code is running on the
// GPU in single threaded mode. The GlyphVector is created in a multi-threaded environment, but
// the GrStrikeCache is only single threaded (and must be single threaded because of the atlas).
class GlyphVector {
public:
    union Variant {
        // Initially, filled with packed id, but changed to GrGlyph* in the onPrepare stage.
        SkPackedGlyphID packedGlyphID;
        GrGlyph* grGlyph;
        // Add ctors to help SkArenaAlloc create arrays.
        Variant() : grGlyph{nullptr} {}
        Variant(SkPackedGlyphID id) : packedGlyphID{id} {}
    };

    GlyphVector(sk_sp<SkStrike>&& strike, SkSpan<Variant> glyphs);

    static GlyphVector Make(
            sk_sp<SkStrike>&& strike, SkSpan<SkGlyphVariant> glyphs, GrSubRunAllocator* alloc);
    SkSpan<const GrGlyph*> glyphs() const;

    void packedGlyphIDToGrGlyph(GrStrikeCache* cache);

    std::tuple<bool, int> regenerateAtlas(
            int begin, int end,
            GrMaskFormat maskFormat,
            int srcPadding,
            GrMeshDrawTarget *,
            bool bilerpPadding = false);

    static size_t GlyphVectorSize(size_t count) {
        return sizeof(Variant) * count;
    }

private:
    sk_sp<SkStrike> fStrike;
    SkSpan<Variant> fGlyphs;
    sk_sp<GrTextStrike> fGrStrike{nullptr};
    uint64_t fAtlasGeneration{GrDrawOpAtlas::kInvalidAtlasGeneration};
    GrDrawOpAtlas::BulkUseTokenUpdater fBulkUseToken;
};

GlyphVector::GlyphVector(sk_sp<SkStrike>&& strike, SkSpan<Variant> glyphs)
        : fStrike{std::move(strike)}
        , fGlyphs{glyphs} {
    SkASSERT(fStrike != nullptr);
}

GlyphVector GlyphVector::Make(
        sk_sp<SkStrike>&& strike, SkSpan<SkGlyphVariant> glyphs, GrSubRunAllocator* alloc) {
    Variant* variants = alloc->makePODArray<Variant>(glyphs.size());
    for (auto [i, gv] : SkMakeEnumerate(glyphs)) {
        variants[i] = gv.glyph()->getPackedID();
    }

    return GlyphVector{std::move(strike), SkMakeSpan(variants, glyphs.size())};
}

SkSpan<const GrGlyph*> GlyphVector::glyphs() const {
    return SkMakeSpan(reinterpret_cast<const GrGlyph**>(fGlyphs.data()), fGlyphs.size());
}

// packedGlyphIDToGrGlyph must be run in single-threaded mode.
// If fStrike != nullptr then the conversion to GrGlyph* has not happened.
void GlyphVector::packedGlyphIDToGrGlyph(GrStrikeCache* cache) {
    if (fStrike != nullptr) {
        fGrStrike = cache->findOrCreateStrike(fStrike->strikeSpec());

        for (auto& variant : fGlyphs) {
            variant.grGlyph = fGrStrike->getGlyph(variant.packedGlyphID);
        }

        // Drop the ref on the strike that was taken in the SkGlyphRunPainter process* methods.
        fStrike = nullptr;
    }
}

std::tuple<bool, int> GlyphVector::regenerateAtlas(int begin, int end,
                                                   GrMaskFormat maskFormat,
                                                   int srcPadding,
                                                   GrMeshDrawTarget* target,
                                                   bool bilerpPadding) {
    GrAtlasManager* atlasManager = target->atlasManager();
    GrDeferredUploadTarget* uploadTarget = target->deferredUploadTarget();

    uint64_t currentAtlasGen = atlasManager->atlasGeneration(maskFormat);

    this->packedGlyphIDToGrGlyph(target->strikeCache());

    if (fAtlasGeneration != currentAtlasGen) {
        // Calculate the texture coordinates for the vertexes during first use (fAtlasGeneration
        // is set to kInvalidAtlasGeneration) or the atlas has changed in subsequent calls..
        fBulkUseToken.reset();

        SkBulkGlyphMetricsAndImages metricsAndImages{fGrStrike->strikeSpec()};

        // Update the atlas information in the GrStrike.
        auto tokenTracker = uploadTarget->tokenTracker();
        auto glyphs = fGlyphs.subspan(begin, end - begin);
        int glyphsPlacedInAtlas = 0;
        bool success = true;
        for (const Variant& variant : glyphs) {
            GrGlyph* grGlyph = variant.grGlyph;
            SkASSERT(grGlyph != nullptr);

            if (!atlasManager->hasGlyph(maskFormat, grGlyph)) {
                const SkGlyph& skGlyph = *metricsAndImages.glyph(grGlyph->fPackedID);
                auto code = atlasManager->addGlyphToAtlas(
                        skGlyph, grGlyph, srcPadding, target->resourceProvider(),
                        uploadTarget, bilerpPadding);
                if (code != GrDrawOpAtlas::ErrorCode::kSucceeded) {
                    success = code != GrDrawOpAtlas::ErrorCode::kError;
                    break;
                }
            }
            atlasManager->addGlyphToBulkAndSetUseToken(
                    &fBulkUseToken, maskFormat, grGlyph,
                    tokenTracker->nextDrawToken());
            glyphsPlacedInAtlas++;
        }

        // Update atlas generation if there are no more glyphs to put in the atlas.
        if (success && begin + glyphsPlacedInAtlas == SkCount(fGlyphs)) {
            // Need to get the freshest value of the atlas' generation because
            // updateTextureCoordinates may have changed it.
            fAtlasGeneration = atlasManager->atlasGeneration(maskFormat);
        }

        return {success, glyphsPlacedInAtlas};
    } else {
        // The atlas hasn't changed, so our texture coordinates are still valid.
        if (end == SkCount(fGlyphs)) {
            // The atlas hasn't changed and the texture coordinates are all still valid. Update
            // all the plots used to the new use token.
            atlasManager->setUseTokenBulk(fBulkUseToken,
                                          uploadTarget->tokenTracker()->nextDrawToken(),
                                          maskFormat);
        }
        return {true, end - begin};
    }
}

// -- DirectMaskSubRun -----------------------------------------------------------------------------
class DirectMaskSubRun final : public GrSubRun, public GrAtlasSubRun {
public:
    using DevicePosition = skvx::Vec<2, int16_t>;

    DirectMaskSubRun(GrMaskFormat format,
                     GrTextBlob* blob,
                     const SkGlyphRect& deviceBounds,
                     SkSpan<const DevicePosition> devicePositions,
                     GlyphVector&& glyphs,
                     bool glyphsOutOfBounds);

    static GrSubRunOwner Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                              sk_sp<SkStrike>&& strike,
                              GrMaskFormat format,
                              GrTextBlob* blob,
                              GrSubRunAllocator* alloc);

    void draw(const GrClip*,
              const SkMatrixProvider& viewMatrix,
              SkPoint drawOrigin,
              const SkPaint& paint,
              skgpu::v1::SurfaceDrawContext*) const override;

    std::tuple<const GrClip*, GrOp::Owner>
    makeAtlasTextOp(const GrClip* clip,
                    const SkMatrixProvider& viewMatrix,
                    SkPoint drawOrigin,
                    const SkPaint& paint,
                    skgpu::v1::SurfaceDrawContext* sdc,
                    GrAtlasSubRunOwner) const override;

    bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const override;

    GrAtlasSubRun* testingOnly_atlasSubRun() override;

    size_t vertexStride(const SkMatrix& drawMatrix) const override;

    int glyphCount() const override;

    void testingOnly_packedGlyphIDToGrGlyph(GrStrikeCache *cache) override;

    std::tuple<bool, int>
    regenerateAtlas(int begin, int end, GrMeshDrawTarget*) const override;

    void fillVertexData(void* vertexDst, int offset, int count,
                        GrColor color,
                        const SkMatrix& drawMatrix, SkPoint drawOrigin,
                        SkIRect clip) const override;
private:
    // The rectangle that surrounds all the glyph bounding boxes in device space.
    SkRect deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const;

    const GrMaskFormat fMaskFormat;
    GrTextBlob* const fBlob;

    // The union of all the glyph bounds in device space.
    const SkGlyphRect fGlyphDeviceBounds;
    const SkSpan<const DevicePosition> fLeftTopDevicePos;
    const bool fSomeGlyphsExcluded;

    // The regenerateAtlas method mutates fGlyphs. It should be called from onPrepare which must
    // be single threaded.
    mutable GlyphVector fGlyphs;
};

DirectMaskSubRun::DirectMaskSubRun(GrMaskFormat format,
                                   GrTextBlob* blob,
                                   const SkGlyphRect& deviceBounds,
                                   SkSpan<const DevicePosition> devicePositions,
                                   GlyphVector&& glyphs,
                                   bool glyphsOutOfBounds)
        : fMaskFormat{format}
        , fBlob{blob}
        , fGlyphDeviceBounds{deviceBounds}
        , fLeftTopDevicePos{devicePositions}
        , fSomeGlyphsExcluded{glyphsOutOfBounds}
        , fGlyphs{std::move(glyphs)} {}

GrSubRunOwner DirectMaskSubRun::Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                     sk_sp<SkStrike>&& strike,
                                     GrMaskFormat format,
                                     GrTextBlob* blob,
                                     GrSubRunAllocator* alloc) {
    DevicePosition* glyphLeftTop = alloc->makePODArray<DevicePosition>(drawables.size());
    GlyphVector::Variant* glyphIDs = alloc->makePODArray<GlyphVector::Variant>(drawables.size());

    // Because this is the direct case, the maximum width or height is the size that fits in the
    // atlas. This boundary is checked below to ensure that the call to SkGlyphRect below will
    // not overflow.
    constexpr SkScalar kMaxPos =
            std::numeric_limits<int16_t>::max() - SkStrikeCommon::kSkSideTooBigForAtlas;
    SkGlyphRect runBounds = skglyph::empty_rect();
    size_t goodPosCount = 0;
    for (auto [variant, pos] : drawables) {
        auto [x, y] = pos;
        // Ensure that the .offset() call below does not overflow. And, at this point none of the
        // rectangles are empty because they were culled before the run was created. Basically,
        // cull all the glyphs that can't appear on the screen.
        if (-kMaxPos < x && x < kMaxPos && -kMaxPos  < y && y < kMaxPos) {
            const SkGlyph* const skGlyph = variant;
            const SkGlyphRect deviceBounds =
                    skGlyph->glyphRect().offset(SkScalarRoundToInt(x), SkScalarRoundToInt(y));
            runBounds = skglyph::rect_union(runBounds, deviceBounds);
            glyphLeftTop[goodPosCount] = deviceBounds.topLeft();
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
    bool glyphsExcluded = goodPosCount != drawables.size();
    SkSpan<const DevicePosition> leftTop{glyphLeftTop, goodPosCount};
    return alloc->makeUnique<DirectMaskSubRun>(
            format, blob, runBounds, leftTop,
            GlyphVector{std::move(strike), {glyphIDs, goodPosCount}}, glyphsExcluded);
}

bool DirectMaskSubRun::canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const {
    auto [reuse, translation] = can_use_direct(fBlob->initialPositionMatrix(), positionMatrix);

    // If glyphs were excluded because of position bounds, then this subrun can only be reused if
    // there is no change in position.
    if (fSomeGlyphsExcluded) {
        return translation.x() == 0 && translation.y() == 0;
    }

    return reuse;
}

size_t DirectMaskSubRun::vertexStride(const SkMatrix&) const {
    if (fMaskFormat != kARGB_GrMaskFormat) {
        return sizeof(Mask2DVertex);
    } else {
        return sizeof(ARGB2DVertex);
    }
}

int DirectMaskSubRun::glyphCount() const {
    return SkCount(fGlyphs.glyphs());
}

void DirectMaskSubRun::draw(const GrClip* clip,
                            const SkMatrixProvider& viewMatrix,
                            SkPoint drawOrigin,
                            const SkPaint& paint,
                            skgpu::v1::SurfaceDrawContext* sdc) const{
    auto[drawingClip, op] = this->makeAtlasTextOp(
            clip, viewMatrix, drawOrigin, paint, sdc, nullptr);
    if (op != nullptr) {
        sdc->addDrawOp(drawingClip, std::move(op));
    }
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
}  // namespace

std::tuple<const GrClip*, GrOp::Owner>
DirectMaskSubRun::makeAtlasTextOp(const GrClip* clip,
                                  const SkMatrixProvider& viewMatrix,
                                  SkPoint drawOrigin,
                                  const SkPaint& paint,
                                  skgpu::v1::SurfaceDrawContext* sdc,
                                  GrAtlasSubRunOwner) const {
    SkASSERT(this->glyphCount() != 0);

    const SkMatrix& drawMatrix = viewMatrix.localToDevice();

    // We can clip geometrically using clipRect and ignore clip when an axis-aligned rectangular
    // non-AA clip is used. If clipRect is empty, and clip is nullptr, then there is no clipping
    // needed.
    const SkRect subRunBounds = this->deviceRect(drawMatrix, drawOrigin);
    const SkRect deviceBounds = SkRect::MakeWH(sdc->width(), sdc->height());
    auto [clipMethod, clipRect] = calculate_clip(clip, deviceBounds, subRunBounds);

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
            // Use the the GPU clip; clipRect is ignored.
            break;
    }

    if (!clipRect.isEmpty()) { SkASSERT(clip == nullptr); }

    GrPaint grPaint;
    const SkPMColor4f drawingColor =
            calculate_colors(sdc, paint, viewMatrix, fMaskFormat, &grPaint);

    auto geometry = AtlasTextOp::Geometry::MakeForBlob(*this,
                                                       drawMatrix,
                                                       drawOrigin,
                                                       clipRect,
                                                       sk_ref_sp<GrTextBlob>(fBlob),
                                                       drawingColor,
                                                       sdc->arenaAlloc());

    GrRecordingContext* const rContext = sdc->recordingContext();
    GrOp::Owner op = GrOp::Make<AtlasTextOp>(rContext,
                                             op_mask_type(fMaskFormat),
                                             false,
                                             this->glyphCount(),
                                             subRunBounds,
                                             geometry,
                                             std::move(grPaint));

    return {clip, std::move(op)};
}

void DirectMaskSubRun::testingOnly_packedGlyphIDToGrGlyph(GrStrikeCache *cache) {
    fGlyphs.packedGlyphIDToGrGlyph(cache);
}

std::tuple<bool, int>
DirectMaskSubRun::regenerateAtlas(int begin, int end, GrMeshDrawTarget* target) const {
    if (fBlob->supportBilerpAtlas()) {
        return fGlyphs.regenerateAtlas(begin, end, fMaskFormat, 1, target, true);
    } else {
        return fGlyphs.regenerateAtlas(begin, end, fMaskFormat, 0, target, false);
    }
}

// The 99% case. No clip. Non-color only.
void direct_2D(SkZip<Mask2DVertex[4],
               const GrGlyph*,
               const DirectMaskSubRun::DevicePosition> quadData,
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

template <typename Rect>
auto ltbr(const Rect& r) {
    return std::make_tuple(r.left(), r.top(), r.right(), r.bottom());
}

// Handle any combination of BW or color and clip or no clip.
template<typename Quad, typename VertexData>
void generalized_direct_2D(SkZip<Quad, const GrGlyph*, const VertexData> quadData,
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

void DirectMaskSubRun::fillVertexData(void* vertexDst, int offset, int count,
                                      GrColor color,
                                      const SkMatrix& drawMatrix, SkPoint drawOrigin,
                                      SkIRect clip) const {
    const SkMatrix positionMatrix = position_matrix(drawMatrix, drawOrigin);
    auto quadData = [&](auto dst) {
        return SkMakeZip(dst,
                         fGlyphs.glyphs().subspan(offset, count),
                         fLeftTopDevicePos.subspan(offset, count));
    };

    SkPoint originOffset = positionMatrix.mapOrigin() - fBlob->initialPositionMatrix().mapOrigin();

    if (clip.isEmpty()) {
        if (fMaskFormat != kARGB_GrMaskFormat) {
            using Quad = Mask2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
            direct_2D(quadData((Quad*)vertexDst), color, originOffset);
        } else {
            using Quad = ARGB2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
            generalized_direct_2D(quadData((Quad*)vertexDst), color, originOffset);
        }
    } else {
        if (fMaskFormat != kARGB_GrMaskFormat) {
            using Quad = Mask2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
            generalized_direct_2D(quadData((Quad*)vertexDst), color, originOffset, &clip);
        } else {
            using Quad = ARGB2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
            generalized_direct_2D(quadData((Quad*)vertexDst), color, originOffset, &clip);
        }
    }
}

SkRect DirectMaskSubRun::deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const {
    SkIRect outBounds = fGlyphDeviceBounds.iRect();

    // Calculate the offset from the initial device origin to the current device origin.
    SkVector offset = drawMatrix.mapPoint(drawOrigin) - fBlob->initialPositionMatrix().mapOrigin();

    // The offset should be integer, but make sure.
    SkIVector iOffset = {SkScalarRoundToInt(offset.x()), SkScalarRoundToInt(offset.y())};

    return SkRect::Make(outBounds.makeOffset(iOffset));
}

GrAtlasSubRun* DirectMaskSubRun::testingOnly_atlasSubRun() {
    return this;
}

// -- TransformedMaskSubRun ------------------------------------------------------------------------
class TransformedMaskSubRun final : public GrSubRun, public GrAtlasSubRun {
public:
    struct VertexData {
        const SkPoint pos;
        // The rectangle of the glyphs in strike space. But, for kDirectMask this also implies a
        // device space rect.
        GrIRect16 rect;
    };

    TransformedMaskSubRun(GrMaskFormat format,
                          GrTextBlob* blob,
                          SkScalar strikeToSourceScale,
                          const SkRect& bounds,
                          SkSpan<const VertexData> vertexData,
                          GlyphVector&& glyphs);

    static GrSubRunOwner Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                              sk_sp<SkStrike>&& strike,
                              SkScalar strikeToSourceScale,
                              GrMaskFormat format,
                              GrTextBlob* blob,
                              GrSubRunAllocator* alloc);

    void draw(const GrClip*,
              const SkMatrixProvider& viewMatrix,
              SkPoint drawOrigin,
              const SkPaint& paint,
              skgpu::v1::SurfaceDrawContext*) const override;

    std::tuple<const GrClip*, GrOp::Owner>
    makeAtlasTextOp(const GrClip*,
                    const SkMatrixProvider& viewMatrix,
                    SkPoint drawOrigin,
                    const SkPaint&,
                    skgpu::v1::SurfaceDrawContext*,
                    GrAtlasSubRunOwner) const override;

    bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const override;

    GrAtlasSubRun* testingOnly_atlasSubRun() override;

    void testingOnly_packedGlyphIDToGrGlyph(GrStrikeCache *cache) override;

    std::tuple<bool, int> regenerateAtlas(int begin, int end, GrMeshDrawTarget*) const override;

    void fillVertexData(
            void* vertexDst, int offset, int count,
            GrColor color,
            const SkMatrix& drawMatrix, SkPoint drawOrigin,
            SkIRect clip) const override;

    size_t vertexStride(const SkMatrix& drawMatrix) const override;
    int glyphCount() const override;

private:
    // The rectangle that surrounds all the glyph bounding boxes in device space.
    SkRect deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const;

    const GrMaskFormat fMaskFormat;
    GrTextBlob* fBlob;

    // The scale factor between the strike size, and the source size.
    const SkScalar fStrikeToSourceScale;

    // The bounds in source space. The bounds are the joined rectangles of all the glyphs.
    const SkRect fVertexBounds;
    const SkSpan<const VertexData> fVertexData;

    // The regenerateAtlas method mutates fGlyphs. It should be called from onPrepare which must
    // be single threaded.
    mutable GlyphVector fGlyphs;
};

TransformedMaskSubRun::TransformedMaskSubRun(GrMaskFormat format,
                                             GrTextBlob* blob,
                                             SkScalar strikeToSourceScale,
                                             const SkRect& bounds,
                                             SkSpan<const VertexData> vertexData,
                                             GlyphVector&& glyphs)
        : fMaskFormat{format}
        , fBlob{blob}
        , fStrikeToSourceScale{strikeToSourceScale}
        , fVertexBounds{bounds}
        , fVertexData{vertexData}
        , fGlyphs{std::move(glyphs)} { }

GrSubRunOwner TransformedMaskSubRun::Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                          sk_sp<SkStrike>&& strike,
                                          SkScalar strikeToSourceScale,
                                          GrMaskFormat format,
                                          GrTextBlob* blob,
                                          GrSubRunAllocator* alloc) {
    SkRect bounds = SkRectPriv::MakeLargestInverted();

    SkSpan<VertexData> vertexData = alloc->makePODArray<VertexData>(
            drawables,
            [&](auto e) {
                auto [variant, pos] = e;
                SkGlyph* skGlyph = variant;
                int16_t l = skGlyph->left(),
                        t = skGlyph->top(),
                        r = l + skGlyph->width(),
                        b = t + skGlyph->height();
                SkPoint lt = SkPoint::Make(l, t) * strikeToSourceScale + pos,
                        rb = SkPoint::Make(r, b) * strikeToSourceScale + pos;

                bounds.joinPossiblyEmptyRect(SkRect::MakeLTRB(lt.x(), lt.y(), rb.x(), rb.y()));
                return VertexData{pos, {l, t, r, b}};
            });

    return alloc->makeUnique<TransformedMaskSubRun>(
            format, blob, strikeToSourceScale, bounds, vertexData,
            GlyphVector::Make(std::move(strike), drawables.get<0>(), alloc));
}

void TransformedMaskSubRun::draw(const GrClip* clip,
                                 const SkMatrixProvider& viewMatrix,
                                 SkPoint drawOrigin,
                                 const SkPaint& paint,
                                 skgpu::v1::SurfaceDrawContext* sdc) const {
    auto[drawingClip, op] = this->makeAtlasTextOp(
            clip, viewMatrix, drawOrigin, paint, sdc, nullptr);
    if (op != nullptr) {
        sdc->addDrawOp(drawingClip, std::move(op));
    }
}

std::tuple<const GrClip*, GrOp::Owner>
TransformedMaskSubRun::makeAtlasTextOp(const GrClip* clip,
                                       const SkMatrixProvider& viewMatrix,
                                       SkPoint drawOrigin,
                                       const SkPaint& paint,
                                       skgpu::v1::SurfaceDrawContext* sdc,
                                       GrAtlasSubRunOwner) const {
    SkASSERT(this->glyphCount() != 0);

    const SkMatrix& drawMatrix = viewMatrix.localToDevice();

    GrPaint grPaint;
    SkPMColor4f drawingColor = calculate_colors(sdc, paint, viewMatrix, fMaskFormat, &grPaint);

    auto geometry = AtlasTextOp::Geometry::MakeForBlob(*this,
                                                       drawMatrix,
                                                       drawOrigin,
                                                       SkIRect::MakeEmpty(),
                                                       sk_ref_sp<GrTextBlob>(fBlob),
                                                       drawingColor,
                                                       sdc->arenaAlloc());

    GrRecordingContext* const rContext = sdc->recordingContext();
    GrOp::Owner op = GrOp::Make<AtlasTextOp>(rContext,
                                             op_mask_type(fMaskFormat),
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
    if (fBlob->initialPositionMatrix().getMaxScale() < 1) {
        return false;
    }
    return true;
}

void TransformedMaskSubRun::testingOnly_packedGlyphIDToGrGlyph(GrStrikeCache *cache) {
    fGlyphs.packedGlyphIDToGrGlyph(cache);
}

std::tuple<bool, int> TransformedMaskSubRun::regenerateAtlas(int begin, int end,
                                                             GrMeshDrawTarget* target) const {
    return fGlyphs.regenerateAtlas(begin, end, fMaskFormat, 1, target, true);
}

void TransformedMaskSubRun::fillVertexData(void* vertexDst, int offset, int count,
                                           GrColor color,
                                           const SkMatrix& drawMatrix, SkPoint drawOrigin,
                                           SkIRect clip) const {
    const SkMatrix positionMatrix = position_matrix(drawMatrix, drawOrigin);
    constexpr SkScalar kDstPadding = 0.f;

    auto quadData = [&](auto dst) {
        return SkMakeZip(dst,
                         fGlyphs.glyphs().subspan(offset, count),
                         fVertexData.subspan(offset, count));
    };

    if (!positionMatrix.hasPerspective()) {
        if (fMaskFormat == GrMaskFormat::kARGB_GrMaskFormat) {
            using Quad = ARGB2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
            fill_transformed_vertices_2D(
                    quadData((Quad*) vertexDst),
                    kDstPadding,
                    fStrikeToSourceScale,
                    color,
                    positionMatrix);
        } else {
            using Quad = Mask2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
            fill_transformed_vertices_2D(
                    quadData((Quad*) vertexDst),
                    kDstPadding,
                    fStrikeToSourceScale,
                    color,
                    positionMatrix);
        }
    } else {
        if (fMaskFormat == GrMaskFormat::kARGB_GrMaskFormat) {
            using Quad = ARGB3DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
            fill_transformed_vertices_3D(
                    quadData((Quad*) vertexDst),
                    kDstPadding,
                    fStrikeToSourceScale,
                    color,
                    positionMatrix);
        } else {
            using Quad = Mask3DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
            fill_transformed_vertices_3D(
                    quadData((Quad*) vertexDst),
                    kDstPadding,
                    fStrikeToSourceScale,
                    color,
                    positionMatrix);
        }
    }
}

size_t TransformedMaskSubRun::vertexStride(const SkMatrix& drawMatrix) const {
    switch (fMaskFormat) {
        case kA8_GrMaskFormat:
            return drawMatrix.hasPerspective() ? sizeof(Mask3DVertex) : sizeof(Mask2DVertex);
        case kARGB_GrMaskFormat:
            return drawMatrix.hasPerspective() ? sizeof(ARGB3DVertex) : sizeof(ARGB2DVertex);
        default:
            SkASSERT(!drawMatrix.hasPerspective());
            return sizeof(Mask2DVertex);
    }
    SkUNREACHABLE;
}

int TransformedMaskSubRun::glyphCount() const {
    return SkCount(fVertexData);
}

SkRect TransformedMaskSubRun::deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const {
    SkRect outBounds = fVertexBounds;
    outBounds.offset(drawOrigin);
    return drawMatrix.mapRect(outBounds);
}

GrAtlasSubRun* TransformedMaskSubRun::testingOnly_atlasSubRun() {
    return this;
}

// -- SDFTSubRun -----------------------------------------------------------------------------------
class SDFTSubRun final : public GrSubRun, public GrAtlasSubRun {
public:
    struct VertexData {
        const SkPoint pos;
        // The rectangle of the glyphs in strike space.
        GrIRect16 rect;
    };

    SDFTSubRun(GrMaskFormat format,
               GrTextBlob* blob,
               SkScalar strikeToSource,
               SkRect vertexBounds,
               SkSpan<const VertexData> vertexData,
               GlyphVector&& glyphs,
               bool useLCDText,
               bool antiAliased);

    static GrSubRunOwner Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                              const SkFont& runFont,
                              sk_sp<SkStrike>&& strike,
                              SkScalar strikeToSourceScale,
                              GrTextBlob* blob,
                              GrSubRunAllocator* alloc);

    void draw(const GrClip*,
              const SkMatrixProvider& viewMatrix,
              SkPoint drawOrigin,
              const SkPaint&,
              skgpu::v1::SurfaceDrawContext*) const override;

    std::tuple<const GrClip*, GrOp::Owner>
    makeAtlasTextOp(const GrClip*,
                    const SkMatrixProvider& viewMatrix,
                    SkPoint drawOrigin,
                    const SkPaint&,
                    skgpu::v1::SurfaceDrawContext*,
                    GrAtlasSubRunOwner) const override;

    bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const override;

    GrAtlasSubRun* testingOnly_atlasSubRun() override;

    void testingOnly_packedGlyphIDToGrGlyph(GrStrikeCache *cache) override;

    std::tuple<bool, int> regenerateAtlas(int begin, int end, GrMeshDrawTarget*) const override;

    void fillVertexData(
            void* vertexDst, int offset, int count,
            GrColor color,
            const SkMatrix& drawMatrix, SkPoint drawOrigin,
            SkIRect clip) const override;

    size_t vertexStride(const SkMatrix& drawMatrix) const override;
    int glyphCount() const override;

private:
    // The rectangle that surrounds all the glyph bounding boxes in device space.
    SkRect deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const;

    const GrMaskFormat fMaskFormat;
    GrTextBlob* fBlob;

    // The scale factor between the strike size, and the source size.
    const SkScalar fStrikeToSourceScale;

    // The bounds in source space. The bounds are the joined rectangles of all the glyphs.
    const SkRect fVertexBounds;
    const SkSpan<const VertexData> fVertexData;

    // The regenerateAtlas method mutates fGlyphs. It should be called from onPrepare which must
    // be single threaded.
    mutable GlyphVector fGlyphs;

    const bool fUseLCDText;
    const bool fAntiAliased;
};

SDFTSubRun::SDFTSubRun(GrMaskFormat format,
                       GrTextBlob* textBlob,
                       SkScalar strikeToSource,
                       SkRect vertexBounds,
                       SkSpan<const VertexData> vertexData,
                       GlyphVector&& glyphs,
                       bool useLCDText,
                       bool antiAliased)
        : fMaskFormat{format}
        , fBlob{textBlob}
        , fStrikeToSourceScale{strikeToSource}
        , fVertexBounds{vertexBounds}
        , fVertexData{vertexData}
        , fGlyphs{std::move(glyphs)}
        , fUseLCDText{useLCDText}
        , fAntiAliased{antiAliased} {}

bool has_some_antialiasing(const SkFont& font ) {
    SkFont::Edging edging = font.getEdging();
    return edging == SkFont::Edging::kAntiAlias
           || edging == SkFont::Edging::kSubpixelAntiAlias;
}

GrSubRunOwner SDFTSubRun::Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                               const SkFont& runFont,
                               sk_sp<SkStrike>&& strike,
                               SkScalar strikeToSourceScale,
                               GrTextBlob* blob,
                               GrSubRunAllocator* alloc) {
    SkRect bounds = SkRectPriv::MakeLargestInverted();
    auto mapper = [&](const auto& d) {
        auto& [variant, pos] = d;
        SkGlyph* skGlyph = variant;
        int16_t l = skGlyph->left(),
                t = skGlyph->top(),
                r = l + skGlyph->width(),
                b = t + skGlyph->height();
        SkPoint lt = SkPoint::Make(l, t) * strikeToSourceScale + pos,
                rb = SkPoint::Make(r, b) * strikeToSourceScale + pos;

        bounds.joinPossiblyEmptyRect(SkRect::MakeLTRB(lt.x(), lt.y(), rb.x(), rb.y()));
        return VertexData{pos, {l, t, r, b}};
    };

    SkSpan<VertexData> vertexData = alloc->makePODArray<VertexData>(drawables, mapper);

    return alloc->makeUnique<SDFTSubRun>(
            kA8_GrMaskFormat,
            blob,
            strikeToSourceScale,
            bounds,
            vertexData,
            GlyphVector::Make(std::move(strike), drawables.get<0>(), alloc),
            runFont.getEdging() == SkFont::Edging::kSubpixelAntiAlias,
            has_some_antialiasing(runFont));
}

void SDFTSubRun::draw(const GrClip* clip,
                      const SkMatrixProvider& viewMatrix,
                      SkPoint drawOrigin,
                      const SkPaint& paint,
                      skgpu::v1::SurfaceDrawContext* sdc) const {
    auto[drawingClip, op] = this->makeAtlasTextOp(
            clip, viewMatrix, drawOrigin, paint, sdc, nullptr);
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
                            skgpu::v1::SurfaceDrawContext* sdc,
                            GrAtlasSubRunOwner) const {
    SkASSERT(this->glyphCount() != 0);
    SkASSERT(!viewMatrix.localToDevice().hasPerspective());

    const SkMatrix& drawMatrix = viewMatrix.localToDevice();

    GrPaint grPaint;
    SkPMColor4f drawingColor = calculate_colors(sdc, paint, viewMatrix, fMaskFormat, &grPaint);

    auto [maskType, DFGPFlags, useGammaCorrectDistanceTable] =
        calculate_sdf_parameters(*sdc, drawMatrix, fUseLCDText, fAntiAliased);

    auto geometry = AtlasTextOp::Geometry::MakeForBlob(*this,
                                                       drawMatrix,
                                                       drawOrigin,
                                                       SkIRect::MakeEmpty(),
                                                       sk_ref_sp<GrTextBlob>(fBlob),
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
    const SkMatrix& initialPositionMatrix = fBlob->initialPositionMatrix();

    // A scale outside of [blob.fMaxMinScale, blob.fMinMaxScale] would result in a different
    // distance field being generated, so we have to regenerate in those cases
    SkScalar newMaxScale = positionMatrix.getMaxScale();
    SkScalar oldMaxScale = initialPositionMatrix.getMaxScale();
    SkScalar scaleAdjust = newMaxScale / oldMaxScale;
    auto [maxMinScale, minMaxScale] = fBlob->scaleBounds();
    if (scaleAdjust < maxMinScale || scaleAdjust > minMaxScale) {
        return false;
    }
    return true;
}

void SDFTSubRun::testingOnly_packedGlyphIDToGrGlyph(GrStrikeCache *cache) {
    fGlyphs.packedGlyphIDToGrGlyph(cache);
}

std::tuple<bool, int> SDFTSubRun::regenerateAtlas(
        int begin, int end, GrMeshDrawTarget *target) const {

    return fGlyphs.regenerateAtlas(begin, end, fMaskFormat, SK_DistanceFieldInset, target);
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

    using Quad = Mask2DVertex[4];
    SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
    fill_transformed_vertices_2D(
            SkMakeZip((Quad*)vertexDst,
                      fGlyphs.glyphs().subspan(offset, count),
                      fVertexData.subspan(offset, count)),
            SK_DistanceFieldInset,
            fStrikeToSourceScale,
            color,
            positionMatrix);
}

int SDFTSubRun::glyphCount() const {
    return SkCount(fVertexData);
}

SkRect SDFTSubRun::deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const {
    SkRect outBounds = fVertexBounds;
    outBounds.offset(drawOrigin);
    return drawMatrix.mapRect(outBounds);
}

GrAtlasSubRun* SDFTSubRun::testingOnly_atlasSubRun() {
    return this;
}

template<typename AddSingleMaskFormat>
void add_multi_mask_format(
        AddSingleMaskFormat addSingleMaskFormat,
        const SkZip<SkGlyphVariant, SkPoint>& drawables,
        sk_sp<SkStrike>&& strike) {
    if (drawables.empty()) { return; }

    auto glyphSpan = drawables.get<0>();
    SkGlyph* glyph = glyphSpan[0];
    GrMaskFormat format = GrGlyph::FormatFromSkGlyph(glyph->maskFormat());
    size_t startIndex = 0;
    for (size_t i = 1; i < drawables.size(); i++) {
        glyph = glyphSpan[i];
        GrMaskFormat nextFormat = GrGlyph::FormatFromSkGlyph(glyph->maskFormat());
        if (format != nextFormat) {
            auto glyphsWithSameFormat = drawables.subspan(startIndex, i - startIndex);
            // Take a ref on the strike. This should rarely happen.
            addSingleMaskFormat(glyphsWithSameFormat, format, sk_sp<SkStrike>(strike));
            format = nextFormat;
            startIndex = i;
        }
    }
    auto glyphsWithSameFormat = drawables.last(drawables.size() - startIndex);
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
        key.fScalerContextFlags = scalerContextFlags;

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
                                   bool supportBilerpAtlas,
                                   const GrSDFTControl& control,
                                   SkGlyphRunListPainter* painter) {
    // The difference in alignment from the per-glyph data to the SubRun;
    constexpr size_t alignDiff =
            alignof(DirectMaskSubRun) - alignof(DirectMaskSubRun::DevicePosition);
    constexpr size_t vertexDataToSubRunPadding = alignDiff > 0 ? alignDiff : 0;
    size_t totalGlyphCount = glyphRunList.totalGlyphCount();

    // The neededForSubRun is optimized for DirectMaskSubRun which is by far the most common case.
    size_t bytesNeededForSubRun = GrBagOfBytes::PlatformMinimumSizeWithOverhead(
            totalGlyphCount * sizeof(DirectMaskSubRun::DevicePosition)
            + GlyphVector::GlyphVectorSize(totalGlyphCount)
            + glyphRunList.runCount() * (sizeof(DirectMaskSubRun) + vertexDataToSubRunPadding),
            alignof(GrTextBlob));

    size_t allocationSize = sizeof(GrTextBlob) + bytesNeededForSubRun;

    void* allocation = ::operator new (allocationSize);

    SkColor initialLuminance = SkPaintPriv::ComputeLuminanceColor(paint);
    sk_sp<GrTextBlob> blob{
        new (allocation) GrTextBlob(
                bytesNeededForSubRun, supportBilerpAtlas, positionMatrix, initialLuminance)};

    const uint64_t uniqueID = glyphRunList.uniqueID();
    for (auto& glyphRun : glyphRunList) {
        painter->processGlyphRun(glyphRun,
                                 positionMatrix,
                                 paint,
                                 control,
                                 blob.get(),
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
size_t GrTextBlob::size() const { return fSize; }

GrTextBlob::GrTextBlob(int allocSize,
                       bool supportBilerpAtlas,
                       const SkMatrix& positionMatrix,
                       SkColor initialLuminance)
        : fAlloc{SkTAddOffset<char>(this, sizeof(GrTextBlob)), allocSize, allocSize/2}
        , fSize{allocSize}
        , fSupportBilerpAtlas{supportBilerpAtlas}
        , fInitialPositionMatrix{positionMatrix}
        , fInitialLuminance{initialLuminance} { }

void GrTextBlob::processDeviceMasks(
        const SkZip<SkGlyphVariant, SkPoint>& drawables, sk_sp<SkStrike>&& strike) {
    SkASSERT(strike != nullptr);
    auto addGlyphsWithSameFormat = [&] (const SkZip<SkGlyphVariant, SkPoint>& drawable,
                                        GrMaskFormat format,
                                        sk_sp<SkStrike>&& runStrike) {
        GrSubRunOwner subRun = DirectMaskSubRun::Make(
                drawable, std::move(runStrike), format, this, &fAlloc);
        if (subRun != nullptr) {
            fSubRunList.append(std::move(subRun));
        } else {
            fSomeGlyphsExcluded = true;
        }
    };
    add_multi_mask_format(addGlyphsWithSameFormat, drawables, std::move(strike));
}

void GrTextBlob::processSourcePaths(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                    const SkFont& runFont,
                                    SkScalar strikeToSourceScale) {
    fSubRunList.append(PathSubRun::Make(drawables,
                                        has_some_antialiasing(runFont),
                                        strikeToSourceScale,
                                        &fAlloc));
}

void GrTextBlob::processSourceSDFT(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                   sk_sp<SkStrike>&& strike,
                                   SkScalar strikeToSourceScale,
                                   const SkFont& runFont,
                                   SkScalar minScale,
                                   SkScalar maxScale) {

    fMaxMinScale = std::max(minScale, fMaxMinScale);
    fMinMaxScale = std::min(maxScale, fMinMaxScale);
    fSubRunList.append(
        SDFTSubRun::Make(drawables, runFont, std::move(strike), strikeToSourceScale,this, &fAlloc));
}

void GrTextBlob::processSourceMasks(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                    sk_sp<SkStrike>&& strike,
                                    SkScalar strikeToSourceScale) {
    auto addGlyphsWithSameFormat = [&] (const SkZip<SkGlyphVariant, SkPoint>& drawable,
                                        GrMaskFormat format,
                                        sk_sp<SkStrike>&& runStrike) {
        GrSubRunOwner subRun = TransformedMaskSubRun::Make(
                drawable, std::move(runStrike), strikeToSourceScale, format, this, &fAlloc);
        if (subRun != nullptr) {
            fSubRunList.append(std::move(subRun));
        } else {
            fSomeGlyphsExcluded = true;
        }
    };
    add_multi_mask_format(addGlyphsWithSameFormat, drawables, std::move(strike));
}

// ----------------------------- Begin no cache implementation -------------------------------------
namespace {
// -- DirectMaskSubRunNoCache ----------------------------------------------------------------------
class DirectMaskSubRunNoCache final : public GrAtlasSubRun {
public:
    using DevicePosition = skvx::Vec<2, int16_t>;

    DirectMaskSubRunNoCache(GrMaskFormat format,
                            bool supportBilerpAtlas,
                            const SkRect& bounds,
                            SkSpan<const DevicePosition> devicePositions,
                            GlyphVector&& glyphs);

    static GrAtlasSubRunOwner Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                   sk_sp<SkStrike>&& strike,
                                   GrMaskFormat format,
                                   bool supportBilerpAtlas,
                                   GrSubRunAllocator* alloc);

    size_t vertexStride(const SkMatrix& drawMatrix) const override;

    int glyphCount() const override;

    std::tuple<const GrClip*, GrOp::Owner>
    makeAtlasTextOp(const GrClip*,
                    const SkMatrixProvider& viewMatrix,
                    SkPoint,
                    const SkPaint&,
                    skgpu::v1::SurfaceDrawContext*,
                    GrAtlasSubRunOwner) const override;

    void testingOnly_packedGlyphIDToGrGlyph(GrStrikeCache *cache) override;

    std::tuple<bool, int>
    regenerateAtlas(int begin, int end, GrMeshDrawTarget*) const override;

    void fillVertexData(void* vertexDst, int offset, int count,
                        GrColor color,
                        const SkMatrix& drawMatrix, SkPoint drawOrigin,
                        SkIRect clip) const override;

private:
    const GrMaskFormat fMaskFormat;

    // Support bilerping from the atlas.
    const bool fSupportBilerpAtlas;

    // The vertex bounds in device space. The bounds are the joined rectangles of all the glyphs.
    const SkRect fGlyphDeviceBounds;
    const SkSpan<const DevicePosition> fLeftTopDevicePos;

    // Space for geometry
    alignas(alignof(AtlasTextOp::Geometry)) char fGeom[sizeof(AtlasTextOp::Geometry)];

    // The regenerateAtlas method mutates fGlyphs. It should be called from onPrepare which must
    // be single threaded.
    mutable GlyphVector fGlyphs;
};

DirectMaskSubRunNoCache::DirectMaskSubRunNoCache(GrMaskFormat format,
                                                 bool supportBilerpAtlas,
                                                 const SkRect& deviceBounds,
                                                 SkSpan<const DevicePosition> devicePositions,
                                                 GlyphVector&& glyphs)
        : fMaskFormat{format}
        , fSupportBilerpAtlas{supportBilerpAtlas}
        , fGlyphDeviceBounds{deviceBounds}
        , fLeftTopDevicePos{devicePositions}
        , fGlyphs{std::move(glyphs)} { }

GrAtlasSubRunOwner DirectMaskSubRunNoCache::Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                                 sk_sp<SkStrike>&& strike,
                                                 GrMaskFormat format,
                                                 bool supportBilerpAtlas,
                                                 GrSubRunAllocator* alloc) {
    DevicePosition* glyphLeftTop = alloc->makePODArray<DevicePosition>(drawables.size());

    GlyphVector::Variant* glyphIDs = static_cast<GlyphVector::Variant*>(
            alloc->alignedBytes(drawables.size() * sizeof(GlyphVector::Variant),
                                alignof(GlyphVector::Variant)));

    // Because this is the direct case, the maximum width or height is the size that fits in the
    // atlas. This boundary is checked below to ensure that the call to SkGlyphRect below will
    // not overflow.
    constexpr SkScalar kMaxPos =
            std::numeric_limits<int16_t>::max() - SkStrikeCommon::kSkSideTooBigForAtlas;
    SkGlyphRect runBounds = skglyph::empty_rect();
    size_t goodPosCount = 0;
    for (auto [variant, pos] : drawables) {
        auto [x, y] = pos;
        // Ensure that the .offset() call below does not overflow. And, at this point none of the
        // rectangles are empty because they were culled before the run was created. Basically,
        // cull all the glyphs that can't appear on the screen.
        if (-kMaxPos < x && x < kMaxPos && -kMaxPos  < y && y < kMaxPos) {
            const SkGlyph* const skGlyph = variant;
            const SkGlyphRect deviceBounds =
                    skGlyph->glyphRect().offset(SkScalarRoundToInt(x), SkScalarRoundToInt(y));
            runBounds = skglyph::rect_union(runBounds, deviceBounds);
            glyphLeftTop[goodPosCount] = deviceBounds.topLeft();
            glyphIDs[goodPosCount].packedGlyphID = skGlyph->getPackedID();
            goodPosCount += 1;
        }
    }

    // Wow! no glyphs are in bounds and had non-empty bounds.
    if (goodPosCount == 0) {
        return nullptr;
    }

    SkSpan<const DevicePosition> leftTop{glyphLeftTop, goodPosCount};
    return alloc->makeUnique<DirectMaskSubRunNoCache>(
            format, supportBilerpAtlas, runBounds.rect(), leftTop,
            GlyphVector{std::move(strike), {glyphIDs, goodPosCount}});
}

size_t DirectMaskSubRunNoCache::vertexStride(const SkMatrix&) const {
    if (fMaskFormat != kARGB_GrMaskFormat) {
        return sizeof(Mask2DVertex);
    } else {
        return sizeof(ARGB2DVertex);
    }
}

int DirectMaskSubRunNoCache::glyphCount() const {
    return SkCount(fGlyphs.glyphs());
}

std::tuple<const GrClip*, GrOp::Owner>
DirectMaskSubRunNoCache::makeAtlasTextOp(const GrClip* clip,
                                         const SkMatrixProvider& viewMatrix,
                                         SkPoint drawOrigin,
                                         const SkPaint& paint,
                                         skgpu::v1::SurfaceDrawContext* sdc,
                                         GrAtlasSubRunOwner subRunOwner) const {
    SkASSERT(this->glyphCount() != 0);

    const SkMatrix& drawMatrix = viewMatrix.localToDevice();

    // We can clip geometrically using clipRect and ignore clip when an axis-aligned rectangular
    // non-AA clip is used. If clipRect is empty, and clip is nullptr, then there is no clipping
    // needed.
    const SkRect deviceBounds = SkRect::MakeWH(sdc->width(), sdc->height());
    auto [clipMethod, clipRect] = calculate_clip(clip, deviceBounds, fGlyphDeviceBounds);

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
            // Use the the GPU clip; clipRect is ignored.
            break;
    }

    if (!clipRect.isEmpty()) { SkASSERT(clip == nullptr); }

    GrPaint grPaint;
    const SkPMColor4f drawingColor =
            calculate_colors(sdc, paint, viewMatrix, fMaskFormat, &grPaint);

    GrRecordingContext* const rContext = sdc->recordingContext();

    auto geometry = new ((void*)fGeom) AtlasTextOp::Geometry{
            *this,
            drawMatrix,
            drawOrigin,
            clipRect,
            nullptr,
            std::move(subRunOwner),
            drawingColor
    };

    GrOp::Owner op = GrOp::Make<AtlasTextOp>(rContext,
                                             op_mask_type(fMaskFormat),
                                             false,
                                             this->glyphCount(),
                                             fGlyphDeviceBounds,
                                             geometry,
                                             std::move(grPaint));

    return {clip, std::move(op)};
}

void DirectMaskSubRunNoCache::testingOnly_packedGlyphIDToGrGlyph(GrStrikeCache *cache) {
    fGlyphs.packedGlyphIDToGrGlyph(cache);
}

std::tuple<bool, int>
DirectMaskSubRunNoCache::regenerateAtlas(int begin, int end, GrMeshDrawTarget* target) const {
    if (fSupportBilerpAtlas) {
        return fGlyphs.regenerateAtlas(begin, end, fMaskFormat, 1, target, true);
    } else {
        return fGlyphs.regenerateAtlas(begin, end, fMaskFormat, 0, target, false);
    }
}

// The 99% case. No clip. Non-color only.
void direct_2D2(SkZip<Mask2DVertex[4],
        const GrGlyph*,
        const DirectMaskSubRunNoCache::DevicePosition> quadData,
        GrColor color) {
    for (auto[quad, glyph, leftTop] : quadData) {
        auto[al, at, ar, ab] = glyph->fAtlasLocator.getUVs();
        SkScalar dl = leftTop[0],
                 dt = leftTop[1],
                 dr = dl + (ar - al),
                 db = dt + (ab - at);

        quad[0] = {{dl, dt}, color, {al, at}};  // L,T
        quad[1] = {{dl, db}, color, {al, ab}};  // L,B
        quad[2] = {{dr, dt}, color, {ar, at}};  // R,T
        quad[3] = {{dr, db}, color, {ar, ab}};  // R,B
    }
}

void DirectMaskSubRunNoCache::fillVertexData(void* vertexDst, int offset, int count,
                                             GrColor color,
                                             const SkMatrix& drawMatrix, SkPoint drawOrigin,
                                             SkIRect clip) const {
    auto quadData = [&](auto dst) {
        return SkMakeZip(dst,
                         fGlyphs.glyphs().subspan(offset, count),
                         fLeftTopDevicePos.subspan(offset, count));
    };

    // Notice that no matrix manipulation is needed because all the rectangles are already mapped
    // to device space.
    if (clip.isEmpty()) {
        if (fMaskFormat != kARGB_GrMaskFormat) {
            using Quad = Mask2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(SkMatrix::I()) * kVerticesPerGlyph);
            direct_2D2(quadData((Quad*)vertexDst), color);
        } else {
            using Quad = ARGB2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(SkMatrix::I()) * kVerticesPerGlyph);
            generalized_direct_2D(quadData((Quad*)vertexDst), color, {0,0});
        }
    } else {
        if (fMaskFormat != kARGB_GrMaskFormat) {
            using Quad = Mask2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(SkMatrix::I()) * kVerticesPerGlyph);
            generalized_direct_2D(quadData((Quad*)vertexDst), color, {0,0}, &clip);
        } else {
            using Quad = ARGB2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(SkMatrix::I()) * kVerticesPerGlyph);
            generalized_direct_2D(quadData((Quad*)vertexDst), color, {0,0}, &clip);
        }
    }
}

// -- TransformedMaskSubRunNoCache -----------------------------------------------------------------
class TransformedMaskSubRunNoCache final : public GrAtlasSubRun {
public:
    struct VertexData {
        const SkPoint pos;
        // The rectangle of the glyphs in strike space. But, for kDirectMask this also implies a
        // device space rect.
        GrIRect16 rect;
    };

    TransformedMaskSubRunNoCache(GrMaskFormat format,
                                 SkScalar strikeToSourceScale,
                                 const SkRect& bounds,
                                 SkSpan<const VertexData> vertexData,
                                 GlyphVector&& glyphs);

    static GrAtlasSubRunOwner Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                   sk_sp<SkStrike>&& strike,
                                   SkScalar strikeToSourceScale,
                                   GrMaskFormat format,
                                   GrSubRunAllocator* alloc);

    std::tuple<const GrClip*, GrOp::Owner>
    makeAtlasTextOp(const GrClip*,
                    const SkMatrixProvider& viewMatrix,
                    SkPoint drawOrigin,
                    const SkPaint&,
                    skgpu::v1::SurfaceDrawContext*,
                    GrAtlasSubRunOwner) const override;

    void testingOnly_packedGlyphIDToGrGlyph(GrStrikeCache *cache) override;

    std::tuple<bool, int> regenerateAtlas(int begin, int end, GrMeshDrawTarget*) const override;

    void fillVertexData(
            void* vertexDst, int offset, int count,
            GrColor color,
            const SkMatrix& drawMatrix, SkPoint drawOrigin,
            SkIRect clip) const override;

    size_t vertexStride(const SkMatrix& drawMatrix) const override;
    int glyphCount() const override;

private:
    // The rectangle that surrounds all the glyph bounding boxes in device space.
    SkRect deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const;

    const GrMaskFormat fMaskFormat;

    // The scale factor between the strike size, and the source size.
    const SkScalar fStrikeToSourceScale;

    // The bounds in source space. The bounds are the joined rectangles of all the glyphs.
    const SkRect fVertexBounds;
    const SkSpan<const VertexData> fVertexData;

    // Space for geometry
    alignas(alignof(AtlasTextOp::Geometry)) char fGeom[sizeof(AtlasTextOp::Geometry)];

    // The regenerateAtlas method mutates fGlyphs. It should be called from onPrepare which must
    // be single threaded.
    mutable GlyphVector fGlyphs;
};

TransformedMaskSubRunNoCache::TransformedMaskSubRunNoCache(GrMaskFormat format,
                                                           SkScalar strikeToSourceScale,
                                                           const SkRect& bounds,
                                                           SkSpan<const VertexData> vertexData,
                                                           GlyphVector&& glyphs)
        : fMaskFormat{format}
        , fStrikeToSourceScale{strikeToSourceScale}
        , fVertexBounds{bounds}
        , fVertexData{vertexData}
        , fGlyphs{std::move(glyphs)} {}

GrAtlasSubRunOwner TransformedMaskSubRunNoCache::Make(
        const SkZip<SkGlyphVariant, SkPoint>& drawables,
        sk_sp<SkStrike>&& strike,
        SkScalar strikeToSourceScale,
        GrMaskFormat format,
        GrSubRunAllocator* alloc) {
    SkRect bounds = SkRectPriv::MakeLargestInverted();
    auto initializer = [&](auto drawable) {
        auto [variant, pos] = drawable;
        SkGlyph* skGlyph = variant;
        int16_t l = skGlyph->left(),
                t = skGlyph->top(),
                r = l + skGlyph->width(),
                b = t + skGlyph->height();
        SkPoint lt = SkPoint::Make(l, t) * strikeToSourceScale + pos,
                rb = SkPoint::Make(r, b) * strikeToSourceScale + pos;

        bounds.joinPossiblyEmptyRect(SkRect::MakeLTRB(lt.x(), lt.y(), rb.x(), rb.y()));
        return VertexData{pos, {l, t, r, b}};
    };

    SkSpan<VertexData> vertexData = alloc->makePODArray<VertexData>(drawables, initializer);

    return alloc->makeUnique<TransformedMaskSubRunNoCache>(
            format, strikeToSourceScale, bounds, vertexData,
            GlyphVector::Make(std::move(strike), drawables.get<0>(), alloc));
}

std::tuple<const GrClip*, GrOp::Owner>
TransformedMaskSubRunNoCache::makeAtlasTextOp(const GrClip* clip,
                                              const SkMatrixProvider& viewMatrix,
                                              SkPoint drawOrigin,
                                              const SkPaint& paint,
                                              skgpu::v1::SurfaceDrawContext* sdc,
                                              GrAtlasSubRunOwner subRunOwner) const {
    SkASSERT(this->glyphCount() != 0);

    const SkMatrix& drawMatrix = viewMatrix.localToDevice();

    GrPaint grPaint;
    SkPMColor4f drawingColor = calculate_colors(sdc, paint, viewMatrix, fMaskFormat, &grPaint);

    // We can clip geometrically using clipRect and ignore clip if we're not using SDFs or
    // transformed glyphs, and we have an axis-aligned rectangular non-AA clip.
    auto geometry = new ((void*)fGeom) AtlasTextOp::Geometry{
            *this,
            drawMatrix,
            drawOrigin,
            SkIRect::MakeEmpty(),
            nullptr,
            std::move(subRunOwner),
            drawingColor
    };

    GrRecordingContext* rContext = sdc->recordingContext();
    GrOp::Owner op = GrOp::Make<AtlasTextOp>(rContext,
                                             op_mask_type(fMaskFormat),
                                             true,
                                             this->glyphCount(),
                                             this->deviceRect(drawMatrix, drawOrigin),
                                             geometry,
                                             std::move(grPaint));
    return {clip, std::move(op)};
}

void TransformedMaskSubRunNoCache::testingOnly_packedGlyphIDToGrGlyph(GrStrikeCache *cache) {
    fGlyphs.packedGlyphIDToGrGlyph(cache);
}

std::tuple<bool, int> TransformedMaskSubRunNoCache::regenerateAtlas(
        int begin, int end, GrMeshDrawTarget* target) const {
    return fGlyphs.regenerateAtlas(begin, end, fMaskFormat, 1, target, true);
}

void TransformedMaskSubRunNoCache::fillVertexData(
        void* vertexDst, int offset, int count,
        GrColor color,
        const SkMatrix& drawMatrix, SkPoint drawOrigin,
        SkIRect clip) const {
    constexpr SkScalar kDstPadding = 0.f;
    const SkMatrix positionMatrix = position_matrix(drawMatrix, drawOrigin);

    auto quadData = [&](auto dst) {
        return SkMakeZip(dst,
                         fGlyphs.glyphs().subspan(offset, count),
                         fVertexData.subspan(offset, count));
    };

    if (!positionMatrix.hasPerspective()) {
        if (fMaskFormat == GrMaskFormat::kARGB_GrMaskFormat) {
            using Quad = ARGB2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
            fill_transformed_vertices_2D(
                    quadData((Quad*) vertexDst),
                    kDstPadding,
                    fStrikeToSourceScale,
                    color,
                    positionMatrix);
        } else {
            using Quad = Mask2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
            fill_transformed_vertices_2D(
                    quadData((Quad*) vertexDst),
                    kDstPadding,
                    fStrikeToSourceScale,
                    color,
                    positionMatrix);
        }
    } else {
        if (fMaskFormat == GrMaskFormat::kARGB_GrMaskFormat) {
            using Quad = ARGB3DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
            fill_transformed_vertices_3D(
                    quadData((Quad*) vertexDst),
                    kDstPadding,
                    fStrikeToSourceScale,
                    color,
                    positionMatrix);
        } else {
            using Quad = Mask3DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
            fill_transformed_vertices_3D(
                    quadData((Quad*) vertexDst),
                    kDstPadding,
                    fStrikeToSourceScale,
                    color,
                    positionMatrix);
        }
    }
}

size_t TransformedMaskSubRunNoCache::vertexStride(const SkMatrix& drawMatrix) const {
    switch (fMaskFormat) {
        case kA8_GrMaskFormat:
            return drawMatrix.hasPerspective() ? sizeof(Mask3DVertex) : sizeof(Mask2DVertex);
        case kARGB_GrMaskFormat:
            return drawMatrix.hasPerspective() ? sizeof(ARGB3DVertex) : sizeof(ARGB2DVertex);
        default:
            SkASSERT(!drawMatrix.hasPerspective());
            return sizeof(Mask2DVertex);
    }
    SkUNREACHABLE;
}

int TransformedMaskSubRunNoCache::glyphCount() const {
    return SkCount(fVertexData);
}

SkRect TransformedMaskSubRunNoCache::deviceRect(
        const SkMatrix& drawMatrix, SkPoint drawOrigin) const {
    SkRect outBounds = fVertexBounds;
    outBounds.offset(drawOrigin);
    return drawMatrix.mapRect(outBounds);
}

// -- SDFTSubRunNoCache ----------------------------------------------------------------------------
class SDFTSubRunNoCache final : public GrAtlasSubRun {
public:
    struct VertexData {
        const SkPoint pos;
        // The rectangle of the glyphs in strike space.
        GrIRect16 rect;
    };

    SDFTSubRunNoCache(GrMaskFormat format,
                      SkScalar strikeToSourceScale,
                      SkRect vertexBounds,
                      SkSpan<const VertexData> vertexData,
                      GlyphVector&& glyphs,
                      bool useLCDText,
                      bool antiAliased);

    static GrAtlasSubRunOwner Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                   const SkFont& runFont,
                                   sk_sp<SkStrike>&& strike,
                                   SkScalar strikeToSourceScale,
                                   GrSubRunAllocator* alloc);

    std::tuple<const GrClip*, GrOp::Owner>
    makeAtlasTextOp(const GrClip*,
                    const SkMatrixProvider& viewMatrix,
                    SkPoint drawOrigin,
                    const SkPaint&,
                    skgpu::v1::SurfaceDrawContext*,
                    GrAtlasSubRunOwner) const override;

    void testingOnly_packedGlyphIDToGrGlyph(GrStrikeCache *cache) override;

    std::tuple<bool, int> regenerateAtlas(int begin, int end, GrMeshDrawTarget*) const override;

    void fillVertexData(
            void* vertexDst, int offset, int count,
            GrColor color,
            const SkMatrix& drawMatrix, SkPoint drawOrigin,
            SkIRect clip) const override;

    size_t vertexStride(const SkMatrix& drawMatrix) const override;
    int glyphCount() const override;

private:
    // The rectangle that surrounds all the glyph bounding boxes in device space.
    SkRect deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const;

    const GrMaskFormat fMaskFormat;

    // The scale factor between the strike size, and the source size.
    const SkScalar fStrikeToSourceScale;

    // The bounds in source space. The bounds are the joined rectangles of all the glyphs.
    const SkRect fVertexBounds;
    const SkSpan<const VertexData> fVertexData;

    // Space for geometry
    alignas(alignof(AtlasTextOp::Geometry)) char fGeom[sizeof(AtlasTextOp::Geometry)];

    // The regenerateAtlas method mutates fGlyphs. It should be called from onPrepare which must
    // be single threaded.
    mutable GlyphVector fGlyphs;

    const bool fUseLCDText;
    const bool fAntiAliased;
};

SDFTSubRunNoCache::SDFTSubRunNoCache(GrMaskFormat format,
                                     SkScalar strikeToSourceScale,
                                     SkRect vertexBounds,
                                     SkSpan<const VertexData> vertexData,
                                     GlyphVector&& glyphs,
                                     bool useLCDText,
                                     bool antiAliased)
        : fMaskFormat{format}
        , fStrikeToSourceScale{strikeToSourceScale}
        , fVertexBounds{vertexBounds}
        , fVertexData{vertexData}
        , fGlyphs{std::move(glyphs)}
        , fUseLCDText{useLCDText}
        , fAntiAliased{antiAliased} {}


GrAtlasSubRunOwner SDFTSubRunNoCache::Make(
        const SkZip<SkGlyphVariant, SkPoint>& drawables,
        const SkFont& runFont,
        sk_sp<SkStrike>&& strike,
        SkScalar strikeToSourceScale,
        GrSubRunAllocator* alloc) {

    SkRect bounds = SkRectPriv::MakeLargestInverted();
    auto initializer = [&](auto drawable) {
        auto [variant, pos] = drawable;
        SkGlyph* skGlyph = variant;
        int16_t l = skGlyph->left(),
                t = skGlyph->top(),
                r = l + skGlyph->width(),
                b = t + skGlyph->height();
        SkPoint lt = SkPoint::Make(l, t) * strikeToSourceScale + pos,
                rb = SkPoint::Make(r, b) * strikeToSourceScale + pos;

        bounds.joinPossiblyEmptyRect(SkRect::MakeLTRB(lt.x(), lt.y(), rb.x(), rb.y()));
        return VertexData{pos, {l, t, r, b}};
    };

    SkSpan<VertexData> vertexData = alloc->makePODArray<VertexData>(drawables, initializer);

    return alloc->makeUnique<SDFTSubRunNoCache>(
            kA8_GrMaskFormat,
            strikeToSourceScale,
            bounds,
            vertexData,
            GlyphVector::Make(std::move(strike), drawables.get<0>(), alloc),
            runFont.getEdging() == SkFont::Edging::kSubpixelAntiAlias,
            has_some_antialiasing(runFont));
}

std::tuple<const GrClip*, GrOp::Owner>
SDFTSubRunNoCache::makeAtlasTextOp(const GrClip* clip,
                                   const SkMatrixProvider& viewMatrix,
                                   SkPoint drawOrigin,
                                   const SkPaint& paint,
                                   skgpu::v1::SurfaceDrawContext* sdc,
                                   GrAtlasSubRunOwner subRunOwner) const {
    SkASSERT(this->glyphCount() != 0);

    const SkMatrix& drawMatrix = viewMatrix.localToDevice();

    GrPaint grPaint;
    SkPMColor4f drawingColor = calculate_colors(sdc, paint, viewMatrix, fMaskFormat, &grPaint);

    auto [maskType, DFGPFlags, useGammaCorrectDistanceTable] =
    calculate_sdf_parameters(*sdc, drawMatrix, fUseLCDText, fAntiAliased);

    auto geometry = new ((void*)fGeom) AtlasTextOp::Geometry {
            *this,
            drawMatrix,
            drawOrigin,
            SkIRect::MakeEmpty(),
            nullptr,
            std::move(subRunOwner),
            drawingColor
    };

    GrRecordingContext* rContext = sdc->recordingContext();
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

void SDFTSubRunNoCache::testingOnly_packedGlyphIDToGrGlyph(GrStrikeCache *cache) {
    fGlyphs.packedGlyphIDToGrGlyph(cache);
}

std::tuple<bool, int> SDFTSubRunNoCache::regenerateAtlas(
        int begin, int end, GrMeshDrawTarget *target) const {

    return fGlyphs.regenerateAtlas(begin, end, fMaskFormat, SK_DistanceFieldInset, target);
}

size_t SDFTSubRunNoCache::vertexStride(const SkMatrix& drawMatrix) const {
    return sizeof(Mask2DVertex);
}

void SDFTSubRunNoCache::fillVertexData(
        void *vertexDst, int offset, int count,
        GrColor color,
        const SkMatrix& drawMatrix, SkPoint drawOrigin,
        SkIRect clip) const {
    using Quad = Mask2DVertex[4];

    const SkMatrix positionMatrix = position_matrix(drawMatrix, drawOrigin);

    SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
    fill_transformed_vertices_2D(
            SkMakeZip((Quad*)vertexDst,
                      fGlyphs.glyphs().subspan(offset, count),
                      fVertexData.subspan(offset, count)),
            SK_DistanceFieldInset,
            fStrikeToSourceScale,
            color,
            positionMatrix);
}

int SDFTSubRunNoCache::glyphCount() const {
    return SkCount(fVertexData);
}

SkRect SDFTSubRunNoCache::deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const {
    SkRect outBounds = fVertexBounds;
    outBounds.offset(drawOrigin);
    return drawMatrix.mapRect(outBounds);
}
}  // namespace

GrSubRunNoCachePainter::GrSubRunNoCachePainter(skgpu::v1::SurfaceDrawContext* sdc,
                                               GrSubRunAllocator* alloc,
                                               const GrClip* clip,
                                               const SkMatrixProvider& viewMatrix,
                                               const SkGlyphRunList& glyphRunList,
                                               const SkPaint& paint)
            : fSDC{sdc}
            , fAlloc{alloc}
            , fClip{clip}
            , fViewMatrix{viewMatrix}
            , fGlyphRunList{glyphRunList}
            , fPaint {paint} {}

void GrSubRunNoCachePainter::processDeviceMasks(
        const SkZip<SkGlyphVariant, SkPoint>& drawables, sk_sp<SkStrike>&& strike) {
    auto addGlyphsWithSameFormat = [&] (const SkZip<SkGlyphVariant, SkPoint>& drawable,
                                        GrMaskFormat format,
                                        sk_sp<SkStrike>&& runStrike) {
        const bool padAtlas =
                fSDC->recordingContext()->priv().options().fSupportBilerpFromGlyphAtlas;
        this->draw(DirectMaskSubRunNoCache::Make(
                drawable, std::move(runStrike), format, padAtlas, fAlloc));
    };

    add_multi_mask_format(addGlyphsWithSameFormat, drawables, std::move(strike));
}

void GrSubRunNoCachePainter::processSourceMasks(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                                sk_sp<SkStrike>&& strike,
                                                SkScalar strikeToSourceScale) {
    auto addGlyphsWithSameFormat = [&] (const SkZip<SkGlyphVariant, SkPoint>& drawable,
                                        GrMaskFormat format,
                                        sk_sp<SkStrike>&& runStrike) {
        this->draw(TransformedMaskSubRunNoCache::Make(
                drawable, std::move(runStrike), strikeToSourceScale, format, fAlloc));
    };
    add_multi_mask_format(addGlyphsWithSameFormat, drawables, std::move(strike));
}

void GrSubRunNoCachePainter::processSourcePaths(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                                const SkFont& runFont,
                                                SkScalar strikeToSourceScale) {
    SkASSERT(!drawables.empty());
    SkPoint drawOrigin = fGlyphRunList.origin();
    const SkPaint& drawPaint = fPaint;
    SkPaint runPaint{drawPaint};
    runPaint.setAntiAlias(has_some_antialiasing(runFont));
    // If there are shaders, blurs or styles, the path must be scaled into source
    // space independently of the CTM. This allows the CTM to be correct for the
    // different effects.
    GrStyle style(runPaint);

    bool needsExactCTM = runPaint.getShader()
                         || style.applies()
                         || runPaint.getMaskFilter();

    // Calculate the matrix that maps the path glyphs from their size in the strike to
    // the graphics source space.
    SkMatrix strikeToSource = SkMatrix::Scale(strikeToSourceScale, strikeToSourceScale);
    strikeToSource.postTranslate(drawOrigin.x(), drawOrigin.y());
    if (!needsExactCTM) {
        for (auto [variant, pos] : drawables) {
            const SkPath& path = *variant.path();
            SkMatrix pathMatrix = strikeToSource;
            pathMatrix.postTranslate(pos.x(), pos.y());
            SkPreConcatMatrixProvider strikeToDevice(fViewMatrix, pathMatrix);

            GrStyledShape shape(path, drawPaint);
            GrBlurUtils::drawShapeWithMaskFilter(
                    fSDC->recordingContext(), fSDC, fClip, runPaint,
                    strikeToDevice, shape);
        }
    } else {
        // Transform the path to device space because the deviceMatrix must be unchanged to
        // draw effect, filter or shader paths.
        for (auto [variant, pos] : drawables) {
            const SkPath& path = *variant.path();
            // Transform the glyph to source space.
            SkMatrix pathMatrix = strikeToSource;
            pathMatrix.postTranslate(pos.x(), pos.y());

            SkPath sourceSpacePath;
            path.transform(pathMatrix, &sourceSpacePath);
            sourceSpacePath.setIsVolatile(true);
            GrStyledShape shape(sourceSpacePath, drawPaint);
            GrBlurUtils::drawShapeWithMaskFilter(
                    fSDC->recordingContext(), fSDC, fClip, runPaint, fViewMatrix, shape);
        }
    }
}

void GrSubRunNoCachePainter::processSourceSDFT(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                               sk_sp<SkStrike>&& strike,
                                               SkScalar strikeToSourceScale,
                                               const SkFont& runFont,
                                               SkScalar minScale, SkScalar maxScale) {
    if (drawables.empty()) {
        return;
    }
    this->draw(SDFTSubRunNoCache::Make(
            drawables, runFont, std::move(strike), strikeToSourceScale, fAlloc));
}

void GrSubRunNoCachePainter::draw(GrAtlasSubRunOwner subRun) {
    if (subRun == nullptr) {
        return;
    }
    GrAtlasSubRun* subRunPtr = subRun.get();
    auto [drawingClip, op] = subRunPtr->makeAtlasTextOp(
            fClip, fViewMatrix, fGlyphRunList.origin(), fPaint, fSDC, std::move(subRun));
    if (op != nullptr) {
        fSDC->addDrawOp(drawingClip, std::move(op));
    }
}

namespace {
// ----------------------------- Begin slug implementation -----------------------------------------
class SlugSubRun;
using SlugSubRunOwner = std::unique_ptr<SlugSubRun, GrSubRunAllocator::Destroyer>;
class SlugSubRun : public GrDrawableSubRun {
private:
    friend struct SlugSubRunList;
    SlugSubRunOwner fNext;
};

class SlugAtlasSubRun : public SlugSubRun, public GrAtlasSubRun { };

struct SlugSubRunList {
    class Iterator {
    public:
        using value_type = SlugSubRun;
        using difference_type = ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::input_iterator_tag;
        Iterator(SlugSubRun* subRun) : fPtr{subRun} { }
        Iterator& operator++() { fPtr = fPtr->fNext.get(); return *this; }
        Iterator operator++(int) { Iterator tmp(*this); operator++(); return tmp; }
        bool operator==(const Iterator& rhs) const { return fPtr == rhs.fPtr; }
        bool operator!=(const Iterator& rhs) const { return fPtr != rhs.fPtr; }
        reference operator*() { return *fPtr; }

    private:
        SlugSubRun* fPtr;
    };

    void append(SlugSubRunOwner subRun) {
        SlugSubRunOwner* newTail = &subRun->fNext;
        *fTail = std::move(subRun);
        fTail = newTail;
    }
    bool isEmpty() const { return fHead == nullptr; }
    Iterator begin() { return Iterator{ fHead.get()}; }
    Iterator end() { return Iterator{nullptr}; }
    Iterator begin() const { return Iterator{ fHead.get()}; }
    Iterator end() const { return Iterator{nullptr}; }
    SlugSubRun& front() const {return *fHead; }

    SlugSubRunOwner fHead{nullptr};
    SlugSubRunOwner* fTail{&fHead};
};

// -- Slug -----------------------------------------------------------------------------------------
class Slug final : public GrSlug, public SkGlyphRunPainterInterface {
public:
    static sk_sp<Slug> Make(const SkMatrixProvider& viewMatrix,
                            const SkGlyphRunList& glyphRunList,
                            const SkPaint& paint,
                            const GrSDFTControl& control,
                            SkGlyphRunListPainter* painter);
    Slug(SkRect sourceBounds,
         const SkPaint& paint,
         const SkMatrix& positionMatrix,
         SkPoint origin,
         int allocSize);
    ~Slug() override = default;

    void surfaceDraw(const GrClip* clip,
                     const SkMatrixProvider& viewMatrix,
                     skgpu::v1::SurfaceDrawContext* sdc);

    SkRect sourceBounds() const override { return fSourceBounds; }
    const SkPaint& paint() const override { return fPaint; }

    // SkGlyphRunPainterInterface
    void processDeviceMasks(
            const SkZip<SkGlyphVariant, SkPoint>& drawables, sk_sp<SkStrike>&& strike) override;
    void processSourceMasks(
            const SkZip<SkGlyphVariant, SkPoint>& drawables, sk_sp<SkStrike>&& strike,
            SkScalar strikeToSourceScale) override;
    void processSourcePaths(
            const SkZip<SkGlyphVariant, SkPoint>& drawables, const SkFont& runFont,
            SkScalar strikeToSourceScale) override;
    void processSourceSDFT(
            const SkZip<SkGlyphVariant, SkPoint>& drawables, sk_sp<SkStrike>&& strike,
            SkScalar strikeToSourceScale, const SkFont& runFont, SkScalar minScale,
            SkScalar maxScale) override;

    const SkMatrix& initialPositionMatrix() const { return fInitialPositionMatrix; }
    SkPoint origin() const { return fOrigin; }

    // Change memory management to handle the data after Slug, but in the same allocation
    // of memory. Only allow placement new.
    void operator delete(void* p) { ::operator delete(p); }
    void* operator new(size_t) { SK_ABORT("All slugs are created by placement new."); }
    void* operator new(size_t, void* p) { return p; }

private:
    const SkRect fSourceBounds;
    const SkPaint fPaint;
    const SkMatrix fInitialPositionMatrix;
    const SkPoint fOrigin;
    GrSubRunAllocator fAlloc;
    SlugSubRunList fSubRuns;
};

Slug::Slug(SkRect sourceBounds,
           const SkPaint& paint,
           const SkMatrix& positionMatrix,
           SkPoint origin,
           int allocSize)
           : fSourceBounds{sourceBounds}
           , fPaint{paint}
           , fInitialPositionMatrix{positionMatrix}
           , fOrigin{origin}
           , fAlloc {SkTAddOffset<char>(this, sizeof(Slug)), allocSize, allocSize/2} { }

void Slug::surfaceDraw(const GrClip* clip, const SkMatrixProvider& viewMatrix,
                       skgpu::v1::SurfaceDrawContext* sdc) {
    for (const SlugSubRun& subRun : fSubRuns) {
        subRun.draw(clip, viewMatrix, fOrigin, fPaint, sdc);
    }
}

// -- DirectMaskSubRunSlug -------------------------------------------------------------------------
class DirectMaskSubRunSlug final : public SlugAtlasSubRun {
public:
    using DevicePosition = skvx::Vec<2, int16_t>;

    DirectMaskSubRunSlug(Slug* slug,
                         GrMaskFormat format,
                         SkGlyphRect deviceBounds,
                         SkSpan<const DevicePosition> devicePositions,
                         GlyphVector&& glyphs);

    static SlugSubRunOwner Make(Slug* slug,
                                const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                sk_sp<SkStrike>&& strike,
                                GrMaskFormat format,
                                GrSubRunAllocator* alloc);

    void draw(const GrClip* clip,
              const SkMatrixProvider& viewMatrix,
              SkPoint drawOrigin,
              const SkPaint& paint,
              skgpu::v1::SurfaceDrawContext* sdc) const override {
        auto [drawingClip, op] = this->makeAtlasTextOp(
                clip, viewMatrix, drawOrigin, paint, sdc, nullptr);
        if (op != nullptr) {
            sdc->addDrawOp(drawingClip, std::move(op));
        }
    }

    size_t vertexStride(const SkMatrix& drawMatrix) const override;

    int glyphCount() const override;

    std::tuple<const GrClip*, GrOp::Owner>
    makeAtlasTextOp(const GrClip*,
                    const SkMatrixProvider& viewMatrix,
                    SkPoint,
                    const SkPaint&,
                    skgpu::v1::SurfaceDrawContext*,
                    GrAtlasSubRunOwner) const override;

    void testingOnly_packedGlyphIDToGrGlyph(GrStrikeCache *cache) override;

    std::tuple<bool, int>
    regenerateAtlas(int begin, int end, GrMeshDrawTarget*) const override;

    void fillVertexData(void* vertexDst, int offset, int count,
                        GrColor color,
                        const SkMatrix& drawMatrix, SkPoint drawOrigin,
                        SkIRect clip) const override;

private:
    // Return true if the positionMatrix represents an integer translation. Return the device
    // bounding box of all the glyphs. If the bounding box is empty, then something went singular
    // and this operation should be dropped.
    std::tuple<bool, SkRect> deviceRectAndCheckTransform(const SkMatrix& positionMatrix) const;

    Slug* const fSlug;
    const GrMaskFormat fMaskFormat;

    // The vertex bounds in device space. The bounds are the joined rectangles of all the glyphs.
    const SkGlyphRect fGlyphDeviceBounds;
    const SkSpan<const DevicePosition> fLeftTopDevicePos;

    // The regenerateAtlas method mutates fGlyphs. It should be called from onPrepare which must
    // be single threaded.
    mutable GlyphVector fGlyphs;
};

DirectMaskSubRunSlug::DirectMaskSubRunSlug(Slug* slug,
                                           GrMaskFormat format,
                                           SkGlyphRect deviceBounds,
                                           SkSpan<const DevicePosition> devicePositions,
                                           GlyphVector&& glyphs)
        : fSlug{slug}
        , fMaskFormat{format}
        , fGlyphDeviceBounds{deviceBounds}
        , fLeftTopDevicePos{devicePositions}
        , fGlyphs{std::move(glyphs)} { }

SlugSubRunOwner DirectMaskSubRunSlug::Make(Slug* slug,
                                           const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                           sk_sp<SkStrike>&& strike,
                                           GrMaskFormat format,
                                           GrSubRunAllocator* alloc) {
    DevicePosition* glyphLeftTop = alloc->makePODArray<DevicePosition>(drawables.size());

    GlyphVector::Variant* glyphIDs = static_cast<GlyphVector::Variant*>(
            alloc->alignedBytes(drawables.size() * sizeof(GlyphVector::Variant),
                                alignof(GlyphVector::Variant)));

    // Because this is the direct case, the maximum width or height is the size that fits in the
    // atlas. This boundary is checked below to ensure that the call to SkGlyphRect below will
    // not overflow.
    constexpr SkScalar kMaxPos =
            std::numeric_limits<int16_t>::max() - SkStrikeCommon::kSkSideTooBigForAtlas;
    SkGlyphRect runBounds = skglyph::empty_rect();
    size_t goodPosCount = 0;
    for (auto [variant, pos] : drawables) {
        auto [x, y] = pos;
        // Ensure that the .offset() call below does not overflow. And, at this point none of the
        // rectangles are empty because they were culled before the run was created. Basically,
        // cull all the glyphs that can't appear on the screen.
        if (-kMaxPos < x && x < kMaxPos && -kMaxPos  < y && y < kMaxPos) {
            const SkGlyph* const skGlyph = variant;
            const SkGlyphRect deviceBounds =
                    skGlyph->glyphRect().offset(SkScalarRoundToInt(x), SkScalarRoundToInt(y));
            runBounds = skglyph::rect_union(runBounds, deviceBounds);
            glyphLeftTop[goodPosCount] = deviceBounds.topLeft();
            glyphIDs[goodPosCount].packedGlyphID = skGlyph->getPackedID();
            goodPosCount += 1;
        }
    }

    // Wow! no glyphs are in bounds and had non-empty bounds.
    if (goodPosCount == 0) {
        return nullptr;
    }

    SkSpan<const DevicePosition> leftTop{glyphLeftTop, goodPosCount};
    return alloc->makeUnique<DirectMaskSubRunSlug>(
            slug, format, runBounds, leftTop,
            GlyphVector{std::move(strike), {glyphIDs, goodPosCount}});
}

size_t DirectMaskSubRunSlug::vertexStride(const SkMatrix& positionMatrix) const {
    if (!positionMatrix.hasPerspective()) {
        if (fMaskFormat != kARGB_GrMaskFormat) {
            return sizeof(Mask2DVertex);
        } else {
            return sizeof(ARGB2DVertex);
        }
    } else {
        if (fMaskFormat != kARGB_GrMaskFormat) {
            return sizeof(Mask3DVertex);
        } else {
            return sizeof(ARGB3DVertex);
        }
    }
}

int DirectMaskSubRunSlug::glyphCount() const {
    return SkCount(fGlyphs.glyphs());
}

std::tuple<const GrClip*, GrOp::Owner>
DirectMaskSubRunSlug::makeAtlasTextOp(const GrClip* clip,
                                      const SkMatrixProvider& viewMatrix,
                                      SkPoint drawOrigin,
                                      const SkPaint& paint,
                                      skgpu::v1::SurfaceDrawContext* sdc,
                                      GrAtlasSubRunOwner subRunOwner) const {
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
                                                       sk_ref_sp<GrSlug>(fSlug),
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

void DirectMaskSubRunSlug::testingOnly_packedGlyphIDToGrGlyph(GrStrikeCache *cache) {
    fGlyphs.packedGlyphIDToGrGlyph(cache);
}

std::tuple<bool, int>
DirectMaskSubRunSlug::regenerateAtlas(int begin, int end, GrMeshDrawTarget* target) const {
    return fGlyphs.regenerateAtlas(begin, end, fMaskFormat, 1, target, true);
}

// The 99% case. No clip. Non-color only.
void direct_2D3(SkZip<Mask2DVertex[4],
        const GrGlyph*,
        const DirectMaskSubRunNoCache::DevicePosition> quadData,
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

template<typename Quad, typename VertexData>
void transformed_direct_2D(SkZip<Quad, const GrGlyph*, const VertexData> quadData,
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
void transformed_direct_3D(SkZip<Quad, const GrGlyph*, const VertexData> quadData,
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

void DirectMaskSubRunSlug::fillVertexData(void* vertexDst, int offset, int count,
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
            can_use_direct(fSlug->initialPositionMatrix(), positionMatrix);

    if (noTransformNeeded) {
        if (clip.isEmpty()) {
            if (fMaskFormat != kARGB_GrMaskFormat) {
                using Quad = Mask2DVertex[4];
                SkASSERT(sizeof(Quad) == this->vertexStride(SkMatrix::I()) * kVerticesPerGlyph);
                direct_2D3(quadData((Quad*)vertexDst), color, originOffset);
            } else {
                using Quad = ARGB2DVertex[4];
                SkASSERT(sizeof(Quad) == this->vertexStride(SkMatrix::I()) * kVerticesPerGlyph);
                generalized_direct_2D(quadData((Quad*)vertexDst), color, originOffset);
            }
        } else {
            if (fMaskFormat != kARGB_GrMaskFormat) {
                using Quad = Mask2DVertex[4];
                SkASSERT(sizeof(Quad) == this->vertexStride(SkMatrix::I()) * kVerticesPerGlyph);
                generalized_direct_2D(quadData((Quad*)vertexDst), color, originOffset, &clip);
            } else {
                using Quad = ARGB2DVertex[4];
                SkASSERT(sizeof(Quad) == this->vertexStride(SkMatrix::I()) * kVerticesPerGlyph);
                generalized_direct_2D(quadData((Quad*)vertexDst), color, originOffset, &clip);
            }
        }
    } else if (SkMatrix inverse; fSlug->initialPositionMatrix().invert(&inverse)) {
        SkMatrix viewDifference = SkMatrix::Concat(positionMatrix, inverse);
        if (!viewDifference.hasPerspective()) {
            if (fMaskFormat != kARGB_GrMaskFormat) {
                using Quad = Mask2DVertex[4];
                SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
                transformed_direct_2D(quadData((Quad*)vertexDst), color, viewDifference);
            } else {
                using Quad = ARGB2DVertex[4];
                SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
                transformed_direct_2D(quadData((Quad*)vertexDst), color, viewDifference);
            }
        } else {
            if (fMaskFormat != kARGB_GrMaskFormat) {
                using Quad = Mask3DVertex[4];
                SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
                transformed_direct_3D(quadData((Quad*)vertexDst), color, viewDifference);
            } else {
                using Quad = ARGB3DVertex[4];
                SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
                transformed_direct_3D(quadData((Quad*)vertexDst), color, viewDifference);
            }
        }
    }
}

// true if only need to translate by integer amount, device rect.
std::tuple<bool, SkRect>
DirectMaskSubRunSlug::deviceRectAndCheckTransform(const SkMatrix& positionMatrix) const {
    SkPoint offset = positionMatrix.mapOrigin() - fSlug->initialPositionMatrix().mapOrigin();
    if (positionMatrix.isTranslate() && SkScalarIsInt(offset.x()) && SkScalarIsInt(offset.y())) {
        // Handle the integer offset case.
        // The offset should be integer, but make sure.
        SkIVector iOffset = {SkScalarRoundToInt(offset.x()), SkScalarRoundToInt(offset.y())};

        SkIRect outBounds = fGlyphDeviceBounds.iRect();
        return {true, SkRect::Make(outBounds.makeOffset(iOffset))};
    } else if (SkMatrix inverse; fSlug->initialPositionMatrix().invert(&inverse)) {
        SkMatrix viewDifference = SkMatrix::Concat(positionMatrix, inverse);
        return {false, viewDifference.mapRect(fGlyphDeviceBounds.rect())};
    }

    // initialPositionMatrix is singular. Do nothing.
    return {false, SkRect::MakeEmpty()};
}

void Slug::processDeviceMasks(
        const SkZip<SkGlyphVariant, SkPoint>& drawables, sk_sp<SkStrike>&& strike) {
    auto addGlyphsWithSameFormat = [&] (const SkZip<SkGlyphVariant, SkPoint>& drawable,
                                        GrMaskFormat format,
                                        sk_sp<SkStrike>&& runStrike) {
        SlugSubRunOwner subRun = DirectMaskSubRunSlug::Make(
                this, drawable, std::move(runStrike), format, &fAlloc);
        if (subRun != nullptr) {
            fSubRuns.append(std::move(subRun));
        }
    };

    add_multi_mask_format(addGlyphsWithSameFormat, drawables, std::move(strike));
}

sk_sp<Slug> Slug::Make(const SkMatrixProvider& viewMatrix,
                       const SkGlyphRunList& glyphRunList,
                       const SkPaint& paint,
                       const GrSDFTControl& control,
                       SkGlyphRunListPainter* painter) {
    // The difference in alignment from the per-glyph data to the SubRun;
    constexpr size_t alignDiff =
            alignof(DirectMaskSubRun) - alignof(DirectMaskSubRun::DevicePosition);
    constexpr size_t vertexDataToSubRunPadding = alignDiff > 0 ? alignDiff : 0;
    size_t totalGlyphCount = glyphRunList.totalGlyphCount();
    // The bytesNeededForSubRun is optimized for DirectMaskSubRun which is by far the most
    // common case.
    size_t bytesNeededForSubRun = GrBagOfBytes::PlatformMinimumSizeWithOverhead(
            totalGlyphCount * sizeof(DirectMaskSubRunSlug::DevicePosition)
            + GlyphVector::GlyphVectorSize(totalGlyphCount)
            + glyphRunList.runCount() * (sizeof(DirectMaskSubRunSlug) + vertexDataToSubRunPadding),
            alignof(Slug));

    size_t allocationSize = sizeof(GrTextBlob) + bytesNeededForSubRun;

    void* allocation = ::operator new (allocationSize);

    const SkMatrix positionMatrix =
            position_matrix(viewMatrix.localToDevice(), glyphRunList.origin());

    sk_sp<Slug> slug{new (allocation)
                             Slug(glyphRunList.sourceBounds(),
                                  paint,
                                  positionMatrix,
                                  glyphRunList.origin(),
                                  bytesNeededForSubRun)};

    const uint64_t uniqueID = glyphRunList.uniqueID();
    for (auto& glyphRun : glyphRunList) {
        painter->processGlyphRun(glyphRun,
                                 positionMatrix,
                                 paint,
                                 control,
                                 slug.get(),
                                 "Slug",
                                 uniqueID);
    }

    return slug;
}

// -- PathSubRunSlug -------------------------------------------------------------------------------
class PathSubRunSlug final : public SlugSubRun {
public:
    PathSubRunSlug(bool isAntiAliased,
                   SkScalar strikeToSourceScale,
                   SkSpan<PathGlyph> paths,
                   std::unique_ptr<PathGlyph[], GrSubRunAllocator::ArrayDestroyer> pathData);

    void draw(const GrClip* clip,
              const SkMatrixProvider& viewMatrix,
              SkPoint drawOrigin,
              const SkPaint& paint,
              skgpu::v1::SurfaceDrawContext* sdc) const override;

    static SlugSubRunOwner Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                bool isAntiAliased,
                                SkScalar strikeToSourceScale,
                                GrSubRunAllocator* alloc);

private:

    const bool fIsAntiAliased;
    const SkScalar fStrikeToSourceScale;
    const SkSpan<const PathGlyph> fPaths;
    const std::unique_ptr<PathGlyph[], GrSubRunAllocator::ArrayDestroyer> fPathData;
};

PathSubRunSlug::PathSubRunSlug(bool isAntiAliased,
                               SkScalar strikeToSourceScale,
                               SkSpan<PathGlyph> paths,
                               std::unique_ptr<PathGlyph[],
                               GrSubRunAllocator::ArrayDestroyer> pathData)
        : fIsAntiAliased{isAntiAliased}
        , fStrikeToSourceScale{strikeToSourceScale}
        , fPaths{paths}
        , fPathData{std::move(pathData)} {}

void PathSubRunSlug::draw(const GrClip* clip,
                          const SkMatrixProvider& viewMatrix,
                          SkPoint drawOrigin,
                          const SkPaint& paint,
                          skgpu::v1::SurfaceDrawContext* sdc) const {
    SkASSERT(!fPaths.empty());
    SkPaint runPaint{paint};
    runPaint.setAntiAlias(fIsAntiAliased);
    // If there are shaders, blurs or styles, the path must be scaled into source
    // space independently of the CTM. This allows the CTM to be correct for the
    // different effects.
    GrStyle style(runPaint);

    bool needsExactCTM = runPaint.getShader()
                         || style.applies()
                         || runPaint.getMaskFilter();

    // Calculate the matrix that maps the path glyphs from their size in the strike to
    // the graphics source space.
    SkMatrix strikeToSource = SkMatrix::Scale(fStrikeToSourceScale, fStrikeToSourceScale);
    strikeToSource.postTranslate(drawOrigin.x(), drawOrigin.y());
    if (!needsExactCTM) {
        for (const auto& pathPos : fPaths) {
            const SkPath& path = pathPos.fPath;
            const SkPoint pos = pathPos.fOrigin;  // Transform the glyph to source space.
            SkMatrix pathMatrix = strikeToSource;
            pathMatrix.postTranslate(pos.x(), pos.y());
            SkPreConcatMatrixProvider strikeToDevice(viewMatrix, pathMatrix);

            GrStyledShape shape(path, paint);
            GrBlurUtils::drawShapeWithMaskFilter(
                    sdc->recordingContext(), sdc, clip, runPaint, strikeToDevice, shape);
        }
    } else {
        // Transform the path to device because the deviceMatrix must be unchanged to
        // draw effect, filter or shader paths.
        for (const auto& pathPos : fPaths) {
            const SkPath& path = pathPos.fPath;
            const SkPoint pos = pathPos.fOrigin;
            // Transform the glyph to source space.
            SkMatrix pathMatrix = strikeToSource;
            pathMatrix.postTranslate(pos.x(), pos.y());

            SkPath deviceOutline;
            path.transform(pathMatrix, &deviceOutline);
            deviceOutline.setIsVolatile(true);
            GrStyledShape shape(deviceOutline, paint);
            GrBlurUtils::drawShapeWithMaskFilter(sdc->recordingContext(), sdc, clip, runPaint,
                                                 viewMatrix, shape);
        }
    }
}

SlugSubRunOwner PathSubRunSlug::Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                     bool isAntiAliased,
                                     SkScalar strikeToSourceScale,
                                     GrSubRunAllocator* alloc) {
    auto pathData = alloc->makeUniqueArray<PathGlyph>(
            drawables.size(),
            [&](int i){
                auto [variant, pos] = drawables[i];
                return PathGlyph{*variant.path(), pos};
            });
    SkSpan<PathGlyph> paths{pathData.get(), drawables.size()};

    return alloc->makeUnique<PathSubRunSlug>(
            isAntiAliased, strikeToSourceScale, paths, std::move(pathData));
}

void Slug::processSourcePaths(const SkZip<SkGlyphVariant,
                              SkPoint>& drawables,
                              const SkFont& runFont,
                              SkScalar strikeToSourceScale) {
    fSubRuns.append(PathSubRunSlug::Make(drawables,
                                         has_some_antialiasing(runFont),
                                         strikeToSourceScale,
                                         &fAlloc));
}

// -- SDFTSubRunSlug -------------------------------------------------------------------------------
class SDFTSubRunSlug final : public SlugAtlasSubRun {
public:
    struct VertexData {
        const SkPoint pos;
        // The rectangle of the glyphs in strike space.
        GrIRect16 rect;
    };

    SDFTSubRunSlug(Slug* slug,
                   GrMaskFormat format,
                   SkScalar strikeToSource,
                   SkRect vertexBounds,
                   SkSpan<const VertexData> vertexData,
                   GlyphVector&& glyphs,
                   bool useLCDText,
                   bool antiAliased);

    static SlugSubRunOwner Make(Slug* slug,
                                const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                const SkFont& runFont,
                                sk_sp<SkStrike>&& strike,
                                SkScalar strikeToSourceScale,
                                GrSubRunAllocator* alloc);

    void draw(const GrClip* clip,
              const SkMatrixProvider& viewMatrix,
              SkPoint drawOrigin,
              const SkPaint& paint,
              skgpu::v1::SurfaceDrawContext* sdc) const override;

    std::tuple<const GrClip*, GrOp::Owner>
    makeAtlasTextOp(const GrClip* clip,
                    const SkMatrixProvider& viewMatrix,
                    SkPoint drawOrigin,
                    const SkPaint& paint,
                    skgpu::v1::SurfaceDrawContext* sdc,
                    GrAtlasSubRunOwner subRunOwner) const override;

    void testingOnly_packedGlyphIDToGrGlyph(GrStrikeCache *cache) override;

    std::tuple<bool, int> regenerateAtlas(int begin, int end, GrMeshDrawTarget*) const override;

    void fillVertexData(void* vertexDst, int offset, int count,
                        GrColor color,
                        const SkMatrix& drawMatrix, SkPoint drawOrigin,
                        SkIRect clip) const override;

    size_t vertexStride(const SkMatrix& drawMatrix) const override;
    int glyphCount() const override;

private:
    Slug* const fSlug;

    // The rectangle that surrounds all the glyph bounding boxes in device space.
    SkRect deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const;

    const GrMaskFormat fMaskFormat;

    // The scale factor between the strike size, and the source size.
    const SkScalar fStrikeToSourceScale;

    // The bounds in source space. The bounds are the joined rectangles of all the glyphs.
    const SkRect fVertexBounds;
    const SkSpan<const VertexData> fVertexData;

    // The regenerateAtlas method mutates fGlyphs. It should be called from onPrepare which must
    // be single threaded.
    mutable GlyphVector fGlyphs;

    const bool fUseLCDText;
    const bool fAntiAliased;
};

SDFTSubRunSlug::SDFTSubRunSlug(Slug* slug,
                               GrMaskFormat format,
                               SkScalar strikeToSource,
                               SkRect vertexBounds,
                               SkSpan<const VertexData> vertexData,
                               GlyphVector&& glyphs,
                               bool useLCDText,
                               bool antiAliased)
        : fSlug{slug}
        , fMaskFormat{format}
        , fStrikeToSourceScale{strikeToSource}
        , fVertexBounds{vertexBounds}
        , fVertexData{vertexData}
        , fGlyphs{std::move(glyphs)}
        , fUseLCDText{useLCDText}
        , fAntiAliased{antiAliased} {}

SlugSubRunOwner SDFTSubRunSlug::Make(Slug* slug,
                                     const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                     const SkFont& runFont,
                                     sk_sp<SkStrike>&& strike,
                                     SkScalar strikeToSourceScale,
                                     GrSubRunAllocator* alloc) {
    SkRect bounds = SkRectPriv::MakeLargestInverted();
    auto mapper = [&](const auto& d) {
        auto& [variant, pos] = d;
        SkGlyph* skGlyph = variant;
        int16_t l = skGlyph->left(),
                t = skGlyph->top(),
                r = l + skGlyph->width(),
                b = t + skGlyph->height();
        SkPoint lt = SkPoint::Make(l, t) * strikeToSourceScale + pos,
                rb = SkPoint::Make(r, b) * strikeToSourceScale + pos;

        bounds.joinPossiblyEmptyRect(SkRect::MakeLTRB(lt.x(), lt.y(), rb.x(), rb.y()));
        return VertexData{pos, {l, t, r, b}};
    };

    SkSpan<VertexData> vertexData = alloc->makePODArray<VertexData>(drawables, mapper);

    return alloc->makeUnique<SDFTSubRunSlug>(
            slug,
            kA8_GrMaskFormat,
            strikeToSourceScale,
            bounds,
            vertexData,
            GlyphVector::Make(std::move(strike), drawables.get<0>(), alloc),
            runFont.getEdging() == SkFont::Edging::kSubpixelAntiAlias,
            has_some_antialiasing(runFont));
}

void SDFTSubRunSlug::draw(const GrClip* clip,
                      const SkMatrixProvider& viewMatrix,
                      SkPoint drawOrigin,
                      const SkPaint& paint,
                      skgpu::v1::SurfaceDrawContext* sdc) const {
    auto[drawingClip, op] = this->makeAtlasTextOp(
            clip, viewMatrix, drawOrigin, paint, sdc, nullptr);
    if (op != nullptr) {
        sdc->addDrawOp(drawingClip, std::move(op));
    }
}

std::tuple<const GrClip*, GrOp::Owner >
SDFTSubRunSlug::makeAtlasTextOp(const GrClip* clip,
                                const SkMatrixProvider& viewMatrix,
                                SkPoint drawOrigin,
                                const SkPaint& paint,
                                skgpu::v1::SurfaceDrawContext* sdc,
                                GrAtlasSubRunOwner subRunOwner) const {
    SkASSERT(this->glyphCount() != 0);
    SkASSERT(!viewMatrix.localToDevice().hasPerspective());

    const SkMatrix& drawMatrix = viewMatrix.localToDevice();

    GrPaint grPaint;
    SkPMColor4f drawingColor = calculate_colors(sdc, paint, viewMatrix, fMaskFormat, &grPaint);

    auto [maskType, DFGPFlags, useGammaCorrectDistanceTable] =
    calculate_sdf_parameters(*sdc, drawMatrix, fUseLCDText, fAntiAliased);

    auto geometry = AtlasTextOp::Geometry::MakeForBlob(*this,
                                                       drawMatrix,
                                                       drawOrigin,
                                                       SkIRect::MakeEmpty(),
                                                       sk_ref_sp<Slug>(fSlug),
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

void SDFTSubRunSlug::testingOnly_packedGlyphIDToGrGlyph(GrStrikeCache *cache) {
    fGlyphs.packedGlyphIDToGrGlyph(cache);
}

std::tuple<bool, int> SDFTSubRunSlug::regenerateAtlas(
        int begin, int end, GrMeshDrawTarget *target) const {

    return fGlyphs.regenerateAtlas(begin, end, fMaskFormat, SK_DistanceFieldInset, target);
}

size_t SDFTSubRunSlug::vertexStride(const SkMatrix& drawMatrix) const {
    return sizeof(Mask2DVertex);
}

void SDFTSubRunSlug::fillVertexData(void* vertexDst, int offset, int count,
                                    GrColor color,
                                    const SkMatrix& drawMatrix, SkPoint drawOrigin,
                                    SkIRect clip) const {
    using Quad = Mask2DVertex[4];

    const SkMatrix positionMatrix = position_matrix(drawMatrix, drawOrigin);

    SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
    fill_transformed_vertices_2D(
            SkMakeZip((Quad*)vertexDst,
                      fGlyphs.glyphs().subspan(offset, count),
                      fVertexData.subspan(offset, count)),
            SK_DistanceFieldInset,
            fStrikeToSourceScale,
            color,
            positionMatrix);
}

int SDFTSubRunSlug::glyphCount() const {
    return SkCount(fVertexData);
}

SkRect SDFTSubRunSlug::deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const {
    SkRect outBounds = fVertexBounds;
    outBounds.offset(drawOrigin);
    return drawMatrix.mapRect(outBounds);
}
void Slug::processSourceSDFT(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                   sk_sp<SkStrike>&& strike,
                                   SkScalar strikeToSourceScale,
                                   const SkFont& runFont,
                                   SkScalar minScale,
                                   SkScalar maxScale) {

    fSubRuns.append(SDFTSubRunSlug::Make(
            this, drawables, runFont, std::move(strike), strikeToSourceScale, &fAlloc));
}

class TransformedMaskSubRunSlug final : public SlugAtlasSubRun {
public:
    struct VertexData {
        const SkPoint pos;
        // The rectangle of the glyphs in strike space. But, for kDirectMask this also implies a
        // device space rect.
        GrIRect16 rect;
    };

    TransformedMaskSubRunSlug(Slug* slug,
                              GrMaskFormat format,
                              SkScalar strikeToSourceScale,
                              const SkRect& bounds,
                              SkSpan<const VertexData> vertexData,
                              GlyphVector&& glyphs);

    static SlugSubRunOwner Make(Slug* slug,
                                const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                sk_sp<SkStrike>&& strike,
                                SkScalar strikeToSourceScale,
                                GrMaskFormat format,
                                GrSubRunAllocator* alloc);

    void draw(const GrClip*,
              const SkMatrixProvider& viewMatrix,
              SkPoint drawOrigin,
              const SkPaint&,
              skgpu::v1::SurfaceDrawContext*) const override;

    std::tuple<const GrClip*, GrOp::Owner>
    makeAtlasTextOp(const GrClip* clip,
                    const SkMatrixProvider& viewMatrix,
                    SkPoint drawOrigin,
                    const SkPaint& paint,
                    skgpu::v1::SurfaceDrawContext* sdc,
                    GrAtlasSubRunOwner subRunOwner) const override;

    void testingOnly_packedGlyphIDToGrGlyph(GrStrikeCache *cache) override;

    std::tuple<bool, int> regenerateAtlas(int begin, int end, GrMeshDrawTarget*) const override;

    void fillVertexData(void* vertexDst, int offset, int count,
                        GrColor color,
                        const SkMatrix& drawMatrix, SkPoint drawOrigin,
                        SkIRect clip) const override;

    size_t vertexStride(const SkMatrix& drawMatrix) const override;
    int glyphCount() const override;

private:
    Slug* const fSlug;
    // The rectangle that surrounds all the glyph bounding boxes in device space.
    SkRect deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const;

    const GrMaskFormat fMaskFormat;

    // The scale factor between the strike size, and the source size.
    const SkScalar fStrikeToSourceScale;

    // The bounds in source space. The bounds are the joined rectangles of all the glyphs.
    const SkRect fVertexBounds;
    const SkSpan<const VertexData> fVertexData;

    // The regenerateAtlas method mutates fGlyphs. It should be called from onPrepare which must
    // be single threaded.
    mutable GlyphVector fGlyphs;
};

TransformedMaskSubRunSlug::TransformedMaskSubRunSlug(
        Slug* slug,
        GrMaskFormat format,
        SkScalar strikeToSourceScale,
        const SkRect& bounds,
        SkSpan<const VertexData> vertexData,
        GlyphVector&& glyphs)
            : fSlug{slug}
            , fMaskFormat{format}
            , fStrikeToSourceScale{strikeToSourceScale}
            , fVertexBounds{bounds}
            , fVertexData{vertexData}
            , fGlyphs{std::move(glyphs)} { }

SlugSubRunOwner TransformedMaskSubRunSlug::Make(Slug* slug,
                                                const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                                sk_sp<SkStrike>&& strike,
                                                SkScalar strikeToSourceScale,
                                                GrMaskFormat format,
                                                GrSubRunAllocator* alloc) {
    SkRect bounds = SkRectPriv::MakeLargestInverted();

    SkSpan<VertexData> vertexData = alloc->makePODArray<VertexData>(
            drawables,
            [&](auto e) {
                auto [variant, pos] = e;
                SkGlyph* skGlyph = variant;
                int16_t l = skGlyph->left(),
                        t = skGlyph->top(),
                        r = l + skGlyph->width(),
                        b = t + skGlyph->height();
                SkPoint lt = SkPoint::Make(l, t) * strikeToSourceScale + pos,
                        rb = SkPoint::Make(r, b) * strikeToSourceScale + pos;

                bounds.joinPossiblyEmptyRect(SkRect::MakeLTRB(lt.x(), lt.y(), rb.x(), rb.y()));
                return VertexData{pos, {l, t, r, b}};
            });

    return alloc->makeUnique<TransformedMaskSubRunSlug>(
            slug, format, strikeToSourceScale, bounds, vertexData,
            GlyphVector::Make(std::move(strike), drawables.get<0>(), alloc));
}

void TransformedMaskSubRunSlug::draw(const GrClip* clip,
                                     const SkMatrixProvider& viewMatrix,
                                     SkPoint drawOrigin,
                                     const SkPaint& paint,
                                     skgpu::v1::SurfaceDrawContext* sdc) const {
    auto[drawingClip, op] = this->makeAtlasTextOp(
            clip, viewMatrix, drawOrigin, paint, sdc, nullptr);
    if (op != nullptr) {
        sdc->addDrawOp(drawingClip, std::move(op));
    }
}

std::tuple<const GrClip*, GrOp::Owner>
TransformedMaskSubRunSlug::makeAtlasTextOp(const GrClip* clip,
                                           const SkMatrixProvider& viewMatrix,
                                           SkPoint drawOrigin,
                                           const SkPaint& paint,
                                           skgpu::v1::SurfaceDrawContext* sdc,
                                           GrAtlasSubRunOwner) const {
    SkASSERT(this->glyphCount() != 0);

    const SkMatrix& drawMatrix = viewMatrix.localToDevice();

    GrPaint grPaint;
    SkPMColor4f drawingColor = calculate_colors(sdc, paint, viewMatrix, fMaskFormat, &grPaint);

    auto geometry = AtlasTextOp::Geometry::MakeForBlob(*this,
                                                       drawMatrix,
                                                       drawOrigin,
                                                       SkIRect::MakeEmpty(),
                                                       sk_ref_sp<Slug>(fSlug),
                                                       drawingColor,
                                                       sdc->arenaAlloc());

    GrRecordingContext* const rContext = sdc->recordingContext();
    GrOp::Owner op = GrOp::Make<AtlasTextOp>(rContext,
                                             op_mask_type(fMaskFormat),
                                             true,
                                             this->glyphCount(),
                                             this->deviceRect(drawMatrix, drawOrigin),
                                             geometry,
                                             std::move(grPaint));
    return {clip, std::move(op)};
}

void TransformedMaskSubRunSlug::testingOnly_packedGlyphIDToGrGlyph(GrStrikeCache *cache) {
    fGlyphs.packedGlyphIDToGrGlyph(cache);
}

std::tuple<bool, int> TransformedMaskSubRunSlug::regenerateAtlas(int begin, int end,
                                                             GrMeshDrawTarget* target) const {
    return fGlyphs.regenerateAtlas(begin, end, fMaskFormat, 1, target, true);
}

void TransformedMaskSubRunSlug::fillVertexData(void* vertexDst, int offset, int count,
                                               GrColor color,
                                               const SkMatrix& drawMatrix, SkPoint drawOrigin,
                                               SkIRect clip) const {
    constexpr SkScalar kDstPadding = 0.f;

    const SkMatrix positionMatrix = position_matrix(drawMatrix, drawOrigin);

    auto quadData = [&](auto dst) {
        return SkMakeZip(dst,
                         fGlyphs.glyphs().subspan(offset, count),
                         fVertexData.subspan(offset, count));
    };

    if (!positionMatrix.hasPerspective()) {
        if (fMaskFormat == GrMaskFormat::kARGB_GrMaskFormat) {
            using Quad = ARGB2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
            fill_transformed_vertices_2D(
                    quadData((Quad*) vertexDst),
                    kDstPadding,
                    fStrikeToSourceScale,
                    color,
                    positionMatrix);
        } else {
            using Quad = Mask2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
            fill_transformed_vertices_2D(
                    quadData((Quad*) vertexDst),
                    kDstPadding,
                    fStrikeToSourceScale,
                    color,
                    positionMatrix);
        }
    } else {
        if (fMaskFormat == GrMaskFormat::kARGB_GrMaskFormat) {
            using Quad = ARGB3DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
            fill_transformed_vertices_3D(
                    quadData((Quad*) vertexDst),
                    kDstPadding,
                    fStrikeToSourceScale,
                    color,
                    positionMatrix);
        } else {
            using Quad = Mask3DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
            fill_transformed_vertices_3D(
                    quadData((Quad*) vertexDst),
                    kDstPadding,
                    fStrikeToSourceScale,
                    color,
                    positionMatrix);
        }
    }
}

size_t TransformedMaskSubRunSlug::vertexStride(const SkMatrix& drawMatrix) const {
    switch (fMaskFormat) {
        case kA8_GrMaskFormat:
            return drawMatrix.hasPerspective() ? sizeof(Mask3DVertex) : sizeof(Mask2DVertex);
        case kARGB_GrMaskFormat:
            return drawMatrix.hasPerspective() ? sizeof(ARGB3DVertex) : sizeof(ARGB2DVertex);
        default:
            SkASSERT(!drawMatrix.hasPerspective());
            return sizeof(Mask2DVertex);
    }
    SkUNREACHABLE;
}

int TransformedMaskSubRunSlug::glyphCount() const {
    return SkCount(fVertexData);
}

SkRect TransformedMaskSubRunSlug::deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const {
    SkRect outBounds = fVertexBounds;
    outBounds.offset(drawOrigin);
    return drawMatrix.mapRect(outBounds);
}

void Slug::processSourceMasks(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                              sk_sp<SkStrike>&& strike,
                              SkScalar strikeToSourceScale) {

    auto addGlyphsWithSameFormat = [&] (const SkZip<SkGlyphVariant, SkPoint>& drawable,
                                        GrMaskFormat format,
                                        sk_sp<SkStrike>&& runStrike) {
        SlugSubRunOwner subRun = TransformedMaskSubRunSlug::Make(
                this, drawable, std::move(runStrike), strikeToSourceScale, format, &fAlloc);
        if (subRun != nullptr) {
            fSubRuns.append(std::move(subRun));
        }
    };

    add_multi_mask_format(addGlyphsWithSameFormat, drawables, std::move(strike));
}
}  // namespace

namespace skgpu {
namespace v1 {
sk_sp<GrSlug>
Device::convertGlyphRunListToSlug(const SkGlyphRunList& glyphRunList, const SkPaint& paint) const {
    return fSurfaceDrawContext->convertGlyphRunListToSlug(
            this->asMatrixProvider(), glyphRunList, paint);
}

void Device::drawSlug(GrSlug* slug) {
    fSurfaceDrawContext->drawSlug(this->clip(), this->asMatrixProvider(), slug);
}

sk_sp<GrSlug>
SurfaceDrawContext::convertGlyphRunListToSlug(const SkMatrixProvider& viewMatrix,
                                              const SkGlyphRunList& glyphRunList,
                                              const SkPaint& paint) {
    SkASSERT(fContext->priv().options().fSupportBilerpFromGlyphAtlas);

    GrSDFTControl control =
            this->recordingContext()->priv().getSDFTControl(
                    this->surfaceProps().isUseDeviceIndependentFonts());

    return Slug::Make(viewMatrix, glyphRunList, paint, control, &fGlyphPainter);
}

void SurfaceDrawContext::drawSlug(const GrClip* clip,
                                  const SkMatrixProvider& viewMatrix,
                                  GrSlug* slugPtr) {
    Slug* slug = static_cast<Slug*>(slugPtr);

    slug->surfaceDraw(clip, viewMatrix, this);
}
}  // namespace v1
}  // namespace skgpu



