/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkStrikeSpec.h"
#include "src/gpu/GrBlurUtils.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrDistanceFieldGeoProc.h"
#include "src/gpu/geometry/GrStyledShape.h"
#include "src/gpu/ops/GrAtlasTextOp.h"
#include "src/gpu/text/GrAtlasManager.h"
#include "src/gpu/text/GrStrikeCache.h"
#include "src/gpu/text/GrTextBlob.h"

#include <cstddef>
#include <memory>
#include <new>

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

GrAtlasTextOp::MaskType op_mask_type(GrMaskFormat grMaskFormat) {
    switch (grMaskFormat) {
        case kA8_GrMaskFormat:   return GrAtlasTextOp::MaskType::kGrayscaleCoverage;
        case kA565_GrMaskFormat: return GrAtlasTextOp::MaskType::kLCDCoverage;
        case kARGB_GrMaskFormat: return GrAtlasTextOp::MaskType::kColorBitmap;
    }
    SkUNREACHABLE;
}

SkPMColor4f calculate_colors(GrSurfaceDrawContext* sdc,
                             const SkPaint& paint,
                             const SkMatrixProvider& matrix,
                             GrMaskFormat grMaskFormat,
                             GrPaint* grPaint) {
    GrRecordingContext* rContext = sdc->recordingContext();
    const GrColorInfo& colorInfo = sdc->colorInfo();
    if (grMaskFormat == kARGB_GrMaskFormat) {
        SkPaintToGrPaintWithPrimitiveColor(rContext, colorInfo, paint, matrix, grPaint);
        return SK_PMColor4fWHITE;
    } else {
        SkPaintToGrPaint(rContext, colorInfo, paint, matrix, grPaint);
        return grPaint->getColor4f();
    }
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
std::tuple<bool, SkVector> check_integer_translate(
        const SkMatrix& initialMatrix, const SkMatrix& drawMatrix) {
    if (initialMatrix.getScaleX() != drawMatrix.getScaleX() ||
        initialMatrix.getScaleY() != drawMatrix.getScaleY() ||
        initialMatrix.getSkewX()  != drawMatrix.getSkewX()  ||
        initialMatrix.getSkewY()  != drawMatrix.getSkewY()) {
        return {false, {0, 0}};
    }

    // We can update the positions in the text blob without regenerating the whole
    // blob, but only for integer translations.
    // Calculate the translation in source space to a translation in device space by mapping
    // (0, 0) through both the initial matrix and the draw matrix; take the difference.
    SkVector translation = drawMatrix.mapXY(0, 0) - initialMatrix.mapXY(0, 0);

    return {SkScalarIsInt(translation.x()) && SkScalarIsInt(translation.y()), translation};
}

// -- PathSubRun -----------------------------------------------------------------------------------
class PathSubRun final : public GrSubRun {
    struct PathGlyph;

public:
    PathSubRun(bool isAntiAliased,
               const SkStrikeSpec& strikeSpec,
               const GrTextBlob& blob,
               SkSpan<PathGlyph> paths,
               std::unique_ptr<PathGlyph[], GrSubRunAllocator::ArrayDestroyer> pathData);

    void draw(const GrClip* clip,
              const SkMatrixProvider& viewMatrix,
              const SkGlyphRunList& glyphRunList,
              GrSurfaceDrawContext* sdc) const override;

    bool canReuse(const SkPaint& paint, const SkMatrix& drawMatrix) const override;

    GrAtlasSubRun* testingOnly_atlasSubRun() override;

    static GrSubRunOwner Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                              bool isAntiAliased,
                              const SkStrikeSpec& strikeSpec,
                              const GrTextBlob& blob,
                              GrSubRunAllocator* alloc);

private:
    struct PathGlyph {
        PathGlyph(const SkPath& path, SkPoint origin);
        SkPath fPath;
        SkPoint fOrigin;
    };

    const bool fIsAntiAliased;
    const SkStrikeSpec fStrikeSpec;
    const SkSpan<const PathGlyph> fPaths;
    const std::unique_ptr<PathGlyph[], GrSubRunAllocator::ArrayDestroyer> fPathData;
};

PathSubRun::PathSubRun(bool isAntiAliased,
                       const SkStrikeSpec& strikeSpec,
                       const GrTextBlob& blob,
                       SkSpan<PathGlyph> paths,
                       std::unique_ptr<PathGlyph[], GrSubRunAllocator::ArrayDestroyer> pathData)
    : fIsAntiAliased{isAntiAliased}
    , fStrikeSpec{strikeSpec}
    , fPaths{paths}
    , fPathData{std::move(pathData)} {}

void PathSubRun::draw(const GrClip* clip,
                      const SkMatrixProvider& viewMatrix,
                      const SkGlyphRunList& glyphRunList,
                      GrSurfaceDrawContext* sdc) const {
    SkASSERT(!fPaths.empty());
    SkPoint drawOrigin = glyphRunList.origin();
    const SkPaint& drawPaint = glyphRunList.paint();
    SkPaint runPaint{drawPaint};
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
    SkScalar scale = this->fStrikeSpec.strikeToSourceRatio();
    SkMatrix strikeToSource = SkMatrix::Scale(scale, scale);
    strikeToSource.postTranslate(drawOrigin.x(), drawOrigin.y());
    if (!needsExactCTM) {
        for (const auto& pathPos : fPaths) {
            const SkPath& path = pathPos.fPath;
            const SkPoint pos = pathPos.fOrigin;  // Transform the glyph to source space.
            SkMatrix pathMatrix = strikeToSource;
            pathMatrix.postTranslate(pos.x(), pos.y());
            SkPreConcatMatrixProvider strikeToDevice(viewMatrix, pathMatrix);

            GrStyledShape shape(path, drawPaint);
            GrBlurUtils::drawShapeWithMaskFilter(sdc->recordingContext(), sdc, clip, runPaint,
                                                 strikeToDevice, shape);
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
            GrStyledShape shape(deviceOutline, drawPaint);
            GrBlurUtils::drawShapeWithMaskFilter(sdc->recordingContext(), sdc, clip, runPaint,
                                                 viewMatrix, shape);
        }
    }
}

bool PathSubRun::canReuse(const SkPaint& paint, const SkMatrix& drawMatrix) const {
    return true;
}

GrSubRunOwner PathSubRun::Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                               bool isAntiAliased,
                               const SkStrikeSpec& strikeSpec,
                               const GrTextBlob& blob,
                               GrSubRunAllocator* alloc) {
    auto pathData = alloc->makeUniqueArray<PathGlyph>(
            drawables.size(),
            [&](int i){
                auto [variant, pos] = drawables[i];
                return PathGlyph{*variant.path(), pos};
            });
    SkSpan<PathGlyph> paths{pathData.get(), drawables.size()};

    return alloc->makeUnique<PathSubRun>(
            isAntiAliased, strikeSpec, blob, paths, std::move(pathData));
}

GrAtlasSubRun* PathSubRun::testingOnly_atlasSubRun() {
    return nullptr;
};

// -- PathSubRun::PathGlyph ------------------------------------------------------------------------
PathSubRun::PathGlyph::PathGlyph(const SkPath& path, SkPoint origin)
        : fPath(path)
        , fOrigin(origin) {}

// -- GlyphVector ----------------------------------------------------------------------------------
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

    GlyphVector(const SkStrikeSpec& spec, SkSpan<Variant> glyphs);

    static GlyphVector Make(
            const SkStrikeSpec& spec, SkSpan<SkGlyphVariant> glyphs, GrSubRunAllocator* alloc);
    SkSpan<const GrGlyph*> glyphs() const;

    SkScalar strikeToSourceRatio() const { return fStrikeSpec.strikeToSourceRatio(); }

    void packedGlyphIDToGrGlyph(GrStrikeCache* cache);

    std::tuple<bool, int> regenerateAtlas(
            int begin, int end,
            GrMaskFormat maskFormat,
            int srcPadding,
            GrMeshDrawOp::Target *target,
            bool bilerpPadding = false);

    static size_t GlyphVectorSize(size_t count) {
        return sizeof(Variant) * count;
    }

