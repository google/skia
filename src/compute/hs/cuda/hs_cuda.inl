/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
//
//

#ifdef __cplusplus
extern "C" {
#endif

#include "common/cuda/assert_cuda.h"
#include "common/macros.h"
#include "common/util.h"

#ifdef __cplusplus
}
#endif

//
// We want concurrent kernel execution to occur in a few places.
//
// The summary is:
//
//   1) If necessary, some max valued keys are written to the end of
//      the vin/vout buffers.
//
//   2) Blocks of slabs of keys are sorted.
//
//   3) If necesary, the blocks of slabs are merged until complete.
//
//   4) If requested, the slabs will be converted from slab ordering
//      to linear ordering.
//
// Below is the general "happens-before" relationship between HotSort
// compute kernels.
//
// Note the diagram assumes vin and vout are different buffers.  If
// they're not, then the first merge doesn't include the pad_vout
// event in the wait list.
//
//                    +----------+            +---------+
//                    | pad_vout |            | pad_vin |
//                    +----+-----+            +----+----+
//                         |                       |
//                         |                WAITFOR(pad_vin)
//                         |                       |
//                         |                 +-----v-----+
//                         |                 |           |
//                         |            +----v----+ +----v----+
//                         |            | bs_full | | bs_frac |
//                         |            +----+----+ +----+----+
//                         |                 |           |
//                         |                 +-----v-----+
//                         |                       |
//                         |  +------NO------JUST ONE BLOCK?
//                         | /                     |
//                         |/                     YES
//                         +                       |
//                         |                       v
//                         |         END_WITH_EVENTS(bs_full,bs_frac)
//                         |
//                         |
//        WAITFOR(pad_vout,bs_full,bs_frac) >>> first iteration of loop <<<
//                         |
//                         |
//                         +-----------<------------+
//                         |                        |
//                   +-----v-----+                  |
//                   |           |                  |
//              +----v----+ +----v----+             |
//              | fm_full | | fm_frac |             |
//              +----+----+ +----+----+             |
//                   |           |                  ^
//                   +-----v-----+                  |
//                         |                        |
//              WAITFOR(fm_full,fm_frac)            |
//                         |                        |
//                         v                        |
//                      +--v--+                WAITFOR(bc)
//                      | hm  |                     |
//                      +-----+                     |
//                         |                        |
//                    WAITFOR(hm)                   |
//                         |                        ^
//                      +--v--+                     |
//                      | bc  |                     |
//                      +-----+                     |
//                         |                        |
//                         v                        |
//                  MERGING COMPLETE?-------NO------+
//                         |
//                        YES
//                         |
//                         v
//                END_WITH_EVENTS(bc)
//
//
// NOTE: CUDA streams are in-order so a dependency isn't required for
// kernels launched on the same stream.
//
// This is actually a more subtle problem than it appears.
//
// We'll take a different approach and declare the "happens before"
// kernel relationships:
//
//      concurrent (pad_vin,pad_vout) -> (pad_vin)  happens_before (bs_full,bs_frac)
//                                       (pad_vout) happens_before (fm_full,fm_frac)
//
//      concurrent (bs_full,bs_frac)  -> (bs_full)  happens_before (fm_full,fm_frac)
//                                       (bs_frac)  happens_before (fm_full,fm_frac)
//
//      concurrent (fm_full,fm_frac)  -> (fm_full)  happens_before (hm)
//                                       (fm_frac)  happens_before (hm)
//
//      concurrent (fm_full,fm_frac)  -> (fm_full)  happens_before (hm)
//                                       (fm_frac)  happens_before (hm)
//
//      launch     (hm)               -> (hm)       happens_before (hm)
//                                       (hm)       happens_before (bc)
//
//      launch     (bc)               -> (bc)       happens_before (fm_full,fm_frac)
//
//
// We can go ahead and permanently map kernel launches to our 3
// streams.  As an optimization, we'll dynamically assign each kernel
// to the lowest available stream.  This transforms the problem into
// one that considers streams happening before streams -- which
// kernels are involved doesn't matter.
//
//      STREAM0   STREAM1   STREAM2
//      -------   -------   -------
//
//      pad_vin             pad_vout     (pad_vin)  happens_before (bs_full,bs_frac)
//                                       (pad_vout) happens_before (fm_full,fm_frac)
//
//      bs_full   bs_frac                (bs_full)  happens_before (fm_full,fm_frac)
//                                       (bs_frac)  happens_before (fm_full,fm_frac)
//
//      fm_full   fm_frac                (fm_full)  happens_before (hm or bc)
//                                       (fm_frac)  happens_before (hm or bc)
//
//      hm                               (hm)       happens_before (hm or bc)
//
//      bc                               (bc)       happens_before (fm_full,fm_frac)
//
// A single final kernel will always complete on stream 0.
//
// This simplifies reasoning about concurrency that's downstream of
// hs_cuda_sort().
//

