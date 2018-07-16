/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#include <stdlib.h>
#include <string.h>

#include "common/vk/assert_vk.h"
#include "common/util.h"

#include "hs_vk_launcher.h"
#include "hs_spirv_target.h"

//
//
//

struct hs_vk
{
  struct hs_spirv_target_config config;

  uint32_t                      key_val_size;
  uint32_t                      slab_keys;
  uint32_t                      bs_slabs_log2_ru;
  uint32_t                      bc_slabs_log2_max;

  VkDevice                      device;
  VkAllocationCallbacks const * allocator;

  struct {
    uint32_t                    count;
    VkPipeline                * transpose;
    VkPipeline                * bs;
    VkPipeline                * bc;
    VkPipeline                * fm[3];
    VkPipeline                * hm[3];
    VkPipeline                  all[];
  } pipelines;
};

//
//
//

struct hs_vk *
hs_vk_create(struct hs_spirv_target const * const target,
             VkDevice                             device,
             VkAllocationCallbacks  const *       allocator,
             VkPipelineCache                      pipeline_cache)
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
  // allocate hs_vk
  //
  struct hs_vk * hs;

  if (allocator == NULL)
    {
      hs = malloc(sizeof(*hs) + sizeof(VkPipeline*) * count_all);
    }
  else
    {
      hs = NULL;
    }

  // save the config
  memcpy(&hs->config,&target->config,sizeof(hs->config));

  // save some frequently used calculated values
  hs->key_val_size      = (target->config.words.key + target->config.words.val) * 4;
  hs->slab_keys         = target->config.slab.height << target->config.slab.width_log2;
  hs->bs_slabs_log2_ru  = bs_slabs_log2_ru;
  hs->bc_slabs_log2_max = bc_slabs_log2_max;

  // save device & allocator
  hs->device            = device;
  hs->allocator         = allocator;

  // save kernel count
  hs->pipelines.count   = count_all;

  //
  // create all the compute pipelines
  //
  VkComputePipelineCreateInfo cpci = {
    .sType                 = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
    .pNext                 = NULL,
    .flags                 = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT,
    .stage = {
      .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .pNext               = NULL,
      .flags               = 0,
      .stage               = VK_SHADER_STAGE_COMPUTE_BIT,
      .module              = VK_NULL_HANDLE,
      .pName               = "main",
      .pSpecializationInfo = NULL
    },
    .basePipelineHandle    = VK_NULL_HANDLE,
    .basePipelineIndex     = -1
  };

  //
  // Create a shader module, use it to create a pipeline... and
  // dispose of the shader module.
  //
  uint32_t const * modules = target->modules.words;

  for (uint32_t ii=0; ii<count_all; ii++)
    {
      size_t const module_size = *modules++;

      VkShaderModuleCreateInfo const smci = {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext    = NULL,
        .flags    = 0,
        .codeSize = module_size,
        .pCode    = modules
      };

      modules += module_size;

      vk(CreateShaderModule(device,
                            &smci,
                            allocator,
                            &cpci.stage.module));


      vk(CreateComputePipelines(device,
                                pipeline_cache,
                                count_all,
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

  // TRANSPOSE
  hs->pipelines.transpose = pipeline_next;
  pipeline_next          += 1;

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

  return hs;
}

//
//
//

void
hs_vk_release(struct hs_vk * const hs)
{
  for (uint32_t ii=0; ii<hs->pipelines.count; ii++)
    vkDestroyPipeline(hs->device,
                      hs->pipelines.all[ii],
                      hs->allocator);

  if (hs->allocator == NULL)
    {
      free(hs);
    }
  else
    {
      ;
    }
}

//
//
//
