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
// While always 32-bit, the output format for bilinear filtered and unfiltered
// samplers is different:
//
//   - filtered:  (14 bits of low  x/y coordinate) << 18
//              | ( 4 bits of interpolant        ) << 14
//              | (14 bits of high x/y corodinate) <<  0
//
//   - unfiltered:
//         TODO
//
// In all cases we now only support at most a scale+translate matrix,
// which means each row of the output stays on the same row in the input.
//
// We take advantage of this by encoding the row y-values only once, first, then all
// x-values from there.  (This "DX" mode used to contrast a general "DXDY" mode.)

// We support 3 tiling modes here:
//   -  clamp in both x and y;
//   - repeat in both x and y;
//   - mirror in both x and y.

static void   clamp_nofilter(const SkBitmapProcState&, uint32_t* xy, int count, int x, int y) {}
static void  repeat_nofilter(const SkBitmapProcState&, uint32_t* xy, int count, int x, int y) {}
static void  mirror_nofilter(const SkBitmapProcState&, uint32_t* xy, int count, int x, int y) {}

static void   clamp_filter(const SkBitmapProcState&, uint32_t* xy, int count, int x, int y) {}
static void  repeat_filter(const SkBitmapProcState&, uint32_t* xy, int count, int x, int y) {}
static void  mirror_filter(const SkBitmapProcState&, uint32_t* xy, int count, int x, int y) {}

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
