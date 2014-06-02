
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrPoint_DEFINED
#define GrPoint_DEFINED

#include "GrTypes.h"
#include "SkScalar.h"
#include "SkPoint.h"

struct GrIPoint16 {
    int16_t fX, fY;

    static GrIPoint16 Make(intptr_t x, intptr_t y) {
        GrIPoint16 pt;
        pt.set(x, y);
        return pt;
    }

    int16_t x() const { return fX; }
    int16_t y() const { return fY; }

    void set(intptr_t x, intptr_t y) {
        fX = SkToS16(x);
        fY = SkToS16(y);
    }
};

#endif