private:
    const SkStrikeSpec fStrikeSpec;
    SkSpan<Variant> fGlyphs;
    sk_sp<GrTextStrike> fStrike{nullptr};
    uint64_t fAtlasGeneration{GrDrawOpAtlas::kInvalidAtlasGeneration};
    GrDrawOpAtlas::BulkUseTokenUpdater fBulkUseToken;
};

GlyphVector::GlyphVector(const SkStrikeSpec& spec, SkSpan<Variant> glyphs)
    : fStrikeSpec{spec}
    , fGlyphs{glyphs} { }

GlyphVector GlyphVector::Make(
        const SkStrikeSpec &spec, SkSpan<SkGlyphVariant> glyphs, GrSubRunAllocator* alloc) {

    Variant* variants = alloc->makePODArray<Variant>(glyphs.size());
    for (auto [i, gv] : SkMakeEnumerate(glyphs)) {
        variants[i] = gv.glyph()->getPackedID();
    }

    return GlyphVector{spec, SkSpan(variants, glyphs.size())};
}

SkSpan<const GrGlyph*> GlyphVector::glyphs() const {
    return SkSpan(reinterpret_cast<const GrGlyph**>(fGlyphs.data()), fGlyphs.size());
}

void GlyphVector::packedGlyphIDToGrGlyph(GrStrikeCache* cache) {
    if (fStrike == nullptr) {
        fStrike = fStrikeSpec.findOrCreateGrStrike(cache);

        for (auto& variant : fGlyphs) {
            variant.grGlyph = fStrike->getGlyph(variant.packedGlyphID);
        }
    }
}

std::tuple<bool, int> GlyphVector::regenerateAtlas(int begin, int end,
                                                   GrMaskFormat maskFormat,
                                                   int srcPadding,
                                                   GrMeshDrawOp::Target* target,
                                                   bool bilerpPadding) {
    GrAtlasManager* atlasManager = target->atlasManager();
    GrDeferredUploadTarget* uploadTarget = target->deferredUploadTarget();

    uint64_t currentAtlasGen = atlasManager->atlasGeneration(maskFormat);

    this->packedGlyphIDToGrGlyph(target->strikeCache());

    if (fAtlasGeneration != currentAtlasGen) {
        // Calculate the texture coordinates for the vertexes during first use (fAtlasGeneration
        // is set to kInvalidAtlasGeneration) or the atlas has changed in subsequent calls..
        fBulkUseToken.reset();

        SkBulkGlyphMetricsAndImages metricsAndImages{fStrikeSpec};

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
        if (success && begin + glyphsPlacedInAtlas == fGlyphs.count()) {
            // Need to get the freshest value of the atlas' generation because
            // updateTextureCoordinates may have changed it.
            fAtlasGeneration = atlasManager->atlasGeneration(maskFormat);
        }

        return {success, glyphsPlacedInAtlas};
    } else {
        // The atlas hasn't changed, so our texture coordinates are still valid.
        if (end == fGlyphs.count()) {
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
                     GlyphVector glyphs,
                     bool glyphsOutOfBounds);

    static GrSubRunOwner Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                              const SkStrikeSpec& strikeSpec,
                              GrMaskFormat format,
                              GrTextBlob* blob,
                              GrSubRunAllocator* alloc);

    void draw(const GrClip* clip,
              const SkMatrixProvider& viewMatrix,
              const SkGlyphRunList& glyphRunList,
              GrSurfaceDrawContext* sdc) const override;

    bool canReuse(const SkPaint& paint, const SkMatrix& drawMatrix) const override;

    GrAtlasSubRun* testingOnly_atlasSubRun() override;

    size_t vertexStride(const SkMatrix& drawMatrix) const override;

    int glyphCount() const override;

    std::tuple<const GrClip*, GrOp::Owner>
    makeAtlasTextOp(const GrClip* clip,
                    const SkMatrixProvider& viewMatrix,
                    const SkGlyphRunList& glyphRunList,
                    GrSurfaceDrawContext* sdc,
                    GrAtlasSubRunOwner) const override;

    void testingOnly_packedGlyphIDToGrGlyph(GrStrikeCache *cache) override;

    std::tuple<bool, int>
    regenerateAtlas(int begin, int end, GrMeshDrawOp::Target* target) const override;

    void fillVertexData(void* vertexDst, int offset, int count, GrColor color,
                        const SkMatrix& positionMatrix, SkIRect clip) const override;
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
                                   GlyphVector glyphs,
                                   bool glyphsOutOfBounds)
        : fMaskFormat{format}
        , fBlob{blob}
        , fGlyphDeviceBounds{deviceBounds}
        , fLeftTopDevicePos{devicePositions}
        , fSomeGlyphsExcluded{glyphsOutOfBounds}
        , fGlyphs{glyphs} {}

GrSubRunOwner DirectMaskSubRun::Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                     const SkStrikeSpec& strikeSpec,
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

    // If some of the glyphs were excluded by the bounds, then this subrun can't be generally be
    // used for other draws. Mark the subrun as not general.
    bool glyphsExcluded = goodPosCount != drawables.size();
    SkSpan<const DevicePosition> leftTop{glyphLeftTop, goodPosCount};
    return alloc->makeUnique<DirectMaskSubRun>(
            format, blob, runBounds, leftTop,
            GlyphVector{strikeSpec, {glyphIDs, goodPosCount}}, glyphsExcluded);
}

void DirectMaskSubRun::draw(const GrClip* clip, const SkMatrixProvider& viewMatrix,
                            const SkGlyphRunList& glyphRunList,
                            GrSurfaceDrawContext* sdc) const{
    auto[drawingClip, op] = this->makeAtlasTextOp(clip, viewMatrix, glyphRunList, sdc, nullptr);
    if (op != nullptr) {
        sdc->addDrawOp(drawingClip, std::move(op));
    }
}

