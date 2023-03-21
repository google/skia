/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/text/gpu/SubRunContainer.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkOnce.h"
#include "include/private/base/SkTArray.h"
#include "include/private/chromium/SkChromeRemoteGlyphCache.h"
#include "src/core/SkDescriptor.h"
#include "src/core/SkDistanceFieldGen.h"
#include "src/core/SkEnumerate.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeCache.h"
#include "src/gpu/AtlasTypes.h"
#include "src/text/GlyphRun.h"
#include "src/text/StrikeForGPU.h"
#include "src/text/gpu/Glyph.h"
#include "src/text/gpu/GlyphVector.h"
#include "src/text/gpu/SubRunAllocator.h"

#if defined(SK_GANESH)  // Ganesh Support
#include "src/gpu/ganesh/GrClip.h"
#include "src/gpu/ganesh/GrStyle.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/ops/AtlasTextOp.h"
using AtlasTextOp = skgpu::ganesh::AtlasTextOp;
#endif  // defined(SK_GANESH)

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/RendererProvider.h"
#endif

#include <cinttypes>
#include <cmath>
#include <optional>

using namespace skglyph;

// -- GPU Text -------------------------------------------------------------------------------------
// Naming conventions
//  * drawMatrix - the CTM from the canvas.
//  * drawOrigin - the x, y location of the drawTextBlob call.
//  * positionMatrix - this is the combination of the drawMatrix and the drawOrigin:
//        positionMatrix = drawMatrix * TranslationMatrix(drawOrigin.x, drawOrigin.y);
//
// Note:
//   In order to transform Slugs, you need to set the fSupportBilerpFromGlyphAtlas on
//   GrContextOptions.

namespace sktext::gpu {
// -- SubRunType -----------------------------------------------------------------------------------
enum SubRun::SubRunType : int {
    kBad = 0,  // Make this 0 to line up with errors from readInt.
    kDirectMask,
#if !defined(SK_DISABLE_SDF_TEXT)
    kSDFT,
#endif
    kTransformMask,
    kPath,
    kDrawable,
    kSubRunTypeCount,
};

#if defined(SK_GRAPHITE)
// AtlasSubRun provides a draw() function that grants the anonymous subclasses access to
// Device::drawAtlasSubRun.
void AtlasSubRun::draw(skgpu::graphite::Device* device,
                       SkPoint drawOrigin,
                       const SkPaint& paint,
                       sk_sp<SkRefCnt> subRunStorage) const {
    device->drawAtlasSubRun(this, drawOrigin, paint, std::move(subRunStorage));
}
#endif

}  // namespace sktext::gpu

using MaskFormat = skgpu::MaskFormat;

using namespace sktext;
using namespace sktext::gpu;

#if defined(SK_GRAPHITE)
namespace gr = skgpu::graphite;

using BindBufferInfo = gr::BindBufferInfo;
using BufferType = gr::BufferType;
using Device = gr::Device;
using DrawWriter = gr::DrawWriter;
using Recorder = gr::Recorder;
using Renderer = gr::Renderer;
using RendererProvider = gr::RendererProvider;
using TextureProxy = gr::TextureProxy;
using Transform = gr::Transform;
#endif

namespace {
// Returns the empty span if there is a problem reading the positions.
SkSpan<SkPoint> make_points_from_buffer(SkReadBuffer& buffer, SubRunAllocator* alloc) {
    uint32_t glyphCount = buffer.getArrayCount();

    // Zero indicates a problem with serialization.
    if (!buffer.validate(glyphCount != 0)) { return {}; }

    // Check that the count will not overflow the arena.
    if (!buffer.validate(glyphCount <= INT_MAX &&
                         BagOfBytes::WillCountFit<SkPoint>(glyphCount))) { return {}; }

    SkPoint* positionsData = alloc->makePODArray<SkPoint>(glyphCount);
    if (!buffer.readPointArray(positionsData, glyphCount)) { return {}; }
    return {positionsData, glyphCount};
}

// Check for integer translate with the same 2x2 matrix.
// Returns the translation, and true if the change from creation matrix to the position matrix
// supports using direct glyph masks.
std::tuple<bool, SkVector> can_use_direct(
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

enum FillerType {
    kIsDirect,
    kIsTransformed
};

// -- VertexFiller ---------------------------------------------------------------------------------
// The VertexFiller assumes that all points, glyph atlas entries, and bounds are created with
// respect to the CreationMatrix. This assumes that mapping any point, mask or bounds through the
// CreationMatrix will result in the proper device position. In order to draw using an arbitrary
// PositionMatrix, calculate a
//
//    viewDifference = [PositionMatrix] * [CreationMatrix] ^ -1.
//
// The viewDifference is used to map all points, masks and bounds to position to the device
// respecting the PositionMatrix.
class VertexFiller {
public:
    VertexFiller(MaskFormat maskFormat,
                 const SkMatrix& creationMatrix,
                 SkRect creationBounds,
                 SkSpan<const SkPoint> leftTop,
                 bool canDrawDirect)
            : fMaskType{maskFormat}
            , fCanDrawDirect{canDrawDirect}
            , fCreationMatrix{creationMatrix}
            , fCreationBounds{creationBounds}
            , fLeftTop{leftTop} {}

    static VertexFiller Make(MaskFormat maskType,
                             const SkMatrix& creationMatrix,
                             SkRect creationBounds,
                             SkSpan<const SkPoint> positions,
                             SubRunAllocator* alloc,
                             FillerType fillerType) {
        SkSpan<SkPoint> leftTop = alloc->makePODSpan<SkPoint>(positions);
        return VertexFiller{
            maskType, creationMatrix, creationBounds, leftTop, fillerType == kIsDirect};
    }

    static std::optional<VertexFiller> MakeFromBuffer(
            SkReadBuffer& buffer, SubRunAllocator* alloc) {
        int checkingMaskType = buffer.readInt();
        if (!buffer.validate(0 <= checkingMaskType && checkingMaskType < skgpu::kMaskFormatCount)) {
            return std::nullopt;
        }
        MaskFormat maskType = (MaskFormat)checkingMaskType;

        const bool canDrawDirect = buffer.readBool();

        SkMatrix creationMatrix;
        buffer.readMatrix(&creationMatrix);

        SkRect creationBounds = buffer.readRect();

        SkSpan<SkPoint> leftTop = make_points_from_buffer(buffer, alloc);
        if (leftTop.empty()) { return std::nullopt; }

        SkASSERT(buffer.isValid());
        return VertexFiller{maskType, creationMatrix, creationBounds, leftTop, canDrawDirect};
    }

    int unflattenSize() const {
        return fLeftTop.size_bytes();
    }

    void flatten(SkWriteBuffer& buffer) const {
        buffer.writeInt(static_cast<int>(fMaskType));
        buffer.writeBool(fCanDrawDirect);
        buffer.writeMatrix(fCreationMatrix);
        buffer.writeRect(fCreationBounds);
        buffer.writePointArray(fLeftTop.data(), SkCount(fLeftTop));
    }

    SkMatrix viewDifference(const SkMatrix& positionMatrix) const {
        if (SkMatrix inverse; fCreationMatrix.invert(&inverse)) {
            return SkMatrix::Concat(positionMatrix, inverse);
        }
        return SkMatrix::I();
    }

#if defined(SK_GANESH)
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
                        this->fillDirectNoClipping(
                                quadData((Quad*)vertexBuffer), color, originOffset);
                    } else {
                        using Quad = ARGB2DVertex[4];
                        SkASSERT(sizeof(ARGB2DVertex) == this->vertexStride(SkMatrix::I()));
                        this->fillDirectClipped(quadData((Quad*)vertexBuffer), color, originOffset);
                    }
                } else {
                    if (fMaskType != MaskFormat::kARGB) {
                        using Quad = Mask2DVertex[4];
                        SkASSERT(sizeof(Mask2DVertex) == this->vertexStride(SkMatrix::I()));
                        this->fillDirectClipped(
                                quadData((Quad*)vertexBuffer), color, originOffset, &clip);
                    } else {
                        using Quad = ARGB2DVertex[4];
                        SkASSERT(sizeof(ARGB2DVertex) == this->vertexStride(SkMatrix::I()));
                        this->fillDirectClipped(
                                quadData((Quad*)vertexBuffer), color, originOffset, &clip);
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
                this->fill2D(quadData((Quad*)vertexBuffer), color, viewDifference);
            } else {
                using Quad = Mask2DVertex[4];
                SkASSERT(sizeof(Mask2DVertex) == this->vertexStride(positionMatrix));
                this->fill2D(quadData((Quad*)vertexBuffer), color, viewDifference);
            }
        } else {
            if (fMaskType == MaskFormat::kARGB) {
                using Quad = ARGB3DVertex[4];
                SkASSERT(sizeof(ARGB3DVertex) == this->vertexStride(positionMatrix));
                this->fill3D(quadData((Quad*)vertexBuffer), color, viewDifference);
            } else {
                using Quad = Mask3DVertex[4];
                SkASSERT(sizeof(Mask3DVertex) == this->vertexStride(positionMatrix));
                this->fill3D(quadData((Quad*)vertexBuffer), color, viewDifference);
            }
        }
    }

    AtlasTextOp::MaskType opMaskType() const {
        switch (fMaskType) {
            case MaskFormat::kA8:   return AtlasTextOp::MaskType::kGrayscaleCoverage;
            case MaskFormat::kA565: return AtlasTextOp::MaskType::kLCDCoverage;
            case MaskFormat::kARGB: return AtlasTextOp::MaskType::kColorBitmap;
        }
        SkUNREACHABLE;
    }
#endif  // defined(SK_GANESH)

