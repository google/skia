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
#include <string.h>

//
//
//

#include "common/cl/assert_cl.h"
#include "common/macros.h"
#include "common/util.h"

//
//
//

#include "hs_cl_launcher.h"

//
//
//

struct hs_cl
{
  struct hs_cl_target_config config;

  uint32_t                   key_val_size;
  uint32_t                   slab_keys;
  uint32_t                   bs_slabs_log2_ru;
  uint32_t                   bc_slabs_log2_max;

  struct {
    uint32_t                 count;
    cl_kernel              * transpose;
    cl_kernel              * bs;
    cl_kernel              * bc;
    cl_kernel              * fm[3];
    cl_kernel              * hm[3];
    cl_kernel                all[];
  } kernels;
};

//
//
//

struct hs_state
{
#ifndef NDEBUG
  cl_ulong         t_total; // 0
#endif

  cl_command_queue cq;

  // key buffers
  cl_mem           vin;
  cl_mem           vout; // can be vin

  // enforces ordering on out-of-order queue
  cl_event         wait_list[3]; // worst case
  uint32_t         wait_list_size;

  // bx_ru is number of rounded up warps in vin
  uint32_t         bx_ru;
};

//
//
//

static
void
hs_state_wait_list_release(struct hs_state * const state)
{
  for (uint32_t ii=0; ii<state->wait_list_size; ii++)
    cl(ReleaseEvent(state->wait_list[ii]));

  state->wait_list_size = 0;
}

static
void
hs_state_wait_list_update(struct hs_state * const state,
                          uint32_t          const wait_list_size,
                          cl_event  const * const wait_list)
{
  uint32_t const new_size = state->wait_list_size + wait_list_size;

  for (uint32_t ii=state->wait_list_size; ii<new_size; ii++)
    state->wait_list[ii] = wait_list[ii];

  state->wait_list_size = new_size;
}

//
//
//

#ifdef NDEBUG

#define HS_STATE_WAIT_LIST_PROFILE(state)
#define HS_STATE_WAIT_LIST_PROFILE_EX(state,wait_list_size,wait_list)

#else

#include <stdio.h>

#define HS_STATE_WAIT_LIST_PROFILE(state)               \
  hs_state_wait_list_profile(state,                     \
                             state->wait_list_size,     \
                             state->wait_list)

#define HS_STATE_WAIT_LIST_PROFILE_EX(state,wait_list_size,wait_list)   \
  hs_state_wait_list_profile(state,                                     \
                             wait_list_size,                            \
                             wait_list)

static
void
hs_state_wait_list_profile(struct hs_state  * const state,
                           uint32_t           const wait_list_size,
                           cl_event   const * const wait_list)
{
  cl(Finish(state->cq));

  cl_command_queue_properties props;

  cl(GetCommandQueueInfo(state->cq,
                         CL_QUEUE_PROPERTIES,
                         sizeof(props),
                         &props,
                         NULL));

  for (uint32_t ii=0; ii<wait_list_size; ii++)
    {
      cl_event event = wait_list[ii];

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

          state->t_total += t_end - t_start;
        }

      //
      // status
      //
      cl_int          status;
      cl_command_type type;

      cl_get_event_info(event,&status,&type);

      fprintf(stdout,"%-13s, %-28s, %20llu, %20llu, %20llu, %20llu\n",
              cl_get_event_command_status_string(status),
              cl_get_event_command_type_string(type),
              t_start,t_end,t_end-t_start,state->t_total);
    }
}

#endif

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
hs_launch_trace(cl_kernel            kernel,
                uint32_t       const dim,
                size_t const * const global_work_size)
{
  if (kernel == NULL)
    return;

  char name[HS_KERNEL_NAME_MAX];

  cl(GetKernelInfo(kernel,CL_KERNEL_FUNCTION_NAME,HS_KERNEL_NAME_MAX,name,NULL));

  fprintf(stderr,"%-19s ( %6zu, %6zu, %6zu )\n",
          name,
          global_work_size[0],
          dim < 2 ? 0 : global_work_size[1],
          dim < 3 ? 0 : global_work_size[2]);
}

