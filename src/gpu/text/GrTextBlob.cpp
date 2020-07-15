/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/gpu/GrContext.h"
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
                        GrRenderTargetContext* rtc) {
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
                    rtc->priv().getContext(), rtc, clip, runPaint, strikeToDevice, shape);
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
                    rtc->priv().getContext(), rtc, clip, runPaint, viewMatrix, shape);
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

// -- GrAtlasSubRun --------------------------------------------------------------------------------
GrAtlasSubRun::GrAtlasSubRun(SubRunType type, GrTextBlob* textBlob, const SkStrikeSpec& strikeSpec,
                             GrMaskFormat format, SkRect vertexBounds,
                             const SkSpan<VertexData>& vertexData)
        : fBlob{textBlob}
        , fType{type}
        , fMaskFormat{format}
        , fStrikeSpec{strikeSpec}
        , fVertexBounds{vertexBounds}
        , fVertexData{vertexData} {
}

static SkPMColor4f generate_filtered_color(const SkPaint& paint, const GrColorInfo& colorInfo) {
    SkColor4f c = paint.getColor4f();
    if (auto* xform = colorInfo.colorSpaceXformFromSRGB()) {
        c = xform->apply(c);
    }
    if (auto* cf = paint.getColorFilter()) {
        c = cf->filterColor4f(c, colorInfo.colorSpace(), colorInfo.colorSpace());
    }
    return c.premul();
}

std::tuple<const GrClip*, std::unique_ptr<GrDrawOp> >
GrAtlasSubRun::makeAtlasTextOp(const GrClip* clip,
                               const SkMatrixProvider& viewMatrix,
                               const SkGlyphRunList& glyphRunList,
                               GrRenderTargetContext* rtc) {
    SkASSERT(this->glyphCount() != 0);

    SkPoint drawOrigin = glyphRunList.origin();
    const SkPaint& drawPaint = glyphRunList.paint();
    const SkMatrix& drawMatrix = viewMatrix.localToDevice();
    GrRecordingContext* context = rtc->priv().getContext();
    GrOpMemoryPool* pool = context->priv().opMemoryPool();
    const GrColorInfo& colorInfo = rtc->colorInfo();

    // This is the color the op will use to draw.
    SkPMColor4f drawingColor = generate_filtered_color(drawPaint, colorInfo);

    GrPaint grPaint;
    if (this->maskFormat() == kARGB_GrMaskFormat) {
        SkPaintToGrPaintWithPrimitiveColor(
                context, colorInfo, drawPaint, viewMatrix, &grPaint);
    } else {
        SkPaintToGrPaint(context, colorInfo, drawPaint, viewMatrix, &grPaint);
    }

    // We can clip geometrically using clipRect and ignore clip if we're not using SDFs or
    // transformed glyphs, and we have an axis-aligned rectangular non-AA clip.
    std::unique_ptr<GrDrawOp> op;
    if (!this->drawAsDistanceFields()) {
        SkIRect clipRect = SkIRect::MakeEmpty();
        if (!this->needsTransform()) {
            // We only need to do clipping work if the SubRun isn't contained by the clip
            SkRect subRunBounds = this->deviceRect(drawMatrix, drawOrigin);
            SkRect renderTargetBounds = SkRect::MakeWH(rtc->width(), rtc->height());
            if (clip == nullptr && !renderTargetBounds.intersects(subRunBounds)) {
                // If the SubRun is completely outside, don't add an op for it.
                return {nullptr, nullptr};
            } else if (clip != nullptr) {
                GrClip::PreClipResult result = clip->preApply(subRunBounds);
                if (result.fEffect == GrClip::Effect::kClipped) {
                    if (result.fIsRRect && result.fRRect.isRect() &&
                        result.fAA == GrAA::kNo) {
                        // Clip geometrically during onPrepare using clipRect.
                        result.fRRect.getBounds().round(&clipRect);
                        clip = nullptr;
                    }
                } else if (result.fEffect == GrClip::Effect::kClippedOut) {
                    return {nullptr, nullptr};
                }
            }
        }

        if (!clipRect.isEmpty()) { SkASSERT(clip == nullptr); }

        GrAtlasTextOp::MaskType maskType = [&]() {
            switch (this->maskFormat()) {
                case kA8_GrMaskFormat: return GrAtlasTextOp::kGrayscaleCoverageMask_MaskType;
                case kA565_GrMaskFormat: return GrAtlasTextOp::kLCDCoverageMask_MaskType;
                case kARGB_GrMaskFormat: return GrAtlasTextOp::kColorBitmapMask_MaskType;
                    // Needed to placate some compilers.
                default: return GrAtlasTextOp::kGrayscaleCoverageMask_MaskType;
            }
        }();

        op = pool->allocate<GrAtlasTextOp>(maskType,
                                           std::move(grPaint),
                                           this,
                                           drawMatrix,
                                           drawOrigin,
                                           clipRect,
                                           drawingColor,
                                           0,
                                           false,
                                           0);
    } else {
        const SkSurfaceProps& props = rtc->surfaceProps();
        bool isBGR = SkPixelGeometryIsBGR(props.pixelGeometry());
        bool isLCD = this->hasUseLCDText() && SkPixelGeometryIsH(props.pixelGeometry());
        using MT = GrAtlasTextOp::MaskType;
        MT maskType = !this->isAntiAliased() ? MT::kAliasedDistanceField_MaskType
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

        op = pool->allocate<GrAtlasTextOp>(maskType,
                                           std::move(grPaint),
                                           this,
                                           drawMatrix,
                                           drawOrigin,
                                           SkIRect::MakeEmpty(),
                                           drawingColor,
                                           SkPaintPriv::ComputeLuminanceColor(drawPaint),
                                           useGammaCorrectDistanceTable,
                                           DFGPFlags);
    }

    return {clip, std::move(op)};
}