typedef void (*hs_kernel_offset_bs_pfn)(HS_KEY_TYPE       * const HS_RESTRICT vout,
                                        HS_KEY_TYPE const * const HS_RESTRICT vin,
                                        uint32_t            const slab_offset);

static hs_kernel_offset_bs_pfn const hs_kernels_offset_bs[]
{
#if HS_BS_SLABS_LOG2_RU >= 1
  hs_kernel_bs_0,
#endif
#if HS_BS_SLABS_LOG2_RU >= 2
  hs_kernel_bs_1,
#endif
#if HS_BS_SLABS_LOG2_RU >= 3
  hs_kernel_bs_2,
#endif
#if HS_BS_SLABS_LOG2_RU >= 4
  hs_kernel_bs_3,
#endif
#if HS_BS_SLABS_LOG2_RU >= 5
  hs_kernel_bs_4,
#endif
#if HS_BS_SLABS_LOG2_RU >= 6
  hs_kernel_bs_5,
#endif
#if HS_BS_SLABS_LOG2_RU >= 7
  hs_kernel_bs_6,
#endif
#if HS_BS_SLABS_LOG2_RU >= 8
  hs_kernel_bs_7,
#endif
};

//
//
//

typedef void (*hs_kernel_bc_pfn)(HS_KEY_TYPE * const HS_RESTRICT vout);

static hs_kernel_bc_pfn const hs_kernels_bc[]
{
  hs_kernel_bc_0,
#if HS_BC_SLABS_LOG2_MAX >= 1
  hs_kernel_bc_1,
#endif
#if HS_BC_SLABS_LOG2_MAX >= 2
  hs_kernel_bc_2,
#endif
#if HS_BC_SLABS_LOG2_MAX >= 3
  hs_kernel_bc_3,
#endif
#if HS_BC_SLABS_LOG2_MAX >= 4
  hs_kernel_bc_4,
#endif
#if HS_BC_SLABS_LOG2_MAX >= 5
  hs_kernel_bc_5,
#endif
#if HS_BC_SLABS_LOG2_MAX >= 6
  hs_kernel_bc_6,
#endif
#if HS_BC_SLABS_LOG2_MAX >= 7
  hs_kernel_bc_7,
#endif
#if HS_BC_SLABS_LOG2_MAX >= 8
  hs_kernel_bc_8,
#endif
};

//
//
//

typedef void (*hs_kernel_hm_pfn)(HS_KEY_TYPE * const HS_RESTRICT vout);

static hs_kernel_hm_pfn const hs_kernels_hm[]
{
#if (HS_HM_SCALE_MIN == 0)
  hs_kernel_hm_0,
#endif
#if (HS_HM_SCALE_MIN <= 1) && (1 <= HS_HM_SCALE_MAX)
  hs_kernel_hm_1,
#endif
#if (HS_HM_SCALE_MIN <= 2) && (2 <= HS_HM_SCALE_MAX)
  hs_kernel_hm_2,
#endif
};

//
//
//

typedef void (*hs_kernel_fm_pfn)(HS_KEY_TYPE * const HS_RESTRICT vout);

