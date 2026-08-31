/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_WideTiles_DEFINED
#define skgpu_graphite_geom_WideTiles_DEFINED

#include "include/private/SkTArray.h"
#include "src/gpu/graphite/geom/Rect.h"
#include <cstdint>

namespace skgpu::graphite {

// Container holding the "WideTiles" produced by SparseStrips rendering. A WideTile stores the inner
// fill of a path which requires no antialiasing. Also spans a variable non-zero number of tiles.
class WideTiles {
public:
    struct WideTile {
        WideTile(uint16_t x, uint16_t y, uint16_t width)
            : fX(x), fY(y), fWidth(width), fPadding(0xffff) {}
        uint16_t fX;        // Top left coordinate of the WideTile
        uint16_t fY;        // ``
        uint16_t fWidth;    // The width of the WideTile in pixels
        uint16_t fPadding;  // Pad to vec2 alignment
    };

    WideTiles() = default;

    void addTile(uint16_t x, uint16_t y, uint16_t width) {
        fTiles.push_back(WideTile(x, y, width));
    }

    const skia_private::TArray<WideTile>& tiles() const { return fTiles; }
    skia_private::TArray<WideTile>& tiles() { return fTiles; }
    bool empty() const { return fTiles.empty(); }
    size_t size() const { return fTiles.size(); }
    void clear() { fTiles.clear(); }

private:
    skia_private::TArray<WideTile> fTiles;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_WideTiles_DEFINED
