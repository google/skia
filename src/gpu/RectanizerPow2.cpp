/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/RectanizerPow2.h"

namespace skgpu {

bool RectanizerPow2::addRect(int width, int height, SkIPoint16* loc) {
    if ((unsigned)width > (unsigned)this->width() ||
        (unsigned)height > (unsigned)this->height()) {
        return false;
    }

    int32_t area = width * height; // computed here since height will be modified

    // SkNextPow2 is undefined for inputs <= 0. If small values happen
    // to creep in here, round them all up to the minimum power of 2.
    static_assert(kMIN_HEIGHT_POW2 > 0);
    static_assert(kMIN_HEIGHT_POW2 == SkNextPow2_portable(kMIN_HEIGHT_POW2));
    if (height < kMIN_HEIGHT_POW2) {
        height = kMIN_HEIGHT_POW2;
    } else {
        height = SkNextPow2(height);
    }


    Row* row = &fRows[HeightToRowIndex(height)];
    SkASSERT(row->fRowHeight == 0 || row->fRowHeight == height);

    if (0 == row->fRowHeight) {
        if (!this->canAddStrip(height)) {
            return false;
        }
        this->initRow(row, height);
    } else {
        if (!row->canAddWidth(width, this->width())) {
            if (!this->canAddStrip(height)) {
                return false;
            }
            // that row is now "full", so retarget our Row record for
            // another one
            this->initRow(row, height);
        }
    }

    SkASSERT(row->fRowHeight == height);
    SkASSERT(row->canAddWidth(width, this->width()));
    *loc = row->fLoc;
    row->fLoc.fX += width;

    SkASSERT(row->fLoc.fX <= this->width());
    SkASSERT(row->fLoc.fY <= this->height());
    SkASSERT(fNextStripY <= this->height());
    fAreaSoFar += area;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

// factory is now in RectanizerSkyline.cpp
//Rectanizer* Rectanizer::Factory(int width, int height) {
//    return new RectanizerPow2(width, height);
//}

}  // End of namespace skgpu
