//
// Copyright 2016 Google Inc.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
//

#ifndef HS_CUDA_CONFIG_ONCE
#define HS_CUDA_CONFIG_ONCE

#define HS_SLAB_THREADS_LOG2    5
#define HS_SLAB_THREADS         (1 << HS_SLAB_THREADS_LOG2)
#define HS_SLAB_WIDTH_LOG2      5
#define HS_SLAB_WIDTH           (1 << HS_SLAB_WIDTH_LOG2)
#define HS_SLAB_HEIGHT          16
#define HS_SLAB_KEYS            (HS_SLAB_WIDTH * HS_SLAB_HEIGHT)
#define HS_REG_LAST(c)          c##16
#define HS_KEY_TYPE_PRETTY      u32
#define HS_KEY_WORDS            1
#define HS_VAL_WORDS            0
#define HS_BS_SLABS             16
#define HS_BS_SLABS_LOG2_RU     4
#define HS_BC_SLABS_LOG2_MAX    4
#define HS_FM_BLOCK_HEIGHT      1
#define HS_FM_SCALE_MIN         0
#define HS_FM_SCALE_MAX         0
#define HS_HM_BLOCK_HEIGHT      1
#define HS_HM_SCALE_MIN         0
#define HS_HM_SCALE_MAX         0
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
  HS_TRANSPOSE_BLEND( s, t,  2,   3,   1 ) \
  HS_TRANSPOSE_BLEND( s, t,  2,   4,   2 ) \
  HS_TRANSPOSE_BLEND( s, t,  2,   7,   5 ) \
  HS_TRANSPOSE_BLEND( s, t,  2,   8,   6 ) \
  HS_TRANSPOSE_BLEND( s, t,  2,  11,   9 ) \
  HS_TRANSPOSE_BLEND( s, t,  2,  12,  10 ) \
  HS_TRANSPOSE_BLEND( s, t,  2,  15,  13 ) \
  HS_TRANSPOSE_BLEND( s, t,  2,  16,  14 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,   5,   1 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,   6,   2 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,   7,   3 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,   8,   4 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,  13,   9 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,  14,  10 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,  15,  11 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,  16,  12 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,   9,   1 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,  10,   2 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,  11,   3 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,  12,   4 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,  13,   5 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,  14,   6 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,  15,   7 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,  16,   8 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,   2,   1 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,   4,   3 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,   6,   5 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,   8,   7 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,  10,   9 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,  12,  11 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,  14,  13 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,  16,  15 ) \
  HS_TRANSPOSE_REMAP( w,   1,   1 )        \
  HS_TRANSPOSE_REMAP( w,   2,   9 )        \
  HS_TRANSPOSE_REMAP( w,   3,   2 )        \
  HS_TRANSPOSE_REMAP( w,   4,  10 )        \
  HS_TRANSPOSE_REMAP( w,   5,   3 )        \
  HS_TRANSPOSE_REMAP( w,   6,  11 )        \
  HS_TRANSPOSE_REMAP( w,   7,   4 )        \
  HS_TRANSPOSE_REMAP( w,   8,  12 )        \
  HS_TRANSPOSE_REMAP( w,   9,   5 )        \
  HS_TRANSPOSE_REMAP( w,  10,  13 )        \
  HS_TRANSPOSE_REMAP( w,  11,   6 )        \
  HS_TRANSPOSE_REMAP( w,  12,  14 )        \
  HS_TRANSPOSE_REMAP( w,  13,   7 )        \
  HS_TRANSPOSE_REMAP( w,  14,  15 )        \
  HS_TRANSPOSE_REMAP( w,  15,   8 )        \
  HS_TRANSPOSE_REMAP( w,  16,  16 )        \
  HS_EMPTY

#endif

//
//
//