bool
DirectMaskSubRun::canReuse(const SkPaint& paint, const SkMatrix& drawMatrix) const {
    auto [reuse, translation] = check_integer_translate(fBlob->initialMatrix(), drawMatrix);

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
    return fGlyphs.glyphs().count();
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
DirectMaskSubRun::makeAtlasTextOp(const GrClip* clip, const SkMatrixProvider& viewMatrix,
                                  const SkGlyphRunList& glyphRunList,
                                  GrSurfaceDrawContext* sdc,
                                  GrAtlasSubRunOwner) const {
    SkASSERT(this->glyphCount() != 0);

    const SkMatrix& drawMatrix = viewMatrix.localToDevice();
    const SkPoint drawOrigin = glyphRunList.origin();

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
    const SkPaint& drawPaint = glyphRunList.paint();
    const SkPMColor4f drawingColor =
            calculate_colors(sdc, drawPaint, viewMatrix, fMaskFormat, &grPaint);

    GrRecordingContext* const rContext = sdc->recordingContext();
    GrAtlasTextOp::Geometry* geometry = GrAtlasTextOp::Geometry::MakeForBlob(
            rContext,
            *this,
            drawMatrix,
            drawOrigin,
            clipRect,
            sk_ref_sp<GrTextBlob>(fBlob),
            drawingColor);

    GrOp::Owner op = GrOp::Make<GrAtlasTextOp>(rContext,
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
DirectMaskSubRun::regenerateAtlas(int begin, int end, GrMeshDrawOp::Target* target) const {
    return fGlyphs.regenerateAtlas(begin, end, fMaskFormat, 0, target);
}

// The 99% case. No clip. Non-color only.
void direct_2D(SkZip<Mask2DVertex[4],
        const GrGlyph*,
        const DirectMaskSubRun::DevicePosition> quadData,
               GrColor color,
               SkIPoint integralOriginOffset) {
    for (auto[quad, glyph, leftTop] : quadData) {
        auto[al, at, ar, ab] = glyph->fAtlasLocator.getUVs();
        SkScalar dl = leftTop[0] + integralOriginOffset.x(),
                 dt = leftTop[1] + integralOriginOffset.y(),
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
                           SkIPoint integralOriginOffset,
                           SkIRect* clip = nullptr) {
    for (auto[quad, glyph, leftTop] : quadData) {
        auto[al, at, ar, ab] = glyph->fAtlasLocator.getUVs();
        uint16_t w = ar - al,
                 h = ab - at;
        SkScalar l = (SkScalar)leftTop[0] + integralOriginOffset.x(),
                 t = (SkScalar)leftTop[1] + integralOriginOffset.y();
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

void DirectMaskSubRun::fillVertexData(void* vertexDst, int offset, int count, GrColor color,
                                      const SkMatrix& positionMatrix, SkIRect clip) const {
    auto quadData = [&](auto dst) {
        return SkMakeZip(dst,
                         fGlyphs.glyphs().subspan(offset, count),
                         fLeftTopDevicePos.subspan(offset, count));
    };

    SkPoint originOffset = positionMatrix.mapOrigin() - fBlob->initialMatrix().mapOrigin();
    SkIPoint integralOriginOffset =
            {SkScalarRoundToInt(originOffset.x()), SkScalarRoundToInt(originOffset.y())};

    if (clip.isEmpty()) {
        if (fMaskFormat != kARGB_GrMaskFormat) {
            using Quad = Mask2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
            direct_2D(quadData((Quad*)vertexDst), color, integralOriginOffset);
        } else {
            using Quad = ARGB2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
            generalized_direct_2D(quadData((Quad*)vertexDst), color, integralOriginOffset);
        }
    } else {
        if (fMaskFormat != kARGB_GrMaskFormat) {
            using Quad = Mask2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
            generalized_direct_2D(quadData((Quad*)vertexDst), color, integralOriginOffset, &clip);
        } else {
            using Quad = ARGB2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
            generalized_direct_2D(quadData((Quad*)vertexDst), color, integralOriginOffset, &clip);
        }
    }
}

SkRect DirectMaskSubRun::deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const {
    SkIRect outBounds = fGlyphDeviceBounds.iRect();

    // Calculate the offset from the initial device origin to the current device origin.
    SkVector offset = drawMatrix.mapPoint(drawOrigin) - fBlob->initialMatrix().mapOrigin();

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
                          const SkRect& bounds,
                          SkSpan<const VertexData> vertexData,
                          GlyphVector glyphs);

    static GrSubRunOwner Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                              const SkStrikeSpec& strikeSpec,
                              GrMaskFormat format,
                              GrTextBlob* blob,
                              GrSubRunAllocator* alloc);

    void draw(const GrClip* clip,
              const SkMatrixProvider& viewMatrix,
              const SkGlyphRunList& glyphRunList,
              GrSurfaceDrawContext* sdc) const override;

    bool canReuse(const SkPaint& paint, const SkMatrix& drawMatrix) const override;

    GrAtlasSubRun* testingOnly_atlasSubRun() override;

    std::tuple<const GrClip*, GrOp::Owner>
    makeAtlasTextOp(const GrClip* clip,
                    const SkMatrixProvider& viewMatrix,
                    const SkGlyphRunList& glyphRunList,
                    GrSurfaceDrawContext* sdc,
                    GrAtlasSubRunOwner) const override;

    void testingOnly_packedGlyphIDToGrGlyph(GrStrikeCache *cache) override;

    std::tuple<bool, int> regenerateAtlas(
            int begin, int end, GrMeshDrawOp::Target* target) const override;

    void fillVertexData(
            void* vertexDst, int offset, int count,
            GrColor color, const SkMatrix& positionMatrix, SkIRect clip) const override;

    size_t vertexStride(const SkMatrix& drawMatrix) const override;
    int glyphCount() const override;

private:
    // The rectangle that surrounds all the glyph bounding boxes in device space.
    SkRect deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const;

    const GrMaskFormat fMaskFormat;
    GrTextBlob* fBlob;

    // The bounds in source space. The bounds are the joined rectangles of all the glyphs.
    const SkRect fVertexBounds;
    const SkSpan<const VertexData> fVertexData;

    // The regenerateAtlas method mutates fGlyphs. It should be called from onPrepare which must
    // be single threaded.
    mutable GlyphVector fGlyphs;
};

TransformedMaskSubRun::TransformedMaskSubRun(GrMaskFormat format,
                                             GrTextBlob* blob,
                                             const SkRect& bounds,
                                             SkSpan<const VertexData> vertexData,
                                             GlyphVector glyphs)
        : fMaskFormat{format}
        , fBlob{blob}
        , fVertexBounds{bounds}
        , fVertexData{vertexData}
        , fGlyphs{glyphs} { }

GrSubRunOwner TransformedMaskSubRun::Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                          const SkStrikeSpec& strikeSpec,
                                          GrMaskFormat format,
                                          GrTextBlob* blob,
                                          GrSubRunAllocator* alloc) {
    SkRect bounds = SkRectPriv::MakeLargestInverted();

    SkScalar strikeToSource = strikeSpec.strikeToSourceRatio();
    SkSpan<VertexData> vertexData = alloc->makePODArray<VertexData>(
            drawables,
            [&](auto e) {
                auto [variant, pos] = e;
                SkGlyph* skGlyph = variant;
                int16_t l = skGlyph->left(),
                        t = skGlyph->top(),
                        r = l + skGlyph->width(),
                        b = t + skGlyph->height();
                SkPoint lt = SkPoint::Make(l, t) * strikeToSource + pos,
                        rb = SkPoint::Make(r, b) * strikeToSource + pos;

                bounds.joinPossiblyEmptyRect(SkRect::MakeLTRB(lt.x(), lt.y(), rb.x(), rb.y()));
                return VertexData{pos, {l, t, r, b}};
            });

    return alloc->makeUnique<TransformedMaskSubRun>(
            format, blob, bounds, vertexData,
            GlyphVector::Make(strikeSpec, drawables.get<0>(), alloc));
}

void TransformedMaskSubRun::draw(const GrClip* clip,
                                 const SkMatrixProvider& viewMatrix,
                                 const SkGlyphRunList& glyphRunList,
                                 GrSurfaceDrawContext* sdc) const {
    auto[drawingClip, op] = this->makeAtlasTextOp(clip, viewMatrix, glyphRunList, sdc, nullptr);
    if (op != nullptr) {
        sdc->addDrawOp(drawingClip, std::move(op));
    }
}

// If we are not scaling the cache entry to be larger, than a cache with smaller glyphs may be
// better.
bool TransformedMaskSubRun::canReuse(const SkPaint& paint, const SkMatrix& drawMatrix) const {
    if (fBlob->initialMatrix().getMaxScale() < 1) {
        return false;
    }
    return true;
}

std::tuple<const GrClip*, GrOp::Owner>
TransformedMaskSubRun::makeAtlasTextOp(const GrClip* clip,
                                       const SkMatrixProvider& viewMatrix,
                                       const SkGlyphRunList& glyphRunList,
                                       GrSurfaceDrawContext* sdc,
                                       GrAtlasSubRunOwner) const {
    SkASSERT(this->glyphCount() != 0);

    SkPoint drawOrigin = glyphRunList.origin();
    const SkPaint& drawPaint = glyphRunList.paint();
    const SkMatrix& drawMatrix = viewMatrix.localToDevice();

    GrPaint grPaint;
    SkPMColor4f drawingColor = calculate_colors(sdc, drawPaint, viewMatrix, fMaskFormat, &grPaint);

    GrRecordingContext* const rContext = sdc->recordingContext();
    GrAtlasTextOp::Geometry* geometry = GrAtlasTextOp::Geometry::MakeForBlob(
            rContext,
            *this,
            drawMatrix,
            drawOrigin,
            SkIRect::MakeEmpty(),
            sk_ref_sp<GrTextBlob>(fBlob),
            drawingColor);

    GrOp::Owner op = GrOp::Make<GrAtlasTextOp>(
            rContext,
            op_mask_type(fMaskFormat),
            true,
            this->glyphCount(),
            this->deviceRect(drawMatrix, drawOrigin),
            geometry,
            std::move(grPaint));
    return {clip, std::move(op)};
}

void TransformedMaskSubRun::testingOnly_packedGlyphIDToGrGlyph(GrStrikeCache *cache) {
    fGlyphs.packedGlyphIDToGrGlyph(cache);
}

std::tuple<bool, int> TransformedMaskSubRun::regenerateAtlas(int begin, int end,
                                                             GrMeshDrawOp::Target* target) const {
    return fGlyphs.regenerateAtlas(begin, end, fMaskFormat, 1, target, true);
}

void TransformedMaskSubRun::fillVertexData(
        void* vertexDst, int offset, int count, GrColor color, const SkMatrix& positionMatrix,
        SkIRect clip) const {
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
                    fGlyphs.strikeToSourceRatio(),
                    color,
                    positionMatrix);
        } else {
            using Quad = Mask2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
            fill_transformed_vertices_2D(
                    quadData((Quad*) vertexDst),
                    kDstPadding,
                    fGlyphs.strikeToSourceRatio(),
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
                    fGlyphs.strikeToSourceRatio(),
                    color,
                    positionMatrix);
        } else {
            using Quad = Mask3DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
            fill_transformed_vertices_3D(
                    quadData((Quad*) vertexDst),
                    kDstPadding,
                    fGlyphs.strikeToSourceRatio(),
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
    return fVertexData.count();
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
               SkRect vertexBounds,
               SkSpan<const VertexData> vertexData,
               GlyphVector glyphs,
               bool useLCDText,
               bool antiAliased);

    static GrSubRunOwner Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                              const SkFont& runFont,
                              const SkStrikeSpec& strikeSpec,
                              GrTextBlob* blob,
                              GrSubRunAllocator* alloc);

    void draw(const GrClip* clip,
              const SkMatrixProvider& viewMatrix,
              const SkGlyphRunList& glyphRunList,
              GrSurfaceDrawContext* sdc) const override;

    bool canReuse(const SkPaint& paint, const SkMatrix& drawMatrix) const override;

    GrAtlasSubRun* testingOnly_atlasSubRun() override;

    std::tuple<const GrClip*, GrOp::Owner>
    makeAtlasTextOp(const GrClip* clip,
                    const SkMatrixProvider& viewMatrix,
                    const SkGlyphRunList& glyphRunList,
                    GrSurfaceDrawContext* sdc,
                    GrAtlasSubRunOwner) const override;

    void testingOnly_packedGlyphIDToGrGlyph(GrStrikeCache *cache) override;

    std::tuple<bool, int> regenerateAtlas(
            int begin, int end, GrMeshDrawOp::Target* target) const override;

    void fillVertexData(
            void* vertexDst, int offset, int count,
            GrColor color, const SkMatrix& positionMatrix, SkIRect clip) const override;

    size_t vertexStride(const SkMatrix& drawMatrix) const override;
    int glyphCount() const override;

private:
    // The rectangle that surrounds all the glyph bounding boxes in device space.
    SkRect deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const;

    const GrMaskFormat fMaskFormat;
    GrTextBlob* fBlob;

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
                       SkRect vertexBounds,
                       SkSpan<const VertexData> vertexData,
                       GlyphVector glyphs,
                       bool useLCDText,
                       bool antiAliased)
        : fMaskFormat{format}
        , fBlob{textBlob}
        , fVertexBounds{vertexBounds}
        , fVertexData{vertexData}
        , fGlyphs{glyphs}
        , fUseLCDText{useLCDText}
        , fAntiAliased{antiAliased} { }

bool has_some_antialiasing(const SkFont& font ) {
    SkFont::Edging edging = font.getEdging();
    return edging == SkFont::Edging::kAntiAlias
           || edging == SkFont::Edging::kSubpixelAntiAlias;
}

GrSubRunOwner SDFTSubRun::Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                               const SkFont& runFont,
                               const SkStrikeSpec& strikeSpec,
                               GrTextBlob* blob,
                               GrSubRunAllocator* alloc) {
    SkRect bounds = SkRectPriv::MakeLargestInverted();
    auto mapper = [&, strikeToSource=strikeSpec.strikeToSourceRatio()](const auto& d) {
        auto& [variant, pos] = d;
        SkGlyph* skGlyph = variant;
        int16_t l = skGlyph->left(),
                t = skGlyph->top(),
                r = l + skGlyph->width(),
                b = t + skGlyph->height();
        SkPoint lt = SkPoint::Make(l, t) * strikeToSource + pos,
                rb = SkPoint::Make(r, b) * strikeToSource + pos;

        bounds.joinPossiblyEmptyRect(SkRect::MakeLTRB(lt.x(), lt.y(), rb.x(), rb.y()));
        return VertexData{pos, {l, t, r, b}};
    };

    SkSpan<VertexData> vertexData = alloc->makePODArray<VertexData>(drawables, mapper);

    return alloc->makeUnique<SDFTSubRun>(
            kA8_GrMaskFormat,
            blob,
            bounds,
            vertexData,
            GlyphVector::Make(strikeSpec, drawables.get<0>(), alloc),
            runFont.getEdging() == SkFont::Edging::kSubpixelAntiAlias,
            has_some_antialiasing(runFont));
}

