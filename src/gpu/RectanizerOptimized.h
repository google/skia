/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_RectanizerOptimized_DEFINED
#define skgpu_RectanizerOptimized_DEFINED

#include "include/private/SkBitmaskEnum.h"
#include "include/private/SkTDArray.h"
#include "src/gpu/RectanizerSkyline.h"
#include "src/utils/SkBitSet.h"

#include <vector>

namespace skgpu {
enum class OptFlags {
    None = 0x00,
    T = 0x01,
    L = 0x02,
    R = 0x04,
    TL = 0x03,
    TR = 0x05,
    LR = 0x06,
    TLR = 0x07,
};
}

namespace sknonstd {
template <> struct is_bitmask_enum<skgpu::OptFlags> : std::true_type {};
}

namespace skgpu {
// This class tracks the entire profile line without breaking it into ranges.
// Each column of a plot has an integer indicating the hight of an allocated area in this column.
// It works slightly better that RectanizerSkyline because it always selects a free area with
// the minimum y, minimum x. RectanizerSkyline selects minimin y, minimum width of a range. It
// gives RectanizerOptimized slight advantage (~1% of saved Atlas memory and 1 less plot eviction
// for a very big test).
// That advantage alone would never justify changing already working algorithm, but it makes much
// easier the upcoming zero padding glyphs optimization.
// (It also makes easier changing the skyline somewhat, but again that is not the point)
// Mark this class final in an effort to avoid the vtable when this subclass is used explicitly.
class RectanizerOptimized final : public Rectanizer {
public:
    RectanizerOptimized(int w, int h) : Rectanizer(w, h) {
        this->reset();
    }

    ~RectanizerOptimized() final { }

    void reset() final {
        Rectanizer::reset();
        fProfile.resize(this->width() + 1); // One extra element to avoid extra checking
        std::fill(fProfile.begin(), fProfile.end(), 0);
    }

    bool addRect(int w, int h, SkIPoint16* loc) override;

    PadAllGlyphs padAllGlyphs() const override { return PadAllGlyphs::kYes; }

private:
    // Update the skyline structure to include a width x height rect located
    // at x,y and the information about the pixels
    void markAreaOccupied(SkIPoint16 loc, int width, int height);

    // The distance from the top edge which is already occupied
    std::vector<int16_t> fProfile;
};

} // End of namespace skgpu

#endif
