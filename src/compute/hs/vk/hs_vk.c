/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "common/util.h"
#include "common/macros.h"
#include "common/vk/assert_vk.h"

#include "hs_vk.h"
#include "hs_vk_target.h"

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

struct hs_vk
{
  VkAllocationCallbacks const * allocator;
  VkDevice                      device;

  struct {
    struct {
      VkDescriptorSetLayout     vout_vin;
    } layout;
  } desc_set;

  struct {
    struct {
      VkPipelineLayout          vout_vin;
    } layout;
  } pipeline;

  struct hs_vk_target_config    config;

  uint32_t                      key_val_size;
  uint32_t                      slab_keys;
  uint32_t                      bs_slabs_log2_ru;
  uint32_t                      bc_slabs_log2_max;

  struct {
    uint32_t                    count;
    VkPipeline                * bs;
    VkPipeline                * bc;
    VkPipeline                * fm[3];
    VkPipeline                * hm[3];
    VkPipeline                * transpose;
    VkPipeline                  all[];
  } pipelines;
};

//
//
//

struct hs_state
{
  VkCommandBuffer      cb;

  // If sorting in-place, then vout == vin
  VkBuffer             vout;
  VkBuffer             vin;

  // bx_ru is number of rounded up warps in vin
  uint32_t             bx_ru;
};

//
//
//

static
void
hs_barrier_compute_w_to_compute_r(struct hs_state * const state)
{
  static VkMemoryBarrier const shader_w_to_r = {
    .sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
    .pNext         = NULL,
    .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
    .dstAccessMask = VK_ACCESS_SHADER_READ_BIT
  };

  vkCmdPipelineBarrier(state->cb,
                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       0,
                       1,
                       &shader_w_to_r,
                       0,
                       NULL,
                       0,
                       NULL);
}

//
//
//

static
void
hs_barrier_to_compute_r(struct hs_state    * const state,
                        VkPipelineStageFlags const src_stage,
                        VkAccessFlagBits     const src_access)
{
  if (src_stage == 0)
    return;

  VkMemoryBarrier const compute_r = {
    .sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
    .pNext         = NULL,
    .srcAccessMask = src_access,
    .dstAccessMask = VK_ACCESS_SHADER_READ_BIT
  };

  vkCmdPipelineBarrier(state->cb,
                       src_stage,
                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       0,
                       1,
                       &compute_r,
                       0,
                       NULL,
                       0,
                       NULL);
}

//
//
//

static
void
hs_barrier_to_transfer_fill(struct hs_state    * const state,
                            VkPipelineStageFlags const src_stage,
                            VkAccessFlagBits     const src_access)
{
  if (src_stage == 0)
    return;

  VkMemoryBarrier const fill_w = {
    .sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
    .pNext         = NULL,
    .srcAccessMask = src_access,
    .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT
  };

  vkCmdPipelineBarrier(state->cb,
                       src_stage,
                       VK_PIPELINE_STAGE_TRANSFER_BIT,
                       0,
                       1,
                       &fill_w,
                       0,
                       NULL,
                       0,
                       NULL);
}

//
//
//

static
void
hs_transpose(struct hs_vk const * const hs,
             struct hs_state    * const state)
{
  hs_barrier_compute_w_to_compute_r(state);

  vkCmdBindPipeline(state->cb,
                    VK_PIPELINE_BIND_POINT_COMPUTE,
                    hs->pipelines.transpose[0]);

  vkCmdDispatch(state->cb,state->bx_ru,1,1);
}

//
//
//

static
void
hs_bc(struct hs_vk const * const hs,
      struct hs_state    * const state,
      uint32_t             const down_slabs,
      uint32_t             const clean_slabs_log2)
{
  hs_barrier_compute_w_to_compute_r(state);

  // block clean the minimal number of down_slabs_log2 spans
  uint32_t const frac_ru = (1u << clean_slabs_log2) - 1;
  uint32_t const full_bc = (down_slabs + frac_ru) >> clean_slabs_log2;

  vkCmdBindPipeline(state->cb,
                    VK_PIPELINE_BIND_POINT_COMPUTE,
                    hs->pipelines.bc[clean_slabs_log2]);

  vkCmdDispatch(state->cb,full_bc,1,1);
}