#define HS_LAUNCH_TRACE(k,d,g)  hs_launch_trace(k,d,g)

#endif

//
//
//

static
void
hs_transpose_launcher(struct hs_cl const * const hs,
                      struct hs_state    * const state)
{
  size_t const size[1] = { state->bx_ru << hs->config.slab.threads_log2 };
  cl_kernel    kernel  = hs->kernels.transpose[0];

  HS_LAUNCH_TRACE(kernel,1,size);

  //
  // The transpose kernel operates on a single slab.  For now, let's
  // rely on the driver to choose a workgroup size.
  //
  // size_t local_work_size[1] = { HS_SLAB_THREADS };
  //
  cl(SetKernelArg(kernel,0,sizeof(state->vout),&state->vout));

  cl_event wait_list_out[1];

  cl(EnqueueNDRangeKernel(state->cq,
                          kernel,
                          1,
                          NULL,
                          size,
                          NULL,
                          state->wait_list_size,
                          state->wait_list,
                          wait_list_out));

  hs_state_wait_list_release(state);
  hs_state_wait_list_update(state,1,wait_list_out);

  HS_STATE_WAIT_LIST_PROFILE(state);
}

//
//
//

static
void
hs_launch_bs(struct hs_cl const * const hs,
             struct hs_state    * const state,
             uint32_t             const full,
             uint32_t             const frac,
             uint32_t             const wait_list_size,
             cl_event           *       wait_list)
{
  uint32_t wait_list_out_size = 0;
  cl_event wait_list_out[2];

  if (full > 0)
    {
      size_t const size_full[1] = { full << hs->config.slab.threads_log2 };
      cl_kernel    kernel_full  = hs->kernels.bs[hs->bs_slabs_log2_ru];

      HS_LAUNCH_TRACE(kernel_full,1,size_full);

      cl(SetKernelArg(kernel_full,0,sizeof(state->vin), &state->vin));
      cl(SetKernelArg(kernel_full,1,sizeof(state->vout),&state->vout));

      cl(EnqueueNDRangeKernel(state->cq,
                              kernel_full,
                              1,
                              NULL,
                              size_full,
                              NULL,
                              wait_list_size,
                              wait_list,
                              wait_list_out+wait_list_out_size++));
    }

  if (frac > 0)
    {
      size_t const offset_frac[1] = { full << hs->config.slab.threads_log2 };
      size_t const size_frac  [1] = { frac << hs->config.slab.threads_log2 };
      cl_kernel    kernel_frac    = hs->kernels.bs[msb_idx_u32(frac)];

      HS_LAUNCH_TRACE(kernel_frac,1,size_frac);

      cl(SetKernelArg(kernel_frac,0,sizeof(state->vin), &state->vin));
      cl(SetKernelArg(kernel_frac,1,sizeof(state->vout),&state->vout));

      cl(EnqueueNDRangeKernel(state->cq,
                              kernel_frac,
                              1,
                              offset_frac,
                              size_frac,
                              NULL,
                              wait_list_size,
                              wait_list,
                              wait_list_out+wait_list_out_size++));
    }

  hs_state_wait_list_release(state);
  hs_state_wait_list_update(state,wait_list_out_size,wait_list_out);

  HS_STATE_WAIT_LIST_PROFILE(state);
}

//
//
//

static
void
hs_launch_bc(struct hs_cl const * const hs,
             struct hs_state    * const state,
             uint32_t             const full,
             uint32_t             const clean_slabs_log2)
{
  size_t const size[1] = { full << hs->config.slab.threads_log2 };
  cl_kernel    kernel  = hs->kernels.bc[clean_slabs_log2];

  HS_LAUNCH_TRACE(kernel,1,size);

  cl(SetKernelArg(kernel,0,sizeof(state->vout),&state->vout));

  cl_event wait_list_out[1];

  cl(EnqueueNDRangeKernel(state->cq,
                          kernel,
                          1,
                          NULL,
                          size,
                          NULL,
                          state->wait_list_size,
                          state->wait_list,
                          wait_list_out));

  hs_state_wait_list_release(state);
  hs_state_wait_list_update(state,1,wait_list_out);

  HS_STATE_WAIT_LIST_PROFILE(state);
}

