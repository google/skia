
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "GrRectanizer.h"
#include "GrTBSearch.h"

#define MIN_HEIGHT_POW2     2

class GrRectanizerPow2 : public GrRectanizer {
public:
    GrRectanizerPow2(int w, int h) : GrRectanizer(w, h) {
        fNextStripY = 0;
        fAreaSoFar = 0;
        Gr_bzero(fRows, sizeof(fRows));
    }

    virtual ~GrRectanizerPow2() {
    }

    virtual bool addRect(int w, int h, GrIPoint16* loc);

    virtual float percentFull() const {
        return fAreaSoFar / ((float)this->width() * this->height());
    }

    virtual int stripToPurge(int height) const { return -1; }
    virtual void purgeStripAtY(int yCoord) { }

    ///////////////////////////////////////////////////////////////////////////

    struct Row {
        GrIPoint16  fLoc;
        int         fRowHeight;
        
        bool canAddWidth(int width, int containerWidth) const {
            return fLoc.fX + width <= containerWidth;
        }
    };

    Row fRows[16];

    static int HeightToRowIndex(int height) {
        GrAssert(height >= MIN_HEIGHT_POW2);
        return 32 - Gr_clz(height - 1);
    }

    int fNextStripY;
    int32_t fAreaSoFar;

    bool canAddStrip(int height) const {
        return fNextStripY + height <= this->height();
    }

    void initRow(Row* row, int rowHeight) {
        row->fLoc.set(0, fNextStripY);
        row->fRowHeight = rowHeight;
        fNextStripY += rowHeight;
    }
};

bool GrRectanizerPow2::addRect(int width, int height, GrIPoint16* loc) {
    if ((unsigned)width > (unsigned)this->width() ||
        (unsigned)height > (unsigned)this->height()) {
        return false;
    }

    int32_t area = width * height;

    /*
        We use bsearch, but there may be more than one row with the same height,
        so we actually search for height-1, which can only be a pow2 itself if
        height == 2. Thus we set a minimum height.
     */
    height = GrNextPow2(height);
    if (height < MIN_HEIGHT_POW2) {
        height = MIN_HEIGHT_POW2;
    }

    Row* row = &fRows[HeightToRowIndex(height)];
    GrAssert(row->fRowHeight == 0 || row->fRowHeight == height);

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

    GrAssert(row->fRowHeight == height);
    GrAssert(row->canAddWidth(width, this->width()));
    *loc = row->fLoc;
    row->fLoc.fX += width;

    GrAssert(row->fLoc.fX <= this->width());
    GrAssert(row->fLoc.fY <= this->height());
    GrAssert(fNextStripY <= this->height());
    fAreaSoFar += area;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

GrRectanizer* GrRectanizer::Factory(int width, int height) {
    return new GrRectanizerPow2(width, height);
}


