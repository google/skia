/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "src/gpu/graphite/geom/BoundsManager.h"

namespace skgpu::graphite {

DEF_TEST(BoundsManager, r) {
    // 64 grid cells, each 16x16
    const int n = 8;
    const int w = 16;
    std::unique_ptr<BoundsManager> bm = GridBoundsManager::Make({n * w, n * w}, n);

    CompressedPaintersOrder order = CompressedPaintersOrder::First();
    for (int y = 0; y < n; ++y) {
        for (int x = 0; x < n; ++x) {
            order = order.next();

            // Should only modify a single cell
            Rect b = Rect::XYWH((x + 0.1f) * w, (y + 0.1f) * w, 0.8f * w, 0.8f * w);
            bm->recordDraw(b, order);
        }
    }

    // TODO: repeat these queries using bounds that intersect across levels as well
    order = CompressedPaintersOrder::First();
    for (int y = 0; y < n; ++y) {
        for (int x = 0; x < n; ++x) {
            order = order.next();

            // Should only read a single cell
            Rect b = Rect::XYWH((x + 0.2f) * w, (y + 0.2f) * w, 0.6f * w, 0.6f * w);

            CompressedPaintersOrder actual = bm->getMostRecentDraw(b);
            REPORTER_ASSERT(r, actual == order);
        }
    }

    // TODO: Then call recordDraw with new values that write to multiple cells

    // TODO: Then test calls where the new value is not larger than the current max
}

}  // namespace skgpu::graphite
