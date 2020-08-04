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
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrDistanceFieldGeoProc.h"
#include "src/gpu/geometry/GrStyledShape.h"
#include "src/gpu/ops/GrAtlasTextOp.h"
#include "src/gpu/text/GrAtlasManager.h"
#include "src/gpu/text/GrStrikeCache.h"
#include "src/gpu/text/GrTextBlob.h"

#include <cstddef>
#include <new>

// -- GrTextBlob::Key ------------------------------------------------------------------------------
GrTextBlob::Key::Key() { sk_bzero(this, sizeof(Key)); }

bool GrTextBlob::Key::operator==(const GrTextBlob::Key& other) const {
    return 0 == memcmp(this, &other, sizeof(Key));
}

// -- GrPathSubRun::PathGlyph ----------------------------------------------------------------------
GrPathSubRun::PathGlyph::PathGlyph(const SkPath& path, SkPoint origin)
        : fPath(path)
        , fOrigin(origin) {}

// -- GrPathSubRun ---------------------------------------------------------------------------------
GrPathSubRun::GrPathSubRun(bool isAntiAliased,
                           const SkStrikeSpec& strikeSpec,
                           SkSpan<PathGlyph> paths)
    : fIsAntiAliased{isAntiAliased}
    , fStrikeSpec{strikeSpec}
    , fPaths{paths} {}

void GrPathSubRun::draw(const GrClip* clip,
                        const SkMatrixProvider& viewMatrix,
                        const SkGlyphRunList& glyphRunList,
                        GrRenderTargetContext* rtc) const {
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
            GrBlurUtils::drawShapeWithMaskFilter(
                    rtc->priv().recordingContext(), rtc, clip, runPaint, strikeToDevice, shape);
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
            GrBlurUtils::drawShapeWithMaskFilter(
                    rtc->priv().recordingContext(), rtc, clip, runPaint, viewMatrix, shape);
        }
    }
}

auto GrPathSubRun::Make(
        const SkZip<SkGlyphVariant, SkPoint>& drawables,
        bool isAntiAliased,
        const SkStrikeSpec& strikeSpec,
        SkArenaAlloc* alloc) -> GrSubRun* {
    PathGlyph* pathData = alloc->makeInitializedArray<PathGlyph>(
            drawables.size(),
            [&](size_t i) -> PathGlyph {
                auto [variant, pos] = drawables[i];
                return {*variant.path(), pos};
            });

    return alloc->make<GrPathSubRun>(
            isAntiAliased, strikeSpec, SkMakeSpan(pathData, drawables.size()));
};

// -- GrGlyphVector --------------------------------------------------------------------------------
GrGlyphVector::GrGlyphVector(const SkStrikeSpec& spec, SkSpan<Variant> glyphs)
    : fStrikeSpec{spec}
    , fGlyphs{glyphs} { }

GrGlyphVector GrGlyphVector::Make(
        const SkStrikeSpec &spec, SkSpan<SkGlyphVariant> glyphs, SkArenaAlloc *alloc) {

    Variant* variants = alloc->makeInitializedArray<Variant>(glyphs.size(),
            [&](int i) {
                return Variant{glyphs[i].glyph()->getPackedID()};
            });

    return GrGlyphVector{spec, SkMakeSpan(variants, glyphs.size())};
}

SkSpan<const GrGlyph*> GrGlyphVector::glyphs() const {
    return SkMakeSpan(reinterpret_cast<const GrGlyph**>(fGlyphs.data()), fGlyphs.size());
}

