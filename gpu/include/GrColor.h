
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrColor_DEFINED
#define GrColor_DEFINED

#include "GrTypes.h"

/**
 *  GrColor is 4 bytes for R, G, B, A, in a compile-time specific order. The
 *  components are stored premultiplied.
 */
typedef uint32_t GrColor;

// indices for address a GrColor as an array of bytes

#define GrColor_INDEX_R     0
#define GrColor_INDEX_G     1
#define GrColor_INDEX_B     2
#define GrColor_INDEX_A     3

// shfit amount to assign a component to a GrColor int

#define GrColor_SHIFT_R     0
#define GrColor_SHIFT_G     8
#define GrColor_SHIFT_B     16
#define GrColor_SHIFT_A     24

/**
 *  Pack 4 components (RGBA) into a GrColor int
 */
static inline GrColor GrColorPackRGBA(unsigned r, unsigned g,
                                      unsigned b, unsigned a) {
    GrAssert((uint8_t)r == r);
    GrAssert((uint8_t)g == g);
    GrAssert((uint8_t)b == b);
    GrAssert((uint8_t)a == a);
    return  (r << GrColor_SHIFT_R) |
            (g << GrColor_SHIFT_G) |
            (b << GrColor_SHIFT_B) |
            (a << GrColor_SHIFT_A);
}

// extract a component (byte) from a GrColor int

#define GrColorUnpackR(color)   (((color) >> GrColor_SHIFT_R) & 0xFF)
#define GrColorUnpackG(color)   (((color) >> GrColor_SHIFT_G) & 0xFF)
#define GrColorUnpackB(color)   (((color) >> GrColor_SHIFT_B) & 0xFF)
#define GrColorUnpackA(color)   (((color) >> GrColor_SHIFT_A) & 0xFF)

/**
 *  Since premultiplied means that alpha >= color, we construct a color with
 *  each component==255 and alpha == 0 to be "illegal"
 */
#define GrColor_ILLEGAL     (~(0xFF << GrColor_SHIFT_A))

#endif

