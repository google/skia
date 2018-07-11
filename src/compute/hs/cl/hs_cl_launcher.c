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

#include <stdlib.h>

//
//
//

#include "hs_cl_launcher.h"
#include "assert_cl.h"
#include "macros.h"
#include "util.h"

//
//
//

typedef uint32_t uint;
typedef uint64_t ulong;

//
//
//

#include "hs_cl.h"

//
//
//

#if 0 // #ifndef NDEBUG
#define HS_KERNEL_SOURCE
#else
#define HS_KERNEL_BINARY
#endif

//
// #define HS_KERNEL_SPIRV
//

//
//
//

#ifdef NDEBUG

#define HS_LAUNCH_TRACE(k,g,l)

#else

#include <stdio.h>

#define HS_KERNEL_NAME_MAX 20

static
void
hs_launch_trace(cl_kernel    kernel,
                size_t const global_work_size,
                size_t const local_work_size)
{
  if (kernel == NULL)
    return;

  char name[HS_KERNEL_NAME_MAX];

  cl(GetKernelInfo(kernel,CL_KERNEL_FUNCTION_NAME,HS_KERNEL_NAME_MAX,name,NULL));

  fprintf(stderr,"%-19s ( %6zu, %4zu )\n",name,global_work_size,local_work_size);
}

#define HS_LAUNCH_TRACE(k,g,l)  hs_launch_trace(k,g,l)

#endif

//
//
//

#ifdef NDEBUG

#define HS_EVENT_NEXT()      NULL
#define HS_EVENT_PROFILE(cq)

#else

#define HS_EVENTS_MAX   128

static cl_event events[HS_EVENTS_MAX];
static uint32_t events_count;

static
cl_event *
hs_event_next()
{
  if (events_count + 1 >= HS_EVENTS_MAX) // no events can be recorded?
    {
      return NULL;
    }
  else // return next event slot
    {
      return events + events_count++;
    }
}

static
void
hs_event_profile(cl_command_queue cq)
{
  cl(Finish(cq));

  cl_command_queue_properties props;

  cl(GetCommandQueueInfo(cq,
                         CL_QUEUE_PROPERTIES,
                         sizeof(props),
                         &props,
                         NULL));

  cl_ulong t_min=UINT64_MAX, t_max=0;

  for (uint32_t ee=0; ee<events_count; ee++)
    {
      cl_event event = events[ee];

      //
      // profiling
      //
      cl_ulong t_start=0, t_end=0;

      if (props & CL_QUEUE_PROFILING_ENABLE)
        {
          // start
          cl(GetEventProfilingInfo(event,
                                   CL_PROFILING_COMMAND_START,
                                   sizeof(cl_ulong),
                                   &t_start,
                                   NULL));
          // end
          cl(GetEventProfilingInfo(event,
                                   CL_PROFILING_COMMAND_END,
                                   sizeof(cl_ulong),
                                   &t_end,
                                   NULL));

          t_min = MIN_MACRO(t_min,t_start);
          t_max = MAX_MACRO(t_max,t_end);
        }

      //
      // status
      //
      cl_int          status;
      cl_command_type type;

      cl_get_event_info(event,&status,&type);

      fprintf(stdout,"%-3u, %-13s, %-28s, %20llu, %20llu, %20llu, %20llu\n",
              ee,
              cl_get_event_command_status_string(status),
              cl_get_event_command_type_string(type),
              t_start,t_end,t_end-t_start,t_max-t_min);

      // release
      cl(ReleaseEvent(event));
    }
}

#define HS_EVENT_NEXT()      hs_event_next()
#define HS_EVENT_PROFILE(cq) hs_event_profile(cq);

#endif

//
//
//

struct hs_state
{
  cl_mem       vin;
  cl_mem       vout;

  // bx.ru is number of rounded up warps in vin
  struct {
    uint32_t   ru;
  } bx;

  // these values change on each iteration
  union {
    struct {
      uint32_t full;
      uint32_t frac;
    } bs; // warps
    struct {
      uint32_t full;
      uint32_t na;
    } bc; // warps
    struct {
      uint32_t full;
      uint32_t frac;
    } fm; // rows
  };
};

//
//
//

#define HS_THREADS_PER_BLOCK  (HS_BS_WARPS           * HS_LANES_PER_WARP)
#define HS_KEYS_PER_WARP      (HS_KEYS_PER_LANE      * HS_LANES_PER_WARP)

