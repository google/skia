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
// FYI, restrict shouldn't have any impact on these kernels and
// benchmarks appear to prove that true
//

#define HS_RESTRICT restrict

//
// KERNEL PROTOS
//

#define HS_TRANSPOSE_KERNEL_PROTO(slab_width)                           \
  __kernel                                                              \
  __attribute__((intel_reqd_sub_group_size(slab_width)))                \
  void                                                                  \
  hs_kernel_transpose(__global HS_KEY_TYPE * const HS_RESTRICT vout)

#define HS_BS_KERNEL_PROTO(slab_width,slab_count,slab_count_ru_log2)    \
  __kernel                                                              \
  __attribute__((reqd_work_group_size(slab_count*slab_width,1,1)))      \
  __attribute__((intel_reqd_sub_group_size(slab_width)))                \
  void                                                                  \
  hs_kernel_bs_##slab_count_ru_log2##(__global HS_KEY_TYPE const * const HS_RESTRICT vin, \
                                      __global HS_KEY_TYPE       * const HS_RESTRICT vout)

#define HS_BC_KERNEL_PROTO(slab_width,slab_count,slab_count_log2)       \
  __kernel                                                              \
  __attribute__((reqd_work_group_size(slab_count*slab_width,1,1)))      \
  __attribute__((intel_reqd_sub_group_size(slab_width)))                \
  void                                                                  \
  hs_kernel_bc_##slab_count_log2##(__global HS_KEY_TYPE * const HS_RESTRICT vout)

#define HS_HM_KERNEL_PROTO(s)                                   \
  __kernel void                                                 \
  hs_kernel_hm_##s##(__global HS_KEY_TYPE * const HS_RESTRICT vout)

#define HS_FM_KERNEL_PROTO(s,r)                                         \
  __kernel void                                                         \
  hs_kernel_fm_##s##_##r##(__global HS_KEY_TYPE * const HS_RESTRICT vout)

//
// BLOCK LOCAL MEMORY DECLARATION
//

#define HS_BLOCK_LOCAL_MEM_DECL(width,height)   \
  __local struct {                              \
    HS_KEY_TYPE m[width * height];              \
  } shared

//
// BLOCK BARRIER
//

#define HS_BLOCK_BARRIER()                      \
  barrier(CLK_LOCAL_MEM_FENCE)

//
// SLAB GLOBAL
//

#define HS_SLAB_GLOBAL_PREAMBLE(slab_width,slab_height)         \
  uint const gmem_idx =                                         \
    (get_global_id(0) & ~(slab_width-1)) * slab_height +        \
    get_sub_group_local_id()

#define HS_SLAB_GLOBAL_LOAD(extent,slab_width,row_idx)  \
  extent[gmem_idx + slab_width * row_idx]

#define HS_SLAB_GLOBAL_STORE(slab_width,row_idx,reg)    \
  vout[gmem_idx + slab_width * row_idx] = reg

//
// SLAB LOCAL
//

#define HS_SLAB_LOCAL_L(offset)                 \
  shared.m[smem_l_idx + (offset)]

#define HS_SLAB_LOCAL_R(offset)                 \
  shared.m[smem_r_idx + (offset)]

//
// SLAB LOCAL VERTICAL LOADS
//

#define HS_BX_LOCAL_V(offset)                   \
  shared.m[get_local_id(0) + (offset)]

//
// BLOCK SORT MERGE HORIZONTAL
//

#define HS_BS_MERGE_H_PREAMBLE(slab_width,slab_count)           \
  uint const smem_l_idx =                                       \
    get_sub_group_id() * (slab_width * slab_count) +            \
    get_sub_group_local_id();                                   \
  uint const smem_r_idx =                                       \
    (get_sub_group_id() ^ 1) * (slab_width * slab_count) +      \
    (get_sub_group_local_id() ^ (slab_width - 1))

//
// BLOCK CLEAN MERGE HORIZONTAL
//

#define HS_BC_MERGE_H_PREAMBLE(slab_width,slab_height,slab_count)       \
  uint const gmem_l_idx =                                               \
    (get_global_id(0) & ~(slab_width*slab_count-1)) * slab_height +     \
    get_local_id(0);                                                    \
  uint const smem_l_idx =                                               \
    get_sub_group_id() * (slab_width * slab_count) +                    \
    get_sub_group_local_id()

#define HS_BC_GLOBAL_LOAD_L(slab_width,slab_idx)        \
  vout[gmem_l_idx + (slab_width * slab_idx)]

//
// SLAB FLIP AND HALF PREAMBLES
//