//
//
//

static
void
hs_launch_fm(struct hs_cl const * const hs,
             struct hs_state    * const state,
             uint32_t             const scale_log2,
             uint32_t             const fm_full,
             uint32_t             const fm_frac,
             uint32_t             const span_threads)
{
  //
  // Note that some platforms might need to use .z on large grids
  //
  uint32_t wait_list_out_size = 0;
  cl_event wait_list_out[2];

  if (fm_full > 0)
    {
      size_t const size_full[3] = { span_threads, fm_full, 1 };
      cl_kernel    kernel_full  = hs->kernels.fm[scale_log2][hs->bs_slabs_log2_ru];

      HS_LAUNCH_TRACE(kernel_full,3,size_full);

      cl(SetKernelArg(kernel_full,0,sizeof(state->vout),&state->vout));

      cl(EnqueueNDRangeKernel(state->cq,
                              kernel_full,
                              3,
                              NULL,
                              size_full,
                              NULL,
                              state->wait_list_size,
                              state->wait_list,
                              wait_list_out+wait_list_out_size++));
    }

  if (fm_frac > 0)
    {
      size_t const offset_frac[3] = { 0,            fm_full, 0 };
      size_t const size_frac  [3] = { span_threads, 1,       1 };
      cl_kernel    kernel_frac    = hs->kernels.fm[scale_log2][msb_idx_u32(fm_frac)];

      HS_LAUNCH_TRACE(kernel_frac,3,size_frac);

      cl(SetKernelArg(kernel_frac,0,sizeof(state->vout),&state->vout));

      cl(EnqueueNDRangeKernel(state->cq,
                              kernel_frac,
                              3,
                              offset_frac,
                              size_frac,
                              NULL,
                              state->wait_list_size,
                              state->wait_list,
                              wait_list_out+wait_list_out_size++));
    }

  hs_state_wait_list_release(state);
  hs_state_wait_list_update(state,wait_list_out_size,wait_list_out);

  HS_STATE_WAIT_LIST_PROFILE(state);
}

//
//
//

static
void
hs_launch_hm(struct hs_cl const * const hs,
             struct hs_state    * const state,
             uint32_t             const scale_log2,
             uint32_t             const spans,
             uint32_t             const span_threads)
{
  //
  // Note that some platforms might need to use .z on large grids
  //
  size_t const size[3] = { span_threads, spans, 1 };
  cl_kernel    kernel  = hs->kernels.hm[scale_log2][0];

  HS_LAUNCH_TRACE(kernel,3,size);

  cl(SetKernelArg(kernel,0,sizeof(state->vout),&state->vout));

  cl_event wait_list_out[1];

  cl(EnqueueNDRangeKernel(state->cq,
                          kernel,
                          3,
                          NULL,
                          size,
                          NULL,
                          state->wait_list_size,
                          state->wait_list,
                          wait_list_out));

  hs_state_wait_list_release(state);
  hs_state_wait_list_update(state,1,wait_list_out);

  HS_STATE_WAIT_LIST_PROFILE(state);
}

//
//
//

static
void
hs_keyset_pre_sort(struct hs_cl const * const hs,
                   struct hs_state    * const state,
                   uint32_t             const count,
                   uint32_t             const count_hi,
                   uint32_t             const wait_list_size,
                   cl_event           *       wait_list,
                   cl_event           *       event)
{
  uint32_t const vin_span = count_hi - count;
  uint32_t const pattern  = UINT32_MAX;

  cl(EnqueueFillBuffer(state->cq,
                       state->vin,
                       &pattern,
                       sizeof(pattern),
                       count    * hs->key_val_size,
                       vin_span * hs->key_val_size,
                       wait_list_size,
                       wait_list,
                       event));

  HS_STATE_WAIT_LIST_PROFILE_EX(state,1,event);
}