//
//
//

static
uint32_t
hs_hm(struct hs_vk const * const hs,
      struct hs_state    * const state,
      uint32_t             const down_slabs,
      uint32_t             const clean_slabs_log2)
{
  hs_barrier_compute_w_to_compute_r(state);

  // how many scaled half-merge spans are there?
  uint32_t const frac_ru    = (1 << clean_slabs_log2) - 1;
  uint32_t const spans      = (down_slabs + frac_ru) >> clean_slabs_log2;

  // for now, just clamp to the max
  uint32_t const log2_rem   = clean_slabs_log2 - hs->bc_slabs_log2_max;
  uint32_t const scale_log2 = MIN_MACRO(hs->config.merge.hm.scale_max,log2_rem);
  uint32_t const log2_out   = log2_rem - scale_log2;

  // size the grid
  uint32_t const slab_span  = hs->config.slab.height << log2_out;

  vkCmdBindPipeline(state->cb,
                    VK_PIPELINE_BIND_POINT_COMPUTE,
                    hs->pipelines.hm[scale_log2][0]);

  vkCmdDispatch(state->cb,slab_span,spans,1);

  return log2_out;
}

//
// FIXME -- some of this logic can be skipped if BS is a power-of-two
//

static
uint32_t
hs_fm(struct hs_vk const * const hs,
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

  //
  // size the grid
  //
  uint32_t const slab_span = hs->config.slab.height << clean_log2;

  if (full_fm > 0)
    {
      uint32_t const full_idx = hs->bs_slabs_log2_ru - 1 + scale_log2;

      vkCmdBindPipeline(state->cb,
                        VK_PIPELINE_BIND_POINT_COMPUTE,
                        hs->pipelines.fm[scale_log2][full_idx]);

      vkCmdDispatch(state->cb,slab_span,full_fm,1);
    }

  if (frac_fm > 0)
    {
      vkCmdBindPipeline(state->cb,
                        VK_PIPELINE_BIND_POINT_COMPUTE,
                        hs->pipelines.fm[scale_log2][msb_idx_u32(frac_fm)]);

      vkCmdDispatchBase(state->cb,
                        0,full_fm,0,
                        slab_span,1,1);
    }

  return clean_log2;
}

//
//
//

static
void
hs_bs(struct hs_vk const * const hs,
      struct hs_state    * const state,
      uint32_t             const count_padded_in)
{
  uint32_t const slabs_in = count_padded_in / hs->slab_keys;
  uint32_t const full_bs  = slabs_in / hs->config.block.slabs;
  uint32_t const frac_bs  = slabs_in - full_bs * hs->config.block.slabs;

  if (full_bs > 0)
    {
      vkCmdBindPipeline(state->cb,
                        VK_PIPELINE_BIND_POINT_COMPUTE,
                        hs->pipelines.bs[hs->bs_slabs_log2_ru]);

      vkCmdDispatch(state->cb,full_bs,1,1);
    }

  if (frac_bs > 0)
    {
      uint32_t const frac_idx          = msb_idx_u32(frac_bs);
      uint32_t const full_to_frac_log2 = hs->bs_slabs_log2_ru - frac_idx;

      vkCmdBindPipeline(state->cb,
                        VK_PIPELINE_BIND_POINT_COMPUTE,
                        hs->pipelines.bs[msb_idx_u32(frac_bs)]);

      vkCmdDispatchBase(state->cb,
                        full_bs<<full_to_frac_log2,0,0,
                        1,1,1);
    }
}

//
//
//