static hs_kernel_fm_pfn const hs_kernels_fm[]
{
#if (HS_FM_SCALE_MIN == 0)
#if (HS_BS_SLABS_LOG2_RU == 1)
  hs_kernel_fm_0_0,
#endif
#if (HS_BS_SLABS_LOG2_RU == 2)
  hs_kernel_fm_0_1,
#endif
#if (HS_BS_SLABS_LOG2_RU == 3)
  hs_kernel_fm_0_2,
#endif
#if (HS_BS_SLABS_LOG2_RU == 4)
  hs_kernel_fm_0_3,
#endif
#if (HS_BS_SLABS_LOG2_RU == 5)
  hs_kernel_fm_0_4,
#endif
#if (HS_BS_SLABS_LOG2_RU == 6)
  hs_kernel_fm_0_5,
#endif
#if (HS_BS_SLABS_LOG2_RU == 7)
  hs_kernel_fm_0_6,
#endif
#endif

#if (HS_FM_SCALE_MIN <= 1) && (1 <= HS_FM_SCALE_MAX)
  CONCAT_MACRO(hs_kernel_fm_1_,HS_BS_SLABS_LOG2_RU)
#endif

#if (HS_FM_SCALE_MIN <= 2) && (2 <= HS_FM_SCALE_MAX)
#if (HS_BS_SLABS_LOG2_RU == 1)
  hs_kernel_fm_2_2,
#endif
#if (HS_BS_SLABS_LOG2_RU == 2)
  hs_kernel_fm_2_3,
#endif
#if (HS_BS_SLABS_LOG2_RU == 3)
  hs_kernel_fm_2_4,
#endif
#if (HS_BS_SLABS_LOG2_RU == 4)
  hs_kernel_fm_2_5,
#endif
#if (HS_BS_SLABS_LOG2_RU == 5)
  hs_kernel_fm_2_6,
#endif
#if (HS_BS_SLABS_LOG2_RU == 6)
  hs_kernel_fm_2_7,
#endif
#if (HS_BS_SLABS_LOG2_RU == 7)
  hs_kernel_fm_2_8,
#endif

#endif
};

//
//
//

typedef void (*hs_kernel_offset_fm_pfn)(HS_KEY_TYPE * const HS_RESTRICT vout,
                                        uint32_t const span_offset);

#if (HS_FM_SCALE_MIN == 0)
static hs_kernel_offset_fm_pfn const hs_kernels_offset_fm_0[]
{
#if (HS_BS_SLABS_LOG2_RU >= 2)
  hs_kernel_fm_0_0,
#endif
#if (HS_BS_SLABS_LOG2_RU >= 3)
  hs_kernel_fm_0_1,
#endif
#if (HS_BS_SLABS_LOG2_RU >= 4)
  hs_kernel_fm_0_2,
#endif
#if (HS_BS_SLABS_LOG2_RU >= 5)
  hs_kernel_fm_0_3,
#endif
#if (HS_BS_SLABS_LOG2_RU >= 6)
  hs_kernel_fm_0_4,
#endif
#if (HS_BS_SLABS_LOG2_RU >= 7)
  hs_kernel_fm_0_5,
#endif
};
#endif

#if (HS_FM_SCALE_MIN <= 1) && (1 <= HS_FM_SCALE_MAX)
static hs_kernel_offset_fm_pfn const hs_kernels_offset_fm_1[]
{
#if (HS_BS_SLABS_LOG2_RU >= 1)
  hs_kernel_fm_1_0,
#endif
#if (HS_BS_SLABS_LOG2_RU >= 2)
  hs_kernel_fm_1_1,
#endif
#if (HS_BS_SLABS_LOG2_RU >= 3)
  hs_kernel_fm_1_2,
#endif
#if (HS_BS_SLABS_LOG2_RU >= 4)
  hs_kernel_fm_1_3,
#endif
#if (HS_BS_SLABS_LOG2_RU >= 5)
  hs_kernel_fm_1_4,
#endif
#if (HS_BS_SLABS_LOG2_RU >= 6)
  hs_kernel_fm_1_5,
#endif
#if (HS_BS_SLABS_LOG2_RU >= 7)
  hs_kernel_fm_1_6,
#endif
};
#endif

