/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#pragma once

//
// squelch OpenCL 1.2 deprecation warning
//

#ifndef CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#endif

#include <CL/opencl.h>

//
//
//

#include "skc.h"

//
// Minimal OpenCL state needed by the runtime to get started
//

struct skc_runtime_cl
{
  cl_platform_id platform_id;
  cl_device_id   device_id;
  cl_context     context;
  
  struct {
    cl_uint      major;
    cl_uint      minor;
  } version; // sometimes we need to know this at runtime 

  cl_uint        base_align; // base address alignment for subbuffer origins
};

//
//
//

typedef enum skc_cq_type_e {
  SKC_CQ_TYPE_IN_ORDER               = 0,
  SKC_CQ_TYPE_OUT_OF_ORDER           = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,
  SKC_CQ_TYPE_IN_ORDER_PROFILING     = (SKC_CQ_TYPE_IN_ORDER     | CL_QUEUE_PROFILING_ENABLE),
  SKC_CQ_TYPE_OUT_OF_ORDER_PROFILING = (SKC_CQ_TYPE_OUT_OF_ORDER | CL_QUEUE_PROFILING_ENABLE),
} skc_cq_type_e;

//
// safely creates a generic OpenCL target in very few lines
//

skc_err
skc_runtime_cl_create(struct skc_runtime_cl * const runtime_cl,
                      char const            * const target_platform_substring,
                      char const            * const target_device_substring,
                      cl_context_properties         context_properties[]);

skc_err
skc_runtime_cl_dispose(struct skc_runtime_cl * const runtime_cl);

//
// create a command queue with the non-deprecated function
//

cl_command_queue
skc_runtime_cl_create_cq(struct skc_runtime_cl * const runtime_cl, skc_cq_type_e const type);

//
//
//

