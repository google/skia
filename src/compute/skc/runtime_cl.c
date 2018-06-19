/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
//
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

//
//
//

#include "runtime_cl.h"
#include "common/cl/assert_cl.h"

//
//
//

static is_verbose = true;

//
// FIXME -- all variable length device queries need to start querying
// the parameter's return size before getting its value
//
// FIXME -- this is now handled by the common/cl/find.* routine
//

union skc_cl_device_version {
  struct {
    cl_uchar opencl_space[7]; // "OpenCL_"
    cl_uchar major;
    cl_uchar dot;
    cl_uchar minor;
#if 1 // Intel NEO requires at least 16 bytes
    cl_uchar space;
    cl_uchar vendor[32];
#endif
  };
  struct {
    cl_uchar aN[];
  };
};

typedef cl_bitfield cl_diagnostic_verbose_level_intel;

#define CL_CONTEXT_SHOW_DIAGNOSTICS_INTEL           0x4106
#define CL_CONTEXT_DIAGNOSTICS_LEVEL_BAD_INTEL      0x2
#define CL_CONTEXT_DIAGNOSTICS_LEVEL_GOOD_INTEL     0x1
#define CL_CONTEXT_DIAGNOSTICS_LEVEL_NEUTRAL_INTEL  0x4

static
void 
CL_CALLBACK 
skc_context_callback(char const * error, void const * info, size_t size, void * user)
{
  if (info != NULL )
    {
      fprintf(stderr,"%s\n",error);
    }
}

//
//
//