#define HS_BS_KEYS_PER_BLOCK  (HS_KEYS_PER_WARP      * HS_BS_WARPS)
#define HS_BS_BLOCK_SIZE      (HS_BS_KEYS_PER_BLOCK  * sizeof(HS_KEY_TYPE))

#define HS_BC_KEYS_PER_BLOCK  (HS_KEYS_PER_WARP << HS_BC_WARPS_LOG2_MAX)
#define HS_BC_BLOCK_SIZE      (HS_BC_KEYS_PER_BLOCK  * sizeof(HS_KEY_TYPE))

//
//
//

#if   defined( HS_KERNEL_SOURCE )

#include "hs_cl.pre.src.inl"

#elif defined( HS_KERNEL_BINARY )

#include "hs_cl.pre.bin.inl"

#elif defined( HS_KERNEL_SPIRV )

#include "hs_cl.pre.spv.inl"

#endif

//
//
//

struct hs_transpose_kernel
{
  cl_kernel    kernel;
  char const * name;
};

#define HS_TRANSPOSE_KERNEL_DECLARE(n) { .name = #n }

static struct hs_transpose_kernel transpose_kernels[] =
  {
    HS_TRANSPOSE_KERNEL_DECLARE(hs_kernel_transpose)
  };

//
//
//

struct hs_bs_kernel
{
  cl_kernel    kernel;
  char const * name;
};

#define HS_BS_KERNEL_DECLARE(n) { .name = #n }

static struct hs_bs_kernel bs_kernels[] =
  {
#if 0 <= HS_BS_WARPS_LOG2_RU
    HS_BS_KERNEL_DECLARE(hs_kernel_bs_0),
#endif
#if 1 <= HS_BS_WARPS_LOG2_RU
    HS_BS_KERNEL_DECLARE(hs_kernel_bs_1),
#endif
#if 2 <= HS_BS_WARPS_LOG2_RU
    HS_BS_KERNEL_DECLARE(hs_kernel_bs_2),
#endif
#if 3 <= HS_BS_WARPS_LOG2_RU
    HS_BS_KERNEL_DECLARE(hs_kernel_bs_3),
#endif
#if 4 <= HS_BS_WARPS_LOG2_RU
    HS_BS_KERNEL_DECLARE(hs_kernel_bs_4),
#endif
#if 5 <= HS_BS_WARPS_LOG2_RU
    HS_BS_KERNEL_DECLARE(hs_kernel_bs_5),
#endif
#if 6 <= HS_BS_WARPS_LOG2_RU
    HS_BS_KERNEL_DECLARE(hs_kernel_bs_6),
#endif
#if 7 <= HS_BS_WARPS_LOG2_RU
    HS_BS_KERNEL_DECLARE(hs_kernel_bs_7),
#endif
  };

//
//
//

struct hs_bc_kernel
{
  cl_kernel    kernel;
  char const * name;
};

#define HS_BC_KERNEL_DECLARE(n) { .name = #n }

static struct hs_bc_kernel bc_kernels[] =
  {
#if (0 >= HS_BC_WARPS_LOG2_MIN) && (0 <= HS_BC_WARPS_LOG2_MAX)
    HS_BC_KERNEL_DECLARE(hs_kernel_bc_0),
#endif
#if (1 >= HS_BC_WARPS_LOG2_MIN) && (1 <= HS_BC_WARPS_LOG2_MAX)
    HS_BC_KERNEL_DECLARE(hs_kernel_bc_1),
#endif
#if (2 >= HS_BC_WARPS_LOG2_MIN) && (2 <= HS_BC_WARPS_LOG2_MAX)
    HS_BC_KERNEL_DECLARE(hs_kernel_bc_2),
#endif
#if (3 >= HS_BC_WARPS_LOG2_MIN) && (3 <= HS_BC_WARPS_LOG2_MAX)
    HS_BC_KERNEL_DECLARE(hs_kernel_bc_3),
#endif
#if (4 >= HS_BC_WARPS_LOG2_MIN) && (4 <= HS_BC_WARPS_LOG2_MAX)
    HS_BC_KERNEL_DECLARE(hs_kernel_bc_4),
#endif
#if (5 >= HS_BC_WARPS_LOG2_MIN) && (5 <= HS_BC_WARPS_LOG2_MAX)
    HS_BC_KERNEL_DECLARE(hs_kernel_bc_5),
#endif
#if (6 >= HS_BC_WARPS_LOG2_MIN) && (6 <= HS_BC_WARPS_LOG2_MAX)
    HS_BC_KERNEL_DECLARE(hs_kernel_bc_6),
#endif
#if (7 >= HS_BC_WARPS_LOG2_MIN) && (7 <= HS_BC_WARPS_LOG2_MAX)
    HS_BC_KERNEL_DECLARE(hs_kernel_bc_7),
#endif
  };