static
void
hs_keyset_pre_fm(struct hs_vk const * const hs,
                 struct hs_state    * const state,
                 uint32_t             const count_lo,
                 uint32_t             const count_hi)
{
  uint32_t const vout_span = count_hi - count_lo;

  vkCmdFillBuffer(state->cb,
                  state->vout,
                  count_lo  * hs->key_val_size,
                  vout_span * hs->key_val_size,
                  UINT32_MAX);
}

//
//
//

static
void
hs_keyset_pre_bs(struct hs_vk const * const hs,
                 struct hs_state    * const state,
                 uint32_t             const count,
                 uint32_t             const count_hi)
{
  uint32_t const vin_span = count_hi - count;

  vkCmdFillBuffer(state->cb,
                  state->vin,
                  count    * hs->key_val_size,
                  vin_span * hs->key_val_size,
                  UINT32_MAX);
}

//
//
//

void
hs_vk_ds_bind(struct hs_vk const * const hs,
              VkDescriptorSet            hs_ds,
              VkCommandBuffer            cb,
              VkBuffer                   vin,
              VkBuffer                   vout)
{
  //
  // initialize the HotSort descriptor set
  //
  VkDescriptorBufferInfo const dbi[] = {
    {
      .buffer = vout == VK_NULL_HANDLE ? vin : vout,
      .offset = 0,
      .range  = VK_WHOLE_SIZE
    },
    {
      .buffer = vin,
      .offset = 0,
      .range  = VK_WHOLE_SIZE
    }
  };

  VkWriteDescriptorSet const wds[] = {
    {
      .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .pNext            = NULL,
      .dstSet           = hs_ds,
      .dstBinding       = 0,
      .dstArrayElement  = 0,
      .descriptorCount  = 2,
      .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .pImageInfo       = NULL,
      .pBufferInfo      = dbi,
      .pTexelBufferView = NULL
    }
  };

  vkUpdateDescriptorSets(hs->device,
                         ARRAY_LENGTH_MACRO(wds),
                         wds,
                         0,
                         NULL);

  //
  // All HotSort kernels can use the same descriptor set:
  //
  //   {
  //     HS_KEY_TYPE vout[];
  //     HS_KEY_TYPE vin[];
  //   }
  //
  // Note that only the bs() kernels read from vin().
  //
  vkCmdBindDescriptorSets(cb,
                          VK_PIPELINE_BIND_POINT_COMPUTE,
                          hs->pipeline.layout.vout_vin,
                          0,
                          1,
                          &hs_ds,
                          0,
                          NULL);
}

//
//
//

