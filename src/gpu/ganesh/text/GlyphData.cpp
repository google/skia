/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/text/GlyphData.h"

#include "include/private/base/SkAssert.h"
#include "src/base/SkZip.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkStrike.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/ganesh/GrColor.h"
#include "src/gpu/ganesh/GrMeshDrawTarget.h"
#include "src/gpu/ganesh/text/GrAtlasManager.h"
#include "src/gpu/ganesh/text/TextStrike.h"
#include "src/text/gpu/GlyphVector.h"
#include "src/text/gpu/StrikeCache.h"
#include "src/text/gpu/VertexFiller.h"

namespace {
using Glyph = skgpu::ganesh::Glyph;

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

// The 99% case. Direct Mask, No clip, No RGB.
void fillDirectNoClipping(SkZip<Mask2DVertex[4], const Glyph, const SkPoint> quadData,
                          GrColor color,
                          SkPoint originOffset) {
    for (auto [quad, glyph, leftTop] : quadData) {
        auto [al, at, ar, ab] = glyph.entry().fAtlasLocator.getUVs();
        SkScalar dl = leftTop.x() + originOffset.x(),
                 dt = leftTop.y() + originOffset.y(),
                 dr = dl + (ar - al),
                 db = dt + (ab - at);

        quad[0] = {{dl, dt}, color, {al, at}};  // L,T
        quad[1] = {{dl, db}, color, {al, ab}};  // L,B
        quad[2] = {{dr, dt}, color, {ar, at}};  // R,T
        quad[3] = {{dr, db}, color, {ar, ab}};  // R,B
    }
}

template <typename Rect>
auto LTBR(const Rect& r) {
    return std::make_tuple(r.left(), r.top(), r.right(), r.bottom());
}

// Handle any combination of BW or color and clip or no clip.
template <typename Quad, typename VertexData>
void fillDirectClipped(SkZip<Quad, const Glyph, const VertexData> quadData,
                       GrColor color,
                       SkPoint originOffset,
                       SkIRect* clip = nullptr) {
    for (auto [quad, glyph, leftTop] : quadData) {
        auto [al, at, ar, ab] = glyph.entry().fAtlasLocator.getUVs();
        uint16_t w = ar - al,
                 h = ab - at;
        SkScalar l = leftTop.x() + originOffset.x(),
                 t = leftTop.y() + originOffset.y();
        if (clip == nullptr) {
            auto [dl, dt, dr, db] = SkRect::MakeLTRB(l, t, l + w, t + h);
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
                    std::tie(dl, dt, dr, db) = LTBR(clipped);
                } else {
                    // TODO: omit generating any vertex data for fully clipped glyphs ?
                    std::tie(dl, dt, dr, db) = std::make_tuple(0, 0, 0, 0);
                    std::tie(al, at, ar, ab) = std::make_tuple(0, 0, 0, 0);
                }
            } else {
                std::tie(dl, dt, dr, db) = LTBR(devIRect);
            }
            quad[0] = {{dl, dt}, color, {al, at}};  // L,T
            quad[1] = {{dl, db}, color, {al, ab}};  // L,B
            quad[2] = {{dr, dt}, color, {ar, at}};  // R,T
            quad[3] = {{dr, db}, color, {ar, ab}};  // R,B
        }
    }
}

template <typename Quad, typename VertexData>
void fill2D(SkZip<Quad, const Glyph, const VertexData> quadData,
            GrColor color,
            const SkMatrix& viewDifference) {
    for (auto [quad, glyph, leftTop] : quadData) {
        auto [l, t] = leftTop;
        auto [r, b] = leftTop + glyph.entry().fAtlasLocator.widthHeight();
        SkPoint lt = viewDifference.mapPoint({l, t}),
                lb = viewDifference.mapPoint({l, b}),
                rt = viewDifference.mapPoint({r, t}),
                rb = viewDifference.mapPoint({r, b});
        auto [al, at, ar, ab] = glyph.entry().fAtlasLocator.getUVs();
        quad[0] = {lt, color, {al, at}};  // L,T
        quad[1] = {lb, color, {al, ab}};  // L,B
        quad[2] = {rt, color, {ar, at}};  // R,T
        quad[3] = {rb, color, {ar, ab}};  // R,B
    }
}

