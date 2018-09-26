/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

//
//
//

#include <stdio.h>

//
//
//

#include "cache_vk.h"
#include "assert_vk.h"
#include "host_alloc.h"

//
//
//

void
vk_pipeline_cache_create(VkDevice                            device,
                         VkAllocationCallbacks const *       allocator,
                         char                  const * const name,
                         VkPipelineCache             *       pipeline_cache)
{
  VkPipelineCacheCreateInfo pipeline_cache_info = {
    .sType           = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
    .pNext           = NULL,
    .flags           = 0,
    .initialDataSize = 0,
    .pInitialData    = NULL
  };

  FILE * f    = fopen(name,"rb");
  void * data = NULL;

  if (f != NULL)
    {
      if (fseek(f,0L,SEEK_END) == 0)
        {
          pipeline_cache_info.initialDataSize = ftell(f);

          if (pipeline_cache_info.initialDataSize > 0)
            {
              fseek(f, 0L, SEEK_SET);

              data = vk_host_alloc(allocator,pipeline_cache_info.initialDataSize);

              size_t read_size = fread(data,pipeline_cache_info.initialDataSize,1,f);

              pipeline_cache_info.pInitialData = data;
            }
        }

      fclose(f);
    }

  vk(CreatePipelineCache(device,
                         &pipeline_cache_info,
                         allocator,
                         pipeline_cache));


  if (data != NULL)
    vk_host_free(allocator,data);
}

//
//
//

void
vk_pipeline_cache_destroy(VkDevice                            device,
                          VkAllocationCallbacks const *       allocator,
                          char                  const * const name,
                          VkPipelineCache                     pipeline_cache)
{
  size_t data_size;

  vkGetPipelineCacheData(device,pipeline_cache,&data_size,NULL);

  if (data_size > 0)
    {
      void * data = vk_host_alloc(allocator,data_size);

      vkGetPipelineCacheData(device,pipeline_cache,&data_size,data);

      FILE * f = fopen(name,"wb");

      if (f != NULL)
        {
          fwrite(data,data_size,1,f);
          fclose(f);
        }

      vk_host_free(allocator,data);
    }

  vkDestroyPipelineCache(device,pipeline_cache,allocator);
}

//
//
//