void
hs_vk_sort(struct hs_vk const * const hs,
           VkCommandBuffer            cb,
           VkBuffer                   vin,
           VkPipelineStageFlags const vin_src_stage,
           VkAccessFlagBits     const vin_src_access,
           VkBuffer                   vout,
           VkPipelineStageFlags const vout_src_stage,
           VkAccessFlagBits     const vout_src_access,
           uint32_t             const count,
           uint32_t             const count_padded_in,
           uint32_t             const count_padded_out,
           bool                 const linearize)
{
  // is this sort in place?
  bool const is_in_place = (vout == VK_NULL_HANDLE);

  //
  // create some common state
  //
  struct hs_state state = {
    .cb    = cb,
    .vin   = vin,
    .vout  = is_in_place ? vin : vout,
    .bx_ru = (count + hs->slab_keys - 1) / hs->slab_keys
  };

  // initialize vin
  uint32_t const count_hi          = is_in_place ? count_padded_out : count_padded_in;
  bool     const is_pre_sort_reqd  = count_hi > count;
  bool     const is_pre_merge_reqd = !is_in_place && (count_padded_out > count_padded_in);

  //
  // pre-sort  keyset needs to happen before bs()
  // pre-merge keyset needs to happen before fm()
  //

  VkPipelineStageFlags bs_src_stage  = 0;
  VkAccessFlagBits     bs_src_access = 0;

  // initialize any trailing keys in vin before sorting
  if (is_pre_sort_reqd)
    {
      hs_barrier_to_transfer_fill(&state,vin_src_stage,vin_src_access);

      hs_keyset_pre_bs(hs,&state,count,count_hi);

      bs_src_stage  |= VK_PIPELINE_STAGE_TRANSFER_BIT;
      bs_src_access |= VK_ACCESS_TRANSFER_WRITE_BIT;
    }
  else
    {
      bs_src_stage  = vin_src_stage;
      bs_src_access = vin_src_access;
    }

  hs_barrier_to_compute_r(&state,bs_src_stage,bs_src_access);

  // sort blocks of slabs... after hs_keyset_pre_sort()
  hs_bs(hs,&state,count_padded_in);

  VkPipelineStageFlags fm_src_stage  = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
  VkAccessFlagBits     fm_src_access = VK_ACCESS_SHADER_READ_BIT;

  // initialize any trailing keys in vout before merging
  if (is_pre_merge_reqd)
    {
      hs_barrier_to_transfer_fill(&state,vout_src_stage,vout_src_access);

      hs_keyset_pre_fm(hs,&state,count_padded_in,count_padded_out);

      fm_src_stage  |= VK_PIPELINE_STAGE_TRANSFER_BIT;
      fm_src_access |= VK_ACCESS_TRANSFER_WRITE_BIT;
    }
  else
    {
      fm_src_stage  |= vout_src_stage;
      fm_src_access |= vout_src_access;
    }

  //
  // if this was a single bs block then there is no merging
  //
  if (state.bx_ru > hs->config.block.slabs)
    {
      hs_barrier_to_compute_r(&state,fm_src_stage,fm_src_access);

      //
      // otherwise, merge sorted spans of slabs until done
      //
      int32_t up_scale_log2 = 1;

      while (true)
        {
          uint32_t down_slabs;

          // flip merge slabs -- return span of slabs that must be cleaned
          uint32_t clean_slabs_log2 = hs_fm(hs,&state,
                                            &down_slabs,
                                            up_scale_log2);

          // if span is gt largest slab block cleaner then half merge
          while (clean_slabs_log2 > hs->bc_slabs_log2_max)
            {
              clean_slabs_log2 = hs_hm(hs,&state,
                                       down_slabs,
                                       clean_slabs_log2);
            }

          // launch clean slab grid -- is it the final launch?
          hs_bc(hs,&state,down_slabs,clean_slabs_log2);

          // was this the final block clean?
          if (((uint32_t)hs->config.block.slabs << up_scale_log2) >= state.bx_ru)
            break;

          // otherwise, merge twice as many slabs
          up_scale_log2 += 1;

          // drop a barrier
          hs_barrier_compute_w_to_compute_r(&state);
        }
    }

  // slabs or linear?
  if (linearize)
    hs_transpose(hs,&state);
}

//
//
//

#ifdef HS_VK_VERBOSE_STATISTICS_AMD

#include <stdio.h>