std::tuple<bool, int> GrGlyphVector::regenerateAtlas(int begin, int end,
                                                     GrMaskFormat maskFormat,
                                                     int srcPadding,
                                                     GrMeshDrawOp::Target* target,
                                                     bool bilerpPadding) {
    GrAtlasManager* atlasManager = target->atlasManager();
    GrDeferredUploadTarget* uploadTarget = target->deferredUploadTarget();

    uint64_t currentAtlasGen = atlasManager->atlasGeneration(maskFormat);

    if (fStrike == nullptr) {
        fStrike = fStrikeSpec.findOrCreateGrStrike(target->strikeCache());

        for (auto& variant : fGlyphs) {
            variant.grGlyph = fStrike->getGlyph(variant.packedGlyphID);
        }
    }

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

// -- GrAtlasSubRun --------------------------------------------------------------------------------
static GrAtlasTextOp::MaskType op_mask_type(GrMaskFormat grMaskFormat) {
    switch (grMaskFormat) {
        case kA8_GrMaskFormat: return GrAtlasTextOp::kGrayscaleCoverageMask_MaskType;
        case kA565_GrMaskFormat: return GrAtlasTextOp::kLCDCoverageMask_MaskType;
        case kARGB_GrMaskFormat: return GrAtlasTextOp::kColorBitmapMask_MaskType;
            // Needed to placate some compilers.
        default: return GrAtlasTextOp::kGrayscaleCoverageMask_MaskType;
    }
}

// -- GrDirectMaskSubRun ---------------------------------------------------------------------------
GrDirectMaskSubRun::GrDirectMaskSubRun(GrMaskFormat format,
                                       GrTextBlob* blob,
                                       const SkRect& bounds,
                                       SkSpan<const VertexData> vertexData,
                                       GrGlyphVector glyphs)
        : fMaskFormat{format}
        , fBlob{blob}
        , fVertexBounds{bounds}
        , fVertexData{vertexData}
        , fGlyphs{glyphs} { }

GrSubRun* GrDirectMaskSubRun::Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                   const SkStrikeSpec& strikeSpec,
                                   GrMaskFormat format,
                                   GrTextBlob* blob,
                                   SkArenaAlloc* alloc) {
    size_t vertexCount = drawables.size();
    SkRect bounds = SkRectPriv::MakeLargestInverted();
    auto initializer = [&](size_t i) {
        auto [variant, pos] = drawables[i];
        SkGlyph* skGlyph = variant;
        int16_t l = skGlyph->left();
        int16_t t = skGlyph->top();
        int16_t r = l + skGlyph->width();
        int16_t b = t + skGlyph->height();
        SkPoint lt = SkPoint::Make(l, t) + pos,
                rb = SkPoint::Make(r, b) + pos;

        bounds.joinPossiblyEmptyRect(SkRect::MakeLTRB(lt.x(), lt.y(), rb.x(), rb.y()));
        return VertexData{pos, {l, t, r, b}};
    };

    SkSpan<const VertexData> vertexData{
            alloc->makeInitializedArray<VertexData>(vertexCount, initializer), vertexCount};

    GrDirectMaskSubRun* subRun = alloc->make<GrDirectMaskSubRun>(
            format, blob, bounds, vertexData,
            GrGlyphVector::Make(strikeSpec, drawables.get<0>(), alloc));

    return subRun;
}

void GrDirectMaskSubRun::draw(const GrClip* clip, const SkMatrixProvider& viewMatrix,
                              const SkGlyphRunList& glyphRunList, GrRenderTargetContext* rtc) const{
    auto[drawingClip, op] = this->makeAtlasTextOp(clip, viewMatrix, glyphRunList, rtc);
    if (op != nullptr) {
        rtc->priv().addDrawOp(drawingClip, std::move(op));
    }
}

size_t GrDirectMaskSubRun::vertexStride() const {
    if (fMaskFormat != kARGB_GrMaskFormat) {
        return sizeof(Mask2DVertex);
    } else {
        return sizeof(ARGB2DVertex);
    }
}

int GrDirectMaskSubRun::glyphCount() const {
    return fGlyphs.glyphs().count();
}

static SkPMColor4f calculate_colors(GrRenderTargetContext* rtc,
                                    const SkPaint& paint,
                                    const SkMatrixProvider& matrix,
                                    GrMaskFormat grMaskFormat,
                                    GrPaint* grPaint) {
    GrRecordingContext* rContext = rtc->priv().recordingContext();
    const GrColorInfo& colorInfo = rtc->colorInfo();
    if (grMaskFormat == kARGB_GrMaskFormat) {
        SkPaintToGrPaintWithPrimitiveColor(rContext, colorInfo, paint, matrix, grPaint);
        return SK_PMColor4fWHITE;
    } else {
        SkPaintToGrPaint(rContext, colorInfo, paint, matrix, grPaint);
        return grPaint->getColor4f();
    }
}

std::tuple<const GrClip*, std::unique_ptr<GrDrawOp>>
GrDirectMaskSubRun::makeAtlasTextOp(const GrClip* clip, const SkMatrixProvider& viewMatrix,
                                    const SkGlyphRunList& glyphRunList,
                                    GrRenderTargetContext* rtc) const {
    SkASSERT(this->glyphCount() != 0);

    const SkMatrix& drawMatrix = viewMatrix.localToDevice();
    const SkPoint drawOrigin = glyphRunList.origin();

    // We can clip geometrically using clipRect and ignore clip if we're not using SDFs or
    // transformed glyphs, and we have an axis-aligned rectangular non-AA clip.
    SkIRect clipRect = SkIRect::MakeEmpty();

    // We only need to do clipping work if the SubRun isn't contained by the clip
    const SkRect subRunBounds = this->deviceRect(drawMatrix, drawOrigin);
    const SkRect renderTargetBounds = SkRect::MakeWH(rtc->width(), rtc->height());

    if (clip == nullptr && !renderTargetBounds.intersects(subRunBounds)) {
        // If the SubRun is completely outside, don't add an op for it.
        return {nullptr, nullptr};
    } else if (clip != nullptr) {
        const GrClip::PreClipResult result = clip->preApply(subRunBounds);
        if (result.fEffect == GrClip::Effect::kClipped) {
            if (result.fIsRRect && result.fRRect.isRect() && result.fAA == GrAA::kNo) {
                // Clip geometrically during onPrepare using clipRect.
                result.fRRect.getBounds().round(&clipRect);
                clip = nullptr;
            }
        } else if (result.fEffect == GrClip::Effect::kClippedOut) {
            return {nullptr, nullptr};
        }
    }

    if (!clipRect.isEmpty()) { SkASSERT(clip == nullptr); }

    GrPaint grPaint;
    const SkPaint& drawPaint = glyphRunList.paint();
    const SkPMColor4f drawingColor =
            calculate_colors(rtc, drawPaint, viewMatrix, fMaskFormat, &grPaint);
    GrAtlasTextOp::Geometry geometry = {
            *this,
            drawMatrix,
            drawOrigin,
            clipRect,
            SkRef(fBlob),
            drawingColor
    };

    GrOpMemoryPool* const pool = rtc->priv().recordingContext()->priv().opMemoryPool();
    std::unique_ptr<GrDrawOp> op = pool->allocate<GrAtlasTextOp>(op_mask_type(fMaskFormat),
                                                                 false,
                                                                 this->glyphCount(),
                                                                 subRunBounds,
                                                                 geometry,
                                                                 std::move(grPaint));

    return {clip, std::move(op)};
}