#if (HS_FM_SCALE_MIN <= 2) && (2 <= HS_FM_SCALE_MAX)
static hs_kernel_offset_fm_pfn const hs_kernels_offset_fm_2[]
{
  hs_kernel_fm_2_0,
#if (HS_BS_SLABS_LOG2_RU >= 1)
  hs_kernel_fm_2_1,
#endif
#if (HS_BS_SLABS_LOG2_RU >= 2)
  hs_kernel_fm_2_2,
#endif
#if (HS_BS_SLABS_LOG2_RU >= 3)
  hs_kernel_fm_2_3,
#endif
#if (HS_BS_SLABS_LOG2_RU >= 4)
  hs_kernel_fm_2_4,
#endif
#if (HS_BS_SLABS_LOG2_RU >= 5)
  hs_kernel_fm_2_5,
#endif
#if (HS_BS_SLABS_LOG2_RU >= 6)
  hs_kernel_fm_2_6,
#endif
#if (HS_BS_SLABS_LOG2_RU >= 7)
  hs_kernel_fm_2_7,
#endif
};
#endif

static hs_kernel_offset_fm_pfn const * const hs_kernels_offset_fm[]
{
#if (HS_FM_SCALE_MIN == 0)
  hs_kernels_offset_fm_0,
#endif
#if (HS_FM_SCALE_MIN <= 1) && (1 <= HS_FM_SCALE_MAX)
  hs_kernels_offset_fm_1,
#endif
#if (HS_FM_SCALE_MIN <= 2) && (2 <= HS_FM_SCALE_MAX)
  hs_kernels_offset_fm_2,
#endif
};

//
//
//

typedef uint32_t hs_indices_t;

//
//
//

struct hs_state
{
  // key buffers
  HS_KEY_TYPE *  vin;
  HS_KEY_TYPE *  vout; // can be vin

  cudaStream_t   streams[3];

  // pool of stream indices
  hs_indices_t   pool;

  // bx_ru is number of rounded up warps in vin
  uint32_t       bx_ru;
};

//
//
//

static
uint32_t
hs_indices_acquire(hs_indices_t * const indices)
{
  //
  // FIXME -- an FFS intrinsic might be faster but there are so few
  // bits in this implementation that it might not matter.
  //
  if      (*indices & 1)
    {
      *indices = *indices & ~1;
      return 0;
    }
  else if (*indices & 2)
    {
      *indices = *indices & ~2;
      return 1;
    }
  else // if (*indices & 4)
    {
      *indices = *indices & ~4;
      return 2;
    }
}


static
uint32_t
hs_state_acquire(struct hs_state * const state,
                 hs_indices_t    * const indices)
{
  //
  // FIXME -- an FFS intrinsic might be faster but there are so few
  // bits in this implementation that it might not matter.
  //
  if      (state->pool & 1)
    {
      state->pool &= ~1;
      *indices    |=  1;
      return 0;
    }
  else if (state->pool & 2)
    {
      state->pool &= ~2;
      *indices    |=  2;
      return 1;
    }
  else // (state->pool & 4)
    {
      state->pool &= ~4;
      *indices    |=  4;
      return 2;
    }
}

static
void
hs_indices_merge(hs_indices_t * const to, hs_indices_t const from)
{
  *to |= from;
}

static
void
hs_barrier_enqueue(cudaStream_t to, cudaStream_t from)
{
  cudaEvent_t event_before;

  cuda(EventCreate(&event_before));

  cuda(EventRecord(event_before,from));

  cuda(StreamWaitEvent(to,event_before,0));

  cuda(EventDestroy(event_before));
}

static
hs_indices_t
hs_barrier(struct hs_state * const state,
           hs_indices_t      const before,
           hs_indices_t    * const after,
           uint32_t          const count) // count is 1 or 2
{
  // return streams this stage depends on back into the pool
  hs_indices_merge(&state->pool,before);

  hs_indices_t indices = 0;

  // acquire 'count' stream indices for this stage
  for (uint32_t ii=0; ii<count; ii++)
    {
      hs_indices_t new_indices = 0;

      // new index
      uint32_t const idx = hs_state_acquire(state,&new_indices);

      // add the new index to the indices
      indices |= new_indices;

      // only enqueue barriers when streams are different
      uint32_t const wait = before & ~new_indices;

      if (wait != 0)
        {
          cudaStream_t to = state->streams[idx];

          //
          // FIXME -- an FFS loop might be slower for so few bits. So
          // leave it as is for now.
          //
          if (wait & 1)
            hs_barrier_enqueue(to,state->streams[0]);
          if (wait & 2)
            hs_barrier_enqueue(to,state->streams[1]);
          if (wait & 4)
            hs_barrier_enqueue(to,state->streams[2]);
        }
    }

  hs_indices_merge(after,indices);

  return indices;
}