#if defined(SK_GRAPHITE)
    void fillVertexData(DrawWriter* dw,
                        int offset, int count,
                        int ssboIndex,
                        SkSpan<const Glyph*> glyphs,
                        SkScalar depth,
                        const skgpu::graphite::Transform& toDevice) const {
        auto quadData = [&]() {
        return SkMakeZip(glyphs.subspan(offset, count),
                         fLeftTop.subspan(offset, count));
        };

        // TODO: can't handle perspective right now
        if (toDevice.type() == Transform::Type::kProjection) {
            return;
        }

        DrawWriter::Vertices verts{*dw};
        verts.reserve(6*count);
        for (auto [glyph, leftTop]: quadData()) {
            auto [al, at, ar, ab] = glyph->fAtlasLocator.getUVs();
            auto [l, t] = leftTop;
            auto [r, b] = leftTop + glyph->fAtlasLocator.widthHeight();
            SkV2 localCorners[4] = {{l, t}, {r, t}, {r, b}, {l, b}};
            SkV4 devOut[4];
            toDevice.mapPoints(localCorners, devOut, 4);
            // TODO: Ganesh uses indices but that's not available with dynamic vertex data
            // TODO: we should really use instances as well.
            verts.append(6) << SkPoint{devOut[0].x, devOut[0].y} << depth << AtlasPt{al, at}  // L,T
                            << ssboIndex
                            << SkPoint{devOut[3].x, devOut[3].y} << depth << AtlasPt{al, ab}  // L,B
                            << ssboIndex
                            << SkPoint{devOut[1].x, devOut[1].y} << depth << AtlasPt{ar, at}  // R,T
                            << ssboIndex
                            << SkPoint{devOut[3].x, devOut[3].y} << depth << AtlasPt{al, ab}  // L,B
                            << ssboIndex
                            << SkPoint{devOut[2].x, devOut[2].y} << depth << AtlasPt{ar, ab}  // R,B
                            << ssboIndex
                            << SkPoint{devOut[1].x, devOut[1].y} << depth << AtlasPt{ar, at}  // R,T
                            << ssboIndex;
        }
    }
    void fillInstanceData(DrawWriter* dw,
                          int offset, int count,
                          unsigned short flags,
                          int ssboIndex,
                          SkSpan<const Glyph*> glyphs,
                          SkScalar depth) const {
        auto quadData = [&]() {
        return SkMakeZip(glyphs.subspan(offset, count),
                         fLeftTop.subspan(offset, count));
        };

        DrawWriter::Instances instances{*dw, {}, {}, 4};
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

    std::tuple<gr::Rect, Transform> boundsAndDeviceMatrix(const Transform& localToDevice,
                                                          SkPoint drawOrigin) const {
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
                return {gr::Rect(fCreationBounds),
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
        return {gr::Rect(fCreationBounds), Transform(SkM44(viewDifference))};
    }
#endif  // defined(SK_GRAPHITE)

    // Return true if the positionMatrix represents an integer translation. Return the device
    // bounding box of all the glyphs. If the bounding box is empty, then something went singular
    // and this operation should be dropped.
    std::tuple<bool, SkRect> deviceRectAndCheckTransform(const SkMatrix& positionMatrix) const {
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

    SkRect creationBounds() const { return fCreationBounds; }
    MaskFormat grMaskType() const { return fMaskType; }
    int count() const { return SkCount(fLeftTop); }

private:
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

    // The 99% case. Direct Mask, No clip, No RGB.
    void fillDirectNoClipping(SkZip<Mask2DVertex[4], const Glyph*, const SkPoint> quadData,
                              GrColor color,
                              SkPoint originOffset) const {
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
    void fillDirectClipped(SkZip<Quad, const Glyph*, const VertexData> quadData,
                           GrColor color,
                           SkPoint originOffset,
                           SkIRect* clip = nullptr) const {
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
    void fill2D(SkZip<Quad, const Glyph*, const VertexData> quadData,
                GrColor color,
                const SkMatrix& viewDifference) const {
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
    void fill3D(SkZip<Quad, const Glyph*, const VertexData> quadData,
                GrColor color,
                const SkMatrix& viewDifference) const {
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
#endif  // defined(SK_GANESH)

    const MaskFormat fMaskType;
    const bool fCanDrawDirect;
    const SkMatrix fCreationMatrix;
    const SkRect fCreationBounds;
    const SkSpan<const SkPoint> fLeftTop;
};

#if defined(SK_GANESH)
SkPMColor4f calculate_colors(skgpu::ganesh::SurfaceDrawContext* sdc,
                             const SkPaint& paint,
                             const SkMatrix& matrix,
                             MaskFormat maskFormat,
                             GrPaint* grPaint) {
    GrRecordingContext* rContext = sdc->recordingContext();
    const GrColorInfo& colorInfo = sdc->colorInfo();
    const SkSurfaceProps& props = sdc->surfaceProps();
    if (maskFormat == MaskFormat::kARGB) {
        SkPaintToGrPaintReplaceShader(rContext, colorInfo, paint, matrix, nullptr, props, grPaint);
        float a = grPaint->getColor4f().fA;
        return {a, a, a, a};
    }
    SkPaintToGrPaint(rContext, colorInfo, paint, matrix, props, grPaint);
    return grPaint->getColor4f();
}

SkMatrix position_matrix(const SkMatrix& drawMatrix, SkPoint drawOrigin) {
    SkMatrix position_matrix = drawMatrix;
    return position_matrix.preTranslate(drawOrigin.x(), drawOrigin.y());
}
#endif  // defined(SK_GANESH)

SkSpan<const SkPackedGlyphID> get_packedIDs(SkZip<const SkPackedGlyphID, const SkPoint> accepted) {
    return accepted.get<0>();
}

SkSpan<const SkGlyphID> get_glyphIDs(SkZip<const SkGlyphID, const SkPoint> accepted) {
    return accepted.get<0>();
}

template <typename U>
SkSpan<const SkPoint> get_positions(SkZip<U, const SkPoint> accepted) {
    return accepted.template get<1>();
}

// -- PathOpSubmitter ------------------------------------------------------------------------------
// PathOpSubmitter holds glyph ids until ready to draw. During drawing, the glyph ids are
// converted to SkPaths. PathOpSubmitter can only be serialized when it is holding glyph ids;
// it can only be serialized before submitDraws has been called.
class PathOpSubmitter {
public:
    PathOpSubmitter() = delete;
    PathOpSubmitter(const PathOpSubmitter&) = delete;
    const PathOpSubmitter& operator=(const PathOpSubmitter&) = delete;
    PathOpSubmitter(PathOpSubmitter&& that)
            // Transfer ownership of fIDsOrPaths from that to this.
            : fIDsOrPaths{std::exchange(
                      const_cast<SkSpan<IDOrPath>&>(that.fIDsOrPaths), SkSpan<IDOrPath>{})}
            , fPositions{that.fPositions}
            , fStrikeToSourceScale{that.fStrikeToSourceScale}
            , fIsAntiAliased{that.fIsAntiAliased}
            , fStrikePromise{std::move(that.fStrikePromise)} {}
    PathOpSubmitter& operator=(PathOpSubmitter&& that) {
        this->~PathOpSubmitter();
        new (this) PathOpSubmitter{std::move(that)};
        return *this;
    }
    PathOpSubmitter(bool isAntiAliased,
                    SkScalar strikeToSourceScale,
                    SkSpan<SkPoint> positions,
                    SkSpan<IDOrPath> idsOrPaths,
                    SkStrikePromise&& strikePromise);

    ~PathOpSubmitter();

    static PathOpSubmitter Make(SkZip<const SkGlyphID, const SkPoint> accepted,
                                bool isAntiAliased,
                                SkScalar strikeToSourceScale,
                                SkStrikePromise&& strikePromise,
                                SubRunAllocator* alloc);

    int unflattenSize() const;
    void flatten(SkWriteBuffer& buffer) const;
    static std::optional<PathOpSubmitter> MakeFromBuffer(SkReadBuffer& buffer,
                                                         SubRunAllocator* alloc,
                                                         const SkStrikeClient* client);

    // submitDraws is not thread safe. It only occurs the single thread drawing portion of the GPU
    // rendering.
    void submitDraws(SkCanvas*,
                     SkPoint drawOrigin,
                     const SkPaint& paint) const;

private:
    // When PathOpSubmitter is created only the glyphIDs are needed, during the submitDraws call,
    // the glyphIDs are converted to SkPaths.
    const SkSpan<IDOrPath> fIDsOrPaths;
    const SkSpan<const SkPoint> fPositions;
    const SkScalar fStrikeToSourceScale;
    const bool fIsAntiAliased;

    mutable SkStrikePromise fStrikePromise;
    mutable SkOnce fConvertIDsToPaths;
    mutable bool fPathsAreCreated{false};
};

int PathOpSubmitter::unflattenSize() const {
    return fPositions.size_bytes() + fIDsOrPaths.size_bytes();
}

void PathOpSubmitter::flatten(SkWriteBuffer& buffer) const {
    fStrikePromise.flatten(buffer);

    buffer.writeInt(fIsAntiAliased);
    buffer.writeScalar(fStrikeToSourceScale);
    buffer.writePointArray(fPositions.data(), SkCount(fPositions));
    for (IDOrPath& idOrPath : fIDsOrPaths) {
        buffer.writeInt(idOrPath.fGlyphID);
    }
}

std::optional<PathOpSubmitter> PathOpSubmitter::MakeFromBuffer(SkReadBuffer& buffer,
                                                               SubRunAllocator* alloc,
                                                               const SkStrikeClient* client) {
    std::optional<SkStrikePromise> strikePromise =
            SkStrikePromise::MakeFromBuffer(buffer, client, SkStrikeCache::GlobalStrikeCache());
    if (!buffer.validate(strikePromise.has_value())) {
        return std::nullopt;
    }

    bool isAntiAlias = buffer.readInt();

    SkScalar strikeToSourceScale = buffer.readScalar();
    if (!buffer.validate(0 < strikeToSourceScale)) { return std::nullopt; }

    SkSpan<SkPoint> positions = make_points_from_buffer(buffer, alloc);
    if (positions.empty()) { return std::nullopt; }
    const int glyphCount = SkCount(positions);

    // Remember, we stored an int for glyph id.
    if (!buffer.validateCanReadN<int>(glyphCount)) { return std::nullopt; }
    auto idsOrPaths = SkSpan(alloc->makeUniqueArray<IDOrPath>(glyphCount).release(), glyphCount);
    for (auto& idOrPath : idsOrPaths) {
        idOrPath.fGlyphID = SkTo<SkGlyphID>(buffer.readInt());
    }

    if (!buffer.isValid()) { return std::nullopt; }

    return PathOpSubmitter{isAntiAlias,
                           strikeToSourceScale,
                           positions,
                           idsOrPaths,
                           std::move(strikePromise.value())};
}

PathOpSubmitter::PathOpSubmitter(
        bool isAntiAliased,
        SkScalar strikeToSourceScale,
        SkSpan<SkPoint> positions,
        SkSpan<IDOrPath> idsOrPaths,
        SkStrikePromise&& strikePromise)
        : fIDsOrPaths{idsOrPaths}
        , fPositions{positions}
        , fStrikeToSourceScale{strikeToSourceScale}
        , fIsAntiAliased{isAntiAliased}
        , fStrikePromise{std::move(strikePromise)} {
    SkASSERT(!fPositions.empty());
}

PathOpSubmitter::~PathOpSubmitter() {
    // If we have converted glyph IDs to paths, then clean up the SkPaths.
    if (fPathsAreCreated) {
        for (auto& idOrPath : fIDsOrPaths) {
            idOrPath.fPath.~SkPath();
        }
    }
}

PathOpSubmitter PathOpSubmitter::Make(SkZip<const SkGlyphID, const SkPoint> accepted,
                                      bool isAntiAliased,
                                      SkScalar strikeToSourceScale,
                                      SkStrikePromise&& strikePromise,
                                      SubRunAllocator* alloc) {
    auto mapToIDOrPath = [](SkGlyphID glyphID) { return IDOrPath{glyphID}; };

    IDOrPath* const rawIDsOrPaths =
            alloc->makeUniqueArray<IDOrPath>(get_glyphIDs(accepted), mapToIDOrPath).release();

    return PathOpSubmitter{isAntiAliased,
                           strikeToSourceScale,
                           alloc->makePODSpan(get_positions(accepted)),
                           SkSpan(rawIDsOrPaths, accepted.size()),
                           std::move(strikePromise)};
}

void
PathOpSubmitter::submitDraws(SkCanvas* canvas, SkPoint drawOrigin, const SkPaint& paint) const {
    // Convert the glyph IDs to paths if it hasn't been done yet. This is thread safe.
    fConvertIDsToPaths([&]() {
        if (SkStrike* strike = fStrikePromise.strike()) {
            strike->glyphIDsToPaths(fIDsOrPaths);

            // Drop ref to strike so that it can be purged from the cache if needed.
            fStrikePromise.resetStrike();
            fPathsAreCreated = true;
        }
    });

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
        for (auto [idOrPath, pos] : SkMakeZip(fIDsOrPaths, fPositions)) {
            // Transform the glyph to source space.
            SkMatrix pathMatrix = strikeToSource;
            pathMatrix.postTranslate(pos.x(), pos.y());

            SkAutoCanvasRestore acr(canvas, true);
            canvas->concat(pathMatrix);
            canvas->drawPath(idOrPath.fPath, runPaint);
        }
    } else {
        // Transform the path to device because the deviceMatrix must be unchanged to
        // draw effect, filter or shader paths.
        for (auto [idOrPath, pos] : SkMakeZip(fIDsOrPaths, fPositions)) {
            // Transform the glyph to source space.
            SkMatrix pathMatrix = strikeToSource;
            pathMatrix.postTranslate(pos.x(), pos.y());

            SkPath deviceOutline;
            idOrPath.fPath.transform(pathMatrix, &deviceOutline);
            deviceOutline.setIsVolatile(true);
            canvas->drawPath(deviceOutline, runPaint);
        }
    }
}

// -- PathSubRun -----------------------------------------------------------------------------------
class PathSubRun final : public SubRun {
public:
    PathSubRun(PathOpSubmitter&& pathDrawing) : fPathDrawing(std::move(pathDrawing)) {}

    static SubRunOwner Make(SkZip<const SkGlyphID, const SkPoint> accepted,
                            bool isAntiAliased,
                            SkScalar strikeToSourceScale,
                            SkStrikePromise&& strikePromise,
                            SubRunAllocator* alloc) {
        return alloc->makeUnique<PathSubRun>(
            PathOpSubmitter::Make(
                    accepted, isAntiAliased, strikeToSourceScale, std::move(strikePromise), alloc));
    }

#if defined(SK_GANESH)
    void draw(SkCanvas* canvas,
              const GrClip*,
              const SkMatrixProvider&,
              SkPoint drawOrigin,
              const SkPaint& paint,
              sk_sp<SkRefCnt>,
              skgpu::ganesh::SurfaceDrawContext*) const override {
        fPathDrawing.submitDraws(canvas, drawOrigin, paint);
    }
#endif  // defined(SK_GANESH)
#if defined(SK_GRAPHITE)
    void draw(SkCanvas* canvas,
              SkPoint drawOrigin,
              const SkPaint& paint,
              sk_sp<SkRefCnt> subRunStorage,
              Device* device) const override {
        fPathDrawing.submitDraws(canvas, drawOrigin, paint);
    }
#endif  // SK_GRAPHITE

    int unflattenSize() const override;

    bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const override {
        return true;
    }
    const AtlasSubRun* testingOnly_atlasSubRun() const override { return nullptr; }
    static SubRunOwner MakeFromBuffer(SkReadBuffer& buffer,
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

SubRunOwner PathSubRun::MakeFromBuffer(SkReadBuffer& buffer,
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
    DrawableOpSubmitter() = delete;
    DrawableOpSubmitter(const DrawableOpSubmitter&) = delete;
    const DrawableOpSubmitter& operator=(const DrawableOpSubmitter&) = delete;
    DrawableOpSubmitter(DrawableOpSubmitter&& that)
        : fStrikeToSourceScale{that.fStrikeToSourceScale}
        , fPositions{that.fPositions}
        , fIDsOrDrawables{that.fIDsOrDrawables}
        , fStrikePromise{std::move(that.fStrikePromise)} {}
    DrawableOpSubmitter& operator=(DrawableOpSubmitter&& that) {
        this->~DrawableOpSubmitter();
        new (this) DrawableOpSubmitter{std::move(that)};
        return *this;
    }
    DrawableOpSubmitter(SkScalar strikeToSourceScale,
                        SkSpan<SkPoint> positions,
                        SkSpan<IDOrDrawable> idsOrDrawables,
                        SkStrikePromise&& strikePromise);

    static DrawableOpSubmitter Make(SkZip<const SkGlyphID, const SkPoint> accepted,
                                    SkScalar strikeToSourceScale,
                                    SkStrikePromise&& strikePromise,
                                    SubRunAllocator* alloc) {
        auto mapToIDOrDrawable = [](const SkGlyphID glyphID) { return IDOrDrawable{glyphID}; };

        return DrawableOpSubmitter{
            strikeToSourceScale,
            alloc->makePODSpan(get_positions(accepted)),
            alloc->makePODArray<IDOrDrawable>(get_glyphIDs(accepted), mapToIDOrDrawable),
            std::move(strikePromise)};
    }

    int unflattenSize() const;
    void flatten(SkWriteBuffer& buffer) const;
    static std::optional<DrawableOpSubmitter> MakeFromBuffer(SkReadBuffer& buffer,
                                                             SubRunAllocator* alloc,
                                                             const SkStrikeClient* client);
    void submitDraws(SkCanvas* canvas, SkPoint drawOrigin, const SkPaint& paint) const;

private:
    const SkScalar fStrikeToSourceScale;
    const SkSpan<SkPoint> fPositions;
    const SkSpan<IDOrDrawable> fIDsOrDrawables;
    // When the promise is converted to a strike it acts as the ref on the strike to keep the
    // SkDrawable data alive.
    mutable SkStrikePromise fStrikePromise;
    mutable SkOnce fConvertIDsToDrawables;
};

int DrawableOpSubmitter::unflattenSize() const {
    return fPositions.size_bytes() + fIDsOrDrawables.size_bytes();
}

void DrawableOpSubmitter::flatten(SkWriteBuffer& buffer) const {
    fStrikePromise.flatten(buffer);

    buffer.writeScalar(fStrikeToSourceScale);
    buffer.writePointArray(fPositions.data(), SkCount(fPositions));
    for (IDOrDrawable idOrDrawable : fIDsOrDrawables) {
        buffer.writeInt(idOrDrawable.fGlyphID);
    }
}

std::optional<DrawableOpSubmitter> DrawableOpSubmitter::MakeFromBuffer(
        SkReadBuffer& buffer, SubRunAllocator* alloc, const SkStrikeClient* client) {
    std::optional<SkStrikePromise> strikePromise =
            SkStrikePromise::MakeFromBuffer(buffer, client, SkStrikeCache::GlobalStrikeCache());
    if (!buffer.validate(strikePromise.has_value())) {
        return std::nullopt;
    }

    SkScalar strikeToSourceScale = buffer.readScalar();
    if (!buffer.validate(0 < strikeToSourceScale)) { return std::nullopt; }

    SkSpan<SkPoint> positions = make_points_from_buffer(buffer, alloc);
    if (positions.empty()) { return std::nullopt; }
    const int glyphCount = SkCount(positions);

    if (!buffer.validateCanReadN<int>(glyphCount)) { return std::nullopt; }
    auto idsOrDrawables = alloc->makePODArray<IDOrDrawable>(glyphCount);
    for (int i = 0; i < SkToInt(glyphCount); ++i) {
        // Remember, we stored an int for glyph id.
        idsOrDrawables[i].fGlyphID = SkTo<SkGlyphID>(buffer.readInt());
    }

    SkASSERT(buffer.isValid());
    return DrawableOpSubmitter{strikeToSourceScale,
                               positions,
                               SkSpan(idsOrDrawables, glyphCount),
                               std::move(strikePromise.value())};
}

DrawableOpSubmitter::DrawableOpSubmitter(
        SkScalar strikeToSourceScale,
        SkSpan<SkPoint> positions,
        SkSpan<IDOrDrawable> idsOrDrawables,
        SkStrikePromise&& strikePromise)
        : fStrikeToSourceScale{strikeToSourceScale}
        , fPositions{positions}
        , fIDsOrDrawables{idsOrDrawables}
        , fStrikePromise(std::move(strikePromise)) {
    SkASSERT(!fPositions.empty());
}

void
DrawableOpSubmitter::submitDraws(SkCanvas* canvas, SkPoint drawOrigin,const SkPaint& paint) const {
    // Convert glyph IDs to Drawables if it hasn't been done yet.
    fConvertIDsToDrawables([&]() {
        fStrikePromise.strike()->glyphIDsToDrawables(fIDsOrDrawables);
        // Do not call resetStrike() because the strike must remain owned to ensure the Drawable
        // data is not freed.
    });

    // Calculate the matrix that maps the path glyphs from their size in the strike to
    // the graphics source space.
    SkMatrix strikeToSource = SkMatrix::Scale(fStrikeToSourceScale, fStrikeToSourceScale);
    strikeToSource.postTranslate(drawOrigin.x(), drawOrigin.y());

    // Transform the path to device because the deviceMatrix must be unchanged to
    // draw effect, filter or shader paths.
    for (auto [i, position] : SkMakeEnumerate(fPositions)) {
        SkDrawable* drawable = fIDsOrDrawables[i].fDrawable;

        if (drawable == nullptr) {
            // This better be pinned to keep the drawable data alive.
            fStrikePromise.strike()->verifyPinnedStrike();
            SkDEBUGFAIL("Drawable should not be nullptr.");
            continue;
        }

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

// -- DrawableSubRun -------------------------------------------------------------------------------
class DrawableSubRun : public SubRun {
public:
    DrawableSubRun(DrawableOpSubmitter&& drawingDrawing)
            : fDrawingDrawing(std::move(drawingDrawing)) {}

    static SubRunOwner Make(SkZip<const SkGlyphID, const SkPoint> drawables,
                            SkScalar strikeToSourceScale,
                            SkStrikePromise&& strikePromise,
                            SubRunAllocator* alloc) {
        return alloc->makeUnique<DrawableSubRun>(
                DrawableOpSubmitter::Make(drawables,
                                          strikeToSourceScale,
                                          std::move(strikePromise),
                                          alloc));
    }

    static SubRunOwner MakeFromBuffer(SkReadBuffer& buffer,
                                      SubRunAllocator* alloc,
                                      const SkStrikeClient* client);
#if defined(SK_GANESH)
    void draw(SkCanvas* canvas,
              const GrClip* clip,
              const SkMatrixProvider& viewMatrix,
              SkPoint drawOrigin,
              const SkPaint& paint,
              sk_sp<SkRefCnt> subRunStorage,
              skgpu::ganesh::SurfaceDrawContext* sdc) const override {
        fDrawingDrawing.submitDraws(canvas, drawOrigin, paint);
    }
#endif  // defined(SK_GANESH)
#if defined(SK_GRAPHITE)
    void draw(SkCanvas* canvas,
              SkPoint drawOrigin,
              const SkPaint& paint,
              sk_sp<SkRefCnt> subRunStorage,
              Device* device) const override {
        fDrawingDrawing.submitDraws(canvas, drawOrigin, paint);
    }
#endif  // SK_GRAPHITE

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

SubRunOwner DrawableSubRun::MakeFromBuffer(SkReadBuffer& buffer,
                                           SubRunAllocator* alloc,
                                           const SkStrikeClient* client) {
    auto drawableOpSubmitter = DrawableOpSubmitter::MakeFromBuffer(buffer, alloc, client);
    if (!buffer.validate(drawableOpSubmitter.has_value())) { return nullptr; }
    return alloc->makeUnique<DrawableSubRun>(std::move(*drawableOpSubmitter));
}

bool DrawableSubRun::canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const {
    return true;
}

const AtlasSubRun* DrawableSubRun::testingOnly_atlasSubRun() const {
    return nullptr;
}

#if defined(SK_GANESH)
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
#endif  // defined(SK_GANESH)

// -- DirectMaskSubRun -----------------------------------------------------------------------------
class DirectMaskSubRun final : public SubRun, public AtlasSubRun {
public:
    DirectMaskSubRun(VertexFiller&& vertexFiller,
                     GlyphVector&& glyphs)
            : fVertexFiller{std::move(vertexFiller)}
            , fGlyphs{std::move(glyphs)} {}

    static SubRunOwner Make(SkRect creationBounds,
                            SkZip<const SkPackedGlyphID, const SkPoint> accepted,
                            const SkMatrix& creationMatrix,
                            SkStrikePromise&& strikePromise,
                            MaskFormat maskType,
                            SubRunAllocator* alloc) {
        auto vertexFiller = VertexFiller::Make(maskType,
                                               creationMatrix,
                                               creationBounds,
                                               get_positions(accepted),
                                               alloc,
                                               kIsDirect);

        auto glyphVector =
                GlyphVector::Make(std::move(strikePromise), get_packedIDs(accepted), alloc);

        return alloc->makeUnique<DirectMaskSubRun>(std::move(vertexFiller), std::move(glyphVector));
    }

    static SubRunOwner MakeFromBuffer(SkReadBuffer& buffer,
                                      SubRunAllocator* alloc,
                                      const SkStrikeClient* client) {
        auto vertexFiller = VertexFiller::MakeFromBuffer(buffer, alloc);
        if (!buffer.validate(vertexFiller.has_value())) { return nullptr; }

        auto glyphVector = GlyphVector::MakeFromBuffer(buffer, client, alloc);
        if (!buffer.validate(glyphVector.has_value())) { return nullptr; }
        if (!buffer.validate(SkCount(glyphVector->glyphs()) == vertexFiller->count())) {
            return nullptr;
        }

        SkASSERT(buffer.isValid());
        return alloc->makeUnique<DirectMaskSubRun>(
                std::move(*vertexFiller), std::move(*glyphVector));
    }

#if defined(SK_GANESH)
    void draw(SkCanvas*,
              const GrClip* clip,
              const SkMatrixProvider& viewMatrix,
              SkPoint drawOrigin,
              const SkPaint& paint,
              sk_sp<SkRefCnt> subRunStorage,
              skgpu::ganesh::SurfaceDrawContext* sdc) const override {
        auto[drawingClip, op] = this->makeAtlasTextOp(
                clip, viewMatrix, drawOrigin, paint, std::move(subRunStorage), sdc);
        if (op != nullptr) {
            sdc->addDrawOp(drawingClip, std::move(op));
        }
    }
#endif  // defined(SK_GANESH)

#ifdef SK_GRAPHITE
    void draw(SkCanvas*,
              SkPoint drawOrigin,
              const SkPaint& paint,
              sk_sp<SkRefCnt> subRunStorage,
              Device* device) const override {
    this->AtlasSubRun::draw(device, drawOrigin, paint, std::move(subRunStorage));
}
#endif  // defined(SK_GRAPHITE)

    int unflattenSize() const override {
        return sizeof(DirectMaskSubRun) +
               fGlyphs.unflattenSize() +
               fVertexFiller.unflattenSize();
    }

    int glyphCount() const override {
        return SkCount(fGlyphs.glyphs());
    }

    MaskFormat maskFormat() const override { return fVertexFiller.grMaskType(); }

    void testingOnly_packedGlyphIDToGlyph(StrikeCache* cache) const override {
        fGlyphs.packedGlyphIDToGlyph(cache);
    }

#if defined(SK_GANESH)
    size_t vertexStride(const SkMatrix& drawMatrix) const override {
        return fVertexFiller.vertexStride(drawMatrix);
    }

    std::tuple<const GrClip*, GrOp::Owner> makeAtlasTextOp(
            const GrClip* clip,
            const SkMatrixProvider& viewMatrix,
            SkPoint drawOrigin,
            const SkPaint& paint,
            sk_sp<SkRefCnt>&& subRunStorage,
            skgpu::ganesh::SurfaceDrawContext* sdc) const override {
        SkASSERT(this->glyphCount() != 0);
        const SkMatrix& drawMatrix = viewMatrix.localToDevice();
        const SkMatrix& positionMatrix = position_matrix(drawMatrix, drawOrigin);

        auto [integerTranslate, subRunDeviceBounds] =
                fVertexFiller.deviceRectAndCheckTransform(positionMatrix);
        if (subRunDeviceBounds.isEmpty()) {
            return {nullptr, nullptr};
        }
        // Rect for optimized bounds clipping when doing an integer translate.
        SkIRect geometricClipRect = SkIRect::MakeEmpty();
        if (integerTranslate) {
            // We can clip geometrically using clipRect and ignore clip when an axis-aligned
            // rectangular non-AA clip is used. If clipRect is empty, and clip is nullptr, then
            // there is no clipping needed.
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
        const SkPMColor4f drawingColor = calculate_colors(sdc,
                                                          paint,
                                                          drawMatrix,
                                                          fVertexFiller.grMaskType(),
                                                          &grPaint);

        auto geometry = AtlasTextOp::Geometry::Make(*this,
                                                    drawMatrix,
                                                    drawOrigin,
                                                    geometricClipRect,
                                                    std::move(subRunStorage),
                                                    drawingColor,
                                                    sdc->arenaAlloc());

        GrRecordingContext* const rContext = sdc->recordingContext();
        GrOp::Owner op = GrOp::Make<AtlasTextOp>(rContext,
                                                 fVertexFiller.opMaskType(),
                                                 !integerTranslate,
                                                 this->glyphCount(),
                                                 subRunDeviceBounds,
                                                 geometry,
                                                 std::move(grPaint));
        return {clip, std::move(op)};
    }

    std::tuple<bool, int>
    regenerateAtlas(int begin, int end, GrMeshDrawTarget* target) const override {
        return fGlyphs.regenerateAtlas(begin, end, fVertexFiller.grMaskType(), 0, target);
    }

    void fillVertexData(void* vertexDst, int offset, int count,
                        GrColor color,
                        const SkMatrix& drawMatrix, SkPoint drawOrigin,
                        SkIRect clip) const override {
        const SkMatrix positionMatrix = position_matrix(drawMatrix, drawOrigin);
        fVertexFiller.fillVertexData(offset, count,
                                     fGlyphs.glyphs(),
                                     color,
                                     positionMatrix,
                                     clip,
                                     vertexDst);
    }
#endif  // defined(SK_GANESH)

#if defined(SK_GRAPHITE)
    std::tuple<bool, int>
    regenerateAtlas(int begin, int end, Recorder* target) const override {
        return fGlyphs.regenerateAtlas(begin, end, fVertexFiller.grMaskType(), 0, target);
    }

    std::tuple<gr::Rect, Transform> boundsAndDeviceMatrix(
            const Transform& localToDevice, SkPoint drawOrigin) const override {
        return fVertexFiller.boundsAndDeviceMatrix(localToDevice, drawOrigin);
    }

    const Renderer* renderer(const RendererProvider* renderers) const override {
        return renderers->bitmapText();
    }

    void fillInstanceData(DrawWriter* dw,
                          int offset, int count,
                          int ssboIndex,
                          SkScalar depth) const override {
        unsigned short flags = (unsigned short)fVertexFiller.grMaskType();
        fVertexFiller.fillInstanceData(dw,
                                       offset, count,
                                       flags,
                                       ssboIndex,
                                       fGlyphs.glyphs(),
                                       depth);
    }
#endif  // defined(SK_GRAPHITE)

    bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const override {
        auto [reuse, _] = fVertexFiller.deviceRectAndCheckTransform(positionMatrix);
        return reuse;
    }

    const AtlasSubRun* testingOnly_atlasSubRun() const override {
        return this;
    }

protected:
    SubRunType subRunType() const override { return kDirectMask; }

    void doFlatten(SkWriteBuffer& buffer) const override {
        fVertexFiller.flatten(buffer);
        fGlyphs.flatten(buffer);
    }

private:
    const VertexFiller fVertexFiller;

    // The regenerateAtlas method mutates fGlyphs. It should be called from onPrepare which must
    // be single threaded.
    mutable GlyphVector fGlyphs;
};

// -- TransformedMaskSubRun ------------------------------------------------------------------------
class TransformedMaskSubRun final : public SubRun, public AtlasSubRun {
public:
    TransformedMaskSubRun(bool isBigEnough,
                          VertexFiller&& vertexFiller,
                          GlyphVector&& glyphs)
            : fIsBigEnough{isBigEnough}
            , fVertexFiller{std::move(vertexFiller)}
            , fGlyphs{std::move(glyphs)} {}

    static SubRunOwner Make(SkZip<const SkPackedGlyphID, const SkPoint> accepted,
                            const SkMatrix& initialPositionMatrix,
                            SkStrikePromise&& strikePromise,
                            SkMatrix creationMatrix,
                            SkRect creationBounds,
                            MaskFormat maskType,
                            SubRunAllocator* alloc) {
        auto vertexFiller = VertexFiller::Make(maskType,
                                               creationMatrix,
                                               creationBounds,
                                               get_positions(accepted),
                                               alloc,
                                               kIsTransformed);

        auto glyphVector = GlyphVector::Make(
                std::move(strikePromise), get_packedIDs(accepted), alloc);

        return alloc->makeUnique<TransformedMaskSubRun>(
                initialPositionMatrix.getMaxScale() >= 1,
                std::move(vertexFiller),
                std::move(glyphVector));
    }

    static SubRunOwner MakeFromBuffer(SkReadBuffer& buffer,
                                      SubRunAllocator* alloc,
                                      const SkStrikeClient* client) {
        auto vertexFiller = VertexFiller::MakeFromBuffer(buffer, alloc);
        if (!buffer.validate(vertexFiller.has_value())) { return nullptr; }

        auto glyphVector = GlyphVector::MakeFromBuffer(buffer, client, alloc);
        if (!buffer.validate(glyphVector.has_value())) { return nullptr; }
        if (!buffer.validate(SkCount(glyphVector->glyphs()) == vertexFiller->count())) {
            return nullptr;
        }
        const bool isBigEnough = buffer.readBool();
        return alloc->makeUnique<TransformedMaskSubRun>(
                isBigEnough, std::move(*vertexFiller), std::move(*glyphVector));
    }

    int unflattenSize() const override {
        return sizeof(TransformedMaskSubRun) +
               fGlyphs.unflattenSize() +
               fVertexFiller.unflattenSize();
    }

    bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const override {
        // If we are not scaling the cache entry to be larger, than a cache with smaller glyphs may
        // be better.
        return fIsBigEnough;
    }

    const AtlasSubRun* testingOnly_atlasSubRun() const override { return this; }

    void testingOnly_packedGlyphIDToGlyph(StrikeCache *cache) const override {
        fGlyphs.packedGlyphIDToGlyph(cache);
    }

    int glyphCount() const override { return SkCount(fGlyphs.glyphs()); }

    MaskFormat maskFormat() const override { return fVertexFiller.grMaskType(); }

#if defined(SK_GANESH)

    void draw(SkCanvas*,
              const GrClip* clip,
              const SkMatrixProvider& viewMatrix,
              SkPoint drawOrigin,
              const SkPaint& paint,
              sk_sp<SkRefCnt> subRunStorage,
              skgpu::ganesh::SurfaceDrawContext* sdc) const override {
        auto[drawingClip, op] = this->makeAtlasTextOp(
                clip, viewMatrix, drawOrigin, paint, std::move(subRunStorage), sdc);
        if (op != nullptr) {
            sdc->addDrawOp(drawingClip, std::move(op));
        }
    }

    std::tuple<const GrClip*, GrOp::Owner> makeAtlasTextOp(
            const GrClip* clip,
            const SkMatrixProvider& viewMatrix,
            SkPoint drawOrigin,
            const SkPaint& paint,
            sk_sp<SkRefCnt>&& subRunStorage,
            skgpu::ganesh::SurfaceDrawContext* sdc) const override {
        SkASSERT(this->glyphCount() != 0);

        const SkMatrix& drawMatrix = viewMatrix.localToDevice();

        GrPaint grPaint;
        SkPMColor4f drawingColor = calculate_colors(sdc,
                                                    paint,
                                                    drawMatrix,
                                                    fVertexFiller.grMaskType(),
                                                    &grPaint);

        auto geometry = AtlasTextOp::Geometry::Make(*this,
                                                    drawMatrix,
                                                    drawOrigin,
                                                    SkIRect::MakeEmpty(),
                                                    std::move(subRunStorage),
                                                    drawingColor,
                                                    sdc->arenaAlloc());

        GrRecordingContext* const rContext = sdc->recordingContext();
        SkMatrix positionMatrix = position_matrix(drawMatrix, drawOrigin);
        auto [_, deviceRect] = fVertexFiller.deviceRectAndCheckTransform(positionMatrix);
        GrOp::Owner op = GrOp::Make<AtlasTextOp>(rContext,
                                                 fVertexFiller.opMaskType(),
                                                 true,
                                                 this->glyphCount(),
                                                 deviceRect,
                                                 geometry,
                                                 std::move(grPaint));
        return {clip, std::move(op)};
    }

    std::tuple<bool, int> regenerateAtlas(int begin, int end,
                                          GrMeshDrawTarget* target) const override {
        return fGlyphs.regenerateAtlas(begin, end, fVertexFiller.grMaskType(), 1, target);
    }

    void fillVertexData(
            void* vertexDst, int offset, int count,
            GrColor color,
            const SkMatrix& drawMatrix, SkPoint drawOrigin,
            SkIRect clip) const override {
        const SkMatrix positionMatrix = position_matrix(drawMatrix, drawOrigin);
        fVertexFiller.fillVertexData(offset, count,
                                     fGlyphs.glyphs(),
                                     color,
                                     positionMatrix,
                                     clip,
                                     vertexDst);
    }

    size_t vertexStride(const SkMatrix& drawMatrix) const override {
        return fVertexFiller.vertexStride(drawMatrix);
    }

#endif  // defined(SK_GANESH)

#if defined(SK_GRAPHITE)

    void draw(SkCanvas*,
              SkPoint drawOrigin,
              const SkPaint& paint,
              sk_sp<SkRefCnt> subRunStorage,
              Device* device) const override {
        this->AtlasSubRun::draw(device, drawOrigin, paint, std::move(subRunStorage));
    }

    std::tuple<bool, int> regenerateAtlas(int begin, int end, Recorder* recorder) const override {
        return fGlyphs.regenerateAtlas(begin, end, fVertexFiller.grMaskType(), 1, recorder);
    }

    std::tuple<gr::Rect, Transform> boundsAndDeviceMatrix(const Transform& localToDevice,
                                                          SkPoint drawOrigin) const override {
        return fVertexFiller.boundsAndDeviceMatrix(localToDevice, drawOrigin);
    }

    const Renderer* renderer(const RendererProvider* renderers) const override {
        return renderers->bitmapText();
    }

    void fillInstanceData(DrawWriter* dw,
                          int offset, int count,
                          int ssboIndex,
                          SkScalar depth) const override {
        unsigned short flags = (unsigned short)fVertexFiller.grMaskType();
        fVertexFiller.fillInstanceData(dw,
                                       offset, count,
                                       flags,
                                       ssboIndex,
                                       fGlyphs.glyphs(),
                                       depth);
    }

#endif  // SK_GRAPHITE

protected:
    SubRunType subRunType() const override { return kTransformMask; }

    void doFlatten(SkWriteBuffer& buffer) const override {
        fVertexFiller.flatten(buffer);
        fGlyphs.flatten(buffer);
        buffer.writeBool(fIsBigEnough);
    }

private:
    const bool fIsBigEnough;

    const VertexFiller fVertexFiller;

    // The regenerateAtlas method mutates fGlyphs. It should be called from onPrepare which must
    // be single threaded.
    mutable GlyphVector fGlyphs;
};  // class TransformedMaskSubRun

// -- SDFTSubRun -----------------------------------------------------------------------------------

bool has_some_antialiasing(const SkFont& font ) {
    SkFont::Edging edging = font.getEdging();
    return edging == SkFont::Edging::kAntiAlias
           || edging == SkFont::Edging::kSubpixelAntiAlias;
}

#if !defined(SK_DISABLE_SDF_TEXT)

#if defined(SK_GANESH)

static std::tuple<AtlasTextOp::MaskType, uint32_t, bool> calculate_sdf_parameters(
        const skgpu::ganesh::SurfaceDrawContext& sdc,
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
    DFGPFlags |= drawMatrix.hasPerspective() ? kPerspective_DistanceFieldEffectFlag : 0;

    if (isLCD) {
        DFGPFlags |= kUseLCD_DistanceFieldEffectFlag;
        DFGPFlags |= MT::kLCDBGRDistanceField == maskType ? kBGR_DistanceFieldEffectFlag : 0;
    }
    return {maskType, DFGPFlags, useGammaCorrectDistanceTable};
}

#endif  // defined(SK_GANESH)

class SDFTSubRun final : public SubRun, public AtlasSubRun {
public:
    SDFTSubRun(bool useLCDText,
               bool antiAliased,
               const SDFTMatrixRange& matrixRange,
               VertexFiller&& vertexFiller,
               GlyphVector&& glyphs)
        : fUseLCDText{useLCDText}
        , fAntiAliased{antiAliased}
        , fMatrixRange{matrixRange}
        , fVertexFiller{std::move(vertexFiller)}
        , fGlyphs{std::move(glyphs)} { }

    static SubRunOwner Make(SkZip<const SkPackedGlyphID, const SkPoint> accepted,
                            const SkFont& runFont,
                            SkStrikePromise&& strikePromise,
                            const SkMatrix& creationMatrix,
                            SkRect creationBounds,
                            const SDFTMatrixRange& matrixRange,
                            SubRunAllocator* alloc) {
        auto vertexFiller = VertexFiller::Make(MaskFormat::kA8,
                                               creationMatrix,
                                               creationBounds,
                                               get_positions(accepted),
                                               alloc,
                                               kIsTransformed);

        auto glyphVector = GlyphVector::Make(
                std::move(strikePromise), get_packedIDs(accepted), alloc);

        return alloc->makeUnique<SDFTSubRun>(
                runFont.getEdging() == SkFont::Edging::kSubpixelAntiAlias,
                has_some_antialiasing(runFont),
                matrixRange,
                std::move(vertexFiller),
                std::move(glyphVector));
    }

    static SubRunOwner MakeFromBuffer(SkReadBuffer& buffer,
                                      SubRunAllocator* alloc,
                                      const SkStrikeClient* client) {
        int useLCD = buffer.readInt();
        int isAntiAliased = buffer.readInt();
        SDFTMatrixRange matrixRange = SDFTMatrixRange::MakeFromBuffer(buffer);
        auto vertexFiller = VertexFiller::MakeFromBuffer(buffer, alloc);
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

    int unflattenSize() const override {
        return sizeof(SDFTSubRun) + fGlyphs.unflattenSize() + fVertexFiller.unflattenSize();
    }

    bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const override {
        return fMatrixRange.matrixInRange(positionMatrix);
    }

    const AtlasSubRun* testingOnly_atlasSubRun() const override { return this; }

    void testingOnly_packedGlyphIDToGlyph(StrikeCache *cache) const override {
        fGlyphs.packedGlyphIDToGlyph(cache);
    }

    int glyphCount() const override { return fVertexFiller.count(); }
    MaskFormat maskFormat() const override { return fVertexFiller.grMaskType(); }

#if defined(SK_GANESH)
    void draw(SkCanvas*,
              const GrClip* clip,
              const SkMatrixProvider& viewMatrix,
              SkPoint drawOrigin,
              const SkPaint& paint,
              sk_sp<SkRefCnt> subRunStorage,
              skgpu::ganesh::SurfaceDrawContext* sdc) const override {
        auto[drawingClip, op] = this->makeAtlasTextOp(
                clip, viewMatrix, drawOrigin, paint, std::move(subRunStorage), sdc);
        if (op != nullptr) {
            sdc->addDrawOp(drawingClip, std::move(op));
        }
    }

    std::tuple<const GrClip*, GrOp::Owner> makeAtlasTextOp(
            const GrClip* clip,
            const SkMatrixProvider& viewMatrix,
            SkPoint drawOrigin,
            const SkPaint& paint,
            sk_sp<SkRefCnt>&& subRunStorage,
            skgpu::ganesh::SurfaceDrawContext* sdc) const override {
        SkASSERT(this->glyphCount() != 0);

        const SkMatrix& drawMatrix = viewMatrix.localToDevice();

        GrPaint grPaint;
        SkPMColor4f drawingColor = calculate_colors(sdc,
                                                    paint,
                                                    drawMatrix,
                                                    MaskFormat::kA8,
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
        SkMatrix positionMatrix = position_matrix(drawMatrix, drawOrigin);
        auto [_, deviceRect] = fVertexFiller.deviceRectAndCheckTransform(positionMatrix);
        GrOp::Owner op = GrOp::Make<AtlasTextOp>(rContext,
                                                 maskType,
                                                 true,
                                                 this->glyphCount(),
                                                 deviceRect,
                                                 SkPaintPriv::ComputeLuminanceColor(paint),
                                                 useGammaCorrectDistanceTable,
                                                 DFGPFlags,
                                                 geometry,
                                                 std::move(grPaint));

        return {clip, std::move(op)};
    }

    std::tuple<bool, int> regenerateAtlas(
            int begin, int end, GrMeshDrawTarget* target) const override {
        return fGlyphs.regenerateAtlas(begin, end, MaskFormat::kA8, SK_DistanceFieldInset, target);
    }

    void fillVertexData(
            void *vertexDst, int offset, int count,
            GrColor color,
            const SkMatrix& drawMatrix, SkPoint drawOrigin,
            SkIRect clip) const override {
        const SkMatrix positionMatrix = position_matrix(drawMatrix, drawOrigin);

        fVertexFiller.fillVertexData(offset, count,
                                     fGlyphs.glyphs(),
                                     color,
                                     positionMatrix,
                                     clip,
                                     vertexDst);
    }

    size_t vertexStride(const SkMatrix& drawMatrix) const override {
        return fVertexFiller.vertexStride(drawMatrix);
    }

#endif  // defined(SK_GANESH)

#if defined(SK_GRAPHITE)

    void draw(SkCanvas*,
              SkPoint drawOrigin,
              const SkPaint& paint,
              sk_sp<SkRefCnt> subRunStorage,
              Device* device) const override {
        this->AtlasSubRun::draw(device, drawOrigin, paint, std::move(subRunStorage));
    }

    std::tuple<bool, int> regenerateAtlas(int begin, int end, Recorder *recorder) const override {
        return fGlyphs.regenerateAtlas(
                begin, end, MaskFormat::kA8, SK_DistanceFieldInset, recorder);
    }

    std::tuple<gr::Rect, Transform> boundsAndDeviceMatrix(const Transform& localToDevice,
                                                          SkPoint drawOrigin) const override {
        return fVertexFiller.boundsAndDeviceMatrix(localToDevice, drawOrigin);
    }

    const Renderer* renderer(const RendererProvider* renderers) const override {
        return renderers->sdfText(fUseLCDText);
    }

    void fillInstanceData(DrawWriter* dw,
                          int offset, int count,
                          int ssboIndex,
                          SkScalar depth) const override {
        fVertexFiller.fillInstanceData(dw,
                                       offset, count, /*flags=*/0,
                                       ssboIndex,
                                       fGlyphs.glyphs(),
                                       depth);
    }

#endif  // SK_GRAPHITE

protected:
    SubRunType subRunType() const override { return kSDFT; }
    void doFlatten(SkWriteBuffer& buffer) const override {
        buffer.writeInt(fUseLCDText);
        buffer.writeInt(fAntiAliased);
        fMatrixRange.flatten(buffer);
        fVertexFiller.flatten(buffer);
        fGlyphs.flatten(buffer);
    }

private:
    const bool fUseLCDText;
    const bool fAntiAliased;
    const SDFTMatrixRange fMatrixRange;

    const VertexFiller fVertexFiller;

    // The regenerateAtlas method mutates fGlyphs. It should be called from onPrepare which must
    // be single threaded.
    mutable GlyphVector fGlyphs;
};  // class SDFTSubRun

#endif // !defined(SK_DISABLE_SDF_TEXT)

// -- SubRun ---------------------------------------------------------------------------------------

template<typename AddSingleMaskFormat>
void add_multi_mask_format(
        AddSingleMaskFormat addSingleMaskFormat,
        SkZip<const SkPackedGlyphID, const SkPoint, const SkMask::Format> accepted) {
    if (accepted.empty()) { return; }

    auto maskSpan = accepted.get<2>();
    MaskFormat format = Glyph::FormatFromSkGlyph(maskSpan[0]);
    size_t startIndex = 0;
    for (size_t i = 1; i < accepted.size(); i++) {
        MaskFormat nextFormat = Glyph::FormatFromSkGlyph(maskSpan[i]);
        if (format != nextFormat) {
            auto interval = accepted.subspan(startIndex, i - startIndex);
            // Only pass the packed glyph ids and positions.
            auto glyphsWithSameFormat = SkMakeZip(interval.get<0>(), interval.get<1>());
            // Take a ref on the strike. This should rarely happen.
            addSingleMaskFormat(glyphsWithSameFormat, format);
            format = nextFormat;
            startIndex = i;
        }
    }
    auto interval = accepted.last(accepted.size() - startIndex);
    auto glyphsWithSameFormat = SkMakeZip(interval.get<0>(), interval.get<1>());
    addSingleMaskFormat(glyphsWithSameFormat, format);
}
}  // namespace

namespace sktext::gpu {
SubRun::~SubRun() = default;
void SubRun::flatten(SkWriteBuffer& buffer) const {
    buffer.writeInt(this->subRunType());
    this->doFlatten(buffer);
}

SubRunOwner SubRun::MakeFromBuffer(SkReadBuffer& buffer,
                                   SubRunAllocator* alloc,
                                   const SkStrikeClient* client) {
    using Maker = SubRunOwner (*)(SkReadBuffer&,
                                  SubRunAllocator*,
                                  const SkStrikeClient*);

    static Maker makers[kSubRunTypeCount] = {
            nullptr,                                             // 0 index is bad.
            DirectMaskSubRun::MakeFromBuffer,
#if !defined(SK_DISABLE_SDF_TEXT)
            SDFTSubRun::MakeFromBuffer,
#endif
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
    return maker(buffer, alloc, client);
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
        auto subRunOwner = SubRun::MakeFromBuffer(buffer, alloc, client);
        if (!buffer.validate(subRunOwner != nullptr)) { return nullptr; }
        if (subRunOwner != nullptr) {
            container->fSubRuns.append(std::move(subRunOwner));
        }
    }
    return container;
}

size_t SubRunContainer::EstimateAllocSize(const GlyphRunList& glyphRunList) {
    // The difference in alignment from the per-glyph data to the SubRun;
    constexpr size_t alignDiff = alignof(DirectMaskSubRun) - alignof(SkPoint);
    constexpr size_t vertexDataToSubRunPadding = alignDiff > 0 ? alignDiff : 0;
    size_t totalGlyphCount = glyphRunList.totalGlyphCount();
    // This is optimized for DirectMaskSubRun which is by far the most common case.
    return totalGlyphCount * sizeof(SkPoint)
           + GlyphVector::GlyphVectorSize(totalGlyphCount)
           + glyphRunList.runCount() * (sizeof(DirectMaskSubRun) + vertexDataToSubRunPadding)
           + sizeof(SubRunContainer);
}

SkScalar find_maximum_glyph_dimension(StrikeForGPU* strike, SkSpan<const SkGlyphID> glyphs) {
    StrikeMutationMonitor m{strike};
    SkScalar maxDimension = 0;
    for (SkGlyphID glyphID : glyphs) {
        SkGlyphDigest digest = strike->digestFor(kMask, SkPackedGlyphID{glyphID});
        maxDimension = std::max(static_cast<SkScalar>(digest.maxDimension()), maxDimension);
    }

    return maxDimension;
}

#if !defined(SK_DISABLE_SDF_TEXT)
std::tuple<SkZip<const SkPackedGlyphID, const SkPoint>, SkZip<SkGlyphID, SkPoint>, SkRect>
prepare_for_SDFT_drawing(StrikeForGPU* strike,
                         const SkMatrix& creationMatrix,
                         SkZip<const SkGlyphID, const SkPoint> source,
                         SkZip<SkPackedGlyphID, SkPoint> acceptedBuffer,
                         SkZip<SkGlyphID, SkPoint> rejectedBuffer) {
    int acceptedSize = 0,
        rejectedSize = 0;
    SkGlyphRect boundingRect = skglyph::empty_rect();
    StrikeMutationMonitor m{strike};
    for (const auto [glyphID, pos] : source) {
        if (!SkScalarsAreFinite(pos.x(), pos.y())) {
            continue;
        }

        const SkPackedGlyphID packedID{glyphID};
        switch (const SkGlyphDigest digest = strike->digestFor(kSDFT, packedID);
                digest.actionFor(kSDFT)) {
            case GlyphAction::kAccept: {
                SkPoint mappedPos = creationMatrix.mapPoint(pos);
                const SkGlyphRect glyphBounds =
                    digest.bounds()
                        // The SDFT glyphs have 2-pixel wide padding that should
                        // not be used in calculating the source rectangle.
                        .inset(SK_DistanceFieldInset, SK_DistanceFieldInset)
                        .offset(mappedPos);
                boundingRect = skglyph::rect_union(boundingRect, glyphBounds);
                acceptedBuffer[acceptedSize++] = std::make_tuple(packedID, glyphBounds.leftTop());
                break;
            }
            case GlyphAction::kReject:
                rejectedBuffer[rejectedSize++] = std::make_tuple(glyphID, pos);
            break;
            default:
                break;
        }
    }

    return {acceptedBuffer.first(acceptedSize),
            rejectedBuffer.first(rejectedSize),
            boundingRect.rect()};
}
#endif

std::tuple<SkZip<const SkPackedGlyphID, const SkPoint, const SkMask::Format>,
           SkZip<SkGlyphID, SkPoint>,
           SkRect>
prepare_for_direct_mask_drawing(StrikeForGPU* strike,
                                const SkMatrix& positionMatrix,
                                SkZip<const SkGlyphID, const SkPoint> source,
                                SkZip<SkPackedGlyphID, SkPoint, SkMask::Format> acceptedBuffer,
                                SkZip<SkGlyphID, SkPoint> rejectedBuffer) {
    const SkIPoint mask = strike->roundingSpec().ignorePositionFieldMask;
    const SkPoint halfSampleFreq = strike->roundingSpec().halfAxisSampleFreq;

    // Build up the mapping from source space to device space. Add the rounding constant
    // halfSampleFreq, so we just need to floor to get the device result.
    SkMatrix positionMatrixWithRounding = positionMatrix;
    positionMatrixWithRounding.postTranslate(halfSampleFreq.x(), halfSampleFreq.y());

    int acceptedSize = 0,
        rejectedSize = 0;
    SkGlyphRect boundingRect = skglyph::empty_rect();
    StrikeMutationMonitor m{strike};
    for (auto [glyphID, pos] : source) {
        if (!SkScalarsAreFinite(pos.x(), pos.y())) {
            continue;
        }

        const SkPoint mappedPos = positionMatrixWithRounding.mapPoint(pos);
        const SkPackedGlyphID packedID{glyphID, mappedPos, mask};
        switch (const SkGlyphDigest digest = strike->digestFor(kDirectMask, packedID);
                digest.actionFor(kDirectMask)) {
            case GlyphAction::kAccept: {
                const SkPoint roundedPos{SkScalarFloorToScalar(mappedPos.x()),
                                         SkScalarFloorToScalar(mappedPos.y())};
                const SkGlyphRect glyphBounds = digest.bounds().offset(roundedPos);
                boundingRect = skglyph::rect_union(boundingRect, glyphBounds);
                acceptedBuffer[acceptedSize++] =
                        std::make_tuple(packedID, glyphBounds.leftTop(), digest.maskFormat());
                break;
            }
            case GlyphAction::kReject:
                rejectedBuffer[rejectedSize++] = std::make_tuple(glyphID, pos);
                break;
            default:
                break;
        }
    }

    return {acceptedBuffer.first(acceptedSize),
            rejectedBuffer.first(rejectedSize),
            boundingRect.rect()};
}

std::tuple<SkZip<const SkPackedGlyphID, const SkPoint, const SkMask::Format>,
           SkZip<SkGlyphID, SkPoint>,
           SkRect>
prepare_for_mask_drawing(StrikeForGPU* strike,
                         const SkMatrix& creationMatrix,
                         SkZip<const SkGlyphID, const SkPoint> source,
                         SkZip<SkPackedGlyphID, SkPoint, SkMask::Format> acceptedBuffer,
                         SkZip<SkGlyphID, SkPoint> rejectedBuffer) {
    int acceptedSize = 0,
        rejectedSize = 0;
    SkGlyphRect boundingRect = skglyph::empty_rect();
    StrikeMutationMonitor m{strike};
    for (auto [glyphID, pos] : source) {
        if (!SkScalarsAreFinite(pos.x(), pos.y())) {
            continue;
        }

        const SkPackedGlyphID packedID{glyphID};
        switch (const SkGlyphDigest digest = strike->digestFor(kMask, packedID);
                digest.actionFor(kMask)) {
            case GlyphAction::kAccept: {
                const SkPoint mappedPos = creationMatrix.mapPoint(pos);
                const SkGlyphRect glyphBounds = digest.bounds().offset(mappedPos);
                boundingRect = skglyph::rect_union(boundingRect, glyphBounds);
                acceptedBuffer[acceptedSize++] =
                        std::make_tuple(packedID, glyphBounds.leftTop(), digest.maskFormat());
                break;
            }
            case GlyphAction::kReject:
                rejectedBuffer[rejectedSize++] = std::make_tuple(glyphID, pos);
                break;
            default:
                break;
        }
    }

    return {acceptedBuffer.first(acceptedSize),
            rejectedBuffer.first(rejectedSize),
            boundingRect.rect()};
}

std::tuple<SkZip<const SkGlyphID, const SkPoint>, SkZip<SkGlyphID, SkPoint>>
prepare_for_path_drawing(StrikeForGPU* strike,
                         SkZip<const SkGlyphID, const SkPoint> source,
                         SkZip<SkGlyphID, SkPoint> acceptedBuffer,
                         SkZip<SkGlyphID, SkPoint> rejectedBuffer) {
    int acceptedSize = 0;
    int rejectedSize = 0;
    StrikeMutationMonitor m{strike};
    for (const auto [glyphID, pos] : source) {
        if (!SkScalarsAreFinite(pos.x(), pos.y())) {
            continue;
        }

        switch (strike->digestFor(kPath, SkPackedGlyphID{glyphID}).actionFor(kPath)) {
            case GlyphAction::kAccept:
                acceptedBuffer[acceptedSize++] = std::make_tuple(glyphID, pos);
                break;
            case GlyphAction::kReject:
                rejectedBuffer[rejectedSize++] = std::make_tuple(glyphID, pos);
                break;
            default:
                break;
        }
    }
    return {acceptedBuffer.first(acceptedSize), rejectedBuffer.first(rejectedSize)};
}

std::tuple<SkZip<const SkGlyphID, const SkPoint>, SkZip<SkGlyphID, SkPoint>>
 prepare_for_drawable_drawing(StrikeForGPU* strike,
                             SkZip<const SkGlyphID, const SkPoint> source,
                             SkZip<SkGlyphID, SkPoint> acceptedBuffer,
                             SkZip<SkGlyphID, SkPoint> rejectedBuffer) {
    int acceptedSize = 0;
    int rejectedSize = 0;
    StrikeMutationMonitor m{strike};
    for (const auto [glyphID, pos] : source) {
        if (!SkScalarsAreFinite(pos.x(), pos.y())) {
            continue;
        }

        switch (strike->digestFor(kDrawable, SkPackedGlyphID{glyphID}).actionFor(kDrawable)) {
            case GlyphAction::kAccept:
                acceptedBuffer[acceptedSize++] = std::make_tuple(glyphID, pos);
                break;
            case GlyphAction::kReject:
                rejectedBuffer[rejectedSize++] = std::make_tuple(glyphID, pos);
                break;
            default:
                break;
        }
    }
    return {acceptedBuffer.first(acceptedSize), rejectedBuffer.first(rejectedSize)};
}

SubRunContainerOwner SubRunContainer::MakeInAlloc(
        const GlyphRunList& glyphRunList,
        const SkMatrix& positionMatrix,
        const SkPaint& runPaint,
        SkStrikeDeviceInfo strikeDeviceInfo,
        StrikeForGPUCacheInterface* strikeCache,
        SubRunAllocator* alloc,
        SubRunCreationBehavior creationBehavior,
        const char* tag) {
    SkASSERT(alloc != nullptr);
    SkASSERT(strikeDeviceInfo.fSDFTControl != nullptr);

    SubRunContainerOwner container = alloc->makeUnique<SubRunContainer>(positionMatrix);
    // If there is no SDFT description ignore all SubRuns.
    if (strikeDeviceInfo.fSDFTControl == nullptr) {
        return container;
    }

    const SkSurfaceProps deviceProps = strikeDeviceInfo.fSurfaceProps;
    const SkScalerContextFlags scalerContextFlags = strikeDeviceInfo.fScalerContextFlags;
#if !defined(SK_DISABLE_SDF_TEXT)
    const SDFTControl SDFTControl = *strikeDeviceInfo.fSDFTControl;
    const SkScalar maxMaskSize = SDFTControl.maxSize();
#else
    const SkScalar maxMaskSize = 256;
#endif

    // TODO: hoist the buffer structure to the GlyphRunBuilder. The buffer structure here is
    //  still begin tuned, and this is expected to be slower until tuned.
    const int maxGlyphRunSize = glyphRunList.maxGlyphRunSize();

    // Accepted buffers.
    SkSTArray<64, SkPackedGlyphID> acceptedPackedGlyphIDs;
    SkSTArray<64, SkGlyphID> acceptedGlyphIDs;
    SkSTArray<64, SkPoint> acceptedPositions;
    SkSTArray<64, SkMask::Format> acceptedFormats;
    acceptedPackedGlyphIDs.resize(maxGlyphRunSize);
    acceptedGlyphIDs.resize(maxGlyphRunSize);
    acceptedPositions.resize(maxGlyphRunSize);
    acceptedFormats.resize(maxGlyphRunSize);

    // Rejected buffers.
    SkSTArray<64, SkGlyphID> rejectedGlyphIDs;
    SkSTArray<64, SkPoint> rejectedPositions;
    rejectedGlyphIDs.resize(maxGlyphRunSize);
    rejectedPositions.resize(maxGlyphRunSize);
    const auto rejectedBuffer = SkMakeZip(rejectedGlyphIDs, rejectedPositions);

    const SkPoint glyphRunListLocation = glyphRunList.sourceBounds().center();

    // Handle all the runs in the glyphRunList
    for (auto& glyphRun : glyphRunList) {
        SkZip<const SkGlyphID, const SkPoint> source = glyphRun.source();
        const SkFont& runFont = glyphRun.font();

        const SkScalar approximateDeviceTextSize =
                // Since the positionMatrix has the origin prepended, use the plain
                // sourceBounds from above.
                SkFontPriv::ApproximateTransformedTextSize(runFont, positionMatrix,
                                                           glyphRunListLocation);

        // Atlas mask cases - SDFT and direct mask
        // Only consider using direct or SDFT drawing if not drawing hairlines and not too big.
        if ((runPaint.getStyle() != SkPaint::kStroke_Style || runPaint.getStrokeWidth() != 0) &&
                approximateDeviceTextSize < maxMaskSize) {

#if !defined(SK_DISABLE_SDF_TEXT)
            // SDFT case
            if (SDFTControl.isSDFT(approximateDeviceTextSize, runPaint, positionMatrix)) {
                // Process SDFT - This should be the .009% case.
                const auto& [strikeSpec, strikeToSourceScale, matrixRange] =
                        SkStrikeSpec::MakeSDFT(
                                runFont, runPaint, deviceProps, positionMatrix,
                                glyphRunListLocation, SDFTControl);

                if (!SkScalarNearlyZero(strikeToSourceScale)) {
                    sk_sp<StrikeForGPU> strike = strikeSpec.findOrCreateScopedStrike(strikeCache);

                    // The creationMatrix needs to scale the strike data when inverted and
                    // multiplied by the positionMatrix. The final CTM should be:
                    //   [positionMatrix][scale by strikeToSourceScale],
                    // which should equal the following because of the transform during the vertex
                    // calculation,
                    //   [positionMatrix][creationMatrix]^-1.
                    // So, the creation matrix needs to be
                    //   [scale by 1/strikeToSourceScale].
                    SkMatrix creationMatrix =
                            SkMatrix::Scale(1.f/strikeToSourceScale, 1.f/strikeToSourceScale);

                    auto acceptedBuffer = SkMakeZip(acceptedPackedGlyphIDs, acceptedPositions);
                    auto [accepted, rejected, creationBounds] = prepare_for_SDFT_drawing(
                            strike.get(), creationMatrix, source, acceptedBuffer, rejectedBuffer);
                    source = rejected;

                    if (creationBehavior == kAddSubRuns && !accepted.empty()) {
                        container->fSubRuns.append(SDFTSubRun::Make(
                                accepted,
                                runFont,
                                strike->strikePromise(),
                                creationMatrix,
                                creationBounds,
                                matrixRange,
                                alloc));
                    }
                }
            }
#endif  // !defined(SK_DISABLE_SDF_TEXT)

            // Direct Mask case
            // Handle all the directly mapped mask subruns.
            if (!source.empty() && !positionMatrix.hasPerspective()) {
                // Process masks including ARGB - this should be the 99.99% case.
                // This will handle medium size emoji that are sharing the run with SDFT drawn text.
                // If things are too big they will be passed along to the drawing of last resort
                // below.
                SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(
                        runFont, runPaint, deviceProps, scalerContextFlags, positionMatrix);

                sk_sp<StrikeForGPU> strike = strikeSpec.findOrCreateScopedStrike(strikeCache);

                auto acceptedBuffer = SkMakeZip(acceptedPackedGlyphIDs,
                                                acceptedPositions,
                                                acceptedFormats);
                auto [accepted, rejected, creationBounds] = prepare_for_direct_mask_drawing(
                        strike.get(), positionMatrix, source, acceptedBuffer, rejectedBuffer);
                source = rejected;

                if (creationBehavior == kAddSubRuns && !accepted.empty()) {
                    auto addGlyphsWithSameFormat =
                        [&, bounds = creationBounds](
                                SkZip<const SkPackedGlyphID, const SkPoint> subrun,
                                MaskFormat format) {
                            container->fSubRuns.append(
                                    DirectMaskSubRun::Make(bounds,
                                                           subrun,
                                                           container->initialPosition(),
                                                           strike->strikePromise(),
                                                           format,
                                                           alloc));
                        };
                    add_multi_mask_format(addGlyphsWithSameFormat, accepted);
                }
            }
        }

        // Drawable case
        // Handle all the drawable glyphs - usually large or perspective color glyphs.
        if (!source.empty()) {
            auto [strikeSpec, strikeToSourceScale] =
                    SkStrikeSpec::MakePath(runFont, runPaint, deviceProps, scalerContextFlags);

            if (!SkScalarNearlyZero(strikeToSourceScale)) {
                sk_sp<StrikeForGPU> strike = strikeSpec.findOrCreateScopedStrike(strikeCache);

                auto acceptedBuffer = SkMakeZip(acceptedGlyphIDs, acceptedPositions);
                auto [accepted, rejected] =
                prepare_for_drawable_drawing(strike.get(), source, acceptedBuffer, rejectedBuffer);
                source = rejected;

                if (creationBehavior == kAddSubRuns && !accepted.empty()) {
                    container->fSubRuns.append(
                            DrawableSubRun::Make(
                                accepted,
                                strikeToSourceScale,
                                strike->strikePromise(),
                                alloc));
                }
            }
        }

        // Path case
        // Handle path subruns. Mainly, large or large perspective glyphs with no color.
        if (!source.empty()) {
            auto [strikeSpec, strikeToSourceScale] =
                    SkStrikeSpec::MakePath(runFont, runPaint, deviceProps, scalerContextFlags);

            if (!SkScalarNearlyZero(strikeToSourceScale)) {
                sk_sp<StrikeForGPU> strike = strikeSpec.findOrCreateScopedStrike(strikeCache);

                auto acceptedBuffer = SkMakeZip(acceptedGlyphIDs, acceptedPositions);
                auto [accepted, rejected] =
                prepare_for_path_drawing(strike.get(), source, acceptedBuffer, rejectedBuffer);
                source = rejected;

                if (creationBehavior == kAddSubRuns && !accepted.empty()) {
                    container->fSubRuns.append(
                            PathSubRun::Make(accepted,
                                             has_some_antialiasing(runFont),
                                             strikeToSourceScale,
                                             strike->strikePromise(),
                                             alloc));
                }
            }
        }

        // Drawing of last resort case
        // Draw all the rest of the rejected glyphs from above. This scales out of the atlas to
        // the screen, so quality will suffer. This mainly handles large color or perspective
        // color not handled by Drawables.
        if (!source.empty() && !SkScalarNearlyZero(approximateDeviceTextSize)) {
            // Creation matrix will be changed below to meet the following criteria:
            // * No perspective - the font scaler and the strikes can't handle perspective masks.
            // * Fits atlas - creationMatrix will be conditioned so that the maximum glyph
            //   dimension for this run will be <  kMaxBilerpAtlasDimension.
            SkMatrix creationMatrix = positionMatrix;

            // Condition creationMatrix for perspective.
            if (creationMatrix.hasPerspective()) {
                // Find a scale factor that reduces pixelation caused by keystoning.
                SkPoint center = glyphRunList.sourceBounds().center();
                SkScalar maxAreaScale = SkMatrixPriv::DifferentialAreaScale(creationMatrix, center);
                SkScalar perspectiveFactor = 1;
                if (SkScalarIsFinite(maxAreaScale) && !SkScalarNearlyZero(maxAreaScale)) {
                    perspectiveFactor = SkScalarSqrt(maxAreaScale);
                }

                // Masks can not be created in perspective. Create a non-perspective font with a
                // scale that will support the perspective keystoning.
                creationMatrix = SkMatrix::Scale(perspectiveFactor, perspectiveFactor);
            }

            // Reduce to make a one pixel border for the bilerp padding.
            static const constexpr SkScalar kMaxBilerpAtlasDimension =
                    SkGlyphDigest::kSkSideTooBigForAtlas - 2;

            // Get the raw glyph IDs to simulate device drawing to figure the maximum device
            // dimension.
            const SkSpan<const SkGlyphID> glyphs = get_glyphIDs(source);

            // maxGlyphDimension always returns an integer even though the return type is SkScalar.
            auto maxGlyphDimension = [&](const SkMatrix& m) {
                const SkStrikeSpec strikeSpec = SkStrikeSpec::MakeTransformMask(
                        runFont, runPaint, deviceProps, scalerContextFlags, m);
                const sk_sp<StrikeForGPU> gaugingStrike =
                        strikeSpec.findOrCreateScopedStrike(strikeCache);
                const SkScalar maxDimension =
                        find_maximum_glyph_dimension(gaugingStrike.get(), glyphs);
                if (maxDimension == 0) {
                    // Text Scalers don't create glyphs with a dimension larger than 65535. For very
                    // large sizes, this will cause all the dimensions to go to zero. Use 65535 as
                    // the dimension.
                    // TODO: There is a problem where a small character (say .) and a large
                    //  character (say M) are in the same run. If the run is scaled to be very
                    //  large, then the M may return 0 because its dimensions are > 65535, but
                    //  the small character produces regular result because its largest dimension
                    //  is < 65535. This will create an improper scale factor causing the M to be
                    //  too large to fit in the atlas. Tracked by skia:13714.
                    return 65535.0f;
                }
                return maxDimension;
            };

            // Condition the creationMatrix so that glyphs fit in the atlas.
            for (SkScalar maxDimension = maxGlyphDimension(creationMatrix);
                 maxDimension <= 0 || kMaxBilerpAtlasDimension < maxDimension;
                 maxDimension = maxGlyphDimension(creationMatrix))
            {
                // The SkScalerContext has a limit of 65536 maximum dimension.
                // reductionFactor will always be < 1 because
                // maxDimension > kMaxBilerpAtlasDimension, and because maxDimension will always
                // be an integer the reduction factor will always be at most 254 / 255.
                SkScalar reductionFactor = kMaxBilerpAtlasDimension / maxDimension;
                creationMatrix.postScale(reductionFactor, reductionFactor);
            }

            // Draw using the creationMatrix.
            SkStrikeSpec strikeSpec = SkStrikeSpec::MakeTransformMask(
                    runFont, runPaint, deviceProps, scalerContextFlags, creationMatrix);

            sk_sp<StrikeForGPU> strike = strikeSpec.findOrCreateScopedStrike(strikeCache);

            auto acceptedBuffer =
                    SkMakeZip(acceptedPackedGlyphIDs, acceptedPositions, acceptedFormats);
            auto [accepted, rejected, creationBounds] =
                prepare_for_mask_drawing(
                        strike.get(), creationMatrix, source, acceptedBuffer, rejectedBuffer);
            source = rejected;

            if (creationBehavior == kAddSubRuns && !accepted.empty()) {

                auto addGlyphsWithSameFormat =
                        [&, bounds = creationBounds](
                                SkZip<const SkPackedGlyphID, const SkPoint> subrun,
                                MaskFormat format) {
                            container->fSubRuns.append(
                                    TransformedMaskSubRun::Make(subrun,
                                                                container->initialPosition(),
                                                                strike->strikePromise(),
                                                                creationMatrix,
                                                                bounds,
                                                                format,
                                                                alloc));
                        };
                add_multi_mask_format(addGlyphsWithSameFormat, accepted);
            }
        }
    }

    return container;
}

#if defined(SK_GANESH)
void SubRunContainer::draw(SkCanvas* canvas,
                           const GrClip* clip,
                           const SkMatrixProvider& viewMatrix,
                           SkPoint drawOrigin,
                           const SkPaint& paint,
                           const SkRefCnt* subRunStorage,
                           skgpu::ganesh::SurfaceDrawContext* sdc) const {
    for (auto& subRun : fSubRuns) {
        subRun.draw(canvas, clip, viewMatrix, drawOrigin, paint, sk_ref_sp(subRunStorage), sdc);
    }
}
#endif  // defined(SK_GANESH)

#if defined(SK_GRAPHITE)
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