std::tuple<bool, int>
GrDirectMaskSubRun::regenerateAtlas(int begin, int end, GrMeshDrawOp::Target* target) const {
    return fGlyphs.regenerateAtlas(begin, end, fMaskFormat, 0, target);
}

template <typename Rect>
static auto ltbr(const Rect& r) {
    return std::make_tuple(r.left(), r.top(), r.right(), r.bottom());
}

void GrDirectMaskSubRun::fillVertexData(void* vertexDst, int offset, int count, GrColor color,
                                        const SkMatrix& drawMatrix, SkPoint drawOrigin,
                                        SkIRect clip) const {
    SkMatrix matrix = drawMatrix;
    matrix.preTranslate(drawOrigin.x(), drawOrigin.y());

    auto vertices = [&](auto dst) {
        return SkMakeZip(dst,
                         fGlyphs.glyphs().subspan(offset, count),
                         fVertexData.subspan(offset, count));
    };

    auto direct2D = [&](auto dst, SkIRect* clip) {
        // Rectangles in device space
        SkPoint originInDeviceSpace = matrix.mapXY(0, 0);
        for (auto[quad, glyph, vertexData] : vertices(dst)) {
            auto[pos, rect] = vertexData;
            auto[l, t, r, b] = rect;
            auto[fx, fy] = pos + originInDeviceSpace;
            auto[al, at, ar, ab] = glyph->fAtlasLocator.getUVs();
            if (clip == nullptr) {
                SkScalar dx = SkScalarRoundToScalar(fx),
                         dy = SkScalarRoundToScalar(fy);
                auto[dl, dt, dr, db] = SkRect::MakeLTRB(l + dx, t + dy, r + dx, b + dy);
                quad[0] = {{dl, dt}, color, {al, at}};  // L,T
                quad[1] = {{dl, db}, color, {al, ab}};  // L,B
                quad[2] = {{dr, dt}, color, {ar, at}};  // R,T
                quad[3] = {{dr, db}, color, {ar, ab}};  // R,B
            } else {
                int dx = SkScalarRoundToInt(fx),
                    dy = SkScalarRoundToInt(fy);
                SkIRect devIRect = SkIRect::MakeLTRB(l + dx, t + dy, r + dx, b + dy);
                SkScalar dl, dt, dr, db;
                uint16_t tl, tt, tr, tb;
                if (!clip->containsNoEmptyCheck(devIRect)) {
                    if (SkIRect clipped; clipped.intersect(devIRect, *clip)) {
                        int lD = clipped.left() - devIRect.left();
                        int tD = clipped.top() - devIRect.top();
                        int rD = clipped.right() - devIRect.right();
                        int bD = clipped.bottom() - devIRect.bottom();
                        int indexLT, indexRB;
                        std::tie(dl, dt, dr, db) = ltbr(clipped);
                        std::tie(tl, tt, indexLT) =
                                GrDrawOpAtlas::UnpackIndexFromTexCoords(al, at);
                        std::tie(tr, tb, indexRB) =
                                GrDrawOpAtlas::UnpackIndexFromTexCoords(ar, ab);
                        std::tie(tl, tt) =
                                GrDrawOpAtlas::PackIndexInTexCoords(tl + lD, tt + tD, indexLT);
                        std::tie(tr, tb) =
                                GrDrawOpAtlas::PackIndexInTexCoords(tr + rD, tb + bD, indexRB);
                    } else {
                        // TODO: omit generating any vertex data for fully clipped glyphs ?
                        std::tie(dl, dt, dr, db) = std::make_tuple(0, 0, 0, 0);
                        std::tie(tl, tt, tr, tb) = std::make_tuple(0, 0, 0, 0);
                    }

                } else {
                    std::tie(dl, dt, dr, db) = ltbr(devIRect);
                    std::tie(tl, tt, tr, tb) = std::tie(al, at, ar, ab);
                }
                quad[0] = {{dl, dt}, color, {tl, tt}};  // L,T
                quad[1] = {{dl, db}, color, {tl, tb}};  // L,B
                quad[2] = {{dr, dt}, color, {tr, tt}};  // R,T
                quad[3] = {{dr, db}, color, {tr, tb}};  // R,B
            }
        }
    };

    if (clip.isEmpty()) {
        if (fMaskFormat != kARGB_GrMaskFormat) {
            using Quad = Mask2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride() * kVerticesPerGlyph);
            direct2D((Quad*) vertexDst, nullptr);
        } else {
            using Quad = ARGB2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride() * kVerticesPerGlyph);
            direct2D((Quad*) vertexDst, nullptr);
        }
    } else {
        if (fMaskFormat != kARGB_GrMaskFormat) {
            using Quad = Mask2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride() * kVerticesPerGlyph);
            direct2D((Quad*) vertexDst, &clip);
        } else {
            using Quad = ARGB2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride() * kVerticesPerGlyph);
            direct2D((Quad*) vertexDst, &clip);
        }
    }

}