//
//
//

#ifndef NDEBUG

#include <stdio.h>
#define HS_STREAM_SYNCHRONIZE(s)                \
  cuda(StreamSynchronize(s));                   \
  fprintf(stderr,"%s\n",__func__);
#else

#define HS_STREAM_SYNCHRONIZE(s)

#endif

//
//
//

static
void
hs_transpose(struct hs_state * const state)
{
  HS_TRANSPOSE_KERNEL_NAME()
    <<<state->bx_ru,HS_SLAB_THREADS,0,state->streams[0]>>>
    (state->vout);

  HS_STREAM_SYNCHRONIZE(state->streams[0]);
}

//
//
//

static
void
hs_bc(struct hs_state * const state,
      hs_indices_t      const hs_bc,
      hs_indices_t    * const fm,
      uint32_t          const down_slabs,
      uint32_t          const clean_slabs_log2)
{
  // enqueue any necessary barriers
  hs_indices_t indices = hs_barrier(state,hs_bc,fm,1);

  // block clean the minimal number of down_slabs_log2 spans
  uint32_t const frac_ru = (1u << clean_slabs_log2) - 1;
  uint32_t const full    = (down_slabs + frac_ru) >> clean_slabs_log2;
  uint32_t const threads = HS_SLAB_THREADS << clean_slabs_log2;

  // stream will *always* be stream[0]
  cudaStream_t stream  = state->streams[hs_indices_acquire(&indices)];

  hs_kernels_bc[clean_slabs_log2]
    <<<full,threads,0,stream>>>
    (state->vout);

  HS_STREAM_SYNCHRONIZE(stream);
}

//
//
//

static
uint32_t
hs_hm(struct hs_state  * const state,
      hs_indices_t       const hs_bc,
      hs_indices_t     * const hs_bc_tmp,
      uint32_t           const down_slabs,
      uint32_t           const clean_slabs_log2)
{
  // enqueue any necessary barriers
  hs_indices_t   indices    = hs_barrier(state,hs_bc,hs_bc_tmp,1);

  // how many scaled half-merge spans are there?
  uint32_t const frac_ru    = (1 << clean_slabs_log2) - 1;
  uint32_t const spans      = (down_slabs + frac_ru) >> clean_slabs_log2;

  // for now, just clamp to the max
  uint32_t const log2_rem   = clean_slabs_log2 - HS_BC_SLABS_LOG2_MAX;
  uint32_t const scale_log2 = MIN_MACRO(HS_HM_SCALE_MAX,log2_rem);
  uint32_t const log2_out   = log2_rem - scale_log2;

  //
  // Size the grid
  //
  // The simplifying choices below limit the maximum keys that can be
  // sorted with this grid scheme to around ~2B.
  //
  //   .x : slab height << clean_log2  -- this is the slab span
  //   .y : [1...65535]                -- this is the slab index
  //   .z : ( this could also be used to further expand .y )
  //
  // Note that OpenCL declares a grid in terms of global threads and
  // not grids and blocks
  //
  dim3 grid;

  grid.x = (HS_SLAB_HEIGHT / HS_HM_BLOCK_HEIGHT) << log2_out;
  grid.y = spans;
  grid.z = 1;

  cudaStream_t stream = state->streams[hs_indices_acquire(&indices)];

  hs_kernels_hm[scale_log2-HS_HM_SCALE_MIN]
    <<<grid,HS_SLAB_THREADS * HS_HM_BLOCK_HEIGHT,0,stream>>>
    (state->vout);

  HS_STREAM_SYNCHRONIZE(stream);

  return log2_out;
}