static
void
hs_keyset_pre_merge(struct hs_cl const * const hs,
                    struct hs_state    * const state,
                    uint32_t             const count_lo,
                    uint32_t             const count_hi,
                    uint32_t             const wait_list_size,
                    cl_event           *       wait_list)
{
  uint32_t const vout_span = count_hi - count_lo;
  uint32_t const pattern   = UINT32_MAX;

  // appends event to incoming wait list
  cl(EnqueueFillBuffer(state->cq,
                       state->vout,
                       &pattern,
                       sizeof(pattern),
                       count_lo  * hs->key_val_size,
                       vout_span * hs->key_val_size,
                       wait_list_size,
                       wait_list,
                       state->wait_list+state->wait_list_size++));

  HS_STATE_WAIT_LIST_PROFILE(state);
}

//
//
//

static
void
hs_bs_launcher(struct hs_cl const * const hs,
               struct hs_state    * const state,
               uint32_t             const count_padded_in,
               uint32_t             const wait_list_size,
               cl_event           *       wait_list)
{
  uint32_t const slabs_in = count_padded_in / hs->slab_keys;
  uint32_t const full     = (slabs_in / hs->config.block.slabs) * hs->config.block.slabs;
  uint32_t const frac     = slabs_in - full;

  hs_launch_bs(hs,state,
               full,frac,
               wait_list_size,wait_list);
}

//
//
//

static
void
hs_bc_launcher(struct hs_cl const * const hs,
               struct hs_state    * const state,
               uint32_t             const down_slabs,
               uint32_t             const clean_slabs_log2)
{
  // block clean the minimal number of down_slabs_log2 spans
  uint32_t const frac_ru = (1u << clean_slabs_log2) - 1;
  uint32_t const full    = (down_slabs + frac_ru) & ~frac_ru;

  // we better be capable of cleaning at least two warps !!!
  hs_launch_bc(hs,state,full,clean_slabs_log2);
}

//
// FIXME -- some of this logic can be skipped if BS is a power-of-two
//

static
uint32_t
hs_fm_launcher(struct hs_cl const * const hs,
               struct hs_state    * const state,
               uint32_t           * const down_slabs,
               uint32_t             const up_scale_log2)
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
  uint32_t const scale_log2 = MIN_MACRO(hs->config.merge.fm.scale_max,up_scale_log2);
  uint32_t const clean_log2 = up_scale_log2 - scale_log2;

  // number of slabs in a full-sized scaled flip-merge span
  uint32_t const full_span_slabs = hs->config.block.slabs << up_scale_log2;

  // how many full-sized scaled flip-merge spans are there?
  uint32_t fm_full = state->bx_ru / full_span_slabs;
  uint32_t fm_frac = 0;

  // initialize down_slabs
  *down_slabs = fm_full * full_span_slabs;

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
          fm_full += 1;
        }
      else
        {
          // otherwise, add fractional
          fm_frac  = MAX_MACRO(1,frac_rem_pow2 >> clean_log2);
        }
    }

  // size the grid
  uint32_t const span_threads = hs->slab_keys << clean_log2;

  //
  // launch the fm kernel
  //
  hs_launch_fm(hs,
               state,
               scale_log2,
               fm_full,
               fm_frac,
               span_threads);

  return clean_log2;
}

//
//
//

static
uint32_t
hs_hm_launcher(struct hs_cl const * const hs,
               struct hs_state    * const state,
               uint32_t             const down_slabs,
               uint32_t             const clean_slabs_log2)
{
  // how many scaled half-merge spans are there?
  uint32_t const frac_ru    = (1 << clean_slabs_log2) - 1;
  uint32_t const spans      = (down_slabs + frac_ru) >> clean_slabs_log2;

  // for now, just clamp to the max
  uint32_t const log2_rem   = clean_slabs_log2 - hs->bc_slabs_log2_max;
  uint32_t const scale_log2 = MIN_MACRO(hs->config.merge.hm.scale_max,log2_rem);
  uint32_t const log2_out   = log2_rem - scale_log2;

  // size the grid
  uint32_t const span_threads = hs->slab_keys << log2_out;

  // launch the hm kernel
  hs_launch_hm(hs,
               state,
               scale_log2,
               spans,
               span_threads);

  return log2_out;
}

//
//
//