void GrAtlasSubRun::draw(const GrClip* clip,
                         const SkMatrixProvider& viewMatrix,
                         const SkGlyphRunList& glyphRunList,
                         GrRenderTargetContext* rtc) {
    auto[drawingClip, op] = this->makeAtlasTextOp(clip, viewMatrix, glyphRunList, rtc);
    if (op != nullptr) {
        rtc->priv().addDrawOp(drawingClip, std::move(op));
    }
}

std::tuple<bool, int> GrAtlasSubRun::regenerateAtlas(
        int begin, int end, GrMeshDrawOp::Target *target) {
    GrAtlasManager* atlasManager = target->atlasManager();
    GrDeferredUploadTarget* uploadTarget = target->deferredUploadTarget();

    uint64_t currentAtlasGen = atlasManager->atlasGeneration(this->maskFormat());

    if (fAtlasGeneration != currentAtlasGen) {
        // Calculate the texture coordinates for the vertexes during first use (fAtlasGeneration
        // is set to kInvalidAtlasGeneration) or the atlas has changed in subsequent calls..
        this->resetBulkUseToken();

        SkASSERT(this->isPrepared());

        SkBulkGlyphMetricsAndImages metricsAndImages{this->strikeSpec()};

        // Update the atlas information in the GrStrike.
        auto tokenTracker = uploadTarget->tokenTracker();
        auto vertexData = this->vertexData().subspan(begin, end - begin);
        int glyphsPlacedInAtlas = 0;
        bool success = true;
        for (auto [glyph, pos, rect] : vertexData) {
            GrGlyph* grGlyph = glyph.grGlyph;
            SkASSERT(grGlyph != nullptr);

            if (!atlasManager->hasGlyph(this->maskFormat(), grGlyph)) {
                const SkGlyph& skGlyph = *metricsAndImages.glyph(grGlyph->fPackedID);
                auto code = atlasManager->addGlyphToAtlas(
                        skGlyph, this->atlasPadding(), grGlyph,
                        target->resourceProvider(), uploadTarget);
                if (code != GrDrawOpAtlas::ErrorCode::kSucceeded) {
                    success = code != GrDrawOpAtlas::ErrorCode::kError;
                    break;
                }
            }
            atlasManager->addGlyphToBulkAndSetUseToken(
                    this->bulkUseToken(), this->maskFormat(), grGlyph,
                    tokenTracker->nextDrawToken());
            glyphsPlacedInAtlas++;
        }

        // Update atlas generation if there are no more glyphs to put in the atlas.
        if (success && begin + glyphsPlacedInAtlas == this->glyphCount()) {
            // Need to get the freshest value of the atlas' generation because
            // updateTextureCoordinates may have changed it.
            fAtlasGeneration = atlasManager->atlasGeneration(this->maskFormat());
        }

        return {success, glyphsPlacedInAtlas};
    } else {
        // The atlas hasn't changed, so our texture coordinates are still valid.
        if (end == this->glyphCount()) {
            // The atlas hasn't changed and the texture coordinates are all still valid. Update
            // all the plots used to the new use token.
            atlasManager->setUseTokenBulk(*this->bulkUseToken(),
                                          uploadTarget->tokenTracker()->nextDrawToken(),
                                               this->maskFormat());
        }
        return {true, end - begin};
    }
}