//
// FIXME -- some of this logic can be skipped if BS is a power-of-two
//

static
uint32_t
hs_fm(struct hs_state * const state,
      hs_indices_t      const fm,
      hs_indices_t    * const hs_bc,
      uint32_t        * const down_slabs,
      uint32_t          const up_scale_log2)
{
  //
  // FIXME OPTIMIZATION: in previous HotSort launchers it's sometimes
  // a performance win to bias toward launching the smaller flip merge
  // kernel in order to get more warps in flight (increased
  // occupancy).  This is useful when merging small numbers of slabs.
  //
  // Note that HS_FM_SCALE_MIN will always be 0 or 1.
  //
  // So, for now, just clamp to the max until there is a reason to
  // restore the fancier and probably low-impact approach.
  //
  uint32_t const scale_log2 = MIN_MACRO(HS_FM_SCALE_MAX,up_scale_log2);
  uint32_t const clean_log2 = up_scale_log2 - scale_log2;

  // number of slabs in a full-sized scaled flip-merge span
  uint32_t const full_span_slabs = HS_BS_SLABS << up_scale_log2;

  // how many full-sized scaled flip-merge spans are there?
  uint32_t full_fm = state->bx_ru / full_span_slabs;
  uint32_t frac_fm = 0;

  // initialize down_slabs
  *down_slabs = full_fm * full_span_slabs;

  // how many half-size scaled + fractional scaled spans are there?
  uint32_t const span_rem        = state->bx_ru - *down_slabs;
  uint32_t const half_span_slabs = full_span_slabs >> 1;

  // if we have over a half-span then fractionally merge it
  if (span_rem > half_span_slabs)
    {
      // the remaining slabs will be cleaned
      *down_slabs += span_rem;

      uint32_t const frac_rem      = span_rem - half_span_slabs;
      uint32_t const frac_rem_pow2 = pow2_ru_u32(frac_rem);

      if (frac_rem_pow2 >= half_span_slabs)
        {
          // bump it up to a full span
          full_fm += 1;
        }
      else
        {
          // otherwise, add fractional
          frac_fm  = MAX_MACRO(1,frac_rem_pow2 >> clean_log2);
        }
    }

  // enqueue any necessary barriers
  bool const   both    = (full_fm != 0) && (frac_fm != 0);
  hs_indices_t indices = hs_barrier(state,fm,hs_bc,both ? 2 : 1);

  //
  // Size the grid
  //
  // The simplifying choices below limit the maximum keys that can be
  // sorted with this grid scheme to around ~2B.
  //
  //   .x : slab height << clean_log2  -- this is the slab span
  //   .y : [1...65535]                -- this is the slab index
  //   .z : ( this could also be used to further expand .y )
  //
  // Note that OpenCL declares a grid in terms of global threads and
  // not grids and blocks
  //
  dim3 grid;

  grid.x = (HS_SLAB_HEIGHT / HS_FM_BLOCK_HEIGHT) << clean_log2;
  grid.z = 1;

  if (full_fm > 0)
    {
      cudaStream_t stream = state->streams[hs_indices_acquire(&indices)];

      grid.y = full_fm;

      hs_kernels_fm[scale_log2-HS_FM_SCALE_MIN]
        <<<grid,HS_SLAB_THREADS * HS_FM_BLOCK_HEIGHT,0,stream>>>
          (state->vout);

      HS_STREAM_SYNCHRONIZE(stream);
    }

  if (frac_fm > 0)
    {
      cudaStream_t stream = state->streams[hs_indices_acquire(&indices)];

      grid.y = 1;

      hs_kernels_offset_fm[scale_log2-HS_FM_SCALE_MIN][msb_idx_u32(frac_fm)]
        <<<grid,HS_SLAB_THREADS * HS_FM_BLOCK_HEIGHT,0,stream>>>
        (state->vout,full_fm);

      HS_STREAM_SYNCHRONIZE(stream);
    }

  return clean_log2;
}

//
//
//

