//
// Copyright 2016 Google Inc.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
//

#ifndef HS_GLSL_ONCE
#define HS_GLSL_ONCE

#define HS_SLAB_THREADS_LOG2    6
#define HS_SLAB_THREADS         (1 << HS_SLAB_THREADS_LOG2)
#define HS_SLAB_WIDTH_LOG2      6
#define HS_SLAB_WIDTH           (1 << HS_SLAB_WIDTH_LOG2)
#define HS_SLAB_HEIGHT          8
#define HS_SLAB_KEYS            (HS_SLAB_WIDTH * HS_SLAB_HEIGHT)
#define HS_REG_LAST(c)          c##8
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

#define HS_AMD_GCN

#define HS_SLAB_ROWS()    \
  HS_SLAB_ROW(   1,   0 ) \
  HS_SLAB_ROW(   2,   1 ) \
  HS_SLAB_ROW(   3,   2 ) \
  HS_SLAB_ROW(   4,   3 ) \
  HS_SLAB_ROW(   5,   4 ) \
  HS_SLAB_ROW(   6,   5 ) \
  HS_SLAB_ROW(   7,   6 ) \
  HS_SLAB_ROW(   8,   7 ) \
  HS_EMPTY

#define HS_TRANSPOSE_SLAB()                \
  HS_TRANSPOSE_STAGE( 1 )                  \
  HS_TRANSPOSE_STAGE( 2 )                  \
  HS_TRANSPOSE_STAGE( 3 )                  \
  HS_TRANSPOSE_STAGE( 4 )                  \
  HS_TRANSPOSE_STAGE( 5 )                  \
  HS_TRANSPOSE_STAGE( 6 )                  \
  HS_TRANSPOSE_BLEND( r, s,  1,   2,   1 ) \
  HS_TRANSPOSE_BLEND( r, s,  1,   4,   3 ) \
  HS_TRANSPOSE_BLEND( r, s,  1,   6,   5 ) \
  HS_TRANSPOSE_BLEND( r, s,  1,   8,   7 ) \
  HS_TRANSPOSE_BLEND( s, t,  2,   3,   1 ) \
  HS_TRANSPOSE_BLEND( s, t,  2,   4,   2 ) \
  HS_TRANSPOSE_BLEND( s, t,  2,   7,   5 ) \
  HS_TRANSPOSE_BLEND( s, t,  2,   8,   6 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,   5,   1 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,   6,   2 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,   7,   3 ) \
  HS_TRANSPOSE_BLEND( t, u,  3,   8,   4 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,   2,   1 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,   4,   3 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,   6,   5 ) \
  HS_TRANSPOSE_BLEND( u, v,  4,   8,   7 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,   3,   1 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,   4,   2 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,   7,   5 ) \
  HS_TRANSPOSE_BLEND( v, w,  5,   8,   6 ) \
  HS_TRANSPOSE_BLEND( w, x,  6,   5,   1 ) \
  HS_TRANSPOSE_BLEND( w, x,  6,   6,   2 ) \
  HS_TRANSPOSE_BLEND( w, x,  6,   7,   3 ) \
  HS_TRANSPOSE_BLEND( w, x,  6,   8,   4 ) \
  HS_TRANSPOSE_REMAP( x,   1,   1 )        \
  HS_TRANSPOSE_REMAP( x,   2,   2 )        \
  HS_TRANSPOSE_REMAP( x,   3,   3 )        \
  HS_TRANSPOSE_REMAP( x,   4,   4 )        \
  HS_TRANSPOSE_REMAP( x,   5,   5 )        \
  HS_TRANSPOSE_REMAP( x,   6,   6 )        \
  HS_TRANSPOSE_REMAP( x,   7,   7 )        \
  HS_TRANSPOSE_REMAP( x,   8,   8 )        \
  HS_EMPTY

#endif

//
//
//