//
//
//

struct hs_fm_kernel
{
  cl_kernel        kernel;
  char     const * name;
  uint32_t const   log2;
};

#define HS_FM_KERNEL_DECLARE(n,l) { .name = #n, .log2 = l }

static struct hs_fm_kernel fm_kernels[] =
  {
#ifdef HS_FM_BLOCKS_LOG2_0
    HS_FM_KERNEL_DECLARE(hs_kernel_fm_0,HS_FM_BLOCKS_LOG2_0),
#endif
#ifdef HS_FM_BLOCKS_LOG2_1
    HS_FM_KERNEL_DECLARE(hs_kernel_fm_1,HS_FM_BLOCKS_LOG2_1),
#endif
#ifdef HS_FM_BLOCKS_LOG2_2
    HS_FM_KERNEL_DECLARE(hs_kernel_fm_2,HS_FM_BLOCKS_LOG2_2),
#endif
#ifdef HS_FM_BLOCKS_LOG2_3
    HS_FM_KERNEL_DECLARE(hs_kernel_fm_3,HS_FM_BLOCKS_LOG2_3),
#endif
#ifdef HS_FM_BLOCKS_LOG2_4
    HS_FM_KERNEL_DECLARE(hs_kernel_fm_4,HS_FM_BLOCKS_LOG2_4),
#endif
#ifdef HS_FM_BLOCKS_LOG2_5
    HS_FM_KERNEL_DECLARE(hs_kernel_fm_5,HS_FM_BLOCKS_LOG2_5),
#endif
#ifdef HS_FM_BLOCKS_LOG2_6
    HS_FM_KERNEL_DECLARE(hs_kernel_fm_6,HS_FM_BLOCKS_LOG2_6),
#endif
#ifdef HS_FM_BLOCKS_LOG2_7
    HS_FM_KERNEL_DECLARE(hs_kernel_fm_7,HS_FM_BLOCKS_LOG2_7),
#endif
#ifdef HS_FM_BLOCKS_LOG2_8
    HS_FM_KERNEL_DECLARE(hs_kernel_fm_8,HS_FM_BLOCKS_LOG2_8),
#endif
#ifdef HS_FM_BLOCKS_LOG2_9
    HS_FM_KERNEL_DECLARE(hs_kernel_fm_9,HS_FM_BLOCKS_LOG2_9),
#endif
#ifdef HS_FM_BLOCKS_LOG2_10
    HS_FM_KERNEL_DECLARE(hs_kernel_fm_10,HS_FM_BLOCKS_LOG2_10),
#endif
#ifdef HS_FM_BLOCKS_LOG2_11
    HS_FM_KERNEL_DECLARE(hs_kernel_fm_11,HS_FM_BLOCKS_LOG2_11),
#endif
#ifdef HS_FM_BLOCKS_LOG2_12
    HS_FM_KERNEL_DECLARE(hs_kernel_fm_12,HS_FM_BLOCKS_LOG2_12),
#endif
#ifdef HS_FM_BLOCKS_LOG2_13
    HS_FM_KERNEL_DECLARE(hs_kernel_fm_13,HS_FM_BLOCKS_LOG2_13),
#endif
#ifdef HS_FM_BLOCKS_LOG2_14
    HS_FM_KERNEL_DECLARE(hs_kernel_fm_14,HS_FM_BLOCKS_LOG2_14),
#endif
#ifdef HS_FM_BLOCKS_LOG2_15
    HS_FM_KERNEL_DECLARE(hs_kernel_fm_15,HS_FM_BLOCKS_LOG2_15),
#endif
#ifdef HS_FM_BLOCKS_LOG2_16
    HS_FM_KERNEL_DECLARE(hs_kernel_fm_16,HS_FM_BLOCKS_LOG2_16),
#endif
  };

//
//
//

struct hs_hm_kernel
{
  cl_kernel        kernel;
  char     const * name;
  uint32_t const   log2;
};

