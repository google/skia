/*
 * Copyright 2008 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// The copyright below was added in 2009, but I see no record of moto contributions...?

/* NEON optimized code (C) COPYRIGHT 2009 Motorola
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProcState.h"
#include "SkShader.h"
#include "SkNx.h"

// Matrix procs map (x,y) ... (x+count,y) to XY coordinates in the source bitmap.
//
// In all cases we now only support at most a scale+translate matrix,
// which means each row of the output stays on the same row in the input.
// We take advantage of this by encoding the row y-values only once, first, then all
// x-values from there.  (This "DX" mode used to contrast a general "DXDY" mode.)
//
// While always into a 32-bit buffer, the output format for
// unfiltered or bilinear filtered samplers is different:
//
//   - unfiltered: 32-bit y-coordinate
//                 16-bit x-coordinate
//                 ... repeated count times ...
//
//   - filtered:  (14 bits of low  y-coordinate) << 18
//              | ( 4 bits of y-interpolant    ) << 14
//              | (14 bits of high y-corodinate) <<  0
//
//                (14 bits of low  x-coordinate) << 18
//              | ( 4 bits of x-interpolant    ) << 14
//              | (14 bits of high x-corodinate) <<  0
//              ... repeated count times ...
//
// We support clamp, repeat, and mirror tiling, always the same in both dimensions.

static float fract(float v) {
    return v - floorf(v);
}

static int clamp(float x, int max) {
    if (x <   0) { x =   0; }
    if (x > max) { x = max; }
    return (int)x;
}

static int repeat(float x, int max) {
    // TODO
    return clamp(x, max);
}

static int mirror(float x, int max) {
    // TODO
    return clamp(x, max);
}

static void nofilter(const SkBitmapProcState& s,
                     uint32_t* xy, int count, int dx, int dy,
                     int (*tile)(float, int)) {
    SkPoint xy0;
    SkBitmapProcStateAutoMapper(s,dx,dy, &xy0);

    // First write a (32-bit) y for the whole row.
    *xy++ = tile(xy0.y(), s.fPixmap.height() - 1);

    // Now write out each (16-bit) x.
    auto xs = (uint16_t*)xy;

    // TODO: do we want the width=1 -> memset special case here?

    for (float x = xy0.x(); count --> 0; x += s.fInvMatrix.getScaleX()) {
        // TODO: unroll with SkNx here.
        *xs++ = tile(x, s.fPixmap.width() - 1);
    }
}

static void filter(const SkBitmapProcState& s,
                   uint32_t* xy, int count, int dx, int dy,
                   int (*tile)(float, int)) {

    auto tile_and_pack = [tile](float v, int max) {
        SkASSERT(max < (1<<14));
        return (uint32_t)tile(v  , max) << 18
             | (uint32_t)(fract(v)*15)  << 14
             | (uint32_t)tile(v+1, max) <<  0;
    };

    SkPoint xy0;
    SkBitmapProcStateAutoMapper(s,dx,dy, &xy0);

    // First write a (32-bit) y for the whole row.
    *xy++ = tile_and_pack(xy0.y(), s.fPixmap.height() - 1);

    // Now write out each (32-bit) x.
    for (float x = xy0.x(); count --> 0; x += s.fInvMatrix.getScaleX()) {
        // TODO: unroll with SkNx here.
        *xy++ = tile_and_pack(x, s.fPixmap.width() - 1);
    }
}

static void clamp_nofilter(const SkBitmapProcState& s, uint32_t* xy, int count, int x, int y) {
    nofilter(s,xy,count,x,y, clamp);
}
static void repeat_nofilter(const SkBitmapProcState& s, uint32_t* xy, int count, int x, int y) {
    nofilter(s,xy,count,x,y, repeat);
}
static void mirror_nofilter(const SkBitmapProcState& s, uint32_t* xy, int count, int x, int y) {
    nofilter(s,xy,count,x,y, mirror);
}

static void clamp_filter(const SkBitmapProcState& s, uint32_t* xy, int count, int x, int y) {
    filter(s,xy,count,x,y, clamp);
}
static void repeat_filter(const SkBitmapProcState& s, uint32_t* xy, int count, int x, int y) {
    filter(s,xy,count,x,y, repeat);
}
static void mirror_filter(const SkBitmapProcState& s, uint32_t* xy, int count, int x, int y) {
    filter(s,xy,count,x,y, mirror);
}

SkBitmapProcState::MatrixProc SkBitmapProcState::chooseMatrixProc() {
    SkASSERT(fInvType <= (SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask));
    SkASSERT(fTileModeX == fTileModeY);

    if (fFilterQuality == kNone_SkFilterQuality) {
        switch (fTileModeX) {
            default: SkASSERT(false);
            case SkShader:: kClamp_TileMode: return  clamp_nofilter;
            case SkShader::kRepeat_TileMode: return repeat_nofilter;
            case SkShader::kMirror_TileMode: return mirror_nofilter;
        }
    } else {
        switch (fTileModeX) {
            default: SkASSERT(false);
            case SkShader:: kClamp_TileMode: return  clamp_filter;
            case SkShader::kRepeat_TileMode: return repeat_filter;
            case SkShader::kMirror_TileMode: return mirror_filter;
        }

    }
}
