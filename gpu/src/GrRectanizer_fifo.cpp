/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#include "GrRectanizer.h"
#include "GrTBSearch.h"

#define MIN_HEIGHT_POW2     2

class GrRectanizerFIFO : public GrRectanizer {
public:
    GrRectanizerFIFO(int w, int h) : GrRectanizer(w, h) {
        fNextStripY = 0;
        fAreaSoFar = 0;
        Gr_bzero(fRows, sizeof(fRows));
    }
    
    virtual ~GrRectanizerFIFO() {
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

bool GrRectanizerFIFO::addRect(int width, int height, GrIPoint16* loc) {
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
    return new GrRectanizerFIFO(width, height);
}