template <typename Quad, typename VertexData>
void fill3D(SkZip<Quad, const Glyph, const VertexData> quadData,
            GrColor color,
            const SkMatrix& viewDifference) {
    auto mapXYZ = [&](SkScalar x, SkScalar y) {
        return viewDifference.mapPointToHomogeneous({x, y});
    };
    for (auto [quad, glyph, leftTop] : quadData) {
        auto [l, t] = leftTop;
        auto [r, b] = leftTop + glyph.entry().fAtlasLocator.widthHeight();
        SkPoint3 lt = mapXYZ(l, t),
                lb = mapXYZ(l, b),
                rt = mapXYZ(r, t),
                rb = mapXYZ(r, b);
        auto [al, at, ar, ab] = glyph.entry().fAtlasLocator.getUVs();
        quad[0] = {lt, color, {al, at}};  // L,T
        quad[1] = {lb, color, {al, ab}};  // L,B
        quad[2] = {rt, color, {ar, at}};  // R,T
        quad[3] = {rb, color, {ar, ab}};  // R,B
    }
}
}  // anonymous namespace

namespace skgpu::ganesh {

GlyphData::GlyphData(sk_sp<TextStrike> strike) : fTextStrike{std::move(strike)} {}

GlyphData::~GlyphData() = default;

Glyph GlyphData::makeGlyphFromID(SkPackedGlyphID id) { return Glyph{fTextStrike->getGlyph(id)}; }

std::tuple<bool, int> GlyphData::regenerateAtlas(int begin,
                                                 int end,
                                                 sktext::gpu::GlyphVector& glyphVector,
                                                 MaskFormat maskFormat,
                                                 int srcPadding,
                                                 GrMeshDrawTarget* target) {
    SkASSERT(glyphVector.hasBackendData());
    GrAtlasManager* atlasManager = target->atlasManager();
    GrDeferredUploadTarget* uploadTarget = target->deferredUploadTarget();

    uint64_t currentAtlasGen = atlasManager->atlasGeneration(maskFormat);

    if (fAtlasGeneration != currentAtlasGen) {
        SkSpan<const Glyph> glyphSpan = glyphVector.accessBackendGlyphs<Glyph>();
        // Calculate the texture coordinates for the vertexes during first use (fAtlasGeneration
        // is set to kInvalidAtlasGeneration) or the atlas has changed in subsequent calls.
        fBulkUseUpdater.reset();

        SkBulkGlyphMetricsAndImages metricsAndImages{fTextStrike->strikeSpec()};

        // Update the atlas information in the GrStrike.
        auto tokenTracker = uploadTarget->tokenTracker();
        int glyphsPlacedInAtlas = 0;
        bool success = true;
        for (int i = begin; i < end; i++) {
            const Glyph& glyph = glyphSpan[i];

            if (!atlasManager->hasGlyph(maskFormat, glyph.entry())) {
                const SkGlyph& skGlyph = *metricsAndImages.glyph(glyph.packedID());
                auto code = atlasManager->addGlyphToAtlas(skGlyph,
                                                          &glyph.entry(),
                                                          srcPadding,
                                                          target->resourceProvider(),
                                                          uploadTarget);
                if (code != GrDrawOpAtlas::ErrorCode::kSucceeded) {
                    success = code != GrDrawOpAtlas::ErrorCode::kError;
                    break;
                }
            }
            atlasManager->addGlyphToBulkAndSetUseToken(
                    &fBulkUseUpdater, maskFormat, glyph.entry(), tokenTracker->nextDrawToken());
            glyphsPlacedInAtlas++;
        }

        // Update atlas generation if there are no more glyphs to put in the atlas.
        if (success && begin + glyphsPlacedInAtlas == glyphVector.glyphCount()) {
            // Need to get the freshest value of the atlas' generation because
            // updateTextureCoordinates may have changed it.
            fAtlasGeneration = atlasManager->atlasGeneration(maskFormat);
        }

        return {success, glyphsPlacedInAtlas};
    } else {
        // The atlas hasn't changed, so our texture coordinates are still valid.
        if (end == glyphVector.glyphCount()) {
            // The atlas hasn't changed and the texture coordinates are all still valid. Update
            // all the plots used to the new use token.
            atlasManager->setUseTokenBulk(
                    fBulkUseUpdater, uploadTarget->tokenTracker()->nextDrawToken(), maskFormat);
        }
        return {true, end - begin};
    }
}

size_t GlyphData::vertexStride(MaskFormat mf, const SkMatrix& matrix) const {
    if (mf != skgpu::MaskFormat::kARGB) {
        // For formats MaskFormat::kA565 and MaskFormat::kA8 where A8 include SDF.
        return matrix.hasPerspective() ? sizeof(Mask3DVertex) : sizeof(Mask2DVertex);
    }
    // For format MaskFormat::kARGB
    return matrix.hasPerspective() ? sizeof(ARGB3DVertex) : sizeof(ARGB2DVertex);
}

void GlyphData::fillVertexData(const sktext::gpu::VertexFiller& vf,
                               SkSpan<const Glyph> glyphs,
                               int offset,
                               int count,
                               const SkPMColor4f& pmColor,
                               const SkMatrix& positionMatrix,
                               SkIRect clip,
                               void* vertexBuffer) {
    GrColor color = pmColor.toBytes_RGBA();
    auto quadData = [&](auto dst) {
        return SkMakeZip(dst, glyphs.subspan(offset, count), vf.topLefts().subspan(offset, count));
    };

    // Handle direct mask drawing specifically.
    if (vf.canDrawDirect()) {
        auto [noTransformNeeded, originOffset] = vf.canUseDirect(positionMatrix);

        if (noTransformNeeded) {
            if (clip.isEmpty()) {
                if (vf.maskFormat() != MaskFormat::kARGB) {
                    using Quad = Mask2DVertex[4];
                    SkASSERT(sizeof(Mask2DVertex) ==
                             this->vertexStride(vf.maskFormat(), SkMatrix::I()));
                    fillDirectNoClipping(quadData((Quad*)vertexBuffer), color, originOffset);
                } else {
                    using Quad = ARGB2DVertex[4];
                    SkASSERT(sizeof(ARGB2DVertex) ==
                             this->vertexStride(vf.maskFormat(), SkMatrix::I()));
                    fillDirectClipped(quadData((Quad*)vertexBuffer), color, originOffset);
                }
            } else {
                if (vf.maskFormat() != MaskFormat::kARGB) {
                    using Quad = Mask2DVertex[4];
                    SkASSERT(sizeof(Mask2DVertex) ==
                             this->vertexStride(vf.maskFormat(), SkMatrix::I()));
                    fillDirectClipped(quadData((Quad*)vertexBuffer), color, originOffset, &clip);
                } else {
                    using Quad = ARGB2DVertex[4];
                    SkASSERT(sizeof(ARGB2DVertex) ==
                             this->vertexStride(vf.maskFormat(), SkMatrix::I()));
                    fillDirectClipped(quadData((Quad*)vertexBuffer), color, originOffset, &clip);
                }
            }
            return;
        }
    }

    // Handle the general transformed case.
    SkMatrix viewDifference = vf.viewDifference(positionMatrix);
    if (!positionMatrix.hasPerspective()) {
        if (vf.maskFormat() == MaskFormat::kARGB) {
            using Quad = ARGB2DVertex[4];
            SkASSERT(sizeof(ARGB2DVertex) == this->vertexStride(vf.maskFormat(), positionMatrix));
            fill2D(quadData((Quad*)vertexBuffer), color, viewDifference);
        } else {
            using Quad = Mask2DVertex[4];
            SkASSERT(sizeof(Mask2DVertex) == this->vertexStride(vf.maskFormat(), positionMatrix));
            fill2D(quadData((Quad*)vertexBuffer), color, viewDifference);
        }
    } else {
        if (vf.maskFormat() == MaskFormat::kARGB) {
            using Quad = ARGB3DVertex[4];
            SkASSERT(sizeof(ARGB3DVertex) == this->vertexStride(vf.maskFormat(), positionMatrix));
            fill3D(quadData((Quad*)vertexBuffer), color, viewDifference);
        } else {
            using Quad = Mask3DVertex[4];
            SkASSERT(sizeof(Mask3DVertex) == this->vertexStride(vf.maskFormat(), positionMatrix));
            fill3D(quadData((Quad*)vertexBuffer), color, viewDifference);
        }
    }
}

}  // namespace skgpu::ganesh