static
void
hs_bs(struct hs_state * const state,
      hs_indices_t      const bs,
      hs_indices_t    * const fm,
      uint32_t          const count_padded_in)
{
  uint32_t const slabs_in = count_padded_in / HS_SLAB_KEYS;
  uint32_t const full_bs  = slabs_in / HS_BS_SLABS;
  uint32_t const frac_bs  = slabs_in - full_bs * HS_BS_SLABS;
  bool     const both     = (full_bs != 0) && (frac_bs != 0);

  // enqueue any necessary barriers
  hs_indices_t   indices  = hs_barrier(state,bs,fm,both ? 2 : 1);

  if (full_bs != 0)
    {
      cudaStream_t stream = state->streams[hs_indices_acquire(&indices)];

      CONCAT_MACRO(hs_kernel_bs_,HS_BS_SLABS_LOG2_RU)
        <<<full_bs,HS_BS_SLABS*HS_SLAB_THREADS,0,stream>>>
        (state->vout,state->vin);

      HS_STREAM_SYNCHRONIZE(stream);
    }

  if (frac_bs != 0)
    {
      cudaStream_t stream = state->streams[hs_indices_acquire(&indices)];

      hs_kernels_offset_bs[msb_idx_u32(frac_bs)]
        <<<1,frac_bs*HS_SLAB_THREADS,0,stream>>>
        (state->vout,state->vin,full_bs*HS_BS_SLABS*HS_SLAB_THREADS);

      HS_STREAM_SYNCHRONIZE(stream);
    }
}

//
//
//

static
void
hs_keyset_pre_merge(struct hs_state * const state,
                    hs_indices_t    * const fm,
                    uint32_t          const count_lo,
                    uint32_t          const count_hi)
{
  uint32_t const vout_span = count_hi - count_lo;
  cudaStream_t   stream    = state->streams[hs_state_acquire(state,fm)];

  cuda(MemsetAsync(state->vout + count_lo,
                   0xFF,
                   vout_span * sizeof(HS_KEY_TYPE),
                   stream));
}

//
//
//

static
void
hs_keyset_pre_sort(struct hs_state * const state,
                   hs_indices_t    * const bs,
                   uint32_t          const count,
                   uint32_t          const count_hi)
{
  uint32_t const vin_span = count_hi - count;
  cudaStream_t   stream   = state->streams[hs_state_acquire(state,bs)];

  cuda(MemsetAsync(state->vin + count,
                   0xFF,
                   vin_span * sizeof(HS_KEY_TYPE),
                   stream));
}

//
//
//

void
CONCAT_MACRO(hs_cuda_sort_,HS_KEY_TYPE_PRETTY)
  (HS_KEY_TYPE * const vin,
   HS_KEY_TYPE * const vout,
   uint32_t      const count,
   uint32_t      const count_padded_in,
   uint32_t      const count_padded_out,
   bool          const linearize,
   cudaStream_t        stream0,  // primary stream
   cudaStream_t        stream1,  // auxilary
   cudaStream_t        stream2)  // auxilary
{
  // is this sort in place?
  bool const is_in_place = (vout == NULL);

  // cq, buffers, wait list and slab count
  struct hs_state state;

  state.vin        = vin;
  state.vout       = is_in_place ? vin : vout;
  state.streams[0] = stream0;
  state.streams[1] = stream1;
  state.streams[2] = stream2;
  state.pool       = 0x7; // 3 bits
  state.bx_ru      = (count + HS_SLAB_KEYS - 1) / HS_SLAB_KEYS;

  // initialize vin
  uint32_t const count_hi                 = is_in_place ? count_padded_out : count_padded_in;
  bool     const is_pre_sort_keyset_reqd  = count_hi > count;
  bool     const is_pre_merge_keyset_reqd = !is_in_place && (count_padded_out > count_padded_in);

  hs_indices_t bs = 0;

  // initialize any trailing keys in vin before sorting
  if (is_pre_sort_keyset_reqd)
    hs_keyset_pre_sort(&state,&bs,count,count_hi);

  hs_indices_t fm = 0;

  // concurrently initialize any trailing keys in vout before merging
  if (is_pre_merge_keyset_reqd)
    hs_keyset_pre_merge(&state,&fm,count_padded_in,count_padded_out);

  // immediately sort blocks of slabs
  hs_bs(&state,bs,&fm,count_padded_in);

  //
  // we're done if this was a single bs block...
  //
  // otherwise, merge sorted spans of slabs until done
  //
  if (state.bx_ru > HS_BS_SLABS)
    {
      int32_t up_scale_log2 = 1;

      while (true)
        {
          hs_indices_t hs_or_bc = 0;

          uint32_t down_slabs;

          // flip merge slabs -- return span of slabs that must be cleaned
          uint32_t clean_slabs_log2 = hs_fm(&state,
                                            fm,
                                            &hs_or_bc,
                                            &down_slabs,
                                            up_scale_log2);

          // if span is gt largest slab block cleaner then half merge
          while (clean_slabs_log2 > HS_BC_SLABS_LOG2_MAX)
            {
              hs_indices_t hs_or_bc_tmp;

              clean_slabs_log2 = hs_hm(&state,
                                       hs_or_bc,
                                       &hs_or_bc_tmp,
                                       down_slabs,
                                       clean_slabs_log2);
              hs_or_bc = hs_or_bc_tmp;
            }

          // reset fm
          fm = 0;

          // launch clean slab grid -- is it the final launch?
          hs_bc(&state,
                hs_or_bc,
                &fm,
                down_slabs,
                clean_slabs_log2);

          // was this the final block clean?
          if (((uint32_t)HS_BS_SLABS << up_scale_log2) >= state.bx_ru)
            break;

          // otherwise, merge twice as many slabs
          up_scale_log2 += 1;
        }
    }

  // slabs or linear?
  if (linearize) {
    // guaranteed to be on stream0
    hs_transpose(&state);
  }
}