void
hs_cl_sort(struct hs_cl const * const hs,
           cl_command_queue           cq,
           uint32_t             const wait_list_size,
           cl_event           *       wait_list,
           cl_event           *       event,
           cl_mem                     vin,
           cl_mem                     vout,
           uint32_t             const count,
           uint32_t             const count_padded_in,
           uint32_t             const count_padded_out,
           bool                 const linearize)
{
  // is this sort in place?
  bool const is_in_place = (vout == NULL);

  // cq, buffers, wait list and slab count
  struct hs_state state = {
#ifndef NDEBUG
    .t_total        = 0,
#endif
    .cq             = cq,
    .vin            = vin,
    .vout           = is_in_place ? vin : vout,
    .wait_list_size = 0,
    .bx_ru          = (count + hs->slab_keys - 1) / hs->slab_keys
  };

  // initialize vin
  uint32_t const count_hi                = is_in_place ? count_padded_out : count_padded_in;
  bool     const is_pre_sort_keyset_reqd = count_hi > count;
  cl_event       event_keyset_pre_sort[1];

  // initialize any trailing keys in vin before sorting
  if (is_pre_sort_keyset_reqd)
    {
      hs_keyset_pre_sort(hs,&state,
                         count,count_hi,
                         wait_list_size,wait_list,
                         event_keyset_pre_sort);
    }

  // initialize any trailing keys in vout before merging
  if (!is_in_place && (count_padded_out > count_padded_in))
    {
      hs_keyset_pre_merge(hs,&state,
                          count_padded_in,count_padded_out,
                          wait_list_size,wait_list);
    }

  //
  // sort blocks of slabs
  //
  hs_bs_launcher(hs,&state,
                 count_padded_in,
                 is_pre_sort_keyset_reqd ? 1                     : wait_list_size,
                 is_pre_sort_keyset_reqd ? event_keyset_pre_sort : wait_list);

  // release the event
  if (is_pre_sort_keyset_reqd)
    cl(ReleaseEvent(event_keyset_pre_sort[0]));

  //
  // we're done if this was a single bs block...
  //
  // otherwise, merge sorted spans of slabs until done
  //
  if (state.bx_ru > hs->config.block.slabs)
    {
      int32_t up_scale_log2 = 1;

      while (true)
        {
          uint32_t down_slabs;

          // flip merge slabs -- return span of slabs that must be cleaned
          uint32_t clean_slabs_log2 = hs_fm_launcher(hs,&state,
                                                     &down_slabs,
                                                     up_scale_log2);

          // if span is gt largest slab block cleaner then half merge
          while (clean_slabs_log2 > hs->bc_slabs_log2_max)
            {
              clean_slabs_log2 = hs_hm_launcher(hs,&state,
                                                down_slabs,
                                                clean_slabs_log2);
            }

          // launch clean slab grid -- is it the final launch?
          hs_bc_launcher(hs,&state,
                         down_slabs,
                         clean_slabs_log2);

          // was this the final block clean?
          if (((uint32_t)hs->config.block.slabs << up_scale_log2) >= state.bx_ru)
            break;

          // otherwise, merge twice as many slabs
          up_scale_log2 += 1;
        }
    }

  // slabs or linear?
  if (linearize) {
    hs_transpose_launcher(hs,&state);
  }

  // does the caller want the final event?
  if (event != NULL) {
    *event = state.wait_list[0];
  } else {
    cl(ReleaseEvent(state.wait_list[0]));
  }
}

//
// all grids will be computed as a function of the minimum number of slabs
//