#define HS_SLAB_FLIP_PREAMBLE(mask)                                     \
  uint const flip_lane_idx = get_sub_group_local_id() ^ mask;           \
  int  const t_lt          = get_sub_group_local_id() < flip_lane_idx;

#define HS_SLAB_HALF_PREAMBLE(mask)                                     \
  uint const half_lane_idx = get_sub_group_local_id() ^ mask;           \
  int  const t_lt          = get_sub_group_local_id() < half_lane_idx;

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
// The "flip-merge" and "half-merge" preambles are very similar
//

#define HS_HM_PREAMBLE(half_span)                                       \
  uint const span_idx    = get_global_id(2) * get_global_size(1) + get_global_id(1); \
  uint const span_stride = get_global_size(0);                          \
  uint const span_size   = span_stride * half_span * 2;                 \
  uint const span_base   = span_idx * span_size;                        \
  uint const span_off    = get_global_id(0);                            \
  uint const span_l      = span_base + span_off

#define HS_FM_PREAMBLE(half_span)                                       \
  HS_HM_PREAMBLE(half_span);                                            \
  uint const span_r      = span_base + span_stride * (half_span + 1) - span_off - 1

//
//
//

#define HS_XM_GLOBAL_L(stride_idx)              \
  vout[span_l + span_stride * stride_idx]

#define HS_XM_GLOBAL_LOAD_L(stride_idx)         \
  HS_XM_GLOBAL_L(stride_idx)

#define HS_XM_GLOBAL_STORE_L(stride_idx,reg)    \
  HS_XM_GLOBAL_L(stride_idx) = reg

#define HS_FM_GLOBAL_R(stride_idx)              \
  vout[span_r + span_stride * stride_idx]

#define HS_FM_GLOBAL_LOAD_R(stride_idx)         \
  HS_FM_GLOBAL_R(stride_idx)

#define HS_FM_GLOBAL_STORE_R(stride_idx,reg)    \
  HS_FM_GLOBAL_R(stride_idx) = reg

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

#define HS_SUBGROUP_SHUFFLE_XOR(v,m)   intel_sub_group_shuffle_xor(v,m)

#define HS_TRANSPOSE_REG(prefix,row)   prefix##row
#define HS_TRANSPOSE_DECL(prefix,row)  HS_KEY_TYPE const HS_TRANSPOSE_REG(prefix,row)
#define HS_TRANSPOSE_PRED(level)       is_lo_##level

#define HS_TRANSPOSE_TMP_REG(prefix_curr,row_ll,row_ur)       \
  prefix_curr##row_ll##_##row_ur

#define HS_TRANSPOSE_TMP_DECL(prefix_curr,row_ll,row_ur)      \
  HS_KEY_TYPE const HS_TRANSPOSE_TMP_REG(prefix_curr,row_ll,row_ur)

#define HS_TRANSPOSE_STAGE(level)                       \
  bool const HS_TRANSPOSE_PRED(level) =                 \
    (get_sub_group_local_id() & (1 << (level-1))) == 0;

#define HS_TRANSPOSE_BLEND(prefix_prev,prefix_curr,level,row_ll,row_ur) \
  HS_TRANSPOSE_TMP_DECL(prefix_curr,row_ll,row_ur) =                    \
    HS_SUBGROUP_SHUFFLE_XOR(HS_TRANSPOSE_PRED(level) ?                  \
                            HS_TRANSPOSE_REG(prefix_prev,row_ll) :      \
                            HS_TRANSPOSE_REG(prefix_prev,row_ur),       \
                            1<<(level-1));                              \
                                                                        \
  HS_TRANSPOSE_DECL(prefix_curr,row_ll) =                               \
    HS_TRANSPOSE_PRED(level)                  ?                         \
    HS_TRANSPOSE_TMP_REG(prefix_curr,row_ll,row_ur) :                   \
    HS_TRANSPOSE_REG(prefix_prev,row_ll);                               \
                                                                        \
  HS_TRANSPOSE_DECL(prefix_curr,row_ur) =                               \
    HS_TRANSPOSE_PRED(level)                  ?                         \
    HS_TRANSPOSE_REG(prefix_prev,row_ur)      :                         \
    HS_TRANSPOSE_TMP_REG(prefix_curr,row_ll,row_ur);

#define HS_TRANSPOSE_REMAP(prefix,row_from,row_to)      \
  vout[gmem_idx + ((row_to-1) << HS_SLAB_WIDTH_LOG2)] = \
    HS_TRANSPOSE_REG(prefix,row_from);

//
//
//

#endif

//
//
//
