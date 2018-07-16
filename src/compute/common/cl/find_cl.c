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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//
//
//

#include "find_cl.h"
#include "assert_cl.h"
#include "../macros.h"

//
// search platforms and devices for a match
//

cl_int
clFindIdsByName(char const     * const target_platform_substring,
                char const     * const target_device_substring,
                cl_platform_id * const platform_id,
                cl_device_id   * const device_id,
                size_t           const matched_device_name_size,
                char           * const matched_device_name,
                size_t         * const matched_device_name_size_ret,
                bool             const is_verbose)
{
  bool match_platform=false, match_device=false;

  //
  // get number of platforms
  //
  cl_uint platform_count;

  cl(GetPlatformIDs(0,NULL,&platform_count));

  cl_platform_id * const platform_ids = ALLOCA_MACRO(sizeof(*platform_ids) * platform_count);

  cl(GetPlatformIDs(platform_count,platform_ids,NULL));

  //
  // search platforms
  //
  for (cl_uint ii=0; ii<platform_count; ii++)
    {
      size_t platform_name_size;

      cl(GetPlatformInfo(platform_ids[ii],
                         CL_PLATFORM_NAME,
                         0,
                         NULL,
                         &platform_name_size));

      char * const platform_name = ALLOCA_MACRO(platform_name_size);

      cl(GetPlatformInfo(platform_ids[ii],
                         CL_PLATFORM_NAME,
                         platform_name_size,
                         platform_name,
                         NULL));


      if (!match_platform && (strstr(platform_name,target_platform_substring) != NULL))
        {
          match_platform = true;
          *platform_id   = platform_ids[ii];
        }

      if (is_verbose) {
        fprintf(stdout,"%2u: %s\n",ii,platform_name);
      }

      //
      // find devices for current platform
      //
      cl_uint device_count;
      cl_int  cl_err;

      cl_err = clGetDeviceIDs(platform_ids[ii],
                              CL_DEVICE_TYPE_ALL,
                              0,
                              NULL,
                              &device_count);

      cl_device_id * const device_ids = ALLOCA_MACRO(sizeof(*device_ids) * device_count);

      cl_err = clGetDeviceIDs(platform_ids[ii],
                              CL_DEVICE_TYPE_ALL,
                              device_count,
                              device_ids,
                              NULL);

      if (cl_err != CL_DEVICE_NOT_FOUND)
        cl_ok(cl_err);

      for (cl_uint jj=0; jj<device_count; jj++)
        {
          size_t device_name_size;
          size_t driver_version_size;

          cl(GetDeviceInfo(device_ids[jj],
                           CL_DEVICE_NAME,
                           0,
                           NULL,
                           &device_name_size));

          cl(GetDeviceInfo(device_ids[jj],
                           CL_DRIVER_VERSION,
                           0,
                           NULL,
                           &driver_version_size));

          char * const device_name    = ALLOCA_MACRO(device_name_size);
          char * const driver_version = ALLOCA_MACRO(driver_version_size);

          cl(GetDeviceInfo(device_ids[jj],
                           CL_DEVICE_NAME,
                           device_name_size,
                           device_name,
                           NULL));

          cl(GetDeviceInfo(device_ids[jj],
                           CL_DRIVER_VERSION,
                           driver_version_size,
                           driver_version,
                           NULL));

          if (!match_device && match_platform && (strstr(device_name,target_device_substring) != NULL))
            {
              match_device = true;
              *device_id   = device_ids[jj];

              if (matched_device_name != NULL)
                {
                  size_t bytes = 0;

                  if (matched_device_name_size >= 1)
                    matched_device_name[matched_device_name_size-1] = 0;

                  if (matched_device_name_size > 1)
                    {
                      bytes = MIN_MACRO(device_name_size,matched_device_name_size-1);

                      memcpy(matched_device_name,device_name,bytes);
                    }

                  if (matched_device_name_size_ret != NULL)
                    *matched_device_name_size_ret = bytes;
                }

              if (is_verbose) {
                fprintf(stdout," >>>");
              }
            }
          else if (is_verbose)
            {
              fprintf(stdout,"    ");
            }

          if (is_verbose)
            {
              fprintf(stdout,
                      " %1u: %s [ %s ]\n",
                      jj,
                      device_name,
                      driver_version);
            }
        }
    }

  if (is_verbose) {
    fprintf(stdout,"\n");
  }

  //
  // get target platform and device
  //
  if (!match_platform)
    {
      if (is_verbose)
        fprintf(stderr,"no match for target platform substring %s\n",target_platform_substring);

      return CL_INVALID_PLATFORM;
    }
  if (!match_device)
    {
      if (is_verbose)
        fprintf(stderr,"no match for target device substring %s\n",target_device_substring);

      return CL_DEVICE_NOT_FOUND;
    }

  return CL_SUCCESS;
}

//
//
//