static std::tuple<GrAtlasTextOp::MaskType, uint32_t, bool> calculate_sdf_parameters(
        const GrSurfaceDrawContext& sdc,
        const SkMatrix& drawMatrix,
        bool useLCDText,
        bool isAntiAliased) {
    const GrColorInfo& colorInfo = sdc.colorInfo();
    const SkSurfaceProps& props = sdc.surfaceProps();
    bool isBGR = SkPixelGeometryIsBGR(props.pixelGeometry());
    bool isLCD = useLCDText && SkPixelGeometryIsH(props.pixelGeometry());
    using MT = GrAtlasTextOp::MaskType;
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
                            const SkGlyphRunList& glyphRunList,
                            GrSurfaceDrawContext* sdc,
                            GrAtlasSubRunOwner) const {
    SkASSERT(this->glyphCount() != 0);
    SkASSERT(!viewMatrix.localToDevice().hasPerspective());

    SkPoint drawOrigin = glyphRunList.origin();
    const SkPaint& drawPaint = glyphRunList.paint();
    const SkMatrix& drawMatrix = viewMatrix.localToDevice();

    GrPaint grPaint;
    SkPMColor4f drawingColor = calculate_colors(sdc, drawPaint, viewMatrix, fMaskFormat, &grPaint);

    auto [maskType, DFGPFlags, useGammaCorrectDistanceTable] =
        calculate_sdf_parameters(*sdc, drawMatrix, fUseLCDText, fAntiAliased);

    GrRecordingContext* const rContext = sdc->recordingContext();
    GrAtlasTextOp::Geometry* geometry = GrAtlasTextOp::Geometry::MakeForBlob(
            rContext,
            *this,
            drawMatrix,
            drawOrigin,
            SkIRect::MakeEmpty(),
            sk_ref_sp<GrTextBlob>(fBlob),
            drawingColor);

    GrOp::Owner op = GrOp::Make<GrAtlasTextOp>(
            rContext,
            maskType,
            true,
            this->glyphCount(),
            this->deviceRect(drawMatrix, drawOrigin),
            SkPaintPriv::ComputeLuminanceColor(drawPaint),
            useGammaCorrectDistanceTable,
            DFGPFlags,
            geometry,
            std::move(grPaint));

    return {clip, std::move(op)};
}

void SDFTSubRun::draw(const GrClip* clip,
                      const SkMatrixProvider& viewMatrix,
                      const SkGlyphRunList& glyphRunList,
                      GrSurfaceDrawContext* sdc) const {
    auto[drawingClip, op] = this->makeAtlasTextOp(clip, viewMatrix, glyphRunList, sdc, nullptr);
    if (op != nullptr) {
        sdc->addDrawOp(drawingClip, std::move(op));
    }
}

bool SDFTSubRun::canReuse(const SkPaint& paint, const SkMatrix& drawMatrix) const {
    const SkMatrix& initialMatrix = fBlob->initialMatrix();

    // A scale outside of [blob.fMaxMinScale, blob.fMinMaxScale] would result in a different
    // distance field being generated, so we have to regenerate in those cases
    SkScalar newMaxScale = drawMatrix.getMaxScale();
    SkScalar oldMaxScale = initialMatrix.getMaxScale();
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
        int begin, int end, GrMeshDrawOp::Target *target) const {

    return fGlyphs.regenerateAtlas(begin, end, fMaskFormat, SK_DistanceFieldInset, target);
}

size_t SDFTSubRun::vertexStride(const SkMatrix& drawMatrix) const {
    return sizeof(Mask2DVertex);
}

void SDFTSubRun::fillVertexData(
        void *vertexDst, int offset, int count,
        GrColor color, const SkMatrix& positionMatrix, SkIRect clip) const {

    using Quad = Mask2DVertex[4];
    SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
    fill_transformed_vertices_2D(
            SkMakeZip((Quad*)vertexDst,
                      fGlyphs.glyphs().subspan(offset, count),
                      fVertexData.subspan(offset, count)),
            SK_DistanceFieldInset,
            fGlyphs.strikeToSourceRatio(),
            color,
            positionMatrix);
}