#define HS_HM_KERNEL_DECLARE(n,l) { .name = #n, .log2 = l }

static struct hs_hm_kernel hm_kernels[] =
  {
#ifdef HS_HM_BLOCKS_LOG2_0
    HS_HM_KERNEL_DECLARE(hs_kernel_hm_0,HS_HM_BLOCKS_LOG2_0),
#endif
#ifdef HS_HM_BLOCKS_LOG2_1
    HS_HM_KERNEL_DECLARE(hs_kernel_hm_1,HS_HM_BLOCKS_LOG2_1),
#endif
#ifdef HS_HM_BLOCKS_LOG2_2
    HS_HM_KERNEL_DECLARE(hs_kernel_hm_2,HS_HM_BLOCKS_LOG2_2),
#endif
#ifdef HS_HM_BLOCKS_LOG2_3
    HS_HM_KERNEL_DECLARE(hs_kernel_hm_3,HS_HM_BLOCKS_LOG2_3),
#endif
#ifdef HS_HM_BLOCKS_LOG2_4
    HS_HM_KERNEL_DECLARE(hs_kernel_hm_4,HS_HM_BLOCKS_LOG2_4),
#endif
#ifdef HS_HM_BLOCKS_LOG2_5
    HS_HM_KERNEL_DECLARE(hs_kernel_hm_5,HS_HM_BLOCKS_LOG2_5),
#endif
#ifdef HS_HM_BLOCKS_LOG2_6
    HS_HM_KERNEL_DECLARE(hs_kernel_hm_6,HS_HM_BLOCKS_LOG2_6),
#endif
#ifdef HS_HM_BLOCKS_LOG2_7
    HS_HM_KERNEL_DECLARE(hs_kernel_hm_7,HS_HM_BLOCKS_LOG2_7),
#endif
#ifdef HS_HM_BLOCKS_LOG2_8
    HS_HM_KERNEL_DECLARE(hs_kernel_hm_8,HS_HM_BLOCKS_LOG2_8),
#endif
#ifdef HS_HM_BLOCKS_LOG2_9
    HS_HM_KERNEL_DECLARE(hs_kernel_hm_9,HS_HM_BLOCKS_LOG2_9),
#endif
#ifdef HS_HM_BLOCKS_LOG2_10
    HS_HM_KERNEL_DECLARE(hs_kernel_hm_10,HS_HM_BLOCKS_LOG2_10),
#endif
#ifdef HS_HM_BLOCKS_LOG2_11
    HS_HM_KERNEL_DECLARE(hs_kernel_hm_11,HS_HM_BLOCKS_LOG2_11),
#endif
#ifdef HS_HM_BLOCKS_LOG2_12
    HS_HM_KERNEL_DECLARE(hs_kernel_hm_12,HS_HM_BLOCKS_LOG2_12),
#endif
#ifdef HS_HM_BLOCKS_LOG2_13
    HS_HM_KERNEL_DECLARE(hs_kernel_hm_13,HS_HM_BLOCKS_LOG2_13),
#endif
#ifdef HS_HM_BLOCKS_LOG2_14
    HS_HM_KERNEL_DECLARE(hs_kernel_hm_14,HS_HM_BLOCKS_LOG2_14),
#endif
#ifdef HS_HM_BLOCKS_LOG2_15
    HS_HM_KERNEL_DECLARE(hs_kernel_hm_15,HS_HM_BLOCKS_LOG2_15),
#endif
#ifdef HS_HM_BLOCKS_LOG2_16
    HS_HM_KERNEL_DECLARE(hs_kernel_hm_16,HS_HM_BLOCKS_LOG2_16),
#endif
  };

//
//
//

static
void
hs_barrier(cl_command_queue cq)
{
  cl(EnqueueBarrierWithWaitList(cq,0,NULL,NULL));
}

//
//
//

static
void
hs_launch_transpose(struct hs_state const * const state,
                    cl_command_queue              cq,
                    cl_kernel                     kernel,
                    size_t                  const global_work_size,
                    size_t                  const local_work_size)
{
  HS_LAUNCH_TRACE(kernel,global_work_size,local_work_size);

  cl(SetKernelArg(kernel,0,sizeof(state->vout),&state->vout));

  cl(EnqueueNDRangeKernel(cq,
                          kernel,
                          1,
                          NULL,
                          &global_work_size,
                          &local_work_size,
                          0,
                          NULL,
                          HS_EVENT_NEXT()));
}

