/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_RectanizerSkyline_DEFINED
#define skgpu_RectanizerSkyline_DEFINED

#include "include/private/base/SkTDArray.h"
#include "src/gpu/Rectanizer.h"

#include <cstdint>

struct SkIPoint16;

namespace skgpu {

// Pack rectangles and track the current silhouette
// Based, in part, on Jukka Jylanki's work at http://clb.demon.fi
//
// Mark this class final in an effort to avoid the vtable when this subclass is used explicitly.
class RectanizerSkyline final : public Rectanizer {
public:
    RectanizerSkyline(int w, int h) : Rectanizer(w, h) {
        this->reset();
    }

    ~RectanizerSkyline() final { }

    void reset() final {
        fAreaSoFar = 0;
        fSkyline.clear();
        fSkyline.push_back(SkylineSegment{0, 0, this->width()});
    }

    bool addRect(int w, int h, SkIPoint16* loc) final;

    float percentFull() const final {
        return fAreaSoFar / ((float)this->width() * this->height());
    }

private:
    struct SkylineSegment {
        int  fX;
        int  fY;
        int  fWidth;
    };

    SkTDArray<SkylineSegment> fSkyline;

    int32_t fAreaSoFar;

    // Can a width x height rectangle fit in the free space represented by
    // the skyline segments >= 'skylineIndex'? If so, return true and fill in
    // 'y' with the y-location at which it fits (the x location is pulled from
    // 'skylineIndex's segment.
    bool rectangleFits(int skylineIndex, int width, int height, int* y) const;
    // Update the skyline structure to include a width x height rect located
    // at x,y.
    void addSkylineLevel(int skylineIndex, int x, int y, int width, int height);
};

} // End of namespace skgpu

#endif