void
hs_cl_pad(struct hs_cl const * const hs,
          uint32_t             const count,
          uint32_t           * const count_padded_in,
          uint32_t           * const count_padded_out)
{
  //
  // round up the count to slabs
  //
  uint32_t const slabs_ru        = (count + hs->slab_keys - 1) / hs->slab_keys;
  uint32_t const blocks          = slabs_ru / hs->config.block.slabs;
  uint32_t const block_slabs     = blocks * hs->config.block.slabs;
  uint32_t const slabs_ru_rem    = slabs_ru - block_slabs;
  uint32_t const slabs_ru_rem_ru = MIN_MACRO(pow2_ru_u32(slabs_ru_rem),hs->config.block.slabs);

  *count_padded_in  = (block_slabs + slabs_ru_rem_ru) * hs->slab_keys;
  *count_padded_out = *count_padded_in;

  //
  // will merging be required?
  //
  if (slabs_ru > hs->config.block.slabs)
    {
      // more than one block
      uint32_t const blocks_lo       = pow2_rd_u32(blocks);
      uint32_t const block_slabs_lo  = blocks_lo * hs->config.block.slabs;
      uint32_t const block_slabs_rem = slabs_ru - block_slabs_lo;

      if (block_slabs_rem > 0)
        {
          uint32_t const block_slabs_rem_ru     = pow2_ru_u32(block_slabs_rem);

          uint32_t const block_slabs_hi         = MAX_MACRO(block_slabs_rem_ru,
                                                            blocks_lo << (1 - hs->config.merge.fm.scale_min));

          uint32_t const block_slabs_padded_out = MIN_MACRO(block_slabs_lo+block_slabs_hi,
                                                            block_slabs_lo*2); // clamp non-pow2 blocks

          *count_padded_out = block_slabs_padded_out * hs->slab_keys;
        }
    }
}

//
//
//

static
void
hs_create_kernel(cl_program         program,
                 cl_kernel  * const kernel,
                 char const * const name)
{
  cl_int err;

  *kernel = clCreateKernel(program,name,&err);

  cl_ok(err);
}

static
void
hs_create_kernels(cl_program     program,
                  cl_kernel    * kernels,
                  char           name_template[],
                  size_t   const name_template_size,
                  uint32_t const count)
{
  char const n_max = '0'+(char)count;

  for (char n = '0'; n<n_max; n++)
    {
      cl_int err;

      name_template[name_template_size-2] = n;

      *kernels++ = clCreateKernel(program,name_template,&err);

      cl_ok(err);
    }
}

//
//
//