//
//
//

static
void
hs_launch_bs(struct hs_state const * const state,
             cl_command_queue              cq,
             cl_kernel                     kernel_full,
             cl_kernel                     kernel_frac,
             size_t                  const global_work_size_full,
             size_t                  const local_work_size_full,
             size_t                  const local_work_size_frac)

{
  HS_LAUNCH_TRACE(kernel_full,global_work_size_full,local_work_size_full);
  HS_LAUNCH_TRACE(kernel_frac,local_work_size_frac,local_work_size_frac);

  if (kernel_full != NULL)
    {
      cl(SetKernelArg(kernel_full,0,sizeof(state->vin), &state->vin));
      cl(SetKernelArg(kernel_full,1,sizeof(state->vout),&state->vout));

      cl(EnqueueNDRangeKernel(cq,
                              kernel_full,
                              1,
                              NULL,
                              &global_work_size_full,
                              &local_work_size_full,
                              0,
                              NULL,
                              HS_EVENT_NEXT()));
    }

  if (kernel_frac != NULL)
    {
      cl(SetKernelArg(kernel_frac,0,sizeof(state->vin), &state->vin));
      cl(SetKernelArg(kernel_frac,1,sizeof(state->vout),&state->vout));

      cl(EnqueueNDRangeKernel(cq,
                              kernel_frac,
                              1,
                              &global_work_size_full,
                              &local_work_size_frac,
                              &local_work_size_frac,
                              0,
                              NULL,
                              HS_EVENT_NEXT()));
    }
}

//
//
//

static
void
hs_launch_bc(struct hs_state const * const state,
             cl_command_queue              cq,
             cl_kernel                     kernel,
             size_t                  const global_work_size,
             size_t                  const local_work_size)
{
  HS_LAUNCH_TRACE(kernel,global_work_size,local_work_size);

  cl(SetKernelArg(kernel,0,sizeof(state->vout),&state->vout));

  cl(EnqueueNDRangeKernel(cq,
                          kernel,
                          1,
                          NULL,
                          &global_work_size,
                          &local_work_size,
                          0,
                          NULL,
                          HS_EVENT_NEXT()));
}

//
//
//

static
void
hs_launch_fm(struct hs_state const * const state,
             cl_command_queue              cq,
             cl_kernel                     kernel,
             size_t                  const global_work_size)
{
  HS_LAUNCH_TRACE(kernel,global_work_size,0);

  cl(SetKernelArg(kernel,0,sizeof(state->vout),   &state->vout));
  cl(SetKernelArg(kernel,1,sizeof(state->fm.full),&state->fm.full));
  cl(SetKernelArg(kernel,2,sizeof(state->fm.frac),&state->fm.frac));

  cl(EnqueueNDRangeKernel(cq,
                          kernel,
                          1,
                          NULL,
                          &global_work_size,
                          NULL,
                          0,
                          NULL,
                          HS_EVENT_NEXT()));
}

//
//
//

static
void
hs_launch_hm(struct hs_state const * const state,
             cl_command_queue              cq,
             cl_kernel                     kernel,
             size_t                  const global_work_size)
{
  HS_LAUNCH_TRACE(kernel,global_work_size,0);

  cl(SetKernelArg(kernel,0,sizeof(state->vout),&state->vout));

  cl(EnqueueNDRangeKernel(cq,
                          kernel,
                          1,
                          NULL,
                          &global_work_size,
                          NULL,
                          0,
                          NULL,
                          HS_EVENT_NEXT()));
}

//
//
//

static
void
hs_transpose_launcher(struct hs_state * const state,
                      cl_command_queue        cq)
{
  // transpose each slab
  size_t const global_work_size = state->bx.ru * HS_LANES_PER_WARP;
  size_t const local_work_size  = HS_LANES_PER_WARP; // FIXME -- might not always want to specify this

  hs_launch_transpose(state,
                      cq,
                      transpose_kernels[0].kernel,
                      global_work_size,
                      local_work_size);
}

//
//
//

