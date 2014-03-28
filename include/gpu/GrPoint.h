
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

#if 0
#define GrPoint     SkPoint
#define GrVec       SkVector
#endif

struct GrIPoint16 {
    int16_t fX, fY;

    void set(intptr_t x, intptr_t y) {
        fX = SkToS16(x);
        fY = SkToS16(y);
    }
};

#endif
