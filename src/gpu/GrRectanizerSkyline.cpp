/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkIPoint16.h"
#include "src/gpu/GrRectanizerSkyline.h"

#include <algorithm>

bool GrRectanizerSkyline::addRect(int width, int height, SkIPoint16* loc) {
    if ((unsigned)width > (unsigned)this->width() ||
        (unsigned)height > (unsigned)this->height()) {
        return false;
    }

    // find position for new rectangle
    int bestWidth = this->width() + 1;
    int bestX = 0;
    int bestY = this->height() + 1;
    int bestIndex = -1;
    for (int i = 0; i < fSkyline.count(); ++i) {
        int y;
        if (this->rectangleFits(i, width, height, &y)) {
            // minimize y position first, then width of skyline
            if (y < bestY || (y == bestY && fSkyline[i].fWidth < bestWidth)) {
                bestIndex = i;
                bestWidth = fSkyline[i].fWidth;
                bestX = fSkyline[i].fX;
                bestY = y;
            }
        }
    }

    // add rectangle to skyline
    if (-1 != bestIndex) {
        this->addSkylineLevel(bestIndex, bestX, bestY, width, height);
        loc->fX = bestX;
        loc->fY = bestY;

        fAreaSoFar += width*height;
        return true;
    }

    loc->fX = 0;
    loc->fY = 0;
    return false;
}

bool GrRectanizerSkyline::rectangleFits(int skylineIndex, int width, int height, int* ypos) const {
    int x = fSkyline[skylineIndex].fX;
    if (x + width > this->width()) {
        return false;
    }

    int widthLeft = width;
    int i = skylineIndex;
    int y = fSkyline[skylineIndex].fY;
    while (widthLeft > 0) {
        y = std::max(y, fSkyline[i].fY);
        if (y + height > this->height()) {
            return false;
        }
        widthLeft -= fSkyline[i].fWidth;
        ++i;
        SkASSERT(i < fSkyline.count() || widthLeft <= 0);
    }

    *ypos = y;
    return true;
}

void GrRectanizerSkyline::addSkylineLevel(int skylineIndex, int x, int y, int width, int height) {
    SkylineSegment newSegment;
    newSegment.fX = x;
    newSegment.fY = y + height;
    newSegment.fWidth = width;
    fSkyline.insert(skylineIndex, 1, &newSegment);

    SkASSERT(newSegment.fX + newSegment.fWidth <= this->width());
    SkASSERT(newSegment.fY <= this->height());

    // delete width of the new skyline segment from following ones
    for (int i = skylineIndex+1; i < fSkyline.count(); ++i) {
        // The new segment subsumes all or part of fSkyline[i]
        SkASSERT(fSkyline[i-1].fX <= fSkyline[i].fX);

        if (fSkyline[i].fX < fSkyline[i-1].fX + fSkyline[i-1].fWidth) {
            int shrink = fSkyline[i-1].fX + fSkyline[i-1].fWidth - fSkyline[i].fX;

            fSkyline[i].fX += shrink;
            fSkyline[i].fWidth -= shrink;

            if (fSkyline[i].fWidth <= 0) {
                // fully consumed
                fSkyline.remove(i);
                --i;
            } else {
                // only partially consumed
                break;
            }
        } else {
            break;
        }
    }

    // merge fSkylines
    for (int i = 0; i < fSkyline.count()-1; ++i) {
        if (fSkyline[i].fY == fSkyline[i+1].fY) {
            fSkyline[i].fWidth += fSkyline[i+1].fWidth;
            fSkyline.remove(i+1);
            --i;
        }
    }
}

