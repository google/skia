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
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkStrikeSpec.h"
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
                           GrMaskFormat format, SkRect vertexBounds,
                           const SkSpan<VertexData>& vertexData)
        : fBlob{textBlob}
        , fType{type}
        , fMaskFormat{format}
        , fStrikeSpec{strikeSpec}
        , fVertexBounds{vertexBounds}
        , fVertexData{vertexData} {
    SkASSERT(fType != kTransformedPath);
}

GrTextBlob::SubRun::SubRun(GrTextBlob* textBlob, const SkStrikeSpec& strikeSpec)
        : fBlob{textBlob}
        , fType{kTransformedPath}
        , fMaskFormat{kA8_GrMaskFormat}
        , fStrikeSpec{strikeSpec}
        , fVertexBounds{SkRect::MakeEmpty()}
        , fVertexData{SkSpan<VertexData>{}} { }

void GrTextBlob::SubRun::resetBulkUseToken() { fBulkUseToken.reset(); }

GrDrawOpAtlas::BulkUseTokenUpdater* GrTextBlob::SubRun::bulkUseToken() { return &fBulkUseToken; }
GrMaskFormat GrTextBlob::SubRun::maskFormat() const { return fMaskFormat; }

size_t GrTextBlob::SubRun::vertexStride() const {
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

size_t GrTextBlob::SubRun::quadOffset(size_t index) const {
    return index * kVerticesPerGlyph * this->vertexStride();
}

template <typename Rect>
static auto ltbr(const Rect& r) {
    return std::make_tuple(r.left(), r.top(), r.right(), r.bottom());
}

void GrTextBlob::SubRun::fillVertexData(
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
        case kTransformedPath:
            SK_ABORT("Paths don't generate vertex data.");
    }
}

