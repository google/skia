/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
#include "src/gpu/AtlasTypes.h"
#include "src/text/gpu/VertexFiller.h"
#include "src/gpu/graphite/Device.h"
#include "src/base/SkZip.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/text/gpu/Glyph.h"
#include "src/text/gpu/SubRunAllocator.h"
#include "src/text/gpu/SubRunContainer.h"

namespace sktext::gpu {

struct AtlasPt {
    uint16_t u;
    uint16_t v;
};

void VertexFiller::fillInstanceData(skgpu::graphite::DrawWriter* dw,
                                    int offset, int count,
                                    unsigned short flags,
                                    skvx::ushort2 ssboIndex,
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

std::tuple<Rect, Transform> VertexFiller::boundsAndDeviceMatrix(const Transform& localToDevice,
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
                                  localToDevice.type() != Transform::Type::kPerspective &&
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

}  // namespace sktext::gpu
