/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

//
//
//

#include <stdlib.h>
#include <stdio.h>

//
//
//

#include "assert_vk.h"

//
//
//

#define VK_RESULT_TO_STRING(result) case result: return #result

//
// FIXME -- results and errors
//

char const *
vk_get_result_string(VkResult const result)
{
  switch (result)
    {
      //
      // Results
      //
      VK_RESULT_TO_STRING(VK_SUCCESS);
      VK_RESULT_TO_STRING(VK_NOT_READY);
      VK_RESULT_TO_STRING(VK_TIMEOUT);
      VK_RESULT_TO_STRING(VK_EVENT_SET);
      VK_RESULT_TO_STRING(VK_EVENT_RESET);
      VK_RESULT_TO_STRING(VK_INCOMPLETE);
      //
      // Errors
      //
      VK_RESULT_TO_STRING(VK_ERROR_OUT_OF_HOST_MEMORY);
      VK_RESULT_TO_STRING(VK_ERROR_OUT_OF_DEVICE_MEMORY);
      VK_RESULT_TO_STRING(VK_ERROR_INITIALIZATION_FAILED);
      VK_RESULT_TO_STRING(VK_ERROR_DEVICE_LOST);
      VK_RESULT_TO_STRING(VK_ERROR_MEMORY_MAP_FAILED);
      VK_RESULT_TO_STRING(VK_ERROR_LAYER_NOT_PRESENT);
      VK_RESULT_TO_STRING(VK_ERROR_EXTENSION_NOT_PRESENT);
      VK_RESULT_TO_STRING(VK_ERROR_FEATURE_NOT_PRESENT);
      VK_RESULT_TO_STRING(VK_ERROR_INCOMPATIBLE_DRIVER);
      VK_RESULT_TO_STRING(VK_ERROR_TOO_MANY_OBJECTS);
      VK_RESULT_TO_STRING(VK_ERROR_FORMAT_NOT_SUPPORTED);
      VK_RESULT_TO_STRING(VK_ERROR_FRAGMENTED_POOL);
      VK_RESULT_TO_STRING(VK_ERROR_OUT_OF_POOL_MEMORY);
      VK_RESULT_TO_STRING(VK_ERROR_INVALID_EXTERNAL_HANDLE);
      VK_RESULT_TO_STRING(VK_ERROR_SURFACE_LOST_KHR);
      VK_RESULT_TO_STRING(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
      VK_RESULT_TO_STRING(VK_SUBOPTIMAL_KHR);
      VK_RESULT_TO_STRING(VK_ERROR_OUT_OF_DATE_KHR);
      VK_RESULT_TO_STRING(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);
      VK_RESULT_TO_STRING(VK_ERROR_VALIDATION_FAILED_EXT);
      VK_RESULT_TO_STRING(VK_ERROR_INVALID_SHADER_NV);
      VK_RESULT_TO_STRING(VK_ERROR_FRAGMENTATION_EXT);
      VK_RESULT_TO_STRING(VK_ERROR_NOT_PERMITTED_EXT);

      //
      // Extensions: vk_xyz
      //
    default:
      return "UNKNOWN VULKAN RESULT";
    }
}

//
//
//

VkResult
assert_vk(VkResult const result, char const * const file, int const line, bool const abort)
{
  if (result != VK_SUCCESS)
    {
      char const * const vk_result_str = vk_get_result_string(result);

      fprintf(stderr,
              "\"%s\", line %d: assert_vk( %d ) = \"%s\"",
              file,line,result,vk_result_str);

      if (abort)
        {
          // stop profiling and reset device here if necessary
          exit(result);
        }
    }

  return result;
}

//
//
//