int GrTextBlob::SubRun::glyphCount() const {
    return fVertexData.count();
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

int GrTextBlob::SubRun::atlasPadding() const {
    return SkTo<int>(this->needsPadding());
}

auto GrTextBlob::SubRun::vertexData() const -> SkSpan<const VertexData> {
    return fVertexData;
}

bool GrTextBlob::SubRun::hasW() const {
    if (fType == kTransformedSDFT || fType == kTransformedMask || fType == kTransformedPath) {
        return fBlob->hasPerspective();
    }

    // The viewMatrix is implicitly SkMatrix::I when drawing kDirectMask, because it is not
    // used.
    return false;
}

void GrTextBlob::SubRun::prepareGrGlyphs(GrStrikeCache* strikeCache) {
    if (fStrike) {
        return;
    }

    fStrike = fStrikeSpec.findOrCreateGrStrike(strikeCache);

    for (auto& tmp : fVertexData) {
        tmp.glyph.grGlyph = fStrike->getGlyph(tmp.glyph.packedGlyphID);
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

GrGlyph* GrTextBlob::SubRun::grGlyph(int i) const {
    return fVertexData[i].glyph.grGlyph;
}

void GrTextBlob::SubRun::setUseLCDText(bool useLCDText) { fUseLCDText = useLCDText; }
bool GrTextBlob::SubRun::hasUseLCDText() const { return fUseLCDText; }
void GrTextBlob::SubRun::setAntiAliased(bool antiAliased) { fAntiAliased = antiAliased; }
bool GrTextBlob::SubRun::isAntiAliased() const { return fAntiAliased; }
const SkStrikeSpec& GrTextBlob::SubRun::strikeSpec() const { return fStrikeSpec; }

auto GrTextBlob::SubRun::MakePaths(
        const SkZip<SkGlyphVariant, SkPoint>& drawables,
        const SkFont& runFont,
        const SkStrikeSpec& strikeSpec,
        GrTextBlob* blob,
        SkArenaAlloc* alloc) -> SubRun* {
    SubRun* subRun = alloc->make<SubRun>(blob, strikeSpec);
    subRun->setAntiAliased(runFont.hasSomeAntiAliasing());
    for (auto [variant, pos] : drawables) {
        subRun->fPaths.emplace_back(*variant.path(), pos);
    }
    return subRun;
};

auto GrTextBlob::SubRun::MakeSDFT(
        const SkZip<SkGlyphVariant, SkPoint>& drawables,
        const SkFont& runFont,
        const SkStrikeSpec& strikeSpec,
        GrTextBlob* blob,
        SkArenaAlloc* alloc) -> SubRun* {
    SubRun* subRun = SubRun::InitForAtlas(
            kTransformedSDFT, drawables, strikeSpec, kA8_GrMaskFormat, blob, alloc);
    subRun->setUseLCDText(runFont.getEdging() == SkFont::Edging::kSubpixelAntiAlias);
    subRun->setAntiAliased(runFont.hasSomeAntiAliasing());
    return subRun;
}

auto GrTextBlob::SubRun::MakeDirectMask(
        const SkZip<SkGlyphVariant, SkPoint>& drawables,
        const SkStrikeSpec& strikeSpec,
        GrMaskFormat format,
        GrTextBlob* blob,
        SkArenaAlloc* alloc) -> SubRun* {
    return SubRun::InitForAtlas(kDirectMask, drawables, strikeSpec, format, blob, alloc);
}

auto GrTextBlob::SubRun::MakeTransformedMask(
        const SkZip<SkGlyphVariant, SkPoint>& drawables,
        const SkStrikeSpec& strikeSpec,
        GrMaskFormat format,
        GrTextBlob* blob,
        SkArenaAlloc* alloc) -> SubRun* {
    return SubRun::InitForAtlas(kTransformedMask, drawables, strikeSpec, format, blob, alloc);
}

void GrTextBlob::SubRun::insertSubRunOpsIntoTarget(GrTextTarget* target,
                                                   const SkSurfaceProps& props,
                                                   const SkPaint& paint,
                                                   const GrClip* clip,
                                                   const SkMatrixProvider& deviceMatrix,
                                                   SkPoint drawOrigin) {
    if (this->drawAsPaths()) {
        SkPaint runPaint{paint};
        runPaint.setAntiAlias(this->isAntiAliased());
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
            for (const auto& pathPos : this->fPaths) {
                const SkPath& path = pathPos.fPath;
                const SkPoint pos = pathPos.fOrigin;  // Transform the glyph to source space.
                SkMatrix pathMatrix = strikeToSource;
                pathMatrix.postTranslate(pos.x(), pos.y());
                SkPreConcatMatrixProvider strikeToDevice(deviceMatrix, pathMatrix);

                GrStyledShape shape(path, paint);
                target->drawShape(clip, runPaint, strikeToDevice, shape);
            }
        } else {
            // Transform the path to device because the deviceMatrix must be unchanged to
            // draw effect, filter or shader paths.
            for (const auto& pathPos : this->fPaths) {
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
        int glyphCount = this->glyphCount();
        if (0 == glyphCount) {
            return;
        }

        bool skipClip = false;
        SkIRect clipRect = SkIRect::MakeEmpty();
        SkRect rtBounds = SkRect::MakeWH(target->width(), target->height());
        SkRRect clipRRect = SkRRect::MakeRect(rtBounds);
        GrAA aa;
        // We can clip geometrically if we're not using SDFs or transformed glyphs,
        // and we have an axis-aligned rectangular non-AA clip
        if (!this->drawAsDistanceFields() &&
            !this->needsTransform() &&
            (!clip || (clip->isRRect(&clipRRect, &aa) &&
                       clipRRect.isRect() && GrAA::kNo == aa))) {
            // We only need to do clipping work if the subrun isn't contained by the clip
            SkRect subRunBounds = this->deviceRect(deviceMatrix.localToDevice(), drawOrigin);
            if (!clipRRect.getBounds().contains(subRunBounds)) {
                // If the subrun is completely outside, don't add an op for it
                if (!clipRRect.getBounds().intersects(subRunBounds)) {
                    return;
                } else {
                    clipRRect.getBounds().round(&clipRect);
                }
            }
            skipClip = true;
        }

        auto op = this->makeOp(deviceMatrix, drawOrigin, clipRect, paint, props, target);
        if (op != nullptr) {
            target->addDrawOp(skipClip ? nullptr : clip, std::move(op));
        }
    }
}

SkPMColor4f generate_filtered_color(const SkPaint& paint, const GrColorInfo& colorInfo) {
    SkColor4f c = paint.getColor4f();
    if (auto* xform = colorInfo.colorSpaceXformFromSRGB()) {
        c = xform->apply(c);
    }
    if (auto* cf = paint.getColorFilter()) {
        c = cf->filterColor4f(c, colorInfo.colorSpace(), colorInfo.colorSpace());
    }
    return c.premul();
}

std::unique_ptr<GrAtlasTextOp> GrTextBlob::SubRun::makeOp(const SkMatrixProvider& matrixProvider,
                                                  SkPoint drawOrigin,
                                                  const SkIRect& clipRect,
                                                  const SkPaint& paint,
                                                  const SkSurfaceProps& props,
                                                  GrTextTarget* target) {
    GrPaint grPaint;
    target->makeGrPaint(this->maskFormat(), paint, matrixProvider, &grPaint);
    const GrColorInfo& colorInfo = target->colorInfo();
    // This is the color the op will use to draw.
    SkPMColor4f drawingColor = generate_filtered_color(paint, colorInfo);

    if (this->drawAsDistanceFields()) {
        // TODO: Can we be even smarter based on the dest transfer function?
        return GrAtlasTextOp::MakeDistanceField(target->getContext(),
                                                std::move(grPaint),
                                                this,
                                                matrixProvider.localToDevice(),
                                                drawOrigin,
                                                clipRect,
                                                drawingColor,
                                                target->colorInfo().isLinearlyBlended(),
                                                SkPaintPriv::ComputeLuminanceColor(paint),
                                                props);
    } else {
        return GrAtlasTextOp::MakeBitmap(target->getContext(),
                                         std::move(grPaint),
                                         this,
                                         matrixProvider.localToDevice(),
                                         drawOrigin,
                                         clipRect,
                                         drawingColor);
    }
}

auto GrTextBlob::SubRun::InitForAtlas(SubRunType type,
                                      const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                      const SkStrikeSpec& strikeSpec,
                                      GrMaskFormat format,
                                      GrTextBlob* blob,
                                      SkArenaAlloc* alloc) -> SubRun* {
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

    SubRun* subRun = alloc->make<SubRun>(type, blob, strikeSpec, format, bounds, vertexData);

    return subRun;
}


// -- GrTextBlob -----------------------------------------------------------------------------------
void GrTextBlob::operator delete(void* p) { ::operator delete(p); }
void* GrTextBlob::operator new(size_t) { SK_ABORT("All blobs are created by placement new."); }
void* GrTextBlob::operator new(size_t, void* p) { return p; }

GrTextBlob::~GrTextBlob() = default;

sk_sp<GrTextBlob> GrTextBlob::Make(const SkGlyphRunList& glyphRunList, const SkMatrix& drawMatrix) {
    // The difference in alignment from the storage of VertexData to SubRun;
    constexpr size_t alignDiff = alignof(SubRun) - alignof(SubRun::VertexData);
    constexpr size_t vertexDataToSubRunPadding = alignDiff > 0 ? alignDiff : 0;
    size_t arenaSize = sizeof(SubRun::VertexData) *  glyphRunList.totalGlyphCount()
                     + glyphRunList.runCount() * (sizeof(SubRun) + vertexDataToSubRunPadding);

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

void GrTextBlob::insertOpsIntoTarget(GrTextTarget* target,
                                     const SkSurfaceProps& props,
                                     const SkPaint& paint,
                                     const GrClip* clip,
                                     const SkMatrixProvider& deviceMatrix,
                                     SkPoint drawOrigin) {
    for (SubRun* subRun = fFirstSubRun; subRun != nullptr; subRun = subRun->fNextSubRun) {
        subRun->insertSubRunOpsIntoTarget(target, props, paint, clip, deviceMatrix, drawOrigin);
    }
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
            SubRun* subRun = addSingle(sameFormat, strikeSpec, format, this, &fAlloc);
            this->insertSubRun(subRun);
            format = nextFormat;
            startIndex = i;
        }
    }
    auto sameFormat = drawables.last(drawables.size() - startIndex);
    SubRun* subRun = addSingle(sameFormat, strikeSpec, format, this, &fAlloc);
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

void GrTextBlob::insertSubRun(SubRun* subRun) {
    if (fFirstSubRun == nullptr) {
        fFirstSubRun = subRun;
        fLastSubRun = subRun;
    } else {
        fLastSubRun->fNextSubRun = subRun;
        fLastSubRun = subRun;
    }
}

void GrTextBlob::processDeviceMasks(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                    const SkStrikeSpec& strikeSpec) {

    this->addMultiMaskFormat(SubRun::MakeDirectMask, drawables, strikeSpec);
}

void GrTextBlob::processSourcePaths(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                    const SkFont& runFont,
                                    const SkStrikeSpec& strikeSpec) {
    this->setHasBitmap();
    SubRun* subRun = SubRun::MakePaths(drawables, runFont, strikeSpec, this, &fAlloc);
    this->insertSubRun(subRun);
}

void GrTextBlob::processSourceSDFT(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                   const SkStrikeSpec& strikeSpec,
                                   const SkFont& runFont,
                                   SkScalar minScale,
                                   SkScalar maxScale) {
    this->setHasDistanceField();
    this->setMinAndMaxScale(minScale, maxScale);
    SubRun* subRun = SubRun::MakeSDFT(drawables, runFont, strikeSpec, this, &fAlloc);
    this->insertSubRun(subRun);
}

void GrTextBlob::processSourceMasks(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                    const SkStrikeSpec& strikeSpec) {
    this->addMultiMaskFormat(SubRun::MakeTransformedMask, drawables, strikeSpec);
}

auto GrTextBlob::firstSubRun() const -> SubRun* { return fFirstSubRun; }

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

    SkBulkGlyphMetricsAndImages metricsAndImages{fSubRun->strikeSpec()};

    // Update the atlas information in the GrStrike.
    auto tokenTracker = fUploadTarget->tokenTracker();
    auto vertexData = fSubRun->vertexData().subspan(begin, end - begin);
    int glyphsPlacedInAtlas = 0;
    for (auto [glyph, pos, rect] : vertexData) {
        GrGlyph* grGlyph = glyph.grGlyph;
        SkASSERT(grGlyph != nullptr);

        if (!fFullAtlasManager->hasGlyph(fSubRun->maskFormat(), grGlyph)) {
            const SkGlyph& skGlyph = *metricsAndImages.glyph(grGlyph->fPackedID);
            auto code = fFullAtlasManager->addGlyphToAtlas(
                    skGlyph, fSubRun->atlasPadding(), grGlyph, fResourceProvider, fUploadTarget);
            if (code != GrDrawOpAtlas::ErrorCode::kSucceeded) {
                return {code != GrDrawOpAtlas::ErrorCode::kError, glyphsPlacedInAtlas};
            }
        }
        fFullAtlasManager->addGlyphToBulkAndSetUseToken(
                fSubRun->bulkUseToken(), fSubRun->maskFormat(), grGlyph,
                tokenTracker->nextDrawToken());
        glyphsPlacedInAtlas++;
    }

    return {true, glyphsPlacedInAtlas};
}

std::tuple<bool, int> GrTextBlob::VertexRegenerator::regenerate(int begin, int end) {
    uint64_t currentAtlasGen = fFullAtlasManager->atlasGeneration(fSubRun->maskFormat());

    if (fSubRun->fAtlasGeneration != currentAtlasGen) {
        // Calculate the texture coordinates for the vertexes during first use (fAtlasGeneration
        // is set to kInvalidAtlasGeneration) or the atlas has changed in subsequent calls..
        fSubRun->resetBulkUseToken();
        auto [success, glyphsPlacedInAtlas] = this->updateTextureCoordinates(begin, end);

        // Update atlas generation if there are no more glyphs to put in the atlas.
        if (success && begin + glyphsPlacedInAtlas == fSubRun->glyphCount()) {
            // Need to get the freshest value of the atlas' generation because
            // updateTextureCoordinates may have changed it.
            fSubRun->fAtlasGeneration = fFullAtlasManager->atlasGeneration(fSubRun->maskFormat());
        }

        return {success, glyphsPlacedInAtlas};
    } else {
        // The atlas hasn't changed, so our texture coordinates are still valid.
        if (end == fSubRun->glyphCount()) {
            // The atlas hasn't changed and the texture coordinates are all still valid. Update
            // all the plots used to the new use token.
            fFullAtlasManager->setUseTokenBulk(*fSubRun->bulkUseToken(),
                                               fUploadTarget->tokenTracker()->nextDrawToken(),
                                               fSubRun->maskFormat());
        }
        return {true, end - begin};
    }
}