static
void
hs_bs_launcher(struct hs_state * const state,
               uint32_t          const warps_in,
               cl_command_queue        cq)
{
  // warps_in is already rounded up
  uint32_t const full = (warps_in / HS_BS_WARPS) * HS_BS_WARPS;
  uint32_t const frac = warps_in - full;

  //
  // FIXME -- launch on different queues
  //
  cl_kernel kernel_full = (full == 0) ? NULL : bs_kernels[HS_BS_WARPS_LOG2_RU].kernel;
  cl_kernel kernel_frac = (frac == 0) ? NULL : bs_kernels[msb_idx_u32(frac)].kernel;

  hs_launch_bs(state,
               cq,
               kernel_full,
               kernel_frac,
               full        * HS_LANES_PER_WARP,
               HS_BS_WARPS * HS_LANES_PER_WARP,
               frac        * HS_LANES_PER_WARP);
}

//
//
//

static
void
hs_bc_launcher(struct hs_state * const state,
               uint32_t          const down_warps,
               uint32_t          const down_warps_log2,
               cl_command_queue        cq)
{
  // block clean the minimal number of down_warps_log2 spans
  uint32_t const frac_ru          = (1u << down_warps_log2) - 1;
  state->bc.full                  = (down_warps + frac_ru) & ~frac_ru;

  // launch block slab sorting grid
  size_t   const global_work_size = state->bc.full * HS_LANES_PER_WARP;
  size_t   const local_work_size  = HS_LANES_PER_WARP << down_warps_log2;

  //
  // we better be capable of cleaning at least two warps !!!
  //
  hs_launch_bc(state,
               cq,
               bc_kernels[down_warps_log2].kernel,
               global_work_size,
               local_work_size);
}

//
//
//

static
uint32_t
hs_hm_launcher(struct hs_state * const state,
               uint32_t          const down_warps,
               uint32_t          const down_warps_log2_in,
               cl_command_queue        cq)
{
  // how many scaled half-merge spans are there?
  uint32_t const frac_ru  = (1 << down_warps_log2_in) - 1;
  uint32_t const spans_ru = (down_warps + frac_ru) >> down_warps_log2_in;

  // get the kernel record
  struct hs_hm_kernel const * const hm = hm_kernels + down_warps_log2_in - HS_BC_WARPS_LOG2_MAX - 1;

  // how large is the grid?
  size_t const global_work_size = HS_LANES_PER_WARP * HS_KEYS_PER_LANE * (spans_ru << hm->log2);
  size_t const local_work_size  = HS_LANES_PER_WARP;

  // launch the hm kernel
  hs_launch_hm(state,
               cq,
               hm->kernel,
               global_work_size);

  return hm->log2;
}

//
// FIXME -- some of this logic can be skipped if BS is a power-of-two
//

static
uint32_t
hs_fm_launcher(struct hs_state * const state,
               uint32_t          const up_scale_log2,
               uint32_t        * const down_warps,
               cl_command_queue        cq)
{
  // get the kernel record
  struct hs_fm_kernel const * const fm = fm_kernels + up_scale_log2 - 1;

  // number of warps in a full-sized scaled flip-merge span
  uint32_t const full_span_warps = HS_BS_WARPS << up_scale_log2;

  // how many full-sized scaled flip-merge spans are there?
  state->fm.full = state->bx.ru / full_span_warps;
  state->fm.frac = 0;

  // initialize down_warps
  *down_warps    = state->fm.full * full_span_warps;

  // how many half-size scaled + fractional scaled spans are there?
  uint32_t const span_rem        = state->bx.ru - state->fm.full * full_span_warps;
  uint32_t const half_span_warps = full_span_warps >> 1;

  if (span_rem > half_span_warps)
    {
      uint32_t const frac_rem      = span_rem - half_span_warps;
      uint32_t const frac_rem_pow2 = pow2_ru_u32(frac_rem);

      if (frac_rem_pow2 >= half_span_warps)
        {
          *down_warps    += full_span_warps;
          state->fm.full += 1;
        }
      else
        {
          uint32_t const frac_interleaved = frac_rem_pow2 >> fm->log2;

          *down_warps    += half_span_warps + frac_rem_pow2;
          state->fm.frac  = MAX_MACRO(1,frac_interleaved);
        }
    }

  // size the grid
  uint32_t const spans_frac       = MIN_MACRO(state->fm.frac,1);
  uint32_t const spans_total      = state->fm.full + spans_frac;
  uint32_t const scale            = spans_total << fm->log2;
  size_t   const global_work_size = HS_LANES_PER_WARP * HS_KEYS_PER_LANE * scale;
  size_t   const local_work_size  = HS_LANES_PER_WARP;

  //
  // launch the fm kernel
  //
  hs_launch_fm(state,
               cq,
               fm->kernel,
               global_work_size);

  return fm->log2;
}