SkRect GrDirectMaskSubRun::deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const {
    SkRect outBounds = fVertexBounds;

    SkPoint offset = drawMatrix.mapXY(drawOrigin.x(), drawOrigin.y());
    // The vertex bounds are already {0, 0} based, so just add the new origin offset.
    outBounds.offset(offset);

    // Due to floating point numerical inaccuracies, we have to round out here
    outBounds.roundOut();

    return outBounds;
}

// -- GrTransformedMaskSubRun ----------------------------------------------------------------------
GrTransformedMaskSubRun::GrTransformedMaskSubRun(GrMaskFormat format,
                                                 GrTextBlob* blob,
                                                 const SkRect& bounds,
                                                 SkSpan<const VertexData> vertexData,
                                                 GrGlyphVector glyphs)
        : fMaskFormat{format}
        , fBlob{blob}
        , fVertexBounds{bounds}
        , fVertexData{vertexData}
        , fGlyphs{glyphs} { }

GrSubRun* GrTransformedMaskSubRun::Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                        const SkStrikeSpec& strikeSpec,
                                        GrMaskFormat format,
                                        GrTextBlob* blob,
                                        SkArenaAlloc* alloc) {
    size_t vertexCount = drawables.size();
    SkRect bounds = SkRectPriv::MakeLargestInverted();
    auto initializer = [&, strikeToSource=strikeSpec.strikeToSourceRatio()](size_t i) {
        auto [variant, pos] = drawables[i];
        SkGlyph* skGlyph = variant;
        int16_t l = skGlyph->left();
        int16_t t = skGlyph->top();
        int16_t r = l + skGlyph->width();
        int16_t b = t + skGlyph->height();
        SkPoint lt = SkPoint::Make(l, t) * strikeToSource + pos,
                rb = SkPoint::Make(r, b) * strikeToSource + pos;

        bounds.joinPossiblyEmptyRect(SkRect::MakeLTRB(lt.x(), lt.y(), rb.x(), rb.y()));
        return VertexData{pos, {l, t, r, b}};
    };

    SkSpan<VertexData> vertexData{
            alloc->makeInitializedArray<VertexData>(vertexCount, initializer), vertexCount};

    GrAtlasSubRun* subRun = alloc->make<GrTransformedMaskSubRun>(
            format, blob, bounds, vertexData,
            GrGlyphVector::Make(strikeSpec, drawables.get<0>(), alloc));

    return subRun;
}

void GrTransformedMaskSubRun::draw(const GrClip* clip,
                                   const SkMatrixProvider& viewMatrix,
                                   const SkGlyphRunList& glyphRunList,
                                   GrRenderTargetContext* rtc) const {
    auto[drawingClip, op] = this->makeAtlasTextOp(clip, viewMatrix, glyphRunList, rtc);
    if (op != nullptr) {
        rtc->priv().addDrawOp(drawingClip, std::move(op));
    }
}

std::tuple<const GrClip*, std::unique_ptr<GrDrawOp>>
GrTransformedMaskSubRun::makeAtlasTextOp(const GrClip* clip,
                                         const SkMatrixProvider& viewMatrix,
                                         const SkGlyphRunList& glyphRunList,
                                         GrRenderTargetContext* rtc) const {
    SkASSERT(this->glyphCount() != 0);

    SkPoint drawOrigin = glyphRunList.origin();
    const SkPaint& drawPaint = glyphRunList.paint();
    const SkMatrix& drawMatrix = viewMatrix.localToDevice();
    GrOpMemoryPool* pool = rtc->priv().recordingContext()->priv().opMemoryPool();

    GrPaint grPaint;
    SkPMColor4f drawingColor = calculate_colors(rtc, drawPaint, viewMatrix, fMaskFormat, &grPaint);

    // We can clip geometrically using clipRect and ignore clip if we're not using SDFs or
    // transformed glyphs, and we have an axis-aligned rectangular non-AA clip.
    GrAtlasTextOp::Geometry geometry = {
            *this,
            drawMatrix,
            drawOrigin,
            SkIRect::MakeEmpty(),
            SkRef(fBlob),
            drawingColor
    };

    std::unique_ptr<GrDrawOp> op = pool->allocate<GrAtlasTextOp>(
            op_mask_type(fMaskFormat),
            true,
            this->glyphCount(),
            this->deviceRect(drawMatrix, drawOrigin),
            geometry,
            std::move(grPaint));
    return {clip, std::move(op)};
}

