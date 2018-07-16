//
// Copyright 2016 Google Inc.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
//

#ifndef HS_CL_ONCE
#define HS_CL_ONCE

#define HS_SLAB_THREADS_LOG2    3
#define HS_SLAB_THREADS         (1 << HS_SLAB_THREADS_LOG2)
#define HS_SLAB_WIDTH_LOG2      3
#define HS_SLAB_WIDTH           (1 << HS_SLAB_WIDTH_LOG2)
#define HS_SLAB_HEIGHT          16
#define HS_SLAB_KEYS            (HS_SLAB_WIDTH * HS_SLAB_HEIGHT)
#define HS_REG_LAST(c)          c##16
#define HS_KEY_TYPE             ulong
#define HS_KEY_WORDS            2
#define HS_VAL_WORDS            0
#define HS_BS_SLABS             16
#define HS_BS_SLABS_LOG2_RU     4
#define HS_BC_SLABS_LOG2_MAX    4
#define HS_FM_SCALE_MIN         1
#define HS_FM_SCALE_MAX         1
#define HS_HM_SCALE_MIN         1
#define HS_HM_SCALE_MAX         1
#define HS_EMPTY

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
  HS_TRANSPOSE_REMAP( u,   1,   1 )        \
  HS_TRANSPOSE_REMAP( u,   2,   3 )        \
  HS_TRANSPOSE_REMAP( u,   3,   5 )        \
  HS_TRANSPOSE_REMAP( u,   4,   7 )        \
  HS_TRANSPOSE_REMAP( u,   5,   9 )        \
  HS_TRANSPOSE_REMAP( u,   6,  11 )        \
  HS_TRANSPOSE_REMAP( u,   7,  13 )        \
  HS_TRANSPOSE_REMAP( u,   8,  15 )        \
  HS_TRANSPOSE_REMAP( u,   9,   2 )        \
  HS_TRANSPOSE_REMAP( u,  10,   4 )        \
  HS_TRANSPOSE_REMAP( u,  11,   6 )        \
  HS_TRANSPOSE_REMAP( u,  12,   8 )        \
  HS_TRANSPOSE_REMAP( u,  13,  10 )        \
  HS_TRANSPOSE_REMAP( u,  14,  12 )        \
  HS_TRANSPOSE_REMAP( u,  15,  14 )        \
  HS_TRANSPOSE_REMAP( u,  16,  16 )        \
  HS_EMPTY

#endif

//
//
//

