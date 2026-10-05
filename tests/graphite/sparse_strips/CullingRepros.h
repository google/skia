/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_sparse_strips_CullingRepros_DEFINED
#define skgpu_graphite_sparse_strips_CullingRepros_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkRect.h"
#include "src/core/SkFloatBits.h"

namespace skgpu::graphite::CullingRepros {

// The inputs of a single sparse strips draw: its path, transform, and device space clip.
struct Repro {
    SkPath fPath;
    SkMatrix fCtm;
    SkIRect fClip;
};

// A contour whose verbs lie entirely above, right of, and below `clip` (in that order), so Flatten
// culls all of them. Its closing edge runs down the left of the clip and is the only geometry that
// contributes winding inside it. The path covers the entire clip.
//
// Note: With `close`, SkPath::Iter emits the closing edge as a line verb, which Flatten never
// culls. Only the open (implicitly closed) form exercises Flatten's handling of the closing edge.
inline SkPath AllVerbsCulledContour(const SkIRect& clip, bool close) {
    const float l = static_cast<float>(clip.fLeft);
    const float t = static_cast<float>(clip.fTop);
    const float r = static_cast<float>(clip.fRight);
    const float b = static_cast<float>(clip.fBottom);

    SkPathBuilder builder;
    builder.moveTo(l - 10.0f, t - 20.0f)
            .quadTo((l + r) * 0.5f, t - 60.0f, r + 20.0f, t - 20.0f)         // Above
            .conicTo(r + 60.0f, (t + b) * 0.5f, r + 20.0f, b + 20.0f, 0.7f)  // Right
            .cubicTo(r, b + 60.0f, l, b + 60.0f, l - 12.0f, b + 20.0f);      // Below
    if (close) {
        builder.close();
    }
    return builder.detach();
}

// A draw captured from desk_tiger8svg.skp in the viewer, zoomed in far enough that it lies entirely
// outside of the clip. Its four cubics are culled (above, above, right, below), and the implicit
// closing edge runs down the left of the clip, covering all of it. Flatten used to drop the closing
// edge because no verb in the contour survived culling, leaving the clip unfilled.
inline Repro DeskTiger8Capture3Draw2() {
    SkPathBuilder builder(SkPathFillType::kWinding);
    builder.moveTo(SkBits2Float(0x43e93333), SkBits2Float(0x44059333));  // 466.4, 534.3
    builder.cubicTo(SkBits2Float(0x43e93333), SkBits2Float(0x44059333),
                    SkBits2Float(0x43ebc000), SkBits2Float(0x4400cccd),
                    SkBits2Float(0x43f68ccd), SkBits2Float(0x44017333));
    builder.cubicTo(SkBits2Float(0x43f68ccd), SkBits2Float(0x44017333),
                    SkBits2Float(0x43fdc000), SkBits2Float(0x44018ccd),
                    SkBits2Float(0x4400199a), SkBits2Float(0x44055999));
    builder.cubicTo(SkBits2Float(0x44007334), SkBits2Float(0x44066ccc),
                    SkBits2Float(0x44009334), SkBits2Float(0x44080666),
                    SkBits2Float(0x44000ccd), SkBits2Float(0x44097fff));
    builder.cubicTo(SkBits2Float(0x44000ccd), SkBits2Float(0x44097fff),
                    SkBits2Float(0x43fdc000), SkBits2Float(0x440d1999),
                    SkBits2Float(0x43f5199a), SkBits2Float(0x440cc665));

    // [379.604 0 -190171 | 0 379.604 -204002 | 0 0 1]
    const SkMatrix ctm = SkMatrix::MakeAll(
            SkBits2Float(0x43bdcd5e), SkBits2Float(0x00000000), SkBits2Float(0xc839b6c7),
            SkBits2Float(0x00000000), SkBits2Float(0x43bdcd5e), SkBits2Float(0xc8473891),
            SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x3f800000));

    return {builder.detach(), ctm, SkIRect::MakeLTRB(0, 0, 2560, 1410)};
}

}  // namespace skgpu::graphite::CullingRepros

#endif  // skgpu_graphite_sparse_strips_CullingRepros_DEFINED
