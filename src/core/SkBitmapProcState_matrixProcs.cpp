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

static void TODO_nofilter(uint32_t* xy, int count) {
    *xy++ = 0;  // y = 0
    auto x = (uint16_t*)xy;
    while (count --> 0) {
        *x++ = 0;  // x = 0
    }
}

static void   clamp_nofilter(const SkBitmapProcState&, uint32_t* xy, int count, int x, int y) {
    TODO_nofilter(xy, count);
}
static void  repeat_nofilter(const SkBitmapProcState&, uint32_t* xy, int count, int x, int y) {
    TODO_nofilter(xy, count);
}
static void  mirror_nofilter(const SkBitmapProcState&, uint32_t* xy, int count, int x, int y) {
    TODO_nofilter(xy, count);
}


static void TODO_filter(uint32_t* xy, int count) {
    *xy++ = 0;  // y0 == y1 == 0, wy = 0
    while (count --> 0) {
        *xy++ = 0;  // x0 == x1 == 0, wx = 0
    }
}

static void   clamp_filter(const SkBitmapProcState&, uint32_t* xy, int count, int x, int y) {
    TODO_filter(xy, count);
}
static void  repeat_filter(const SkBitmapProcState&, uint32_t* xy, int count, int x, int y) {
    TODO_filter(xy, count);
}
static void  mirror_filter(const SkBitmapProcState&, uint32_t* xy, int count, int x, int y) {
    TODO_filter(xy, count);
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