static
void
hs_vk_verbose_statistics_amd(VkDevice device, struct hs_vk const * const hs)
{
  PFN_vkGetShaderInfoAMD vkGetShaderInfoAMD =
    (PFN_vkGetShaderInfoAMD)
    vkGetDeviceProcAddr(device,"vkGetShaderInfoAMD");

  if (vkGetShaderInfoAMD == NULL)
    return;

  fprintf(stdout,
          "                                   PHY   PHY  AVAIL AVAIL\n"
          "VGPRs SGPRs LDS_MAX LDS/WG  SPILL VGPRs SGPRs VGPRs SGPRs  WORKGROUP_SIZE\n");

  for (uint32_t ii=0; ii<hs->pipelines.count; ii++)
    {
      VkShaderStatisticsInfoAMD ssi_amd;
      size_t                    ssi_amd_size = sizeof(ssi_amd);

      if (vkGetShaderInfoAMD(hs->device,
                             hs->pipelines.all[ii],
                             VK_SHADER_STAGE_COMPUTE_BIT,
                             VK_SHADER_INFO_TYPE_STATISTICS_AMD,
                             &ssi_amd_size,
                             &ssi_amd) == VK_SUCCESS)
        {
          fprintf(stdout,
                  "%5" PRIu32 " "
                  "%5" PRIu32 "   "
                  "%5" PRIu32 " "

                  "%6zu "
                  "%6zu "

                  "%5" PRIu32 " "
                  "%5" PRIu32 " "
                  "%5" PRIu32 " "
                  "%5" PRIu32 "  "

                  "( %6" PRIu32 ", " "%6" PRIu32 ", " "%6" PRIu32 " )\n",
                  ssi_amd.resourceUsage.numUsedVgprs,
                  ssi_amd.resourceUsage.numUsedSgprs,
                  ssi_amd.resourceUsage.ldsSizePerLocalWorkGroup,
                  ssi_amd.resourceUsage.ldsUsageSizeInBytes,    // size_t
                  ssi_amd.resourceUsage.scratchMemUsageInBytes, // size_t
                  ssi_amd.numPhysicalVgprs,
                  ssi_amd.numPhysicalSgprs,
                  ssi_amd.numAvailableVgprs,
                  ssi_amd.numAvailableSgprs,
                  ssi_amd.computeWorkGroupSize[0],
                  ssi_amd.computeWorkGroupSize[1],
                  ssi_amd.computeWorkGroupSize[2]);
        }
    }
}

#endif

//
//
//

#ifdef HS_VK_VERBOSE_DISASSEMBLY_AMD

#include <stdio.h>

static
void
hs_vk_verbose_disassembly_amd(VkDevice device, struct hs_vk const * const hs)
{
  PFN_vkGetShaderInfoAMD vkGetShaderInfoAMD =
    (PFN_vkGetShaderInfoAMD)
    vkGetDeviceProcAddr(device,"vkGetShaderInfoAMD");

  if (vkGetShaderInfoAMD == NULL)
    return;

  for (uint32_t ii=0; ii<hs->pipelines.count; ii++)
    {
      size_t disassembly_amd_size;

      if (vkGetShaderInfoAMD(hs->device,
                             hs->pipelines.all[ii],
                             VK_SHADER_STAGE_COMPUTE_BIT,
                             VK_SHADER_INFO_TYPE_DISASSEMBLY_AMD,
                             &disassembly_amd_size,
                             NULL) == VK_SUCCESS)
        {
          void * disassembly_amd = malloc(disassembly_amd_size);

          if (vkGetShaderInfoAMD(hs->device,
                                 hs->pipelines.all[ii],
                                 VK_SHADER_STAGE_COMPUTE_BIT,
                                 VK_SHADER_INFO_TYPE_DISASSEMBLY_AMD,
                                 &disassembly_amd_size,
                                 disassembly_amd) == VK_SUCCESS)
            {
              fprintf(stdout,"%s",(char*)disassembly_amd);
            }

          free(disassembly_amd);
        }
    }
}

#endif

//
//
//

