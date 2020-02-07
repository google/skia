/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRectanizerSkyline_DEFINED
#define GrRectanizerSkyline_DEFINED

#include "include/private/SkTDArray.h"
#include "src/core/SkIPoint16.h"

// Pack rectangles and track the current silhouette
// Based, in part, on Jukka Jylanki's work at http://clb.demon.fi
class GrRectanizerSkyline {
public:
    GrRectanizerSkyline(int w, int h) : fWidth{w}, fHeight{h} {
        this->reset();
    }

    void reset() {
        fAreaSoFar = 0;
        fSkyline.reset();
        SkylineSegment* seg = fSkyline.append(1);
        seg->fX = 0;
        seg->fY = 0;
        seg->fWidth = this->width();
    }

    bool addRect(int w, int h, SkIPoint16* loc);

    int width() const { return fWidth; }
    int height() const { return fHeight; }

private:
    struct SkylineSegment {
        int  fX;
        int  fY;
        int  fWidth;
    };

    // Can a width x height rectangle fit in the free space represented by
    // the skyline segments >= 'skylineIndex'? If so, return true and fill in
    // 'y' with the y-location at which it fits (the x location is pulled from
    // 'skylineIndex's segment.
    bool rectangleFits(int skylineIndex, int width, int height, int* y) const;
    // Update the skyline structure to include a width x height rect located
    // at x,y.
    void addSkylineLevel(int skylineIndex, int x, int y, int width, int height);

    const int fWidth;
    const int fHeight;
    SkTDArray<SkylineSegment> fSkyline;
    int32_t fAreaSoFar;
};

#endif  // GrRectanizerSkyline_DEFINED