int SDFTSubRun::glyphCount() const {
    return fVertexData.count();
}

SkRect SDFTSubRun::deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const {
    SkRect outBounds = fVertexBounds;
    outBounds.offset(drawOrigin);
    return drawMatrix.mapRect(outBounds);
}

GrAtlasSubRun* SDFTSubRun::testingOnly_atlasSubRun() {
    return this;
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
                           const SkSurfaceProps& surfaceProps,
                           const GrColorInfo& colorInfo,
                           const SkMatrix& drawMatrix,
                           const GrSDFTControl& control) -> std::tuple<bool, Key> {

    // Get the first paint to use as the key paint.
    const SkPaint& drawPaint = glyphRunList.paint();

    SkMaskFilterBase::BlurRec blurRec;
    // It might be worth caching these things, but its not clear at this time
    // TODO for animated mask filters, this will fill up our cache.  We need a safeguard here
    const SkMaskFilter* maskFilter = drawPaint.getMaskFilter();
    bool canCache = glyphRunList.canCache() &&
                    !(drawPaint.getPathEffect() ||
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

        GrColor canonicalColor = compute_canonical_color(drawPaint, hasLCD);

        key.fPixelGeometry = pixelGeometry;
        key.fUniqueID = glyphRunList.uniqueID();
        key.fStyle = drawPaint.getStyle();
        if (key.fStyle != SkPaint::kFill_Style) {
            key.fFrameWidth = drawPaint.getStrokeWidth();
            key.fMiterLimit = drawPaint.getStrokeMiter();
            key.fJoin = drawPaint.getStrokeJoin();
        }
        key.fHasBlur = maskFilter != nullptr;
        if (key.fHasBlur) {
            key.fBlurRec = blurRec;
        }
        key.fCanonicalColor = canonicalColor;
        key.fScalerContextFlags = scalerContextFlags;

        // Calculate the set of drawing types.
        key.fSetOfDrawingTypes = 0;
        for (auto& run : glyphRunList) {
            key.fSetOfDrawingTypes |= control.drawingType(run.font(), drawPaint, drawMatrix);
        }

        if (key.fSetOfDrawingTypes & GrSDFTControl::kDirect) {
            // Store the fractional offset of the position. We know that the matrix can't be
            // perspective at this point.
            SkPoint mappedOrigin = drawMatrix.mapOrigin();
            key.fDrawMatrix = drawMatrix;
            key.fDrawMatrix.setTranslateX(
                    mappedOrigin.x() - SkScalarFloorToScalar(mappedOrigin.x()));
            key.fDrawMatrix.setTranslateY(
                    mappedOrigin.y() - SkScalarFloorToScalar(mappedOrigin.y()));
        } else {
            // For path and SDFT, the matrix doesn't matter.
            key.fDrawMatrix = SkMatrix::I();
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
    if (fDrawMatrix.hasPerspective()) {
        return false;
    }

    if (fSetOfDrawingTypes != that.fSetOfDrawingTypes) {
        return false;
    }

    if (fSetOfDrawingTypes & GrSDFTControl::kDirect) {
        auto [compatible, _] = check_integer_translate(fDrawMatrix, that.fDrawMatrix);
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
                                   const SkMatrix& drawMatrix,
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

    SkColor initialLuminance = SkPaintPriv::ComputeLuminanceColor(glyphRunList.paint());
    sk_sp<GrTextBlob> blob{new (allocation)
                            GrTextBlob(bytesNeededForSubRun, drawMatrix, initialLuminance)};

    for (auto& glyphRun : glyphRunList) {
        painter->processGlyphRun(glyphRun,
                                 drawMatrix,
                                 glyphRunList.paint(),
                                 control,
                                 blob.get(),
                                 "GrTextBlob");
    }

    return blob;
}

void GrTextBlob::addKey(const Key& key) {
    fKey = key;
}

bool GrTextBlob::hasPerspective() const { return fInitialMatrix.hasPerspective(); }

bool GrTextBlob::canReuse(const SkPaint& paint, const SkMatrix& drawMatrix) const {
    // A singular matrix will create a GrTextBlob with no SubRuns, but unknown glyphs can
    // also cause empty runs. If there are no subRuns, then regenerate.
    if ((fSubRunList.isEmpty() || fSomeGlyphsExcluded) && fInitialMatrix != drawMatrix) {
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
        if (!subRun.canReuse(paint, drawMatrix)) {
            return false;
        }
    }

    return true;
}

const GrTextBlob::Key& GrTextBlob::key() const { return fKey; }
size_t GrTextBlob::size() const { return fSize; }

template<typename AddSingleMaskFormat>
void GrTextBlob::addMultiMaskFormat(
        AddSingleMaskFormat addSingle,
        const SkZip<SkGlyphVariant, SkPoint>& drawables,
        const SkStrikeSpec& strikeSpec) {
    if (drawables.empty()) { return; }

    auto addSameFormat = [&](const SkZip<SkGlyphVariant, SkPoint>& drawable, GrMaskFormat format) {
        GrSubRunOwner subRun = addSingle(drawable, strikeSpec, format, this, &fAlloc);
        if (subRun != nullptr) {
            fSubRunList.append(std::move(subRun));
        } else {
            fSomeGlyphsExcluded = true;
        }
    };

    auto glyphSpan = drawables.get<0>();
    SkGlyph* glyph = glyphSpan[0];
    GrMaskFormat format = GrGlyph::FormatFromSkGlyph(glyph->maskFormat());
    size_t startIndex = 0;
    for (size_t i = 1; i < drawables.size(); i++) {
        glyph = glyphSpan[i];
        GrMaskFormat nextFormat = GrGlyph::FormatFromSkGlyph(glyph->maskFormat());
        if (format != nextFormat) {
            auto sameFormat = drawables.subspan(startIndex, i - startIndex);
            addSameFormat(sameFormat, format);
            format = nextFormat;
            startIndex = i;
        }
    }
    auto sameFormat = drawables.last(drawables.size() - startIndex);
    addSameFormat(sameFormat, format);
}

GrTextBlob::GrTextBlob(int allocSize,
                       const SkMatrix& drawMatrix,
                       SkColor initialLuminance)
        : fAlloc{SkTAddOffset<char>(this, sizeof(GrTextBlob)), allocSize, allocSize/2}
        , fSize{allocSize}
        , fInitialMatrix{drawMatrix}
        , fInitialLuminance{initialLuminance} { }

void GrTextBlob::processDeviceMasks(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                    const SkStrikeSpec& strikeSpec) {

    this->addMultiMaskFormat(DirectMaskSubRun::Make, drawables, strikeSpec);
}

void GrTextBlob::processSourcePaths(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                    const SkFont& runFont,
                                    const SkStrikeSpec& strikeSpec) {
    fSubRunList.append(PathSubRun::Make(drawables,
                                        has_some_antialiasing(runFont),
                                        strikeSpec,
                                        *this,
                                        &fAlloc));
}

void GrTextBlob::processSourceSDFT(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                   const SkStrikeSpec& strikeSpec,
                                   const SkFont& runFont,
                                   SkScalar minScale,
                                   SkScalar maxScale) {

    fMaxMinScale = std::max(minScale, fMaxMinScale);
    fMinMaxScale = std::min(maxScale, fMinMaxScale);
    fSubRunList.append(SDFTSubRun::Make(drawables, runFont, strikeSpec, this, &fAlloc));
}

void GrTextBlob::processSourceMasks(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                    const SkStrikeSpec& strikeSpec) {
    this->addMultiMaskFormat(TransformedMaskSubRun::Make, drawables, strikeSpec);
}

// -- GrBagOfBytes ---------------------------------------------------------------------------------
GrBagOfBytes::GrBagOfBytes(char* bytes, size_t size, size_t firstHeapAllocation)
        : fFibProgression(size, firstHeapAllocation) {
    SkASSERT_RELEASE(size < kMaxByteSize);
    SkASSERT_RELEASE(firstHeapAllocation < kMaxByteSize);

    std::size_t space = size;
    void* ptr = bytes;
    if (bytes && std::align(kMaxAlignment, sizeof(Block), ptr, space)) {
        this->setupBytesAndCapacity(bytes, size);
        new (fEndByte) Block(nullptr, nullptr);
    }
}

GrBagOfBytes::GrBagOfBytes(size_t firstHeapAllocation)
        : GrBagOfBytes(nullptr, 0, firstHeapAllocation) {}

GrBagOfBytes::~GrBagOfBytes() {
    Block* cursor = reinterpret_cast<Block*>(fEndByte);
    while (cursor != nullptr) {
        char* toDelete = cursor->fBlockStart;
        cursor = cursor->fPrevious;
        delete [] toDelete;
    }
}

GrBagOfBytes::Block::Block(char* previous, char* startOfBlock)
        : fBlockStart{startOfBlock}
        , fPrevious{reinterpret_cast<Block*>(previous)} {}

void* GrBagOfBytes::alignedBytes(int size, int alignment) {
    SkASSERT_RELEASE(0 < size && size < kMaxByteSize);
    SkASSERT_RELEASE(0 < alignment && alignment <= kMaxAlignment);
    SkASSERT_RELEASE(SkIsPow2(alignment));

    return this->allocateBytes(size, alignment);
}

void GrBagOfBytes::setupBytesAndCapacity(char* bytes, int size) {
    // endByte must be aligned to the maximum alignment to allow tracking alignment using capacity;
    // capacity and endByte are both aligned to max alignment.
    intptr_t endByte = reinterpret_cast<intptr_t>(bytes + size - sizeof(Block)) & -kMaxAlignment;
    fEndByte  = reinterpret_cast<char*>(endByte);
    fCapacity = fEndByte - bytes;
}

void GrBagOfBytes::needMoreBytes(int requestedSize, int alignment) {
    int nextBlockSize = fFibProgression.nextBlockSize();
    const int size = PlatformMinimumSizeWithOverhead(
            std::max(requestedSize, nextBlockSize), alignof(max_align_t));
    char* const bytes = new char[size];
    // fEndByte is changed by setupBytesAndCapacity. Remember it to link back to.
    char* const previousBlock = fEndByte;
    this->setupBytesAndCapacity(bytes, size);

    // Make a block to delete these bytes, and points to the previous block.
    new (fEndByte) Block{previousBlock, bytes};

    // Make fCapacity the alignment for the requested object.
    fCapacity = fCapacity & -alignment;
    SkASSERT(fCapacity >= requestedSize);
}

// -- GrTextBlobAllocator --------------------------------------------------------------------------
GrSubRunAllocator::GrSubRunAllocator(char* bytes, int size, int firstHeapAllocation)
        : fAlloc{bytes, SkTo<size_t>(size), SkTo<size_t>(firstHeapAllocation)} {}

GrSubRunAllocator::GrSubRunAllocator(int firstHeapAllocation)
        : GrSubRunAllocator(nullptr, 0, firstHeapAllocation) {}

void* GrSubRunAllocator::alignedBytes(int unsafeSize, int unsafeAlignment) {
    return fAlloc.alignedBytes(unsafeSize, unsafeAlignment);
}

// ----------------------------- Begin no cache implementation -------------------------------------
namespace {
// -- DirectMaskSubRunNoCache ----------------------------------------------------------------------
class DirectMaskSubRunNoCache final : public GrAtlasSubRun {
public:
    using DevicePosition = skvx::Vec<2, int16_t>;

    DirectMaskSubRunNoCache(GrMaskFormat format,
                            const SkRect& bounds,
                            SkSpan<const DevicePosition> devicePositions,
                            GlyphVector glyphs);

    static GrAtlasSubRunOwner Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                   const SkStrikeSpec& strikeSpec,
                                   GrMaskFormat format,
                                   GrSubRunAllocator* alloc);

    size_t vertexStride(const SkMatrix& drawMatrix) const override;

    int glyphCount() const override;

    std::tuple<const GrClip*, GrOp::Owner>
    makeAtlasTextOp(const GrClip* clip,
                    const SkMatrixProvider& viewMatrix,
                    const SkGlyphRunList& glyphRunList,
                    GrSurfaceDrawContext* sdc,
                    GrAtlasSubRunOwner subRunOwner) const override;

    void testingOnly_packedGlyphIDToGrGlyph(GrStrikeCache *cache) override;

    std::tuple<bool, int>
    regenerateAtlas(int begin, int end, GrMeshDrawOp::Target* target) const override;

    void fillVertexData(void* vertexDst, int offset, int count, GrColor color,
                        const SkMatrix& positionMatrix, SkIRect clip) const override;

private:
    const GrMaskFormat fMaskFormat;
    // The vertex bounds in device space. The bounds are the joined rectangles of all the glyphs.
    const SkRect fGlyphDeviceBounds;
    const SkSpan<const DevicePosition> fLeftTopDevicePos;

    // Space for geometry
    alignas(alignof(GrAtlasTextOp::Geometry)) char fGeom[sizeof(GrAtlasTextOp::Geometry)];

    // The regenerateAtlas method mutates fGlyphs. It should be called from onPrepare which must
    // be single threaded.
    mutable GlyphVector fGlyphs;
};

DirectMaskSubRunNoCache::DirectMaskSubRunNoCache(GrMaskFormat format,
                                                 const SkRect& deviceBounds,
                                                 SkSpan<const DevicePosition> devicePositions,
                                                 GlyphVector glyphs)
        : fMaskFormat{format}
        , fGlyphDeviceBounds{deviceBounds}
        , fLeftTopDevicePos{devicePositions}
        , fGlyphs{glyphs} {}

GrAtlasSubRunOwner DirectMaskSubRunNoCache::Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                                 const SkStrikeSpec& strikeSpec,
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
    return alloc->makeUnique<DirectMaskSubRunNoCache>(
            format, runBounds.rect(), leftTop,
            GlyphVector{strikeSpec, {glyphIDs, goodPosCount}});
}

size_t DirectMaskSubRunNoCache::vertexStride(const SkMatrix&) const {
    if (fMaskFormat != kARGB_GrMaskFormat) {
        return sizeof(Mask2DVertex);
    } else {
        return sizeof(ARGB2DVertex);
    }
}

int DirectMaskSubRunNoCache::glyphCount() const {
    return fGlyphs.glyphs().count();
}

std::tuple<const GrClip*, GrOp::Owner>
DirectMaskSubRunNoCache::makeAtlasTextOp(const GrClip* clip,
                                         const SkMatrixProvider& viewMatrix,
                                         const SkGlyphRunList& glyphRunList,
                                         GrSurfaceDrawContext* sdc,
                                         GrAtlasSubRunOwner subRunOwner) const {
    SkASSERT(this->glyphCount() != 0);

    const SkMatrix& drawMatrix = viewMatrix.localToDevice();
    const SkPoint drawOrigin = glyphRunList.origin();

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
    const SkPaint& drawPaint = glyphRunList.paint();
    const SkPMColor4f drawingColor =
            calculate_colors(sdc, drawPaint, viewMatrix, fMaskFormat, &grPaint);

    GrRecordingContext* const rContext = sdc->recordingContext();

    GrAtlasTextOp::Geometry* geometry = new ((void*)fGeom) GrAtlasTextOp::Geometry{
            *this,
            drawMatrix,
            drawOrigin,
            clipRect,
            nullptr,
            std::move(subRunOwner),
            drawingColor
    };

    GrOp::Owner op = GrOp::Make<GrAtlasTextOp>(rContext,
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
DirectMaskSubRunNoCache::regenerateAtlas(int begin, int end, GrMeshDrawOp::Target* target) const {
    return fGlyphs.regenerateAtlas(begin, end, fMaskFormat, 0, target);
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

void DirectMaskSubRunNoCache::fillVertexData(void* vertexDst, int offset, int count, GrColor color,
                                             const SkMatrix& positionMatrix, SkIRect clip) const {
    auto quadData = [&](auto dst) {
        return SkMakeZip(dst,
                         fGlyphs.glyphs().subspan(offset, count),
                         fLeftTopDevicePos.subspan(offset, count));
    };

    if (clip.isEmpty()) {
        if (fMaskFormat != kARGB_GrMaskFormat) {
            using Quad = Mask2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
            direct_2D2(quadData((Quad*)vertexDst), color);
        } else {
            using Quad = ARGB2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
            generalized_direct_2D(quadData((Quad*)vertexDst), color, {0,0});
        }
    } else {
        if (fMaskFormat != kARGB_GrMaskFormat) {
            using Quad = Mask2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
            generalized_direct_2D(quadData((Quad*)vertexDst), color, {0,0}, &clip);
        } else {
            using Quad = ARGB2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
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
                                 const SkRect& bounds,
                                 SkSpan<const VertexData> vertexData,
                                 GlyphVector glyphs);

    static GrAtlasSubRunOwner Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                   const SkStrikeSpec& strikeSpec,
                                   GrMaskFormat format,
                                   GrSubRunAllocator* alloc);

    std::tuple<const GrClip*, GrOp::Owner>
    makeAtlasTextOp(const GrClip* clip,
                    const SkMatrixProvider& viewMatrix,
                    const SkGlyphRunList& glyphRunList,
                    GrSurfaceDrawContext* sdc,
                    GrAtlasSubRunOwner subRunOwner) const override;

    void testingOnly_packedGlyphIDToGrGlyph(GrStrikeCache *cache) override;

    std::tuple<bool, int> regenerateAtlas(
            int begin, int end, GrMeshDrawOp::Target* target) const override;

    void fillVertexData(
            void* vertexDst, int offset, int count,
            GrColor color, const SkMatrix& positionMatrix, SkIRect clip) const override;

    size_t vertexStride(const SkMatrix& drawMatrix) const override;
    int glyphCount() const override;

private:
    // The rectangle that surrounds all the glyph bounding boxes in device space.
    SkRect deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const;

    const GrMaskFormat fMaskFormat;

    // The bounds in source space. The bounds are the joined rectangles of all the glyphs.
    const SkRect fVertexBounds;
    const SkSpan<const VertexData> fVertexData;

    // Space for geometry
    alignas(alignof(GrAtlasTextOp::Geometry)) char fGeom[sizeof(GrAtlasTextOp::Geometry)];

    // The regenerateAtlas method mutates fGlyphs. It should be called from onPrepare which must
    // be single threaded.
    mutable GlyphVector fGlyphs;
};

TransformedMaskSubRunNoCache::TransformedMaskSubRunNoCache(GrMaskFormat format,
                                                           const SkRect& bounds,
                                                           SkSpan<const VertexData> vertexData,
                                                           GlyphVector glyphs)
        : fMaskFormat{format}
        , fVertexBounds{bounds}
        , fVertexData{vertexData}
        , fGlyphs{glyphs} { }

GrAtlasSubRunOwner TransformedMaskSubRunNoCache::Make(
        const SkZip<SkGlyphVariant, SkPoint>& drawables,
        const SkStrikeSpec& strikeSpec,
        GrMaskFormat format,
        GrSubRunAllocator* alloc) {
    SkRect bounds = SkRectPriv::MakeLargestInverted();
    auto initializer = [&, strikeToSource=strikeSpec.strikeToSourceRatio()](auto drawable) {
        auto [variant, pos] = drawable;
        SkGlyph* skGlyph = variant;
        int16_t l = skGlyph->left(),
                t = skGlyph->top(),
                r = l + skGlyph->width(),
                b = t + skGlyph->height();
        SkPoint lt = SkPoint::Make(l, t) * strikeToSource + pos,
                rb = SkPoint::Make(r, b) * strikeToSource + pos;

        bounds.joinPossiblyEmptyRect(SkRect::MakeLTRB(lt.x(), lt.y(), rb.x(), rb.y()));
        return VertexData{pos, {l, t, r, b}};
    };

    SkSpan<VertexData> vertexData = alloc->makePODArray<VertexData>(drawables, initializer);

    return alloc->makeUnique<TransformedMaskSubRunNoCache>(
            format, bounds, vertexData,
            GlyphVector::Make(strikeSpec, drawables.get<0>(), alloc));
}

std::tuple<const GrClip*, GrOp::Owner>
TransformedMaskSubRunNoCache::makeAtlasTextOp(const GrClip* clip,
                                              const SkMatrixProvider& viewMatrix,
                                              const SkGlyphRunList& glyphRunList,
                                              GrSurfaceDrawContext* sdc,
                                              GrAtlasSubRunOwner subRunOwner) const {
    SkASSERT(this->glyphCount() != 0);

    SkPoint drawOrigin = glyphRunList.origin();
    const SkPaint& drawPaint = glyphRunList.paint();
    const SkMatrix& drawMatrix = viewMatrix.localToDevice();

    GrPaint grPaint;
    SkPMColor4f drawingColor = calculate_colors(sdc, drawPaint, viewMatrix, fMaskFormat, &grPaint);

    // We can clip geometrically using clipRect and ignore clip if we're not using SDFs or
    // transformed glyphs, and we have an axis-aligned rectangular non-AA clip.
    GrAtlasTextOp::Geometry* geometry = new ((void*)fGeom) GrAtlasTextOp::Geometry{
            *this,
            drawMatrix,
            drawOrigin,
            SkIRect::MakeEmpty(),
            nullptr,
            std::move(subRunOwner),
            drawingColor
    };

    GrRecordingContext* rContext = sdc->recordingContext();
    GrOp::Owner op = GrOp::Make<GrAtlasTextOp>(
            rContext,
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
        int begin, int end, GrMeshDrawOp::Target* target) const {
    return fGlyphs.regenerateAtlas(begin, end, fMaskFormat, 1, target, true);
}

void TransformedMaskSubRunNoCache::fillVertexData(
        void* vertexDst, int offset, int count, GrColor color,
        const SkMatrix& positionMatrix, SkIRect clip) const {
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
                    fGlyphs.strikeToSourceRatio(),
                    color,
                    positionMatrix);
        } else {
            using Quad = Mask2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
            fill_transformed_vertices_2D(
                    quadData((Quad*) vertexDst),
                    kDstPadding,
                    fGlyphs.strikeToSourceRatio(),
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
                    fGlyphs.strikeToSourceRatio(),
                    color,
                    positionMatrix);
        } else {
            using Quad = Mask3DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
            fill_transformed_vertices_3D(
                    quadData((Quad*) vertexDst),
                    kDstPadding,
                    fGlyphs.strikeToSourceRatio(),
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
    return fVertexData.count();
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
                      SkRect vertexBounds,
                      SkSpan<const VertexData> vertexData,
                      GlyphVector glyphs,
                      bool useLCDText,
                      bool antiAliased);

    static GrAtlasSubRunOwner Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                   const SkFont& runFont,
                                   const SkStrikeSpec& strikeSpec,
                                   GrSubRunAllocator* alloc);

    std::tuple<const GrClip*, GrOp::Owner>
    makeAtlasTextOp(const GrClip* clip,
                    const SkMatrixProvider& viewMatrix,
                    const SkGlyphRunList& glyphRunList,
                    GrSurfaceDrawContext* sdc,
                    GrAtlasSubRunOwner subRunOwner) const override;

    void testingOnly_packedGlyphIDToGrGlyph(GrStrikeCache *cache) override;

    std::tuple<bool, int> regenerateAtlas(
            int begin, int end, GrMeshDrawOp::Target* target) const override;

    void fillVertexData(
            void* vertexDst, int offset, int count,
            GrColor color, const SkMatrix& positionMatrix, SkIRect clip) const override;

    size_t vertexStride(const SkMatrix& drawMatrix) const override;
    int glyphCount() const override;

private:
    // The rectangle that surrounds all the glyph bounding boxes in device space.
    SkRect deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const;

    const GrMaskFormat fMaskFormat;

    // The bounds in source space. The bounds are the joined rectangles of all the glyphs.
    const SkRect fVertexBounds;
    const SkSpan<const VertexData> fVertexData;

    // Space for geometry
    alignas(alignof(GrAtlasTextOp::Geometry)) char fGeom[sizeof(GrAtlasTextOp::Geometry)];

    // The regenerateAtlas method mutates fGlyphs. It should be called from onPrepare which must
    // be single threaded.
    mutable GlyphVector fGlyphs;

    const bool fUseLCDText;
    const bool fAntiAliased;
};

SDFTSubRunNoCache::SDFTSubRunNoCache(GrMaskFormat format,
                                     SkRect vertexBounds,
                                     SkSpan<const VertexData> vertexData,
                                     GlyphVector glyphs,
                                     bool useLCDText,
                                     bool antiAliased)
        : fMaskFormat{format}
        , fVertexBounds{vertexBounds}
        , fVertexData{vertexData}
        , fGlyphs{glyphs}
        , fUseLCDText{useLCDText}
        , fAntiAliased{antiAliased} { }


GrAtlasSubRunOwner SDFTSubRunNoCache::Make(
        const SkZip<SkGlyphVariant, SkPoint>& drawables,
        const SkFont& runFont,
        const SkStrikeSpec& strikeSpec,
        GrSubRunAllocator* alloc) {

    SkRect bounds = SkRectPriv::MakeLargestInverted();
    auto initializer = [&, strikeToSource=strikeSpec.strikeToSourceRatio()](auto drawable) {
        auto [variant, pos] = drawable;
        SkGlyph* skGlyph = variant;
        int16_t l = skGlyph->left(),
                t = skGlyph->top(),
                r = l + skGlyph->width(),
                b = t + skGlyph->height();
        SkPoint lt = SkPoint::Make(l, t) * strikeToSource + pos,
                rb = SkPoint::Make(r, b) * strikeToSource + pos;

        bounds.joinPossiblyEmptyRect(SkRect::MakeLTRB(lt.x(), lt.y(), rb.x(), rb.y()));
        return VertexData{pos, {l, t, r, b}};
    };

    SkSpan<VertexData> vertexData = alloc->makePODArray<VertexData>(drawables, initializer);

    return alloc->makeUnique<SDFTSubRunNoCache>(
            kA8_GrMaskFormat,
            bounds,
            vertexData,
            GlyphVector::Make(strikeSpec, drawables.get<0>(), alloc),
            runFont.getEdging() == SkFont::Edging::kSubpixelAntiAlias,
            has_some_antialiasing(runFont));
}

std::tuple<const GrClip*, GrOp::Owner>
SDFTSubRunNoCache::makeAtlasTextOp(const GrClip* clip,
                                   const SkMatrixProvider& viewMatrix,
                                   const SkGlyphRunList& glyphRunList,
                                   GrSurfaceDrawContext* sdc,
                                   GrAtlasSubRunOwner subRunOwner) const {
    SkASSERT(this->glyphCount() != 0);

    SkPoint drawOrigin = glyphRunList.origin();
    const SkPaint& drawPaint = glyphRunList.paint();
    const SkMatrix& drawMatrix = viewMatrix.localToDevice();

    GrPaint grPaint;
    SkPMColor4f drawingColor = calculate_colors(sdc, drawPaint, viewMatrix, fMaskFormat, &grPaint);

    auto [maskType, DFGPFlags, useGammaCorrectDistanceTable] =
    calculate_sdf_parameters(*sdc, drawMatrix, fUseLCDText, fAntiAliased);

    GrAtlasTextOp::Geometry* geometry = new ((void*)fGeom) GrAtlasTextOp::Geometry {
            *this,
            drawMatrix,
            drawOrigin,
            SkIRect::MakeEmpty(),
            nullptr,
            std::move(subRunOwner),
            drawingColor
    };

    GrRecordingContext* rContext = sdc->recordingContext();
    GrOp::Owner op = GrOp::Make<GrAtlasTextOp>(
            rContext,
            maskType,
            true,
            this->glyphCount(),
            this->deviceRect(drawMatrix, drawOrigin),
            SkPaintPriv::ComputeLuminanceColor(drawPaint),
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
        int begin, int end, GrMeshDrawOp::Target *target) const {

    return fGlyphs.regenerateAtlas(begin, end, fMaskFormat, SK_DistanceFieldInset, target);
}

size_t SDFTSubRunNoCache::vertexStride(const SkMatrix& drawMatrix) const {
    return sizeof(Mask2DVertex);
}

void SDFTSubRunNoCache::fillVertexData(
        void *vertexDst, int offset, int count,
        GrColor color, const SkMatrix& positionMatrix, SkIRect clip) const {
    using Quad = Mask2DVertex[4];
    SkASSERT(sizeof(Quad) == this->vertexStride(positionMatrix) * kVerticesPerGlyph);
    fill_transformed_vertices_2D(
            SkMakeZip((Quad*)vertexDst,
                      fGlyphs.glyphs().subspan(offset, count),
                      fVertexData.subspan(offset, count)),
            SK_DistanceFieldInset,
            fGlyphs.strikeToSourceRatio(),
            color,
            positionMatrix);
}

int SDFTSubRunNoCache::glyphCount() const {
    return fVertexData.count();
}

SkRect SDFTSubRunNoCache::deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const {
    SkRect outBounds = fVertexBounds;
    outBounds.offset(drawOrigin);
    return drawMatrix.mapRect(outBounds);
}
}  // namespace


GrSubRunNoCachePainter::GrSubRunNoCachePainter(GrSurfaceDrawContext* sdc,
                                               GrSubRunAllocator* alloc,
                                               const GrClip* clip,
                                               const SkMatrixProvider& viewMatrix,
                                               const SkGlyphRunList& glyphRunList)
            : fSDC{sdc}
            , fAlloc{alloc}
            , fClip{clip}
            , fViewMatrix{viewMatrix}
            , fGlyphRunList{glyphRunList} {}

void GrSubRunNoCachePainter::processDeviceMasks(
        const SkZip<SkGlyphVariant, SkPoint>& drawables, const SkStrikeSpec& strikeSpec) {
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
            this->draw(DirectMaskSubRunNoCache::Make(sameFormat, strikeSpec, format, fAlloc));
            format = nextFormat;
            startIndex = i;
        }
    }
    auto sameFormat = drawables.last(drawables.size() - startIndex);
    this->draw(DirectMaskSubRunNoCache::Make(sameFormat, strikeSpec, format, fAlloc));
}

void GrSubRunNoCachePainter::processSourceMasks(
            const SkZip<SkGlyphVariant, SkPoint>& drawables, const SkStrikeSpec& strikeSpec) {
    if (drawables.empty()) {
        return;
    }

    auto glyphSpan = drawables.get<0>();
    SkGlyph* glyph = glyphSpan[0];
    GrMaskFormat format = GrGlyph::FormatFromSkGlyph(glyph->maskFormat());
    size_t startIndex = 0;
    for (size_t i = 1; i < drawables.size(); i++) {
        glyph = glyphSpan[i];
        GrMaskFormat nextFormat = GrGlyph::FormatFromSkGlyph(glyph->maskFormat());
        if (format != nextFormat) {
            auto sameFormat = drawables.subspan(startIndex, i - startIndex);
            this->draw(
                    TransformedMaskSubRunNoCache::Make(
                            sameFormat, strikeSpec, format, fAlloc));
            format = nextFormat;
            startIndex = i;
        }
    }
    auto sameFormat = drawables.last(drawables.size() - startIndex);
    this->draw(
            TransformedMaskSubRunNoCache::Make(sameFormat, strikeSpec, format, fAlloc));
}

void GrSubRunNoCachePainter::processSourcePaths(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                                const SkFont& runFont,
                                                const SkStrikeSpec& strikeSpec) {
    SkASSERT(!drawables.empty());
    SkPoint drawOrigin = fGlyphRunList.origin();
    const SkPaint& drawPaint = fGlyphRunList.paint();
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
    SkScalar scale = strikeSpec.strikeToSourceRatio();
    SkMatrix strikeToSource = SkMatrix::Scale(scale, scale);
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
                                               const SkStrikeSpec& strikeSpec,
                                               const SkFont& runFont,
                                               SkScalar minScale, SkScalar maxScale) {
    if (drawables.empty()) {
        return;
    }
    this->draw(SDFTSubRunNoCache::Make(drawables, runFont, strikeSpec, fAlloc));
}

void GrSubRunNoCachePainter::draw(GrAtlasSubRunOwner subRun) {
    GrAtlasSubRun* subRunPtr = subRun.get();
    auto [drawingClip, op] = subRunPtr->makeAtlasTextOp(
            fClip, fViewMatrix, fGlyphRunList, fSDC, std::move(subRun));
    if (op != nullptr) {
        fSDC->addDrawOp(drawingClip, std::move(op));
    }
}

