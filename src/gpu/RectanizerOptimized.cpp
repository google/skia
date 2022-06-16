/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkIPoint16.h"
#include "src/gpu/RectanizerOptimized.h"

#include <algorithm>

namespace skgpu {

// This method walks across the entire profile trying to find the area big enough to place a
// rectangle width * hight.
// It returns the area with mininum possible y coordinage,
// and if there are few like this, minumum x.
bool RectanizerOptimized::addRect(int width, int height, SkIPoint16* loc) {

    SkASSERT(width > 0 && height > 0);
    if (width > this->width() || height > this->height()) {
        return false;
    }

    SkIPoint16 bestLoc = SkIPoint16::Make(this->width(), this->height());
    SkIPoint16 currLoc = SkIPoint16::Make(0, 0);
    bool found = false;
    int16_t x = 0;
    // We are looking for a rectangle [currLoc.fX:x) * [0:currLoc.fY)
    while (x < this->width() && currLoc.fX <= this->width() - width) {

        if (currLoc.fX + width == x) {
            // We found the area: [currLoc.fX:x) * [0:currLoc.fY)
            found = true;
            // Let's see if it's better (we do not take in account optimized flag)
            if (currLoc.fY < bestLoc.fY) {
                bestLoc = currLoc;
            }
            // Let's see if we can find better y starting over from the currLoc.fX+1.
            x = (++currLoc.fX);
            currLoc.fY = fProfile[x];
            continue;
        }

        // Still looking
        currLoc.fY = std::max(currLoc.fY, fProfile[x]);
        ++x;
        // If this column does not fit vertically start all over from the next one
        if (this->height() < currLoc.fY + height) {
            // We start over from the next x (nothing before will be good enough)
            currLoc.fY = fProfile[x];
            currLoc.fX = x;
        }
    }

    if (found) {
        // Mark the area as allocated
        fAreaSoFar += width*height;
        this->markAreaOccupied(bestLoc, width, height);
        *loc = bestLoc;
        return true;
    }

    loc->fX = 0;
    loc->fY = 0;
    return false;

}

void RectanizerOptimized::markAreaOccupied(SkIPoint16 loc,
                                           int width,
                                           int height) {
    auto y = loc.fY + SkToS16(height);
    for (int16_t x = loc.fX; x < loc.fX + SkToS16(width); ++x) {
        fProfile[x] = y;
    }
}

///////////////////////////////////////////////////////////////////////////////

Rectanizer* Rectanizer::Factory(int width, int height) {
    return new RectanizerOptimized(width, height);
}

} // End of namespace skgpu
