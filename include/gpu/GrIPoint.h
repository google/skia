
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrIPoint_DEFINED
#define GrIPoint_DEFINED

#include "GrTypes.h"

struct GrIPoint {
public:
    int32_t fX, fY;
    
    GrIPoint(int32_t x, int32_t y) : fX(x), fY(y) {}
   
    void set(int32_t x, int32_t y) {
        fX = x;
        fY = y;
    }
};

#endif
