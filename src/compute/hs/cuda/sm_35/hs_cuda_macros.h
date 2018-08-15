//
// Copyright 2016 Google Inc.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
//

#ifndef HS_CUDA_MACROS_ONCE
#define HS_CUDA_MACROS_ONCE

//
//
//

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifdef __cplusplus
}
#endif

//
// Define the type based on key and val sizes
//

#if   HS_KEY_WORDS == 1
#if   HS_VAL_WORDS == 0
#define HS_KEY_TYPE  uint32_t
#endif
#elif HS_KEY_WORDS == 2
#define HS_KEY_TYPE  uint64_t
#endif

//
// FYI, restrict shouldn't have any impact on these kernels and
// benchmarks appear to prove that true
//

#define HS_RESTRICT  __restrict__

//
//
//

#define HS_SCOPE()                              \
  static

#define HS_KERNEL_QUALIFIER()                   \
  __global__ void

//
// The sm_35 arch has a maximum of 16 blocks per multiprocessor.  Just
// clamp it to 16 when targeting this arch.
//
// This only arises when compiling the 32-bit sorting kernels.
//
// You can also generate a narrower 16-warp wide 32-bit sorting kernel
// which is sometimes faster and sometimes slower than the 32-block
// configuration.
//

#if ( __CUDA_ARCH__ == 350 )
#define HS_CUDA_MAX_BPM  16
#else
#define HS_CUDA_MAX_BPM  UINT32_MAX // 32
#endif

#define HS_CLAMPED_BPM(min_bpm)                                 \
  ((min_bpm) < HS_CUDA_MAX_BPM ? (min_bpm) : HS_CUDA_MAX_BPM)

//
//
//

#define HS_LAUNCH_BOUNDS(max_tpb,min_bpm)       \
  __launch_bounds__(max_tpb,HS_CLAMPED_BPM(min_bpm))

//
// KERNEL PROTOS
//

#define HS_BS_KERNEL_NAME(slab_count_ru_log2)   \
  hs_kernel_bs_##slab_count_ru_log2

#define HS_BS_KERNEL_PROTO(slab_count,slab_count_ru_log2)             \
  HS_SCOPE()                                                          \
  HS_KERNEL_QUALIFIER()                                               \
  HS_LAUNCH_BOUNDS(HS_SLAB_THREADS*slab_count,1)                      \
  HS_BS_KERNEL_NAME(slab_count_ru_log2)(HS_KEY_TYPE       * const HS_RESTRICT vout, \
                                        HS_KEY_TYPE const * const HS_RESTRICT vin)

//

#define HS_OFFSET_BS_KERNEL_NAME(slab_count_ru_log2)    \
  hs_kernel_bs_##slab_count_ru_log2

#define HS_OFFSET_BS_KERNEL_PROTO(slab_count,slab_count_ru_log2)              \
  HS_SCOPE()                                                                  \
  HS_KERNEL_QUALIFIER()                                                       \
  HS_LAUNCH_BOUNDS(HS_SLAB_THREADS*slab_count,HS_BS_SLABS/(1<<slab_count_ru_log2)) \
  HS_OFFSET_BS_KERNEL_NAME(slab_count_ru_log2)(HS_KEY_TYPE       * const HS_RESTRICT vout, \
                                               HS_KEY_TYPE const * const HS_RESTRICT vin,  \
                                               uint32_t            const             slab_offset)

//

#define HS_BC_KERNEL_NAME(slab_count_log2)      \
  hs_kernel_bc_##slab_count_log2

#define HS_BC_KERNEL_PROTO(slab_count,slab_count_log2)                \
  HS_SCOPE()                                                          \
  HS_KERNEL_QUALIFIER()                                               \
  HS_LAUNCH_BOUNDS(HS_SLAB_THREADS*slab_count,HS_BS_SLABS/(1<<slab_count_log2)) \
  HS_BC_KERNEL_NAME(slab_count_log2)(HS_KEY_TYPE * const HS_RESTRICT vout)

//

#define HS_HM_KERNEL_NAME(s)                    \
  hs_kernel_hm_##s

#define HS_HM_KERNEL_PROTO(s)                                 \
  HS_SCOPE()                                                  \
  HS_KERNEL_QUALIFIER()                                       \
  HS_HM_KERNEL_NAME(s)(HS_KEY_TYPE * const HS_RESTRICT vout)

//

#define HS_FM_KERNEL_NAME(s,r)                  \
  hs_kernel_fm_##s##_##r

#define HS_FM_KERNEL_PROTO(s,r)                                      \
  HS_SCOPE()                                                         \
  HS_KERNEL_QUALIFIER()                                              \
  HS_FM_KERNEL_NAME(s,r)(HS_KEY_TYPE * const HS_RESTRICT vout)