std::tuple<bool, int> GrTransformedMaskSubRun::regenerateAtlas(int begin, int end,
                                                               GrMeshDrawOp::Target* target) const {
    return fGlyphs.regenerateAtlas(begin, end, fMaskFormat, 1, target, true);
}

template<typename Quad, typename VertexData>
static void fill_transformed_vertices_2D(SkZip<Quad, const GrGlyph*, const VertexData> quadData,
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
static void fill_transformed_vertices_3D(SkZip<Quad, const GrGlyph*, const VertexData> quadData,
                                         SkScalar dstPadding,
                                         SkScalar strikeToSource,
                                         GrColor color,
                                         const SkMatrix& matrix) {
    SkPoint inset = {dstPadding, dstPadding};
    auto mapXYZ = [&](SkScalar x, SkScalar y) {
        SkPoint pt{x, y};
        SkPoint3 result;
        matrix.mapHomogeneousPoints(&result, &pt, 1);
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

void GrTransformedMaskSubRun::fillVertexData(void* vertexDst,
                                             int offset, int count,
                                             GrColor color,
                                             const SkMatrix& drawMatrix, SkPoint drawOrigin,
                                             SkIRect clip) const {
    constexpr SkScalar kDstPadding = 0.f;
    SkMatrix matrix = drawMatrix;
    matrix.preTranslate(drawOrigin.x(), drawOrigin.y());

    auto quadData = [&](auto dst) {
        return SkMakeZip(dst,
                         fGlyphs.glyphs().subspan(offset, count),
                         fVertexData.subspan(offset, count));
    };

    if (!this->hasW()) {
        if (fMaskFormat == GrMaskFormat::kARGB_GrMaskFormat) {
            using Quad = ARGB2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride() * kVerticesPerGlyph);
            fill_transformed_vertices_2D(
                    quadData((Quad*) vertexDst),
                    kDstPadding,
                    fGlyphs.strikeToSourceRatio(),
                    color,
                    matrix);
        } else {
            using Quad = Mask2DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride() * kVerticesPerGlyph);
            fill_transformed_vertices_2D(
                    quadData((Quad*) vertexDst),
                    kDstPadding,
                    fGlyphs.strikeToSourceRatio(),
                    color,
                    matrix);
        }
    } else {
        if (fMaskFormat == GrMaskFormat::kARGB_GrMaskFormat) {
            using Quad = ARGB3DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride() * kVerticesPerGlyph);
            fill_transformed_vertices_3D(
                    quadData((Quad*) vertexDst),
                    kDstPadding,
                    fGlyphs.strikeToSourceRatio(),
                    color,
                    matrix);
        } else {
            using Quad = Mask3DVertex[4];
            SkASSERT(sizeof(Quad) == this->vertexStride() * kVerticesPerGlyph);
            fill_transformed_vertices_3D(
                    quadData((Quad*) vertexDst),
                    kDstPadding,
                    fGlyphs.strikeToSourceRatio(),
                    color,
                    matrix);
        }
    }
}

size_t GrTransformedMaskSubRun::vertexStride() const {
    switch (fMaskFormat) {
        case kA8_GrMaskFormat:
            return this->hasW() ? sizeof(Mask3DVertex) : sizeof(Mask2DVertex);
        case kARGB_GrMaskFormat:
            return this->hasW() ? sizeof(ARGB3DVertex) : sizeof(ARGB2DVertex);
        default:
            SkASSERT(!this->hasW());
            return sizeof(Mask2DVertex);
    }
    SkUNREACHABLE;
}

int GrTransformedMaskSubRun::glyphCount() const {
    return fVertexData.count();
}

bool GrTransformedMaskSubRun::hasW() const {
    return fBlob->hasPerspective();
}

SkRect GrTransformedMaskSubRun::deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const {
    SkRect outBounds = fVertexBounds;
    outBounds.offset(drawOrigin);
    return drawMatrix.mapRect(outBounds);
}

// -- GrSDFTSubRun ---------------------------------------------------------------------------------
GrSDFTSubRun::GrSDFTSubRun(GrMaskFormat format,
                           GrTextBlob* textBlob,
                           SkRect vertexBounds,
                           SkSpan<const VertexData> vertexData,
                           GrGlyphVector glyphs,
                           bool useLCDText,
                           bool antiAliased)
        : fMaskFormat{format}
        , fBlob{textBlob}
        , fVertexBounds{vertexBounds}
        , fVertexData{vertexData}
        , fGlyphs{glyphs}
        , fUseLCDText{useLCDText}
        , fAntiAliased{antiAliased} { }

