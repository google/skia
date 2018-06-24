/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#ifndef SKC_ONCE_SKC_CREATE_CL
#define SKC_ONCE_SKC_CREATE_CL

//
//
//

#ifdef __APPLE__
#include "OpenCL/opencl.h"
#else
#include "CL/opencl.h"
#endif

//
//
//

#include "skc.h"

//
// CONTEXT CREATION
//

skc_err
skc_context_create_cl(skc_context_t * context,
                      cl_context      context_cl,
                      cl_device_id    device_id_cl);

//
// SURFACE RENDER FRAMEBUFFER TYPES
//

typedef enum skc_framebuffer_cl_mem_type {
  SKC_FRAMEBUFFER_CL_IMAGE2D,
  SKC_FRAMEBUFFER_CL_GL_RENDERBUFFER,
  SKC_FRAMEBUFFER_CL_GL_TEXTURE
} skc_framebuffer_cl_mem_type;

struct skc_framebuffer_cl
{
  skc_framebuffer_cl_mem_type type;
  cl_mem                      mem;
  struct skc_interop        * interop;
  void                     (* post_render)(struct skc_interop * interop);
};

//
//
//

#endif

//
//
//