//
//
//

static
void
hs_keyset_launcher(cl_mem           mem,
                   uint32_t   const offset,
                   uint32_t   const span,
                   cl_command_queue cq)
{


  //
  // DOES NOT TEST FOR SPAN = 0
  //
  HS_KEY_TYPE const pattern = (HS_KEY_TYPE)-1L;

  cl(EnqueueFillBuffer(cq,
                       mem,
                       &pattern,
                       sizeof(HS_KEY_TYPE),
                       offset * sizeof(HS_KEY_TYPE),
                       span   * sizeof(HS_KEY_TYPE),
                       0,
                       NULL,
                       HS_EVENT_NEXT()));
}

//
// all grids will be computed as a function of the minimum number of warps
//

void
hs_pad(uint32_t   const count,
       uint32_t * const count_padded_in,
       uint32_t * const count_padded_out)
{
  //
  // round up the count to warps
  //
  uint32_t const warps_ru     = (count + HS_KEYS_PER_WARP - 1) / HS_KEYS_PER_WARP;
  uint32_t const blocks       = warps_ru / HS_BS_WARPS;
  uint32_t const warps_mod    = warps_ru % HS_BS_WARPS;
  uint32_t const warps_mod_ru = MIN_MACRO(pow2_ru_u32(warps_mod),HS_BS_WARPS);

  *count_padded_in  = (blocks * HS_BS_WARPS + warps_mod_ru) * HS_KEYS_PER_WARP;
  *count_padded_out = *count_padded_in;

  //
  // more than a single block sort?
  //
  if (warps_ru > HS_BS_WARPS)
    {
      // more than one block
      uint32_t const blocks_lo = pow2_rd_u32(blocks);
      uint32_t const warps_lo  = blocks_lo * HS_BS_WARPS;
      uint32_t const warps_rem = warps_ru - warps_lo;

      if (warps_rem > 0)
        {
          uint32_t const warps_rem_ru     = pow2_ru_u32(warps_rem);
          uint32_t const warps_hi         = MAX_MACRO(warps_rem_ru,blocks_lo << HS_FM_BLOCKS_LOG2_1);
          uint32_t const warps_padded_out = MIN_MACRO(warps_lo+warps_hi,warps_lo*2); // clamp non-pow2 blocks

          *count_padded_out = warps_padded_out * HS_KEYS_PER_WARP;
        }
    }
}

//
//
//

void
hs_sort(cl_command_queue cq, // out-of-order cq
        cl_mem           vin,
        cl_mem           vout,
        uint32_t   const count,
        uint32_t   const count_padded_in,
        uint32_t   const count_padded_out,
        bool       const linearize)
{
#ifndef NDEBUG
  events_count = 0;
#endif

  //
  // FIXME -- get rid of this vestigial structure
  //
  struct hs_state state = { .vin = vin, .vout = vout };

  // how many rounded-up key slabs are there?
  state.bx.ru = (count + HS_KEYS_PER_WARP - 1) / HS_KEYS_PER_WARP;

  //
  // init padding with max-valued keys
  //
  bool const split  = state.vout != state.vin; // FIXME -- careful this comparison might not always be correct
  bool       keyset = false;

  if (!split)
    {
      uint32_t const vin_span = count_padded_out - count;

      if (vin_span > 0)
        {
          hs_keyset_launcher(state.vin,
                             count,vin_span,
                             cq);
          keyset = true;
        }
    }
  else
    {
      uint32_t const vin_span = count_padded_in - count;

      if (vin_span > 0)
        {
          hs_keyset_launcher(state.vin,
                             count,vin_span,
                             cq);
          keyset = true;
        }

      uint32_t const vout_span = count_padded_out - count_padded_in;

      if (vout_span > 0)
        {
          hs_keyset_launcher(state.vout,
                             count_padded_in,vout_span,
                             cq);
          keyset = true;
        }
    }

  if (keyset)
    {
      hs_barrier(cq);
    }

  //
  // sort blocks
  //
  uint32_t const warps_in = count_padded_in / HS_KEYS_PER_WARP;

  hs_bs_launcher(&state,warps_in,cq);

  hs_barrier(cq);

  //
  // we're done if only a single bs kernel block was required
  //
  if (state.bx.ru > HS_BS_WARPS)
    {
      //
      // otherwise... merge sorted spans of warps until done
      //
      uint32_t up_scale_log2 = 1;

      while (true)
        {
          uint32_t down_warps;

          // flip merge warps -- return span of warps that must be cleaned
          uint32_t down_warps_log2 = hs_fm_launcher(&state,
                                                    up_scale_log2,
                                                    &down_warps,
                                                    cq);

          hs_barrier(cq);

          // if span is gt largest slab block cleaner then half merge
          while (down_warps_log2 > HS_BC_WARPS_LOG2_MAX)
            {
              down_warps_log2 = hs_hm_launcher(&state,
                                               down_warps,
                                               down_warps_log2,
                                               cq);

              hs_barrier(cq);
            }

               // launch clean slab grid -- is it the final launch?
          hs_bc_launcher(&state,
                         down_warps,
                         down_warps_log2,
                         cq);

          hs_barrier(cq);

          // was this the final block clean?
          if (((uint32_t)HS_BS_WARPS << up_scale_log2) >= state.bx.ru)
            break;

          // otherwise, merge twice as many slabs
          up_scale_log2 += 1;
        }
    }

  if (linearize)
    {
      // launch linearize;
      hs_transpose_launcher(&state,cq);

      hs_barrier(cq);
    }

  HS_EVENT_PROFILE(cq);
}

