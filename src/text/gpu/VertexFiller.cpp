/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/text/gpu/VertexFiller.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkSpan_impl.h"
#include "include/private/base/SkTLogic.h"
#include "src/base/SkZip.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/gpu/AtlasTypes.h"
#include "src/text/gpu/Glyph.h"
#include "src/text/gpu/SubRunAllocator.h"
#include "src/text/gpu/SubRunContainer.h"

#if defined(SK_GANESH)
#include "src/gpu/ganesh/ops/AtlasTextOp.h"
#endif

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/RendererProvider.h"
#endif

#include <cstdint>
#include <initializer_list>
#include <optional>

using MaskFormat = skgpu::MaskFormat;

namespace sktext::gpu {

VertexFiller::VertexFiller(MaskFormat maskFormat,
                           const SkMatrix &creationMatrix,
                           SkRect creationBounds,
                           SkSpan<const SkPoint> leftTop,
                           bool canDrawDirect)
            : fMaskType{maskFormat}, fCanDrawDirect{canDrawDirect},
              fCreationMatrix{creationMatrix}, fCreationBounds{creationBounds},
              fLeftTop{leftTop} {}

VertexFiller VertexFiller::Make(MaskFormat maskType,
                                const SkMatrix &creationMatrix,
                                SkRect creationBounds,
                                SkSpan<const SkPoint> positions,
                                SubRunAllocator *alloc,
                                FillerType fillerType) {
    SkSpan<SkPoint> leftTop = alloc->makePODSpan<SkPoint>(positions);
    return VertexFiller{
            maskType, creationMatrix, creationBounds, leftTop, fillerType == kIsDirect};
}

std::optional<VertexFiller> VertexFiller::MakeFromBuffer(SkReadBuffer &buffer,
                                                         SubRunAllocator *alloc) {
    int checkingMaskType = buffer.readInt();
    if (!buffer.validate(
            0 <= checkingMaskType && checkingMaskType < skgpu::kMaskFormatCount)) {
        return std::nullopt;
    }
    MaskFormat maskType = (MaskFormat) checkingMaskType;

    const bool canDrawDirect = buffer.readBool();

    SkMatrix creationMatrix;
    buffer.readMatrix(&creationMatrix);

    SkRect creationBounds = buffer.readRect();

    SkSpan<SkPoint> leftTop = MakePointsFromBuffer(buffer, alloc);
    if (leftTop.empty()) { return std::nullopt; }

    SkASSERT(buffer.isValid());
    return VertexFiller{maskType, creationMatrix, creationBounds, leftTop, canDrawDirect};
}

void VertexFiller::flatten(SkWriteBuffer &buffer) const {
    buffer.writeInt(static_cast<int>(fMaskType));
    buffer.writeBool(fCanDrawDirect);
    buffer.writeMatrix(fCreationMatrix);
    buffer.writeRect(fCreationBounds);
    buffer.writePointArray(fLeftTop.data(), SkCount(fLeftTop));
}

SkMatrix VertexFiller::viewDifference(const SkMatrix &positionMatrix) const {
    if (SkMatrix inverse; fCreationMatrix.invert(&inverse)) {
        return SkMatrix::Concat(positionMatrix, inverse);
    }
    return SkMatrix::I();
}

// Check for integer translate with the same 2x2 matrix.
// Returns the translation, and true if the change from creation matrix to the position matrix
// supports using direct glyph masks.
static std::tuple<bool, SkVector> can_use_direct(
        const SkMatrix& creationMatrix, const SkMatrix& positionMatrix) {
    // The existing direct glyph info can be used if the creationMatrix, and the
    // positionMatrix have the same 2x2, the translation between them is integer, and no
    // perspective is involved. Calculate the translation in source space to a translation in
    // device space by mapping (0, 0) through both the creationMatrix and the positionMatrix;
    // take the difference.
    SkVector translation = positionMatrix.mapOrigin() - creationMatrix.mapOrigin();
    return {creationMatrix.getScaleX() == positionMatrix.getScaleX() &&
            creationMatrix.getScaleY() == positionMatrix.getScaleY() &&
            creationMatrix.getSkewX()  == positionMatrix.getSkewX()  &&
            creationMatrix.getSkewY()  == positionMatrix.getSkewY()  &&
            !positionMatrix.hasPerspective() && !creationMatrix.hasPerspective() &&
            SkScalarIsInt(translation.x()) && SkScalarIsInt(translation.y()),
            translation};
}

struct AtlasPt {
    uint16_t u;
    uint16_t v;
};

#if defined(SK_GANESH)

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

size_t VertexFiller::vertexStride(const SkMatrix &matrix) const {
    if (fMaskType != MaskFormat::kARGB) {
        // For formats MaskFormat::kA565 and MaskFormat::kA8 where A8 include SDF.
        return matrix.hasPerspective() ? sizeof(Mask3DVertex) : sizeof(Mask2DVertex);
    } else {
        // For format MaskFormat::kARGB
        return matrix.hasPerspective() ? sizeof(ARGB3DVertex) : sizeof(ARGB2DVertex);
    }
}

// The 99% case. Direct Mask, No clip, No RGB.
void fillDirectNoClipping(SkZip<Mask2DVertex[4], const Glyph*, const SkPoint> quadData,
                          GrColor color,
                          SkPoint originOffset) {
    for (auto[quad, glyph, leftTop] : quadData) {
        auto[al, at, ar, ab] = glyph->fAtlasLocator.getUVs();
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
static auto LTBR(const Rect& r) {
    return std::make_tuple(r.left(), r.top(), r.right(), r.bottom());
}

// Handle any combination of BW or color and clip or no clip.
template<typename Quad, typename VertexData>
static void fillDirectClipped(SkZip<Quad, const Glyph*, const VertexData> quadData,
                              GrColor color,
                              SkPoint originOffset,
                              SkIRect* clip = nullptr) {
    for (auto[quad, glyph, leftTop] : quadData) {
        auto[al, at, ar, ab] = glyph->fAtlasLocator.getUVs();
        uint16_t w = ar - al,
                 h = ab - at;
        SkScalar l = leftTop.x() + originOffset.x(),
                 t = leftTop.y() + originOffset.y();
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

template<typename Quad, typename VertexData>
static void fill2D(SkZip<Quad, const Glyph*, const VertexData> quadData,
                   GrColor color,
                   const SkMatrix& viewDifference) {
    for (auto [quad, glyph, leftTop] : quadData) {
        auto [l, t] = leftTop;
        auto [r, b] = leftTop + glyph->fAtlasLocator.widthHeight();
        SkPoint lt = viewDifference.mapXY(l, t),
                lb = viewDifference.mapXY(l, b),
                rt = viewDifference.mapXY(r, t),
                rb = viewDifference.mapXY(r, b);
        auto [al, at, ar, ab] = glyph->fAtlasLocator.getUVs();
        quad[0] = {lt, color, {al, at}};  // L,T
        quad[1] = {lb, color, {al, ab}};  // L,B
        quad[2] = {rt, color, {ar, at}};  // R,T
        quad[3] = {rb, color, {ar, ab}};  // R,B
    }
}

template<typename Quad, typename VertexData>
static void fill3D(SkZip<Quad, const Glyph*, const VertexData> quadData,
                   GrColor color,
                   const SkMatrix& viewDifference) {
    auto mapXYZ = [&](SkScalar x, SkScalar y) {
        SkPoint pt{x, y};
        SkPoint3 result;
        viewDifference.mapHomogeneousPoints(&result, &pt, 1);
        return result;
    };
    for (auto [quad, glyph, leftTop] : quadData) {
        auto [l, t] = leftTop;
        auto [r, b] = leftTop + glyph->fAtlasLocator.widthHeight();
        SkPoint3 lt = mapXYZ(l, t),
                lb = mapXYZ(l, b),
                rt = mapXYZ(r, t),
                rb = mapXYZ(r, b);
        auto [al, at, ar, ab] = glyph->fAtlasLocator.getUVs();
        quad[0] = {lt, color, {al, at}};  // L,T
        quad[1] = {lb, color, {al, ab}};  // L,B
        quad[2] = {rt, color, {ar, at}};  // R,T
        quad[3] = {rb, color, {ar, ab}};  // R,B
    }
}

void VertexFiller::fillVertexData(int offset, int count,
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

    // Handle direct mask drawing specifically.
    if (fCanDrawDirect) {
        auto [noTransformNeeded, originOffset] =
                can_use_direct(fCreationMatrix, positionMatrix);

        if (noTransformNeeded) {
            if (clip.isEmpty()) {
                if (fMaskType != MaskFormat::kARGB) {
                    using Quad = Mask2DVertex[4];
                    SkASSERT(sizeof(Mask2DVertex) == this->vertexStride(SkMatrix::I()));
                    fillDirectNoClipping(quadData((Quad*)vertexBuffer), color, originOffset);
                } else {
                    using Quad = ARGB2DVertex[4];
                    SkASSERT(sizeof(ARGB2DVertex) == this->vertexStride(SkMatrix::I()));
                    fillDirectClipped(quadData((Quad*)vertexBuffer), color, originOffset);
                }
            } else {
                if (fMaskType != MaskFormat::kARGB) {
                    using Quad = Mask2DVertex[4];
                    SkASSERT(sizeof(Mask2DVertex) == this->vertexStride(SkMatrix::I()));
                    fillDirectClipped(quadData((Quad*)vertexBuffer), color, originOffset, &clip);
                } else {
                    using Quad = ARGB2DVertex[4];
                    SkASSERT(sizeof(ARGB2DVertex) == this->vertexStride(SkMatrix::I()));
                    fillDirectClipped(quadData((Quad*)vertexBuffer), color, originOffset, &clip);
                }
            }
            return;
        }
    }

    // Handle the general transformed case.
    SkMatrix viewDifference = this->viewDifference(positionMatrix);
    if (!positionMatrix.hasPerspective()) {
        if (fMaskType == MaskFormat::kARGB) {
            using Quad = ARGB2DVertex[4];
            SkASSERT(sizeof(ARGB2DVertex) == this->vertexStride(positionMatrix));
            fill2D(quadData((Quad*)vertexBuffer), color, viewDifference);
        } else {
            using Quad = Mask2DVertex[4];
            SkASSERT(sizeof(Mask2DVertex) == this->vertexStride(positionMatrix));
            fill2D(quadData((Quad*)vertexBuffer), color, viewDifference);
        }
    } else {
        if (fMaskType == MaskFormat::kARGB) {
            using Quad = ARGB3DVertex[4];
            SkASSERT(sizeof(ARGB3DVertex) == this->vertexStride(positionMatrix));
            fill3D(quadData((Quad*)vertexBuffer), color, viewDifference);
        } else {
            using Quad = Mask3DVertex[4];
            SkASSERT(sizeof(Mask3DVertex) == this->vertexStride(positionMatrix));
            fill3D(quadData((Quad*)vertexBuffer), color, viewDifference);
        }
    }
}

using AtlasTextOp = skgpu::ganesh::AtlasTextOp;
AtlasTextOp::MaskType VertexFiller::opMaskType() const {
    switch (fMaskType) {
        case MaskFormat::kA8:   return AtlasTextOp::MaskType::kGrayscaleCoverage;
        case MaskFormat::kA565: return AtlasTextOp::MaskType::kLCDCoverage;
        case MaskFormat::kARGB: return AtlasTextOp::MaskType::kColorBitmap;
    }
    SkUNREACHABLE;
}
#endif  // defined(SK_GANESH)

#if defined(SK_GRAPHITE)
void VertexFiller::fillInstanceData(skgpu::graphite::DrawWriter* dw,
                                    int offset, int count,
                                    unsigned short flags,
                                    int ssboIndex,
                                    SkSpan<const Glyph*> glyphs,
                                    SkScalar depth) const {
    auto quadData = [&]() {
    return SkMakeZip(glyphs.subspan(offset, count),
                     fLeftTop.subspan(offset, count));
    };

    skgpu::graphite::DrawWriter::Instances instances{*dw, {}, {}, 4};
    instances.reserve(count);
    // Need to send width, height, uvPos, xyPos, and strikeToSourceScale
    // pre-transform coords = (s*w*b_x + t_x, s*h*b_y + t_y)
    // where (b_x, b_y) are the vertexID coords
    for (auto [glyph, leftTop]: quadData()) {
        auto[al, at, ar, ab] = glyph->fAtlasLocator.getUVs();
        instances.append(1) << AtlasPt{uint16_t(ar-al), uint16_t(ab-at)}
                            << AtlasPt{uint16_t(al & 0x1fff), at}
                            << leftTop << /*index=*/uint16_t(al >> 13) << flags
                            << 1.0f
                            << depth << ssboIndex;
    }
}

using Rect = skgpu::graphite::Rect;
using Transform = skgpu::graphite::Transform;

std::tuple<Rect, Transform> VertexFiller::boundsAndDeviceMatrix(
        const Transform& localToDevice, SkPoint drawOrigin) const {
    // The baked-in matrix differs from the current localToDevice by a translation if the
    // upper 2x2 remains the same, and there's no perspective. Since there's no projection,
    // Z is irrelevant, so it's okay that fCreationMatrix is an SkMatrix and has
    // discarded the 3rd row/col, and can ignore those values in localToDevice.
    const SkM44& positionMatrix = localToDevice.matrix();
    const bool compatibleMatrix = positionMatrix.rc(0,0) == fCreationMatrix.rc(0, 0) &&
                                  positionMatrix.rc(0,1) == fCreationMatrix.rc(0, 1) &&
                                  positionMatrix.rc(1,0) == fCreationMatrix.rc(1, 0) &&
                                  positionMatrix.rc(1,1) == fCreationMatrix.rc(1, 1) &&
                                  localToDevice.type() != Transform::Type::kProjection &&
                                  !fCreationMatrix.hasPerspective();

    if (compatibleMatrix) {
        const SkV4 mappedOrigin = positionMatrix.map(drawOrigin.x(), drawOrigin.y(), 0.f, 1.f);
        const SkV2 offset = {mappedOrigin.x - fCreationMatrix.getTranslateX(),
                             mappedOrigin.y - fCreationMatrix.getTranslateY()};
        if (SkScalarIsInt(offset.x) && SkScalarIsInt(offset.y)) {
            // The offset is an integer (but make sure), which means the generated mask can be
            // accessed without changing how texels would be sampled.
            return {Rect(fCreationBounds),
                    Transform(SkM44::Translate(SkScalarRoundToInt(offset.x),
                                               SkScalarRoundToInt(offset.y)))};
        }
    }

    // Otherwise compute the relative transformation from fCreationMatrix to
    // localToDevice, with the drawOrigin applied. If fCreationMatrix or the
    // concatenation is not invertible the returned Transform is marked invalid and the draw
    // will be automatically dropped.
    const SkMatrix viewDifference = this->viewDifference(
            localToDevice.preTranslate(drawOrigin.x(), drawOrigin.y()));
    return {Rect(fCreationBounds), Transform(SkM44(viewDifference))};
}
#endif  // defined(SK_GRAPHITE)

// Return true if the positionMatrix represents an integer translation. Return the device
// bounding box of all the glyphs. If the bounding box is empty, then something went singular
// and this operation should be dropped.
std::tuple<bool, SkRect> VertexFiller::deviceRectAndCheckTransform(
            const SkMatrix &positionMatrix) const {
    if (fCanDrawDirect) {
        const auto [directDrawCompatible, offset] =
                can_use_direct(fCreationMatrix, positionMatrix);

        if (directDrawCompatible) {
            return {true, fCreationBounds.makeOffset(offset)};
        }
    }

    if (SkMatrix inverse; fCreationMatrix.invert(&inverse)) {
        SkMatrix viewDifference = SkMatrix::Concat(positionMatrix, inverse);
        return {false, viewDifference.mapRect(fCreationBounds)};
    }

    // initialPositionMatrix is singular. Do nothing.
    return {false, SkRect::MakeEmpty()};
}

}  // namespace sktext::gpu
