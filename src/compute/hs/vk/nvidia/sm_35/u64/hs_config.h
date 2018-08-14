//
// Copyright 2016 Google Inc.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
//

#ifndef HS_GLSL_ONCE
#define HS_GLSL_ONCE

#define HS_SLAB_THREADS_LOG2    5
#define HS_SLAB_THREADS         (1 << HS_SLAB_THREADS_LOG2)
#define HS_SLAB_WIDTH_LOG2      5
#define HS_SLAB_WIDTH           (1 << HS_SLAB_WIDTH_LOG2)
#define HS_SLAB_HEIGHT          32
#define HS_SLAB_KEYS            (HS_SLAB_WIDTH * HS_SLAB_HEIGHT)
#define HS_REG_LAST(c)          c##32
#define HS_KEY_WORDS            2
#define HS_VAL_WORDS            0
#define HS_BS_SLABS             16
#define HS_BS_SLABS_LOG2_RU     4
#define HS_BC_SLABS_LOG2_MAX    4
#define HS_FM_BLOCK_HEIGHT      1
#define HS_FM_SCALE_MIN         1
#define HS_FM_SCALE_MAX         1
#define HS_HM_BLOCK_HEIGHT      1
#define HS_HM_SCALE_MIN         1
#define HS_HM_SCALE_MAX         1
#define HS_EMPTY

#define HS_NVIDIA_SM35

#define HS_SLAB_ROWS()    \
  HS_SLAB_ROW(   1,   0 ) \
  HS_SLAB_ROW(   2,   1 ) \
  HS_SLAB_ROW(   3,   2 ) \
  HS_SLAB_ROW(   4,   3 ) \
  HS_SLAB_ROW(   5,   4 ) \
  HS_SLAB_ROW(   6,   5 ) \
  HS_SLAB_ROW(   7,   6 ) \
  HS_SLAB_ROW(   8,   7 ) \
  HS_SLAB_ROW(   9,   8 ) \
  HS_SLAB_ROW(  10,   9 ) \
  HS_SLAB_ROW(  11,  10 ) \
  HS_SLAB_ROW(  12,  11 ) \
  HS_SLAB_ROW(  13,  12 ) \
  HS_SLAB_ROW(  14,  13 ) \
  HS_SLAB_ROW(  15,  14 ) \
  HS_SLAB_ROW(  16,  15 ) \
  HS_SLAB_ROW(  17,  16 ) \
  HS_SLAB_ROW(  18,  17 ) \
  HS_SLAB_ROW(  19,  18 ) \
  HS_SLAB_ROW(  20,  19 ) \
  HS_SLAB_ROW(  21,  20 ) \
  HS_SLAB_ROW(  22,  21 ) \
  HS_SLAB_ROW(  23,  22 ) \
  HS_SLAB_ROW(  24,  23 ) \
  HS_SLAB_ROW(  25,  24 ) \
  HS_SLAB_ROW(  26,  25 ) \
  HS_SLAB_ROW(  27,  26 ) \
  HS_SLAB_ROW(  28,  27 ) \
  HS_SLAB_ROW(  29,  28 ) \
  HS_SLAB_ROW(  30,  29 ) \
  HS_SLAB_ROW(  31,  30 ) \
  HS_SLAB_ROW(  32,  31 ) \
  HS_EMPTY