struct hs_vk *
hs_vk_create(struct hs_vk_target   const * const target,
             VkDevice                            device,
             VkAllocationCallbacks const *       allocator,
             VkPipelineCache                     pipeline_cache)
{
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
      uint32_t fm_left = (target->config.block.slabs / 2) << scale;

      count_fm[scale] = msb_idx_u32(pow2_ru_u32(fm_left)) + 1;
    }

  // guaranteed to be in range [0,2]
  for (uint32_t scale = target->config.merge.hm.scale_min;
       scale <= target->config.merge.hm.scale_max;
       scale++)
    {
      count_hm[scale] = 1;
    }

  uint32_t const count_bc_fm_hm_transpose =
    + count_bc
    + count_fm[0] + count_fm[1] + count_fm[2]
    + count_hm[0] + count_hm[1] + count_hm[2] +
    1; // transpose

  uint32_t const count_all = count_bs + count_bc_fm_hm_transpose;

  //
  // allocate hs_vk
  //
  struct hs_vk * hs;

  if (allocator == NULL)
    {
      hs = malloc(sizeof(*hs) + sizeof(VkPipeline*) * count_all);
    }
  else
    {
      hs = allocator->pfnAllocation(NULL,
                                    sizeof(*hs) + sizeof(VkPipeline*) * count_all,
                                    0,
                                    VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
    }

  // save device & allocator
  hs->device    = device;
  hs->allocator = allocator;

  //
  // create one descriptor set layout
  //
  static VkDescriptorSetLayoutBinding const dslb_vout_vin[] = {
    {
      .binding            = 0, // vout
      .descriptorType     = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .descriptorCount    = 1,
      .stageFlags         = VK_SHADER_STAGE_COMPUTE_BIT,
      .pImmutableSamplers = NULL
    },
    {
      .binding            = 1, // vin
      .descriptorType     = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .descriptorCount    = 1,
      .stageFlags         = VK_SHADER_STAGE_COMPUTE_BIT,
      .pImmutableSamplers = NULL
    }
  };

  static VkDescriptorSetLayoutCreateInfo const dscli = {
    .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .pNext        = NULL,
    .flags        = 0,
    .bindingCount = 2, // 0:vout[], 1:vin[]
    .pBindings    = dslb_vout_vin
  };

  vk(CreateDescriptorSetLayout(device,
                               &dscli,
                               allocator,
                               &hs->desc_set.layout.vout_vin));

  //
  // create one pipeline layout
  //
  VkPipelineLayoutCreateInfo plci = {
    .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .pNext                  = NULL,
    .flags                  = 0,
    .setLayoutCount         = 1,
    .pSetLayouts            = &hs->desc_set.layout.vout_vin,
    .pushConstantRangeCount = 0,
    .pPushConstantRanges    = NULL
  };

  vk(CreatePipelineLayout(device,
                          &plci,
                          allocator,
                          &hs->pipeline.layout.vout_vin));

  //
  // copy the config from the target -- we need these values later
  //
  memcpy(&hs->config,&target->config,sizeof(hs->config));

  // save some frequently used calculated values
  hs->key_val_size      = (target->config.words.key + target->config.words.val) * 4;
  hs->slab_keys         = target->config.slab.height << target->config.slab.width_log2;
  hs->bs_slabs_log2_ru  = bs_slabs_log2_ru;
  hs->bc_slabs_log2_max = bc_slabs_log2_max;

  // save kernel count
  hs->pipelines.count   = count_all;

  //
  // create all the compute pipelines by reusing this info
  //
  VkComputePipelineCreateInfo cpci = {
    .sType                 = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
    .pNext                 = NULL,
    .flags                 = VK_PIPELINE_CREATE_DISPATCH_BASE, // | VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT,
    .stage = {
      .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .pNext               = NULL,
      .flags               = 0,
      .stage               = VK_SHADER_STAGE_COMPUTE_BIT,
      .module              = VK_NULL_HANDLE,
      .pName               = "main",
      .pSpecializationInfo = NULL
    },
    .layout                = hs->pipeline.layout.vout_vin,
    .basePipelineHandle    = VK_NULL_HANDLE,
    .basePipelineIndex     = 0
  };

  //
  // Create a shader module, use it to create a pipeline... and
  // dispose of the shader module.
  //
  // The BS     compute shaders have the same layout
  // The non-BS compute shaders have the same layout
  //
  VkShaderModuleCreateInfo smci = {
    .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .pNext    = NULL,
    .flags    = 0,
    .codeSize = 0,
    .pCode    = (uint32_t const *)target->modules // FIXME -- unfortunate typecast
  };

  //
  // bs kernels have layout: (vout,vin)
  // remaining  have layout: (vout)
  //
  for (uint32_t ii=0; ii<count_all; ii++)
    {
      // convert bytes to words
      uint32_t const * const module = smci.pCode + smci.codeSize / sizeof(*module);

      smci.codeSize = NTOHL_MACRO(module[0]);
      smci.pCode    = module + 1;

      vk(CreateShaderModule(device,
                            &smci,
                            allocator,
                            &cpci.stage.module));

      vk(CreateComputePipelines(device,
                                pipeline_cache,
                                1,
                                &cpci,
                                allocator,
                                hs->pipelines.all+ii));

      vkDestroyShaderModule(device,
                            cpci.stage.module,
                            allocator);
    }

  //
  // initialize pointers to pipeline handles
  //
  VkPipeline * pipeline_next = hs->pipelines.all;

  // BS
  hs->pipelines.bs        = pipeline_next;
  pipeline_next          += count_bs;

  // BC
  hs->pipelines.bc        = pipeline_next;
  pipeline_next          += count_bc;

  // FM[0]
  hs->pipelines.fm[0]     = count_fm[0] ? pipeline_next : NULL;
  pipeline_next          += count_fm[0];

  // FM[1]
  hs->pipelines.fm[1]     = count_fm[1] ? pipeline_next : NULL;
  pipeline_next          += count_fm[1];

  // FM[2]
  hs->pipelines.fm[2]     = count_fm[2] ? pipeline_next : NULL;
  pipeline_next          += count_fm[2];

  // HM[0]
  hs->pipelines.hm[0]     = count_hm[0] ? pipeline_next : NULL;
  pipeline_next          += count_hm[0];

  // HM[1]
  hs->pipelines.hm[1]     = count_hm[1] ? pipeline_next : NULL;
  pipeline_next          += count_hm[1];

  // HM[2]
  hs->pipelines.hm[2]     = count_hm[2] ? pipeline_next : NULL;
  pipeline_next          += count_hm[2];

  // TRANSPOSE
  hs->pipelines.transpose = pipeline_next;
  pipeline_next          += 1;

  //
  // optionally dump pipeline stats
  //
#ifdef HS_VK_VERBOSE_STATISTICS_AMD
  hs_vk_verbose_statistics_amd(device,hs);
#endif
#ifdef HS_VK_VERBOSE_DISASSEMBLY_AMD
  hs_vk_verbose_disassembly_amd(device,hs);
#endif

  //
  //
  //

  return hs;
}