void GrAtlasSubRun::resetBulkUseToken() { fBulkUseToken.reset(); }

GrDrawOpAtlas::BulkUseTokenUpdater* GrAtlasSubRun::bulkUseToken() { return &fBulkUseToken; }
GrMaskFormat GrAtlasSubRun::maskFormat() const { return fMaskFormat; }

size_t GrAtlasSubRun::vertexStride() const {
    switch (this->maskFormat()) {
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

size_t GrAtlasSubRun::quadOffset(size_t index) const {
    return index * kVerticesPerGlyph * this->vertexStride();
}

template <typename Rect>
static auto ltbr(const Rect& r) {
    return std::make_tuple(r.left(), r.top(), r.right(), r.bottom());
}

void GrAtlasSubRun::fillVertexData(
        void *vertexDst, int offset, int count,
        GrColor color, const SkMatrix& drawMatrix, SkPoint drawOrigin, SkIRect clip) const {

    SkMatrix matrix = drawMatrix;
    matrix.preTranslate(drawOrigin.x(), drawOrigin.y());

    auto transformed2D = [&](auto dst, SkScalar dstPadding, SkScalar srcPadding) {
        SkScalar strikeToSource = fStrikeSpec.strikeToSourceRatio();
        SkPoint inset = {dstPadding, dstPadding};
        for (auto[quad, vertexData] : SkMakeZip(dst, fVertexData.subspan(offset, count))) {
            auto[glyph, pos, rect] = vertexData;
            auto [l, t, r, b] = rect;
            SkPoint sLT = (SkPoint::Make(l, t) + inset) * strikeToSource + pos,
                    sRB = (SkPoint::Make(r, b) - inset) * strikeToSource + pos;
            SkPoint lt = matrix.mapXY(sLT.x(), sLT.y()),
                    lb = matrix.mapXY(sLT.x(), sRB.y()),
                    rt = matrix.mapXY(sRB.x(), sLT.y()),
                    rb = matrix.mapXY(sRB.x(), sRB.y());
            auto[al, at, ar, ab] = glyph.grGlyph->fAtlasLocator.getUVs(srcPadding);
            quad[0] = {lt, color, {al, at}};  // L,T
            quad[1] = {lb, color, {al, ab}};  // L,B
            quad[2] = {rt, color, {ar, at}};  // R,T
            quad[3] = {rb, color, {ar, ab}};  // R,B
        }
    };

    auto transformed3D = [&](auto dst, SkScalar dstPadding, SkScalar srcPadding) {
        SkScalar strikeToSource = fStrikeSpec.strikeToSourceRatio();
        SkPoint inset = {dstPadding, dstPadding};
        auto mapXYZ = [&](SkScalar x, SkScalar y) {
            SkPoint pt{x, y};
            SkPoint3 result;
            matrix.mapHomogeneousPoints(&result, &pt, 1);
            return result;
        };
        for (auto[quad, vertexData] : SkMakeZip(dst, fVertexData.subspan(offset, count))) {
            auto[glyph, pos, rect] = vertexData;
            auto [l, t, r, b] = rect;
            SkPoint sLT = (SkPoint::Make(l, t) + inset) * strikeToSource + pos,
                    sRB = (SkPoint::Make(r, b) - inset) * strikeToSource + pos;
            SkPoint3 lt = mapXYZ(sLT.x(), sLT.y()),
                     lb = mapXYZ(sLT.x(), sRB.y()),
                     rt = mapXYZ(sRB.x(), sLT.y()),
                     rb = mapXYZ(sRB.x(), sRB.y());
            auto[al, at, ar, ab] = glyph.grGlyph->fAtlasLocator.getUVs(srcPadding);
            quad[0] = {lt, color, {al, at}};  // L,T
            quad[1] = {lb, color, {al, ab}};  // L,B
            quad[2] = {rt, color, {ar, at}};  // R,T
            quad[3] = {rb, color, {ar, ab}};  // R,B
        }
    };

    auto direct2D = [&](auto dst, SkIRect* clip) {
        // Rectangles in device space
        SkPoint originInDeviceSpace = matrix.mapXY(0, 0);
        for (auto[quad, vertexData] : SkMakeZip(dst, fVertexData.subspan(offset, count))) {
            auto[glyph, pos, rect] = vertexData;
            auto[l, t, r, b] = rect;
            auto[fx, fy] = pos + originInDeviceSpace;
            auto[al, at, ar, ab] = glyph.grGlyph->fAtlasLocator.getUVs(0);
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

    switch (fType) {
        case kDirectMask: {
            if (clip.isEmpty()) {
                if (this->maskFormat() != kARGB_GrMaskFormat) {
                    using Quad = Mask2DVertex[4];
                    SkASSERT(sizeof(Quad) == this->vertexStride() * kVerticesPerGlyph);
                    direct2D((Quad*) vertexDst, nullptr);
                } else {
                    using Quad = ARGB2DVertex[4];
                    SkASSERT(sizeof(Quad) == this->vertexStride() * kVerticesPerGlyph);
                    direct2D((Quad*) vertexDst, nullptr);
                }
            } else {
                if (this->maskFormat() != kARGB_GrMaskFormat) {
                    using Quad = Mask2DVertex[4];
                    SkASSERT(sizeof(Quad) == this->vertexStride() * kVerticesPerGlyph);
                    direct2D((Quad*) vertexDst, &clip);
                } else {
                    using Quad = ARGB2DVertex[4];
                    SkASSERT(sizeof(Quad) == this->vertexStride() * kVerticesPerGlyph);
                    direct2D((Quad*) vertexDst, &clip);
                }
            }
            break;
        }
        case kTransformedMask: {
            if (!this->hasW()) {
                if (this->maskFormat() == GrMaskFormat::kARGB_GrMaskFormat) {
                    using Quad = ARGB2DVertex[4];
                    SkASSERT(sizeof(Quad) == this->vertexStride() * kVerticesPerGlyph);
                    transformed2D((Quad*) vertexDst, 0, 1);
                } else {
                    using Quad = Mask2DVertex[4];
                    SkASSERT(sizeof(Quad) == this->vertexStride() * kVerticesPerGlyph);
                    transformed2D((Quad*) vertexDst, 0, 1);
                }
            } else {
                if (this->maskFormat() == GrMaskFormat::kARGB_GrMaskFormat) {
                    using Quad = ARGB3DVertex[4];
                    SkASSERT(sizeof(Quad) == this->vertexStride() * kVerticesPerGlyph);
                    transformed3D((Quad*) vertexDst, 0, 1);
                } else {
                    using Quad = Mask3DVertex[4];
                    SkASSERT(sizeof(Quad) == this->vertexStride() * kVerticesPerGlyph);
                    transformed3D((Quad*) vertexDst, 0, 1);
                }
            }
            break;
        }
        case kTransformedSDFT: {
            if (!this->hasW()) {
                using Quad = Mask2DVertex[4];
                SkASSERT(sizeof(Quad) == this->vertexStride() * kVerticesPerGlyph);
                transformed2D((Quad*) vertexDst, SK_DistanceFieldInset, SK_DistanceFieldInset);
            } else {
                using Quad = Mask3DVertex[4];
                SkASSERT(sizeof(Quad) == this->vertexStride() * kVerticesPerGlyph);
                transformed3D((Quad*) vertexDst, SK_DistanceFieldInset, SK_DistanceFieldInset);
            }
            break;
        }
    }
}

int GrAtlasSubRun::glyphCount() const {
    return fVertexData.count();
}

bool GrAtlasSubRun::drawAsDistanceFields() const { return fType == kTransformedSDFT; }

bool GrAtlasSubRun::needsTransform() const {
    return fType == kTransformedMask ||
           fType == kTransformedSDFT;
}

bool GrAtlasSubRun::needsPadding() const {
    return fType == kTransformedMask;
}

int GrAtlasSubRun::atlasPadding() const {
    return SkTo<int>(this->needsPadding());
}

auto GrAtlasSubRun::vertexData() const -> SkSpan<const VertexData> {
    return fVertexData;
}

bool GrAtlasSubRun::hasW() const {
    if (fType == kTransformedSDFT || fType == kTransformedMask) {
        return fBlob->hasPerspective();
    }

    // The viewMatrix is implicitly SkMatrix::I when drawing kDirectMask, because it is not
    // used.
    return false;
}

void GrAtlasSubRun::prepareGrGlyphs(GrStrikeCache* strikeCache) {
    if (fStrike) {
        return;
    }

    fStrike = fStrikeSpec.findOrCreateGrStrike(strikeCache);

    for (auto& tmp : fVertexData) {
        tmp.glyph.grGlyph = fStrike->getGlyph(tmp.glyph.packedGlyphID);
    }
}

SkRect GrAtlasSubRun::deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const {
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

GrGlyph* GrAtlasSubRun::grGlyph(int i) const {
    return fVertexData[i].glyph.grGlyph;
}

void GrAtlasSubRun::setUseLCDText(bool useLCDText) { fUseLCDText = useLCDText; }
bool GrAtlasSubRun::hasUseLCDText() const { return fUseLCDText; }
void GrAtlasSubRun::setAntiAliased(bool antiAliased) { fAntiAliased = antiAliased; }
bool GrAtlasSubRun::isAntiAliased() const { return fAntiAliased; }
const SkStrikeSpec& GrAtlasSubRun::strikeSpec() const { return fStrikeSpec; }

auto GrAtlasSubRun::MakeSDFT(
        const SkZip<SkGlyphVariant, SkPoint>& drawables,
        const SkFont& runFont,
        const SkStrikeSpec& strikeSpec,
        GrTextBlob* blob,
        SkArenaAlloc* alloc) -> GrSubRun* {
    GrAtlasSubRun* subRun = GrAtlasSubRun::InitForAtlas(
            kTransformedSDFT, drawables, strikeSpec, kA8_GrMaskFormat, blob, alloc);
    subRun->setUseLCDText(runFont.getEdging() == SkFont::Edging::kSubpixelAntiAlias);
    subRun->setAntiAliased(runFont.hasSomeAntiAliasing());
    return subRun;
}

auto GrAtlasSubRun::MakeDirectMask(
        const SkZip<SkGlyphVariant, SkPoint>& drawables,
        const SkStrikeSpec& strikeSpec,
        GrMaskFormat format,
        GrTextBlob* blob,
        SkArenaAlloc* alloc) -> GrSubRun* {
    return GrAtlasSubRun::InitForAtlas(kDirectMask, drawables, strikeSpec, format, blob, alloc);
}

auto GrAtlasSubRun::MakeTransformedMask(
        const SkZip<SkGlyphVariant, SkPoint>& drawables,
        const SkStrikeSpec& strikeSpec,
        GrMaskFormat format,
        GrTextBlob* blob,
        SkArenaAlloc* alloc) -> GrSubRun* {
    return GrAtlasSubRun::InitForAtlas(
            kTransformedMask, drawables, strikeSpec, format, blob, alloc);
}

auto GrAtlasSubRun::InitForAtlas(SubRunType type,
                                 const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                 const SkStrikeSpec& strikeSpec,
                                 GrMaskFormat format,
                                 GrTextBlob* blob,
                                 SkArenaAlloc* alloc) -> GrAtlasSubRun* {
    size_t vertexCount = drawables.size();
    using Data = VertexData;
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
        return Data{{skGlyph->getPackedID()}, pos, {l, t, r, b}};
    };

    SkSpan<Data> vertexData{
            alloc->makeInitializedArray<Data>(vertexCount, initializer), vertexCount};

    GrAtlasSubRun* subRun = alloc->make<GrAtlasSubRun>(
            type, blob, strikeSpec, format, bounds, vertexData);

    return subRun;
}

// -- GrTextBlob -----------------------------------------------------------------------------------
void GrTextBlob::operator delete(void* p) { ::operator delete(p); }
void* GrTextBlob::operator new(size_t) { SK_ABORT("All blobs are created by placement new."); }
void* GrTextBlob::operator new(size_t, void* p) { return p; }

GrTextBlob::~GrTextBlob() = default;

sk_sp<GrTextBlob> GrTextBlob::Make(const SkGlyphRunList& glyphRunList, const SkMatrix& drawMatrix) {
    // The difference in alignment from the storage of VertexData to SubRun;
    constexpr size_t alignDiff = alignof(GrAtlasSubRun) - alignof(GrAtlasSubRun::VertexData);
    constexpr size_t vertexDataToSubRunPadding = alignDiff > 0 ? alignDiff : 0;
    size_t arenaSize = sizeof(GrAtlasSubRun::VertexData) * glyphRunList.totalGlyphCount()
                     + glyphRunList.runCount() * (sizeof(GrAtlasSubRun) + vertexDataToSubRunPadding);

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

    this->addMultiMaskFormat(GrAtlasSubRun::MakeDirectMask, drawables, strikeSpec);
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
    GrSubRun* subRun = GrAtlasSubRun::MakeSDFT(drawables, runFont, strikeSpec, this, &fAlloc);
    this->insertSubRun(subRun);
}

void GrTextBlob::processSourceMasks(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                    const SkStrikeSpec& strikeSpec) {
    this->addMultiMaskFormat(GrAtlasSubRun::MakeTransformedMask, drawables, strikeSpec);
}