//
//
//

void
hs_create(cl_context             context,
          cl_device_id           device_id,
          struct hs_info * const info)
{
  //
  // create and build the program from source or a precompiled binary
  //
  if (info != NULL)
    {
      info->words = HS_KEY_WORDS;
      info->keys  = HS_KEYS_PER_LANE;
      info->lanes = HS_LANES_PER_WARP;
    }

#if defined( HS_KERNEL_SOURCE )

  cl_int err;

  size_t const   strings_sizeof[] = { sizeof(hs_cl_pre_cl) };
  char   const * strings[]        = { (char*)hs_cl_pre_cl  };

  cl_program program = clCreateProgramWithSource(context,
                                                 1,
                                                 strings,
                                                 strings_sizeof,
                                                 &err);
  cl_ok(err);

  char const * const options =
    "-cl-std=CL2.0 -cl-fast-relaxed-math "
    "-cl-no-signed-zeros -cl-mad-enable "
    "-cl-denorms-are-zero "
    "-cl-kernel-arg-info";

  cl(BuildProgram(program,
                  1,
                  &device_id,
                  options,
                  NULL,
                  NULL));

#elif defined( HS_KERNEL_BINARY )

  cl_int status, err;

  size_t        const   bins_sizeof[] = { sizeof(hs_cl_pre_ir) };
  unsigned char const * bins[]        = { hs_cl_pre_ir };

  cl_program program = clCreateProgramWithBinary(context,
                                                 1,
                                                 &device_id,
                                                 bins_sizeof,
                                                 bins,
                                                 &status,
                                                 &err);
  cl_ok(err);

  cl(BuildProgram(program,
                  1,
                  &device_id,
                  NULL,
                  NULL,
                  NULL));
#endif

  //
  // create all the kernels and release the program
  //
#define HS_CREATE_KERNELS(ks)                                   \
  for (uint32_t ii=0; ii<ARRAY_LENGTH(ks); ii++) {              \
    ks[ii].kernel = clCreateKernel(program,ks[ii].name,&err);   \
    cl_ok(err);                                                 \
  }

  HS_CREATE_KERNELS(bs_kernels);
  HS_CREATE_KERNELS(bc_kernels);
  HS_CREATE_KERNELS(fm_kernels);
  HS_CREATE_KERNELS(hm_kernels);
  HS_CREATE_KERNELS(transpose_kernels);

  cl(ReleaseProgram(program));
}

//
//
//

void
hs_release()
{
#define HS_RELEASE_KERNELS(ks)                          \
  for (uint32_t ii=0; ii<ARRAY_LENGTH(ks); ii++)        \
    cl(ReleaseKernel(ks[ii].kernel))

  HS_RELEASE_KERNELS(bs_kernels);
  HS_RELEASE_KERNELS(bc_kernels);
  HS_RELEASE_KERNELS(fm_kernels);
  HS_RELEASE_KERNELS(hm_kernels);
  HS_RELEASE_KERNELS(transpose_kernels);
}

//
//
//