//

#define HS_OFFSET_FM_KERNEL_NAME(s,r)           \
  hs_kernel_fm_##s##_##r

#define HS_OFFSET_FM_KERNEL_PROTO(s,r)                                \
  HS_SCOPE()                                                          \
  HS_KERNEL_QUALIFIER()                                               \
  HS_OFFSET_FM_KERNEL_NAME(s,r)(HS_KEY_TYPE * const HS_RESTRICT vout, \
                                uint32_t      const             span_offset)

//

#define HS_TRANSPOSE_KERNEL_NAME()              \
  hs_kernel_transpose

#define HS_TRANSPOSE_KERNEL_PROTO()                             \
  HS_SCOPE()                                                    \
  HS_KERNEL_QUALIFIER()                                         \
  HS_LAUNCH_BOUNDS(HS_SLAB_THREADS,1)                           \
  HS_TRANSPOSE_KERNEL_NAME()(HS_KEY_TYPE * const HS_RESTRICT vout)

//
// BLOCK LOCAL MEMORY DECLARATION
//

#define HS_BLOCK_LOCAL_MEM_DECL(width,height)   \
  __shared__ struct {                           \
    HS_KEY_TYPE m[width * height];              \
  } shared

//
// BLOCK BARRIER
//

#define HS_BLOCK_BARRIER()                      \
  __syncthreads()

//
// GRID VARIABLES
//

#define HS_GLOBAL_SIZE_X() (gridDim.x * blockDim.x)
#define HS_GLOBAL_ID_X()   (blockDim.x * blockIdx.x + threadIdx.x)
#define HS_LOCAL_ID_X()    threadIdx.x
#define HS_WARP_ID_X()     (threadIdx.x / 32)
#define HS_LANE_ID()       (threadIdx.x & 31)

//
// SLAB GLOBAL
//

#define HS_SLAB_GLOBAL_PREAMBLE()               \
  uint32_t const gmem_idx =                     \
    (HS_GLOBAL_ID_X() & ~(HS_SLAB_THREADS-1)) * \
    HS_SLAB_HEIGHT + HS_LANE_ID()

#define HS_OFFSET_SLAB_GLOBAL_PREAMBLE()                        \
  uint32_t const gmem_idx =                                     \
    ((slab_offset + HS_GLOBAL_ID_X()) & ~(HS_SLAB_THREADS-1)) * \
    HS_SLAB_HEIGHT + HS_LANE_ID()

#define HS_SLAB_GLOBAL_LOAD(extent,row_idx)  \
  extent[gmem_idx + HS_SLAB_THREADS * row_idx]

#define HS_SLAB_GLOBAL_STORE(row_idx,reg)    \
  vout[gmem_idx + HS_SLAB_THREADS * row_idx] = reg

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
  shared.m[HS_LOCAL_ID_X() + (offset)]

//
// BLOCK SORT MERGE HORIZONTAL
//

#define HS_BS_MERGE_H_PREAMBLE(slab_count)                      \
  uint32_t const smem_l_idx =                                   \
    HS_WARP_ID_X() * (HS_SLAB_THREADS * slab_count) +           \
    HS_LANE_ID();                                               \
  uint32_t const smem_r_idx =                                   \
    (HS_WARP_ID_X() ^ 1) * (HS_SLAB_THREADS * slab_count) +     \
    (HS_LANE_ID() ^ (HS_SLAB_THREADS - 1))

//
// BLOCK CLEAN MERGE HORIZONTAL
//

#define HS_BC_MERGE_H_PREAMBLE(slab_count)                      \
  uint32_t const gmem_l_idx =                                   \
    (HS_GLOBAL_ID_X() & ~(HS_SLAB_THREADS*slab_count-1)) *      \
    HS_SLAB_HEIGHT + HS_LOCAL_ID_X();                           \
  uint32_t const smem_l_idx =                                   \
    HS_WARP_ID_X() * (HS_SLAB_THREADS * slab_count) +           \
    HS_LANE_ID()

#define HS_BC_GLOBAL_LOAD_L(slab_idx)                   \
  vout[gmem_l_idx + (HS_SLAB_THREADS * slab_idx)]

//
// SLAB FLIP AND HALF PREAMBLES
//

#define HS_SLAB_FLIP_PREAMBLE(mask)                             \
  uint32_t const flip_lane_idx  = HS_LANE_ID() ^ mask;          \
  int32_t  const t_lt           = HS_LANE_ID() < flip_lane_idx;

// if we want to shlf_xor: uint32_t const flip_lane_mask = mask;

