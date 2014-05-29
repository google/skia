/*
* Copyright 2014 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrRectanizer_pow2_DEFINED
#define GrRectanizer_pow2_DEFINED

#include "GrRectanizer.h"

class GrRectanizerPow2 : public GrRectanizer {
public:
    GrRectanizerPow2(int w, int h) : INHERITED(w, h) {
        this->reset();
    }

    virtual ~GrRectanizerPow2() { }

    virtual void reset() SK_OVERRIDE {
        fNextStripY = 0;
        fAreaSoFar = 0;
        sk_bzero(fRows, sizeof(fRows));
    }

    virtual bool addRect(int w, int h, GrIPoint16* loc) SK_OVERRIDE;

    virtual float percentFull() const SK_OVERRIDE {
        return fAreaSoFar / ((float)this->width() * this->height());
    }

private:
    static const int kMIN_HEIGHT_POW2 = 2;

    struct Row {
        GrIPoint16  fLoc;
        int         fRowHeight;

        bool canAddWidth(int width, int containerWidth) const {
            return fLoc.fX + width <= containerWidth;
        }
    };

    Row fRows[16];

    int fNextStripY;
    int32_t fAreaSoFar;

    static int HeightToRowIndex(int height) {
        SkASSERT(height >= kMIN_HEIGHT_POW2);
        return 32 - SkCLZ(height - 1);
    }

    bool canAddStrip(int height) const {
        return fNextStripY + height <= this->height();
    }

    void initRow(Row* row, int rowHeight) {
        row->fLoc.set(0, fNextStripY);
        row->fRowHeight = rowHeight;
        fNextStripY += rowHeight;
    }

    typedef GrRectanizer INHERITED;
};

#endif
