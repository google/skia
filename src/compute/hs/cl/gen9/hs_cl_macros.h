//
// Copyright 2016 Google Inc.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
//

#ifndef HS_CL_MACROS_ONCE
#define HS_CL_MACROS_ONCE

//
//
//

#include "hs_cl.h"

//
// Inter-lane compare exchange
//

// default
#define HS_CMP_XCHG_V0(a,b)                     \
  {                                             \
    HS_KEY_TYPE const t = min(a,b);             \
    b = max(a,b);                               \
    a = t;                                      \
  }

// super slow
#define HS_CMP_XCHG_V1(a,b)                     \
  {                                             \
    HS_KEY_TYPE const tmp = a;                  \
    a  = (a < b) ? a : b;                       \
    b ^= a ^ tmp;                               \
  }

// best
#define HS_CMP_XCHG_V2(a,b)                     \
  if (a >= b) {                                 \
    HS_KEY_TYPE const t = a;                    \
    a = b;                                      \
    b = t;                                      \
  }

// good
#define HS_CMP_XCHG_V3(a,b)                     \
  {                                             \
    int         const ge = a >= b;              \
    HS_KEY_TYPE const t  = a;                   \
    a = ge ? b : a;                             \
    b = ge ? t : b;                             \
  }

//
//
//

#if   (HS_KEY_WORDS == 1)
#define HS_CMP_XCHG(a,b)  HS_CMP_XCHG_V0(a,b)
#elif (HS_KEY_WORDS == 2)
#define HS_CMP_XCHG(a,b)  HS_CMP_XCHG_V2(a,b)
#endif

//
// Conditional inter-subgroup flip/half compare exchange
//

#define HS_CMP_FLIP(i,a,b)                                              \
  {                                                                     \
    HS_KEY_TYPE const ta = intel_sub_group_shuffle(a,flip_lane_idx);    \
    HS_KEY_TYPE const tb = intel_sub_group_shuffle(b,flip_lane_idx);    \
    a = HS_COND_MIN_MAX(t_lt,a,tb);                                     \
    b = HS_COND_MIN_MAX(t_lt,b,ta);                                     \
  }

#define HS_CMP_HALF(i,a)                                                \
  {                                                                     \
    HS_KEY_TYPE const ta = intel_sub_group_shuffle(a,half_lane_idx);    \
    a = HS_COND_MIN_MAX(t_lt,a,ta);                                     \
  }

//
// The device's comparison operator might return what we actually
// want.  For example, it appears GEN 'cmp' returns {true:-1,false:0}.
//

#define HS_CMP_IS_ZERO_ONE

#ifdef HS_CMP_IS_ZERO_ONE
// OpenCL requires a {true: +1, false: 0} scalar result
// (a < b) -> { +1, 0 } -> NEGATE -> { 0, 0xFFFFFFFF }
#define HS_LTE_TO_MASK(a,b) (HS_KEY_TYPE)(-(a <= b))
#define HS_CMP_TO_MASK(a)   (HS_KEY_TYPE)(-a)
#else
// However, OpenCL requires { -1, 0 } for vectors
// (a < b) -> { 0xFFFFFFFF, 0 }
#define HS_LTE_TO_MASK(a,b) (a <= b) // FIXME for uint64
#define HS_CMP_TO_MASK(a)   (a)
#endif

//
// The flip/half comparisons rely on a "conditional min/max":
//
//  - if the flag is false, return min(a,b)
//  - otherwise, return max(a,b)
//
// What's a little surprising is that sequence (1) is faster than (2)
// for 32-bit keys.
//
// I suspect either a code generation problem or that the sequence
// maps well to the GEN instruction set.
//
// We mostly care about 64-bit keys and unsurprisingly sequence (2) is
// fastest for this wider type.
//

// this is what you would normally use
#define HS_COND_MIN_MAX_V0(lt,a,b) ((a <= b) ^ lt) ? b : a

