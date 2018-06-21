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
// FIXME -- SPECIALIZE SURFACE RENDER
//

#if 0

//
// SURFACE RENDER
//

typedef void (*skc_surface_render_pfn_notify)(skc_surface_t     surface,
                                              skc_styling_t     styling,
                                              skc_composition_t composition,
                                              void            * data);
skc_err
skc_surface_render(skc_surface_t                 surface,
                   uint32_t                const clip[4],
                   skc_styling_t                 styling,
                   skc_composition_t             composition,
                   skc_surface_render_pfn_notify notify,
                   void                        * data,
                   void                        * fb); // FIXME FIXME

#endif

//
//
//

#endif

//
//
//