//
//
//

void
hs_vk_release(struct hs_vk * const hs)
{
  vkDestroyDescriptorSetLayout(hs->device,
                               hs->desc_set.layout.vout_vin,
                               hs->allocator);

  vkDestroyPipelineLayout(hs->device,
                          hs->pipeline.layout.vout_vin,
                          hs->allocator);

  for (uint32_t ii=0; ii<hs->pipelines.count; ii++)
    {
      vkDestroyPipeline(hs->device,
                        hs->pipelines.all[ii],
                        hs->allocator);
    }

  if (hs->allocator == NULL)
    {
      free(hs);
    }
  else
    {
      hs->allocator->pfnFree(NULL,hs);
    }
}

//
// Allocate a per-thread descriptor set for the vin and vout
// VkBuffers.  Note that HotSort uses only one descriptor set.
//

VkDescriptorSet
hs_vk_ds_alloc(struct hs_vk const * const hs, VkDescriptorPool desc_pool)
{
  VkDescriptorSetAllocateInfo const ds_alloc_info = {
    .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .pNext              = NULL,
    .descriptorPool     = desc_pool,
    .descriptorSetCount = 1,
    .pSetLayouts        = &hs->desc_set.layout.vout_vin
  };

  VkDescriptorSet hs_ds;

  vk(AllocateDescriptorSets(hs->device,
                            &ds_alloc_info,
                            &hs_ds));

  return hs_ds;
}

//
//
//

void
hs_vk_pad(struct hs_vk const * const hs,
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
