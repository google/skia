/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_TileTestCases_DEFINED
#define skgpu_graphite_TileTestCases_DEFINED

#include "include/private/base/SkTDArray.h"

#include <initializer_list>
#include <vector>

namespace skgpu::graphite {

struct Tile;
struct Line;

namespace TileTestCases {

struct TileTestCase {
    TileTestCase(const char* name,
                 std::initializer_list<Line> unscaledLines,
                 std::initializer_list<Tile> expectedTiles,
                 float scale);

    const char* fName;
    SkTDArray<Line> fLines;
    SkTDArray<Tile> fExpected;
};

std::vector<TileTestCase> Get(float scale, uint16_t viewportDim);
}  // namespace TileTestCases

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_TileTestCases_DEFINED