#define HS_SLAB_HALF_PREAMBLE(mask)                             \
  uint32_t const half_lane_idx  = HS_LANE_ID() ^ mask;          \
  int32_t  const t_lt           = HS_LANE_ID() < half_lane_idx;

// if we want to shfl_xor: uint32_t const half_lane_mask = mask;

//
// Inter-lane compare exchange
//

// good
#define HS_CMP_XCHG_V0(a,b)                     \
  {                                             \
    HS_KEY_TYPE const t = min(a,b);             \
    b = max(a,b);                               \
    a = t;                                      \
  }

// surprisingly fast -- #1 on 64-bit keys
#define HS_CMP_XCHG_V1(a,b)                     \
  {                                             \
    HS_KEY_TYPE const tmp = a;                  \
    a  = (a < b) ? a : b;                       \
    b ^= a ^ tmp;                               \
  }

// good
#define HS_CMP_XCHG_V2(a,b)                     \
  if (a >= b) {                                 \
    HS_KEY_TYPE const t = a;                    \
    a = b;                                      \
    b = t;                                      \
  }

// good
#define HS_CMP_XCHG_V3(a,b)                     \
  {                                             \
    int32_t     const ge = a >= b;              \
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
#define HS_CMP_XCHG(a,b)  HS_CMP_XCHG_V0(a,b)
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
#define HS_COND_MIN_MAX(lt,a,b) HS_COND_MIN_MAX_V0(lt,a,b)
#elif (HS_KEY_WORDS == 2)
#define HS_COND_MIN_MAX(lt,a,b) HS_COND_MIN_MAX_V0(lt,a,b)
#endif

//
// HotSort shuffles are always warp-wide
//

#define HS_SHFL_ALL 0xFFFFFFFF

//
// Conditional inter-subgroup flip/half compare exchange
//

#define HS_CMP_FLIP(i,a,b)                                              \
  {                                                                     \
    HS_KEY_TYPE const ta = __shfl_sync(HS_SHFL_ALL,a,flip_lane_idx);    \
    HS_KEY_TYPE const tb = __shfl_sync(HS_SHFL_ALL,b,flip_lane_idx);    \
    a = HS_COND_MIN_MAX(t_lt,a,tb);                                     \
    b = HS_COND_MIN_MAX(t_lt,b,ta);                                     \
  }

#define HS_CMP_HALF(i,a)                                                \
  {                                                                     \
    HS_KEY_TYPE const ta = __shfl_sync(HS_SHFL_ALL,a,half_lane_idx);    \
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
// For now, we're only using the .y dimension for the span idx
//

#define HS_OFFSET_HM_PREAMBLE(half_span,span_offset)                    \
  uint32_t const span_idx    = span_offset + blockIdx.y;                \
  uint32_t const span_stride = HS_GLOBAL_SIZE_X();                      \
  uint32_t const span_size   = span_stride * half_span * 2;             \
  uint32_t const span_base   = span_idx * span_size;                    \
  uint32_t const span_off    = HS_GLOBAL_ID_X();                        \
  uint32_t const span_l      = span_base + span_off

#define HS_HM_PREAMBLE(half_span)               \
  HS_OFFSET_HM_PREAMBLE(half_span,0)            \

#define HS_FM_PREAMBLE(half_span)                                       \
  HS_HM_PREAMBLE(half_span);                                            \
  uint32_t const span_r = span_base + span_stride * (half_span + 1) - span_off - 1

#define HS_OFFSET_FM_PREAMBLE(half_span)                                \
  HS_OFFSET_HM_PREAMBLE(half_span,span_offset);                         \
  uint32_t const span_r = span_base + span_stride * (half_span + 1) - span_off - 1

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

#define HS_SUBGROUP_SHUFFLE_XOR(v,m)   __shfl_xor_sync(HS_SHFL_ALL,v,m)

#define HS_TRANSPOSE_REG(prefix,row)   prefix##row
#define HS_TRANSPOSE_DECL(prefix,row)  HS_KEY_TYPE const HS_TRANSPOSE_REG(prefix,row)
#define HS_TRANSPOSE_PRED(level)       is_lo_##level

#define HS_TRANSPOSE_TMP_REG(prefix_curr,row_ll,row_ur)       \
  prefix_curr##row_ll##_##row_ur

#define HS_TRANSPOSE_TMP_DECL(prefix_curr,row_ll,row_ur)      \
  HS_KEY_TYPE const HS_TRANSPOSE_TMP_REG(prefix_curr,row_ll,row_ur)

#define HS_TRANSPOSE_STAGE(level)                       \
  bool const HS_TRANSPOSE_PRED(level) =                 \
    (HS_LANE_ID() & (1 << (level-1))) == 0;

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