struct hs_cl *
hs_cl_create(struct hs_cl_target const * const target,
             cl_context                        context,
             cl_device_id                      device_id)
{
  //
  // immediately try to build the OpenCL program
  //
  bool     const is_binary    = (target->program[0] == 0);
  uint32_t const program_size = NPBTOHL_MACRO(target->program+1);

  cl_program program;

  if (is_binary) // program is a binary
    {
      cl_int status, err;

      size_t        const   bins_sizeof[] = { program_size      };
      unsigned char const * bins[]        = { target->program+5 };

      program = clCreateProgramWithBinary(context,
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
    }
  else // program is source code
    {
      cl_int err;

      size_t const   strings_sizeof[] = { program_size             };
      char   const * strings[]        = { (char*)target->program+5 };

      program = clCreateProgramWithSource(context,
                                          1,
                                          strings,
                                          strings_sizeof,
                                          &err);
      cl_ok(err);

      char const * const options =
        "-cl-std=CL1.2 -cl-fast-relaxed-math " // FIXME FIXME FIXME FIXME 1.2
        "-cl-no-signed-zeros -cl-mad-enable "
        "-cl-denorms-are-zero "
        "-cl-kernel-arg-info";

      cl(BuildProgram(program,
                      1,
                      &device_id,
                      options,
                      NULL,
                      NULL));
    }

  //
  // we reference these values a lot
  //
  uint32_t const bs_slabs_log2_ru  = msb_idx_u32(pow2_ru_u32(target->config.block.slabs));
  uint32_t const bc_slabs_log2_max = msb_idx_u32(pow2_rd_u32(target->config.block.slabs));

  //
  // how many kernels will be created?
  //
  uint32_t const count_bs    = bs_slabs_log2_ru + 1;
  uint32_t const count_bc    = bc_slabs_log2_max + 1;
  uint32_t       count_fm[3] = { 0 };
  uint32_t       count_hm[3] = { 0 };

  // guaranteed to be in range [0,2]
  for (uint32_t scale = target->config.merge.fm.scale_min;
       scale <= target->config.merge.fm.scale_max;
       scale++)
    {
      count_fm[scale] = msb_idx_u32(pow2_ru_u32(target->config.block.slabs>>(scale-1))) + 1;
    }

  // guaranteed to be in range [0,2]
  for (uint32_t scale = target->config.merge.hm.scale_min;
       scale <= target->config.merge.hm.scale_max;
       scale++)
    {
      count_hm[scale] = 1;
    }

  uint32_t const count_all =
    1
    + count_bs
    + count_bc
    + count_fm[0] + count_fm[1] + count_fm[2]
    + count_hm[0] + count_hm[1] + count_hm[2];

  //
  // allocate hs_cl
  //
  struct hs_cl * hs = malloc(sizeof(*hs) + sizeof(cl_kernel) * count_all);

  memcpy(&hs->config,&target->config,sizeof(hs->config));

  // save some frequently used calculated values
  hs->key_val_size      = (target->config.words.key + target->config.words.val) * 4;
  hs->slab_keys         = target->config.slab.height << target->config.slab.width_log2;
  hs->bs_slabs_log2_ru  = bs_slabs_log2_ru;
  hs->bc_slabs_log2_max = bc_slabs_log2_max;

  // save kernel count
  hs->kernels.count     = count_all;

  //
  // create all the kernels and release the program
  //
  cl_kernel * kernel_next = hs->kernels.all;

  //
  // TRANSPOSE
  //
  {
    hs->kernels.transpose = kernel_next;

    hs_create_kernel(program,
                     kernel_next,
                     "hs_kernel_transpose");

    kernel_next += 1;
  }

  //
  // BS
  //
  {
    hs->kernels.bs = kernel_next;

    char bs_name[] = { "hs_kernel_bs_X" };

    hs_create_kernels(program,
                      kernel_next,
                      bs_name,sizeof(bs_name),
                      count_bs);

    kernel_next += count_bs;
  }

  //
  // BC
  //
  {
    hs->kernels.bc = kernel_next;

    char bc_name[] = { "hs_kernel_bc_X" };

    hs_create_kernels(program,
                      kernel_next,
                      bc_name,sizeof(bc_name),
                      count_bc);

    kernel_next += count_bc;
  }

  //
  // FM 0
  //
  if (count_fm[0] > 0)
    {
      hs->kernels.fm[0] = kernel_next;

      char fm_0_name[]  = { "hs_kernel_fm_0_X" };

      hs_create_kernels(program,
                        kernel_next,
                        fm_0_name,sizeof(fm_0_name),
                        count_fm[0]);

      kernel_next += count_fm[0];
    }

  if (count_fm[1] > 0)
    {
      hs->kernels.fm[1] = kernel_next;

      char fm_1_name[]  = { "hs_kernel_fm_1_X" };

      hs_create_kernels(program,
                        kernel_next,
                        fm_1_name,sizeof(fm_1_name),
                        count_fm[1]);

      kernel_next += count_fm[1];
    }

  if (count_fm[2] > 0)
    {
      hs->kernels.fm[2] = kernel_next;

      char fm_2_name[]  = { "hs_kernel_fm_2_X" };

      hs_create_kernels(program,
                        kernel_next,
                        fm_2_name,sizeof(fm_2_name),
                        count_fm[2]);

      kernel_next += count_fm[2];
    }

  if (count_hm[0] > 0)
    {
      hs->kernels.hm[0] = kernel_next;

      hs_create_kernel(program,
                       kernel_next,
                       "hs_kernel_hm_0");

      kernel_next += count_hm[0];
    }

  if (count_hm[1] > 0)
    {
      hs->kernels.hm[1] = kernel_next;

      hs_create_kernel(program,
                       kernel_next,
                       "hs_kernel_hm_1");

      kernel_next += count_hm[1];
    }

  if (count_hm[2] > 0)
    {
      hs->kernels.hm[2] = kernel_next;

      hs_create_kernel(program,
                       kernel_next,
                       "hs_kernel_hm_2");

      kernel_next += count_hm[2]; // unnecessary
    }

  return hs;
}

//
//
//

void
hs_cl_release(struct hs_cl * const hs)
{
  for (uint32_t ii=0; ii<hs->kernels.count; ii++)
    cl(ReleaseKernel(hs->kernels.all[ii]));

  free(hs);
}

//
//
//