// this seems to be faster for 32-bit keys
#define HS_COND_MIN_MAX_V1(lt,a,b) (lt ? b : a) ^ ((a ^ b) & HS_LTE_TO_MASK(a,b))

//
//
//

#if   (HS_KEY_WORDS == 1)
#define HS_COND_MIN_MAX(lt,a,b) HS_COND_MIN_MAX_V1(lt,a,b)
#elif (HS_KEY_WORDS == 2)
#define HS_COND_MIN_MAX(lt,a,b) HS_COND_MIN_MAX_V0(lt,a,b)
#endif

//
// This snarl of macros is for transposing a "slab" of sorted elements
// into linear order.
//
// This can occur as the last step in hs_sort() or via a custom kernel
// that inspects the slab and then transposes and stores it to memory.
//
// The slab format can be inspected more efficiently than a linear
// arrangement.
//
// The prime example is detecting when adjacent keys (in sort order)
// have differing high order bits ("key changes").  The index of each
// change is recorded to an auxilary array.
//
// A post-processing step like this needs to be able to navigate the
// slab and eventually transpose and store the slab in linear order.
//

#define HS_TRANSPOSE_REG(prefix,row)   prefix##row
#define HS_TRANSPOSE_DECL(prefix,row)  HS_KEY_TYPE const HS_TRANSPOSE_REG(prefix,row)

#define HS_TRANSPOSE_DELTA(level)     (HS_LANES_PER_WARP + (1 << (level-1)))
#define HS_TRANSPOSE_IF(level)        ((get_sub_group_local_id() >> (level - 1)) & 1)

#define HS_TRANSPOSE_LL(level)        HS_TRANSPOSE_IF(level) ? 0 : HS_TRANSPOSE_DELTA(level)
#define HS_TRANSPOSE_UR(level)        HS_TRANSPOSE_IF(level) ? HS_TRANSPOSE_DELTA(level) : 0

#define HS_TRANSPOSE_DELTA_LL(level)  delta_ll_##level
#define HS_TRANSPOSE_DELTA_UR(level)  delta_ur_##level

#define HS_TRANSPOSE_STAGE(level)                                       \
  uint const HS_TRANSPOSE_DELTA_LL(level) = HS_TRANSPOSE_LL(level);     \
  uint const HS_TRANSPOSE_DELTA_UR(level) = HS_TRANSPOSE_UR(level);

#define HS_TRANSPOSE_BLEND(prefix_prev,prefix_curr,level,row_ll,row_ur) \
  HS_TRANSPOSE_DECL(prefix_curr,row_ll) =                               \
    intel_sub_group_shuffle_down(HS_TRANSPOSE_REG(prefix_prev,row_ll),  \
                                 HS_TRANSPOSE_REG(prefix_prev,row_ur),  \
                                 HS_TRANSPOSE_DELTA_LL(level));         \
  HS_TRANSPOSE_DECL(prefix_curr,row_ur) =                               \
    intel_sub_group_shuffle_up(HS_TRANSPOSE_REG(prefix_prev,row_ll),    \
                               HS_TRANSPOSE_REG(prefix_prev,row_ur),    \
                               HS_TRANSPOSE_DELTA_UR(level));           \

// #define HS_TRANSPOSE_LOAD(row)                                        \
//   HS_TRANSPOSE_DECL(0,row) = (vout + gmem_idx)[(row-1) << HS_LANES_PER_WARP_LOG2];

#define HS_TRANSPOSE_REMAP(prefix,row_from,row_to)                      \
  (vout + gmem_idx)[(row_to-1) << HS_LANES_PER_WARP_LOG2] =             \
    HS_TRANSPOSE_REG(prefix,row_from);

//
// undefine these if you want to override
//

#define HS_TRANSPOSE_PREAMBLE()
#define HS_TRANSPOSE_BODY()

//
//
//

#endif

//
//
//
