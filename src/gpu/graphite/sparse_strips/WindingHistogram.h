/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_WindingHistogram_DEFINED
#define skgpu_graphite_WindingHistogram_DEFINED

#include "include/private/SkAssert.h"
#include "include/private/SkTDArray.h"

#include <cstring>

namespace skgpu::graphite {

// In SparseStrips, lines which are fully left of the viewport still contribute (coarse) winding but
// can be culled during Tiling. The WindingHistogram holds that winding so that it can be consumed
// during MakeStrips.
class WindingHistogram {
public:
    WindingHistogram() = default;

    WindingHistogram(int maxRows) {
        this->reserve(maxRows);
    }

    void reserve(int maxRows) {
        fWindingData.reserve(maxRows);
    }

    void resize(int numRows) {
        fWindingData.resize(numRows);
    }

    void clear() {
        std::memset(fWindingData.data(), 0, fWindingData.size_bytes());
    }

    int16_t operator[](int row) const {
        SkASSERT(row >= 0 && row < fWindingData.size());
        return fWindingData[row];
    }

    int16_t& operator[](int row) {
        SkASSERT(row >= 0 && row < fWindingData.size());
        return fWindingData[row];
    }

    SK_ALWAYS_INLINE void addWinding(int row, int16_t delta) {
        SkASSERT(row >= 0 && row < fWindingData.size());
        fWindingData[row] += delta;
    }

    SK_ALWAYS_INLINE void addWindingRange(int startRow, int endRow, int16_t delta) {
        SkASSERT(startRow >= 0 && endRow <= fWindingData.size() && startRow <= endRow);
        for (int r = startRow; r < endRow; ++r) {
            fWindingData[r] += delta;
        }
    }

    const SkTDArray<int16_t>& data() const { return fWindingData;        }
    int32_t size()                   const { return fWindingData.size(); }

private:
    SkTDArray<int16_t> fWindingData;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_WindingHistogram_DEFINED