//
// all grids will be computed as a function of the minimum number of slabs
//

void
CONCAT_MACRO(hs_cuda_pad_,HS_KEY_TYPE_PRETTY)
  (uint32_t   const count,
   uint32_t * const count_padded_in,
   uint32_t * const count_padded_out)
{
  //
  // round up the count to slabs
  //
  uint32_t const slabs_ru        = (count + HS_SLAB_KEYS - 1) / HS_SLAB_KEYS;
  uint32_t const blocks          = slabs_ru / HS_BS_SLABS;
  uint32_t const block_slabs     = blocks * HS_BS_SLABS;
  uint32_t const slabs_ru_rem    = slabs_ru - block_slabs;
  uint32_t const slabs_ru_rem_ru = MIN_MACRO(pow2_ru_u32(slabs_ru_rem),HS_BS_SLABS);

  *count_padded_in  = (block_slabs + slabs_ru_rem_ru) * HS_SLAB_KEYS;
  *count_padded_out = *count_padded_in;

  //
  // will merging be required?
  //
  if (slabs_ru > HS_BS_SLABS)
    {
      // more than one block
      uint32_t const blocks_lo       = pow2_rd_u32(blocks);
      uint32_t const block_slabs_lo  = blocks_lo * HS_BS_SLABS;
      uint32_t const block_slabs_rem = slabs_ru - block_slabs_lo;

      if (block_slabs_rem > 0)
        {
          uint32_t const block_slabs_rem_ru     = pow2_ru_u32(block_slabs_rem);

          uint32_t const block_slabs_hi         = MAX_MACRO(block_slabs_rem_ru,
                                                            blocks_lo << (1 - HS_FM_SCALE_MIN));

          uint32_t const block_slabs_padded_out = MIN_MACRO(block_slabs_lo+block_slabs_hi,
                                                            block_slabs_lo*2); // clamp non-pow2 blocks

          *count_padded_out = block_slabs_padded_out * HS_SLAB_KEYS;
        }
    }
}

//
//
//

void
CONCAT_MACRO(hs_cuda_info_,HS_KEY_TYPE_PRETTY)
  (uint32_t * const key_words,
   uint32_t * const val_words,
   uint32_t * const slab_height,
   uint32_t * const slab_width_log2)
{
  *key_words       = HS_KEY_WORDS;
  *val_words       = HS_VAL_WORDS;
  *slab_height     = HS_SLAB_HEIGHT;
  *slab_width_log2 = HS_SLAB_WIDTH_LOG2;
}

//
//
//
