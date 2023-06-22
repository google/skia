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

}  // namespace sktext::gpu
