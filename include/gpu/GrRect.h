
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrRect_DEFINED
#define GrRect_DEFINED

#include "GrPoint.h"
#include "SkRect.h"

typedef SkIRect GrIRect;
typedef SkRect  GrRect;

struct GrIRect16 {
    int16_t fLeft, fTop, fRight, fBottom;
    
    int width() const { return fRight - fLeft; }
    int height() const { return fBottom - fTop; }
    int area() const { return this->width() * this->height(); }
    bool isEmpty() const { return fLeft >= fRight || fTop >= fBottom; }
    
    void set(const GrIRect& r) {
        fLeft   = SkToS16(r.fLeft);
        fTop    = SkToS16(r.fTop);
        fRight  = SkToS16(r.fRight);
        fBottom = SkToS16(r.fBottom);
    }
};

#endif