#define HS_TRANSPOSE_SLAB()                \
  HS_TRANSPOSE_STAGE( 1 )                  \
  HS_TRANSPOSE_STAGE( 2 )                  \
  HS_TRANSPOSE_STAGE( 3 )                  \
  HS_TRANSPOSE_STAGE( 4 )                  \
  HS_TRANSPOSE_STAGE( 5 )                  \
  HS_TRANSPOSE_BLEND( r, s,  1,   2,   1 ) \
  HS_TRANSPOSE_BLEND( r, s,  1,   4,   3 ) \
  HS_TRANSPOSE_BLEND( r, s,  1,   6,   5 ) \
  HS_TRANSPOSE_BLEND( r, s,  1,   8,   7 ) \
  HS_TRANSPOSE_BLEND( r, s,  1,  10,   9 ) \
  HS_TRANSPOSE_BLEND( r, s,  1,  12,  11 ) \
  HS_TRANSPOSE_BLEND( r, s,  1,  14,  13 ) \
  HS_TRANSPOSE_BLEND( r, s,  1,  16,  15 ) \
  HS_TRANSPOSE_BLEND( r, s,  1,  18,  17 ) \
  HS_TRANSPOSE_BLEND( r, s,  1,  20,  19 ) \
  HS_TRANSPOSE_BLEND( r, s,  1,  22,  21 ) \
  HS_TRANSPOSE_BLEND( r, s,  1,  24,  23 ) \
  HS_TRANSPOSE_BLEND( r, s,  1,  26,  25 ) \
  HS_TRANSPOSE_BLEND( r, s,  1,  28,  27 ) \
  HS_TRANSPOSE_BLEND( r, s,  1,  30,  29 ) \
  HS_TRANSPOSE_BLEND( r, s,  1,  32,  31 ) \
  HS_TRANSPOSE_BLEND( s, t,  2,   3,   1 ) \
  HS_TRANSPOSE_BLEND( s, t,  2,   4,   2 ) \
  HS_TRANSPOSE_BLEND( s, t,  2,   7,   5 ) \
  HS_TRANSPOSE_BLEND( s, t,  2,   8,   6 ) \
  HS_TRANSPOSE_BLEND( s, t,  2,  11,   9 ) \
  HS_TRANSPOSE_BLEND( s, t,  2,  12,  10 ) \
  HS_TRANSPOSE_BLEND( s, t,  2,  15,  13 ) \
  HS_TRANSPOSE_BLEND( s, t,  2,  16,  14 ) \
  HS_TRANSPOSE_BLEND( s, t,  2,  19,  17 ) \
  HS_TRANSPOSE_BLEND( s, t,  2,  20,  18 ) \
  HS_TRANSPOSE_BLEND( s, t,  2,  23,  21 ) \
  HS_TRANSPOSE_BLEND( s, t,  2,  24,  22 ) \
  HS_TRANSPOSE_BLEND( s, t,  2,  27,  25 ) \
  HS_TRANSPOSE_BLEND( s, t,  2,  28,  26 ) \
  HS_TRANSPOSE_BLEND( s, t,  2,  31,  29 ) \
  HS_TRANSPOSE_BLEND( s, t,  2,  32,  30 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,   5,   1 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,   6,   2 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,   7,   3 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,   8,   4 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,  13,   9 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,  14,  10 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,  15,  11 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,  16,  12 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,  21,  17 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,  22,  18 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,  23,  19 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,  24,  20 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,  29,  25 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,  30,  26 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,  31,  27 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,  32,  28 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,   9,   1 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,  10,   2 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,  11,   3 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,  12,   4 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,  13,   5 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,  14,   6 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,  15,   7 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,  16,   8 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,  25,  17 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,  26,  18 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,  27,  19 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,  28,  20 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,  29,  21 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,  30,  22 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,  31,  23 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,  32,  24 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,  17,   1 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,  18,   2 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,  19,   3 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,  20,   4 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,  21,   5 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,  22,   6 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,  23,   7 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,  24,   8 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,  25,   9 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,  26,  10 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,  27,  11 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,  28,  12 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,  29,  13 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,  30,  14 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,  31,  15 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,  32,  16 ) \
  HS_TRANSPOSE_REMAP( w,   1,   1 )        \
  HS_TRANSPOSE_REMAP( w,   2,   2 )        \
  HS_TRANSPOSE_REMAP( w,   3,   3 )        \
  HS_TRANSPOSE_REMAP( w,   4,   4 )        \
  HS_TRANSPOSE_REMAP( w,   5,   5 )        \
  HS_TRANSPOSE_REMAP( w,   6,   6 )        \
  HS_TRANSPOSE_REMAP( w,   7,   7 )        \
  HS_TRANSPOSE_REMAP( w,   8,   8 )        \
  HS_TRANSPOSE_REMAP( w,   9,   9 )        \
  HS_TRANSPOSE_REMAP( w,  10,  10 )        \
  HS_TRANSPOSE_REMAP( w,  11,  11 )        \
  HS_TRANSPOSE_REMAP( w,  12,  12 )        \
  HS_TRANSPOSE_REMAP( w,  13,  13 )        \
  HS_TRANSPOSE_REMAP( w,  14,  14 )        \
  HS_TRANSPOSE_REMAP( w,  15,  15 )        \
  HS_TRANSPOSE_REMAP( w,  16,  16 )        \
  HS_TRANSPOSE_REMAP( w,  17,  17 )        \
  HS_TRANSPOSE_REMAP( w,  18,  18 )        \
  HS_TRANSPOSE_REMAP( w,  19,  19 )        \
  HS_TRANSPOSE_REMAP( w,  20,  20 )        \
  HS_TRANSPOSE_REMAP( w,  21,  21 )        \
  HS_TRANSPOSE_REMAP( w,  22,  22 )        \
  HS_TRANSPOSE_REMAP( w,  23,  23 )        \
  HS_TRANSPOSE_REMAP( w,  24,  24 )        \
  HS_TRANSPOSE_REMAP( w,  25,  25 )        \
  HS_TRANSPOSE_REMAP( w,  26,  26 )        \
  HS_TRANSPOSE_REMAP( w,  27,  27 )        \
  HS_TRANSPOSE_REMAP( w,  28,  28 )        \
  HS_TRANSPOSE_REMAP( w,  29,  29 )        \
  HS_TRANSPOSE_REMAP( w,  30,  30 )        \
  HS_TRANSPOSE_REMAP( w,  31,  31 )        \
  HS_TRANSPOSE_REMAP( w,  32,  32 )        \
  HS_EMPTY

#endif

//
//
//

