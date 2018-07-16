/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#ifndef SKC_ONCE_STYLING_TYPES
#define SKC_ONCE_STYLING_TYPES

//
//
//

#include "types.h"
#include "macros.h"
#include "skc_styling.h"

//
//
//

typedef skc_uint  skc_layer_id;
typedef skc_uint  skc_group_id;

//
//
//

union skc_styling_cmd
{
  skc_uint                    u32;
  skc_int                     s32;
  skc_float                   f32;
  skc_half                    f16a2[2];
  skc_ushort2                 u16v2;
  skc_styling_opcode_e        opcode;
  skc_styling_gradient_type_e gradient_type;
  skc_group_id                parent;

#if 0
#if !defined(__OPENCL_C_VERSION__)
  struct {
    skc_uint                  opcode : 31;
    skc_uint                  final  : 1;
  };
#endif
#endif
};

#define SKC_STYLING_OPCODE_MASK_OPCODE    SKC_BITS_TO_MASK(31)
#define SKC_STYLING_OPCODE_MASK_IS_FINAL  SKC_BITS_TO_MASK_AT(1,31)

SKC_STATIC_ASSERT(sizeof(union skc_styling_cmd) == sizeof(skc_uint));

//
//
//

union skc_layer_node
{
  skc_uint2      u32v2;

  struct {
    skc_uint     cmds;   // starting index of sequence of command words
    skc_group_id parent; // index of parent group
  };
};

SKC_STATIC_ASSERT(sizeof(union skc_layer_node) == sizeof(skc_uint2));

//
//
//

union skc_group_parents
{
  skc_uint2  u32v2;

  struct {
    skc_uint depth; // zero-based depth of this group
    skc_uint base;  // index to sequence of group ids leading back to root
  };
};

SKC_STATIC_ASSERT(sizeof(union skc_group_parents) == sizeof(skc_uint2));

//
// inclusive layer range [lo,hi]
//

union skc_group_range
{
  skc_uint2  u32v2;

  struct {
    skc_uint lo; // first layer
    skc_uint hi; // last  layer
  };
};

SKC_STATIC_ASSERT(sizeof(union skc_group_range) == sizeof(skc_uint2));

//
//
//

struct skc_group_node
{
  union skc_group_parents parents; // path of parent groups leading back to root

  union skc_group_range   range;   // range of layers enclosed by this group

  struct {
    skc_uint              enter;   // starting index of sequence of command words
    skc_uint              leave;   // starting index of sequence of command words
  } cmds;
};

SKC_STATIC_ASSERT(sizeof(struct skc_group_node) == sizeof(skc_uint2) * 3); // 6 words

//
//
//

union skc_gradient_slope
{
  skc_float  slope;
  skc_half   color_pair[2];
};

SKC_STATIC_ASSERT(sizeof(union skc_gradient_slope) == sizeof(skc_float));

//
//
//

union skc_gradient_vector
{
  skc_float4               f32v4;

  struct {
    skc_float              dx;
    skc_float              p0;
    skc_float              dy;
    skc_float              denom;
  };

  union skc_gradient_slope slopes[4];
};

SKC_STATIC_ASSERT(sizeof(union skc_gradient_vector) == sizeof(skc_float4));

//
// FIXME -- will eventually need to know if this gradient is
// perspective transformed and if so additional values will need to be
// encoded
//
// VERSION 1
// =============================================================
//
// LINEAR GRADIENT HEADER FOR N STOPS
//
// +----------+----------+------------+----------+-------------+
// |  HEADER  |   INFO   |    LUTS    |  FLOORS  |    COLORS   |
// +----------+----------+------------+----------+-------------+
// |  uintv4  | u32v2[1] | f32v2[N-1] | f32[N-2] | ushort2[4N] |
// +----------+----------+------------+----------+-------------+
//
//   COLOR PAIR            WORD EXPANSION            TOTAL
// +------------+---------------------------------+--------+-------------------------+
// |  ushort2   |  4 + 2 + 2*(N-1) + N - 2 + 4*N  | 7N + 2 | = 7(N-1+1)+2 = 7(N-1)+9 |
// +------------+---------------------------------+--------+-------------------------+
//
// COLOR LAYOUT:
//
//   R[0]R[1], R[1]R[2], ... R[N-1]R[N-1]
//   G[0]G[1], G[1]G[2], ... G[N-1]G[N-1]
//   B[0]B[1], B[1]B[2], ... B[N-1]B[N-1]
//   A[0]A[1], A[1]A[2], ... A[N-1]A[N-1]
//
//
// MINIMUM WORDS:  N=2 --> 16
//
//
// VERSION 2
// =============================================================
//
// LINEAR GRADIENT DESCRIPTOR FOR N STOPS
//
//                           +--------------- REMOVE ME LATER
//                           v
// +--------+------+-------+---+----------+-----------+
// | VECTOR | TYPE | COUNT | N |  SLOPES  |   COLORS  |
// +--------+------+-------+---+----------+-----------+
// |  f32v4 |   1  |   1   | 1 | f32[N-1] | f16v2[4N] |
// +--------+------+-------+---+----------+-----------+
//
//   COLOR PAIR           WORD EXPANSION            TOTAL
// +------------+--------------------------------+--------+
// |   f16v2    |  4 + 1 + 1 + 1 + [N-1] + [4*N] | 5N + 6 |
// +------------+--------------------------------+--------+
//
// COLOR LAYOUT:
//
//   R[0]R[1], R[1]R[2], ... R[N-1]R[N-1] <-------------------------- FIXME -- USE HERB'S SINGLE FMA REPRESENTATION
//   G[0]G[1], G[1]G[2], ... G[N-1]G[N-1] <-------------------------- FIXME -- USE HERB'S SINGLE FMA REPRESENTATION
//   B[0]B[1], B[1]B[2], ... B[N-1]B[N-1] <-------------------------- FIXME -- USE HERB'S SINGLE FMA REPRESENTATION
//   A[0]A[1], A[1]A[2], ... A[N-1]A[N-1] <-------------------------- FIXME -- USE HERB'S SINGLE FMA REPRESENTATION
//
//
// MINIMUM WORDS:  N=2 --> 16
//
//
// VERSION 3+
// =============================================================
//
// FIXME -- will probably want to try using the sampler/texture
// hardware to interpolate colors.
//
// This will require that the colors are laid out in sampler-friendly
// order:
//
//    RGBA[0]RGBA[1], RGBA[1]RGBA[2], ..., RGBA[N-1]RGBA[N-1]
//
//

#if 0
#define SKC_GRADIENT_HEADER_WORDS_LUTS_OFFSET       4
#define SKC_GRADIENT_HEADER_WORDS_TOTAL(n_minus_1)  (7 * (n_minus_1) + 9)
#define SKC_GRADIENT_HEADER_WORDS_MIN               SKC_GRADIENT_HEADER_WORDS_TOTAL(1)
#define SKC_GRADIENT_CMD_WORDS_V1(n)                (1 + SKC_GRADIENT_HEADER_WORDS_TOTAL(n-1))
#endif

#define SKC_GRADIENT_CMD_WORDS_V1(n)                (7 * (n) + 2)
#define SKC_GRADIENT_CMD_WORDS_V2(n)                (5 * (n) + 6)
#define SKC_GRADIENT_CMD_WORDS_V2_ADJUST(v1,v2)     (SKC_GRADIENT_CMD_WORDS_V1(v1) - ((v2) + 6))

//
//
//

#endif

//
//
//