GrSubRun* GrSDFTSubRun::Make(
        const SkZip<SkGlyphVariant, SkPoint>& drawables,
        const SkFont& runFont,
        const SkStrikeSpec& strikeSpec,
        GrTextBlob* blob,
        SkArenaAlloc* alloc) {

    size_t vertexCount = drawables.size();
    SkRect bounds = SkRectPriv::MakeLargestInverted();
    auto initializer = [&, strikeToSource=strikeSpec.strikeToSourceRatio()](size_t i) {
        auto [variant, pos] = drawables[i];
        SkGlyph* skGlyph = variant;
        int16_t l = skGlyph->left();
        int16_t t = skGlyph->top();
        int16_t r = l + skGlyph->width();
        int16_t b = t + skGlyph->height();
        SkPoint lt = SkPoint::Make(l, t) * strikeToSource + pos,
                rb = SkPoint::Make(r, b) * strikeToSource + pos;

        bounds.joinPossiblyEmptyRect(SkRect::MakeLTRB(lt.x(), lt.y(), rb.x(), rb.y()));
        return VertexData{pos, {l, t, r, b}};
    };

    SkSpan<VertexData> vertexData{
            alloc->makeInitializedArray<VertexData>(vertexCount, initializer), vertexCount};

    return alloc->make<GrSDFTSubRun>(
            kA8_GrMaskFormat,
            blob,
            bounds,
            vertexData,
            GrGlyphVector::Make(strikeSpec, drawables.get<0>(), alloc),
            runFont.getEdging() == SkFont::Edging::kSubpixelAntiAlias,
            runFont.hasSomeAntiAliasing());
}