skc_err
skc_runtime_cl_create(struct skc_runtime_cl * const runtime_cl,
                      char const            * const target_platform_substring,
                      char const            * const target_device_substring,
                      cl_context_properties         context_properties[])
{
  skc_err err = SKC_ERR_SUCCESS;
  
  //
  // search available devices for a match
  //
#define PLATFORM_IDS_MAX         16
#define DEVICE_IDS_MAX           16
#define PLATFORM_NAME_SIZE_MAX   64
#define DEVICE_NAME_SIZE_MAX     64
#define DRIVER_VERSION_SIZE_MAX  64

  cl_int         cl_err;

  cl_platform_id platform_ids[PLATFORM_IDS_MAX];
  cl_device_id   device_ids  [PLATFORM_IDS_MAX][DEVICE_IDS_MAX];

  cl_uint        platform_count;
  cl_uint        device_count[PLATFORM_IDS_MAX];
  
  cl_uint        platform_idx = UINT32_MAX, device_idx = UINT32_MAX;

  bool           match = false; // find _first_ match

  //
  // get number of platforms
  //
  cl(GetPlatformIDs(PLATFORM_IDS_MAX,platform_ids,&platform_count));

  //
  // search platforms
  //
  for (cl_uint ii=0; ii<platform_count; ii++)
    {
      char platform_name[PLATFORM_NAME_SIZE_MAX];

      cl(GetPlatformInfo(platform_ids[ii],
                         CL_PLATFORM_NAME,
                         sizeof(platform_name),
                         platform_name,
                         NULL));

      if (!match && (strstr(platform_name,target_platform_substring) != NULL)) 
        {
          platform_idx = ii;
        }

      if (is_verbose) {
        fprintf(stdout,"%2u: %s\n",ii,platform_name);
      }

      cl_err = clGetDeviceIDs(platform_ids[ii],
                              CL_DEVICE_TYPE_ALL,
                              DEVICE_IDS_MAX,
                              device_ids[ii],
                              device_count+ii);

      if (cl_err != CL_DEVICE_NOT_FOUND)
        cl_ok(cl_err);

      for (cl_uint jj=0; jj<device_count[ii]; jj++)
        {
          char                        device_name[DEVICE_NAME_SIZE_MAX];
          union skc_cl_device_version device_version;
          cl_uint                     device_align_bits;
          char                        driver_version[DRIVER_VERSION_SIZE_MAX];

          cl(GetDeviceInfo(device_ids[ii][jj],
                           CL_DEVICE_NAME,
                           sizeof(device_name),
                           device_name,
                           NULL));

          // FIXME -- some of these variable length parameters should
          // use the "size the param before reading" idiom
          cl(GetDeviceInfo(device_ids[ii][jj],
                           CL_DEVICE_VERSION,
                           sizeof(device_version),
                           device_version.aN,
                           NULL));

          cl(GetDeviceInfo(device_ids[ii][jj],
                           CL_DEVICE_MEM_BASE_ADDR_ALIGN,
                           sizeof(device_align_bits),
                           &device_align_bits,
                           NULL));
          
          cl_uint const base_align = device_align_bits / 8; // bytes

          cl(GetDeviceInfo(device_ids[ii][jj],
                           CL_DRIVER_VERSION,
                           sizeof(driver_version),
                           driver_version,
                           NULL));
          
          if (!match && (platform_idx == ii) && (strstr(device_name,target_device_substring) != NULL))
            {
              match      = true;
              device_idx = jj;

              runtime_cl->version.major = device_version.major - 48;
              runtime_cl->version.minor = device_version.minor - 48;
              runtime_cl->base_align    = base_align;

              if (is_verbose) {
                fprintf(stdout," >>>");
              }
            }
          else if (is_verbose) 
            {
              fprintf(stdout,"    ");
            }

          if (is_verbose) {
            fprintf(stdout,
                    " %1u: %s [ %s ] [ %s ] [ %u ]\n",
                    jj,
                    device_name,
                    device_version.aN,
                    driver_version,
                    base_align);
          }
        }
    }

  if (is_verbose) {
    fprintf(stdout,"\n");
  }

  //
  // get target platform and device
  //
  if (platform_idx >= platform_count)
    {
      fprintf(stderr,"no match for target platform substring %s\n",target_platform_substring);
      exit(EXIT_FAILURE);
    }
  if (device_idx >= device_count[platform_idx])
    {
      fprintf(stderr,"no match for target device substring %s\n",target_device_substring);
      exit(EXIT_FAILURE);
    }

  runtime_cl->platform_id = platform_ids[platform_idx];
  runtime_cl->device_id   = device_ids  [platform_idx][device_idx];

  //
  // create context
  //

#if 0
  cl_context_properties context_properties[] = 
    { 
      CL_CONTEXT_PLATFORM,(cl_context_properties)runtime_cl->platform_id,
      0 
    };
#else
  context_properties[1] = (cl_context_properties)runtime_cl->platform_id;
#endif

  runtime_cl->context = clCreateContext(context_properties,
                                    1,
                                    &runtime_cl->device_id,
                                    skc_context_callback,
                                    NULL,
                                    &cl_err);
  cl_ok(cl_err);

  //
  // get device name, driver version, and unified memory flag
  //
  if (is_verbose)
    {
      char                       device_name[DEVICE_NAME_SIZE_MAX];
      char                       driver_version[DRIVER_VERSION_SIZE_MAX];
      cl_bool                    device_is_unified; 
      cl_device_svm_capabilities svm_caps;
      size_t                     printf_buffer_size;

      cl(GetDeviceInfo(runtime_cl->device_id,
                       CL_DEVICE_NAME,
                       sizeof(device_name),
                       device_name,
                       NULL));

      cl(GetDeviceInfo(runtime_cl->device_id,
                       CL_DRIVER_VERSION,
                       sizeof(driver_version),
                       driver_version,
                       NULL));

      cl(GetDeviceInfo(runtime_cl->device_id,
                       CL_DEVICE_HOST_UNIFIED_MEMORY,
                       sizeof(device_is_unified),
                       &device_is_unified,
                       NULL));

      cl(GetDeviceInfo(runtime_cl->device_id,
                       CL_DEVICE_SVM_CAPABILITIES,
                       sizeof(svm_caps),
                       &svm_caps,
                       0));

      cl(GetDeviceInfo(runtime_cl->device_id,
                       CL_DEVICE_PRINTF_BUFFER_SIZE,
                       sizeof(printf_buffer_size),
                       &printf_buffer_size,
                       NULL));

      fprintf(stderr,
              "CL_DEVICE_SVM_COARSE_GRAIN_BUFFER  %c\n"
              "CL_DEVICE_SVM_FINE_GRAIN_BUFFER    %c\n"
              "CL_DEVICE_SVM_FINE_GRAIN_SYSTEM    %c\n"
              "CL_DEVICE_SVM_ATOMICS              %c\n"
              "CL_DEVICE_PRINTF_BUFFER_SIZE       %zu\n\n",
              svm_caps & CL_DEVICE_SVM_COARSE_GRAIN_BUFFER ? '*' : '-',
              svm_caps & CL_DEVICE_SVM_FINE_GRAIN_BUFFER   ? '*' : '-',
              svm_caps & CL_DEVICE_SVM_FINE_GRAIN_SYSTEM   ? '*' : '-',
              svm_caps & CL_DEVICE_SVM_ATOMICS             ? '*' : '-',
              printf_buffer_size);
    }

  return err;
}

//
//
//

skc_err
skc_runtime_cl_dispose(struct skc_runtime_cl * const runtime_cl)
{
  // FIXME
  printf("%s incomplete!\n",__func__);

  return SKC_ERR_SUCCESS;
}

//
//
//

cl_command_queue
skc_runtime_cl_create_cq(struct skc_runtime_cl * const runtime_cl, skc_cq_type_e const type)
{
  cl_command_queue cq;

  if (runtime_cl->version.major < 2)
    {
      //
      // <= OpenCL 1.2
      //
      cl_int cl_err;

      cq = clCreateCommandQueue(runtime_cl->context,
                                runtime_cl->device_id,
                                (cl_command_queue_properties)type,
                                &cl_err); cl_ok(cl_err);  
    }
  else
    {
      //
      // >= OpenCL 2.0
      //
      cl_int                    cl_err;
      cl_queue_properties const queue_properties[] = {
        CL_QUEUE_PROPERTIES,(cl_queue_properties)type,0
      };

      cq = clCreateCommandQueueWithProperties(runtime_cl->context,
                                              runtime_cl->device_id,
                                              queue_properties,
                                              &cl_err); cl_ok(cl_err);
    }

  return cq;
}

//
//
//