std::tuple<const GrClip*, std::unique_ptr<GrDrawOp> >
GrSDFTSubRun::makeAtlasTextOp(const GrClip* clip,
                              const SkMatrixProvider& viewMatrix,
                              const SkGlyphRunList& glyphRunList,
                              GrRenderTargetContext* rtc) const {
    SkASSERT(this->glyphCount() != 0);

    SkPoint drawOrigin = glyphRunList.origin();
    const SkPaint& drawPaint = glyphRunList.paint();
    const SkMatrix& drawMatrix = viewMatrix.localToDevice();
    GrOpMemoryPool* pool = rtc->priv().recordingContext()->priv().opMemoryPool();

    GrPaint grPaint;
    SkPMColor4f drawingColor = calculate_colors(rtc, drawPaint, viewMatrix, fMaskFormat, &grPaint);

    const GrColorInfo& colorInfo = rtc->colorInfo();
    const SkSurfaceProps& props = rtc->surfaceProps();
    bool isBGR = SkPixelGeometryIsBGR(props.pixelGeometry());
    bool isLCD = fUseLCDText && SkPixelGeometryIsH(props.pixelGeometry());
    using MT = GrAtlasTextOp::MaskType;
    MT maskType = !fAntiAliased ? MT::kAliasedDistanceField_MaskType
                                : isLCD ? (isBGR ? MT::kLCDBGRDistanceField_MaskType
                                                 : MT::kLCDDistanceField_MaskType)
                                        : MT::kGrayscaleDistanceField_MaskType;

    bool useGammaCorrectDistanceTable = colorInfo.isLinearlyBlended();
    uint32_t DFGPFlags = drawMatrix.isSimilarity() ? kSimilarity_DistanceFieldEffectFlag : 0;
    DFGPFlags |= drawMatrix.isScaleTranslate() ? kScaleOnly_DistanceFieldEffectFlag : 0;
    DFGPFlags |= drawMatrix.hasPerspective() ? kPerspective_DistanceFieldEffectFlag : 0;
    DFGPFlags |= useGammaCorrectDistanceTable ? kGammaCorrect_DistanceFieldEffectFlag : 0;
    DFGPFlags |= MT::kAliasedDistanceField_MaskType == maskType ?
                 kAliased_DistanceFieldEffectFlag : 0;

    if (isLCD) {
        DFGPFlags |= kUseLCD_DistanceFieldEffectFlag;
        DFGPFlags |= MT::kLCDBGRDistanceField_MaskType == maskType ?
                     kBGR_DistanceFieldEffectFlag : 0;
    }

    GrAtlasTextOp::Geometry geometry = {
            *this,
            drawMatrix,
            drawOrigin,
            SkIRect::MakeEmpty(),
            SkRef(fBlob),
            drawingColor
    };

    std::unique_ptr<GrDrawOp> op = pool->allocate<GrAtlasTextOp>(
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

void GrSDFTSubRun::draw(const GrClip* clip,
                        const SkMatrixProvider& viewMatrix,
                        const SkGlyphRunList& glyphRunList,
                        GrRenderTargetContext* rtc) const {
    auto[drawingClip, op] = this->makeAtlasTextOp(clip, viewMatrix, glyphRunList, rtc);
    if (op != nullptr) {
        rtc->priv().addDrawOp(drawingClip, std::move(op));
    }
}

std::tuple<bool, int> GrSDFTSubRun::regenerateAtlas(
        int begin, int end, GrMeshDrawOp::Target *target) const {

    return fGlyphs.regenerateAtlas(begin, end, fMaskFormat, SK_DistanceFieldInset, target);
}

size_t GrSDFTSubRun::vertexStride() const {
    return this->hasW() ? sizeof(Mask3DVertex) : sizeof(Mask2DVertex);
}

void GrSDFTSubRun::fillVertexData(
        void *vertexDst, int offset, int count,
        GrColor color, const SkMatrix& drawMatrix, SkPoint drawOrigin, SkIRect clip) const {

    SkMatrix matrix = drawMatrix;
    matrix.preTranslate(drawOrigin.x(), drawOrigin.y());

    auto quadData = [&](auto dst) {
        return SkMakeZip(dst,
                         fGlyphs.glyphs().subspan(offset, count),
                         fVertexData.subspan(offset, count));
    };

    if (!this->hasW()) {
        using Quad = Mask2DVertex[4];
        SkASSERT(sizeof(Quad) == this->vertexStride() * kVerticesPerGlyph);
        fill_transformed_vertices_2D(
                quadData((Quad*) vertexDst),
                SK_DistanceFieldInset,
                fGlyphs.strikeToSourceRatio(),
                color,
                matrix);
    } else {
        using Quad = Mask3DVertex[4];
        SkASSERT(sizeof(Quad) == this->vertexStride() * kVerticesPerGlyph);
        fill_transformed_vertices_3D(
                quadData((Quad*) vertexDst),
                SK_DistanceFieldInset,
                fGlyphs.strikeToSourceRatio(),
                color,
                matrix);
    }
}

int GrSDFTSubRun::glyphCount() const {
    return fVertexData.count();
}

bool GrSDFTSubRun::hasW() const {
        return fBlob->hasPerspective();
}

SkRect GrSDFTSubRun::deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const {
    SkRect outBounds = fVertexBounds;
    outBounds.offset(drawOrigin);
    return drawMatrix.mapRect(outBounds);
}

// -- GrTextBlob -----------------------------------------------------------------------------------
void GrTextBlob::operator delete(void* p) { ::operator delete(p); }
void* GrTextBlob::operator new(size_t) { SK_ABORT("All blobs are created by placement new."); }
void* GrTextBlob::operator new(size_t, void* p) { return p; }

GrTextBlob::~GrTextBlob() = default;

sk_sp<GrTextBlob> GrTextBlob::Make(const SkGlyphRunList& glyphRunList, const SkMatrix& drawMatrix) {
    // The difference in alignment from the storage of VertexData to SubRun;
    using AllSubRuns = std::aligned_union_t<1,
            GrDirectMaskSubRun,
            GrTransformedMaskSubRun,
            GrSDFTSubRun,
            GrPathSubRun>;

    using AllVertexData = std::aligned_union<1,
            GrDirectMaskSubRun::VertexData,
            GrTransformedMaskSubRun::VertexData,
            GrSDFTSubRun::VertexData>;
    constexpr size_t alignDiff = alignof(AllSubRuns) - alignof(AllVertexData);
    constexpr size_t vertexDataToSubRunPadding = alignDiff > 0 ? alignDiff : 0;
    size_t totalGlyphCount = glyphRunList.totalGlyphCount();
    size_t arenaSize =
            totalGlyphCount * sizeof(AllVertexData)
            + GrGlyphVector::GlyphVectorSize(totalGlyphCount)
            + glyphRunList.runCount() * (sizeof(AllSubRuns) + vertexDataToSubRunPadding)
            + 32;  // Misc arena overhead.

    size_t allocationSize = sizeof(GrTextBlob) + arenaSize;

    void* allocation = ::operator new (allocationSize);

    SkColor initialLuminance = SkPaintPriv::ComputeLuminanceColor(glyphRunList.paint());
    sk_sp<GrTextBlob> blob{new (allocation) GrTextBlob{
            arenaSize, drawMatrix, glyphRunList.origin(), initialLuminance}};

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

bool GrTextBlob::canReuse(const SkPaint& paint,
                          const SkMaskFilterBase::BlurRec& blurRec,
                          const SkMatrix& drawMatrix,
                          SkPoint drawOrigin) {
    // A singular matrix will create a GrTextBlob with no SubRuns, but unknown glyphs can
    // also cause empty runs. If there are no subRuns, and the matrix is complicated, then
    // regenerate.
    if (fSubRunList.isEmpty() && !fInitialMatrix.rectStaysRect()) {
        return false;
    }

    // If we have LCD text then our canonical color will be set to transparent, in this case we have
    // to regenerate the blob on any color change
    // We use the grPaint to get any color filter effects
    if (fKey.fCanonicalColor == SK_ColorTRANSPARENT &&
        fInitialLuminance != SkPaintPriv::ComputeLuminanceColor(paint)) {
        return false;
    }

    if (fInitialMatrix.hasPerspective() != drawMatrix.hasPerspective()) {
        return false;
    }

    /** This could be relaxed for blobs with only distance field glyphs. */
    if (fInitialMatrix.hasPerspective() && !SkMatrixPriv::CheapEqual(fInitialMatrix, drawMatrix)) {
        return false;
    }

    // We only cache one masked version
    if (fKey.fHasBlur &&
        (fBlurRec.fSigma != blurRec.fSigma || fBlurRec.fStyle != blurRec.fStyle)) {
        return false;
    }

    // Similarly, we only cache one version for each style
    if (fKey.fStyle != SkPaint::kFill_Style &&
        (fStrokeInfo.fFrameWidth != paint.getStrokeWidth() ||
         fStrokeInfo.fMiterLimit != paint.getStrokeMiter() ||
         fStrokeInfo.fJoin != paint.getStrokeJoin())) {
        return false;
    }

    // Mixed blobs must be regenerated.  We could probably figure out a way to do integer scrolls
    // for mixed blobs if this becomes an issue.
    if (this->hasBitmap() && this->hasDistanceField()) {
        // Identical view matrices and we can reuse in all cases
        return SkMatrixPriv::CheapEqual(fInitialMatrix, drawMatrix) && drawOrigin == fInitialOrigin;
    }

    if (this->hasBitmap()) {
        if (fInitialMatrix.getScaleX() != drawMatrix.getScaleX() ||
            fInitialMatrix.getScaleY() != drawMatrix.getScaleY() ||
            fInitialMatrix.getSkewX() != drawMatrix.getSkewX() ||
            fInitialMatrix.getSkewY() != drawMatrix.getSkewY()) {
            return false;
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
            return false;
        }
    } else if (this->hasDistanceField()) {
        // A scale outside of [blob.fMaxMinScale, blob.fMinMaxScale] would result in a different
        // distance field being generated, so we have to regenerate in those cases
        SkScalar newMaxScale = drawMatrix.getMaxScale();
        SkScalar oldMaxScale = fInitialMatrix.getMaxScale();
        SkScalar scaleAdjust = newMaxScale / oldMaxScale;
        if (scaleAdjust < fMaxMinScale || scaleAdjust > fMinMaxScale) {
            return false;
        }
    }

    // If the blob is all paths, there is no reason to regenerate.
    return true;
}

const GrTextBlob::Key& GrTextBlob::key() const { return fKey; }
size_t GrTextBlob::size() const { return fSize; }

template<typename AddSingleMaskFormat>
void GrTextBlob::addMultiMaskFormat(
        AddSingleMaskFormat addSingle,
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
            GrSubRun* subRun = addSingle(sameFormat, strikeSpec, format, this, &fAlloc);
            this->insertSubRun(subRun);
            format = nextFormat;
            startIndex = i;
        }
    }
    auto sameFormat = drawables.last(drawables.size() - startIndex);
    GrSubRun* subRun = addSingle(sameFormat, strikeSpec, format, this, &fAlloc);
    this->insertSubRun(subRun);
}

GrTextBlob::GrTextBlob(size_t allocSize,
                       const SkMatrix& drawMatrix,
                       SkPoint origin,
                       SkColor initialLuminance)
        : fSize{allocSize}
        , fInitialMatrix{drawMatrix}
        , fInitialOrigin{origin}
        , fInitialLuminance{initialLuminance}
        , fAlloc{SkTAddOffset<char>(this, sizeof(GrTextBlob)), allocSize, allocSize/2} { }

void GrTextBlob::insertSubRun(GrSubRun* subRun) {
    fSubRunList.addToTail(subRun);
}

void GrTextBlob::processDeviceMasks(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                    const SkStrikeSpec& strikeSpec) {

    this->addMultiMaskFormat(GrDirectMaskSubRun::Make, drawables, strikeSpec);
}

void GrTextBlob::processSourcePaths(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                    const SkFont& runFont,
                                    const SkStrikeSpec& strikeSpec) {
    this->setHasBitmap();
    GrSubRun* subRun = GrPathSubRun::Make(drawables,
                                          runFont.hasSomeAntiAliasing(),
                                          strikeSpec,
                                          &fAlloc);
    this->insertSubRun(subRun);
}

void GrTextBlob::processSourceSDFT(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                   const SkStrikeSpec& strikeSpec,
                                   const SkFont& runFont,
                                   SkScalar minScale,
                                   SkScalar maxScale) {
    this->setHasDistanceField();
    this->setMinAndMaxScale(minScale, maxScale);
    GrSubRun* subRun = GrSDFTSubRun::Make(drawables, runFont, strikeSpec, this, &fAlloc);
    this->insertSubRun(subRun);
}

void GrTextBlob::processSourceMasks(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                    const SkStrikeSpec& strikeSpec) {
    this->addMultiMaskFormat(GrTransformedMaskSubRun::Make, drawables, strikeSpec);
}
