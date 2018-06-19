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

#include <stdlib.h>
#include <assert.h> // FIXME -- replace with an SKC assert for non-debug builds

#include "skc.h"
#include "context.h"

//
// FIXME -- THE RUNTIME AND DEVICE WILL DYNAMICALLY LOADED
//

// temporarily
#include "runtime_cl_12.h"

//
// CONTEXT
//

skc_err
skc_context_create(skc_context_t       * context,
                   char          const * target_platform_substring,
                   char          const * target_device_substring,
                   intptr_t              context_properties[])
{
  (*context) = malloc(sizeof(**context));

  //
  // FIXME -- don't directly grab a CL runtime but for now juts create
  // the CL_12 runtime here
  //
  skc_err err;

  err = skc_runtime_cl_12_create(*context,
                                 target_platform_substring,
                                 target_device_substring,
                                 context_properties);
  return err;
}

skc_err
skc_context_retain(skc_context_t context)
{
  return SKC_ERR_SUCCESS;
}


skc_err
skc_context_release(skc_context_t context)
{
  skc_err err = skc_runtime_cl_12_dispose(context);

  free(context);

  return err;
}


skc_err
skc_context_reset(skc_context_t context)
{
  return SKC_ERR_SUCCESS;
}

//
//
//

bool
skc_context_yield(skc_context_t context)
{
  return context->yield(context->runtime);
}

void
skc_context_wait(skc_context_t context)
{
  context->wait(context->runtime);
}

//
//
//

skc_err
skc_path_builder_create(skc_context_t context, skc_path_builder_t * path_builder)
{
  return context->path_builder(context,path_builder);
}

skc_err
skc_path_retain(skc_context_t context, skc_path_t const * paths, uint32_t count)
{
  return context->path_retain(context->runtime,paths,count);
}

skc_err
skc_path_release(skc_context_t context, skc_path_t const * paths, uint32_t count)
{
  return context->path_release(context->runtime,paths,count);
}

skc_err
skc_path_flush(skc_context_t context, skc_path_t const * paths, uint32_t count)
{
  return context->path_flush(context->runtime,paths,count);
}

//
//
//

skc_err
skc_raster_builder_create(skc_context_t context, skc_raster_builder_t * raster_builder)
{
  return context->raster_builder(context,raster_builder);
}

skc_err
skc_raster_retain(skc_context_t context, skc_raster_t const * rasters, uint32_t count)
{
  return context->raster_retain(context->runtime,rasters,count);
}

skc_err
skc_raster_release(skc_context_t context, skc_raster_t const * rasters, uint32_t count)
{
  return context->raster_release(context->runtime,rasters,count);
}

skc_err
skc_raster_flush(skc_context_t context, skc_raster_t const * rasters, uint32_t count)
{
  return context->raster_flush(context->runtime,rasters,count);
}

//
//
//

skc_err
skc_styling_create(skc_context_t   context,
                   skc_styling_t * styling,
                   uint32_t        layers_count,
                   uint32_t        groups_count,
                   uint32_t        extras_count)
{
  return context->styling(context,
                          styling,
                          layers_count,
                          groups_count,
                          extras_count);
}

skc_err
skc_composition_create(skc_context_t context, skc_composition_t * composition)
{
  return context->composition(context,composition);
}


skc_err
skc_surface_create(skc_context_t context, skc_surface_t * surface)
{
  return context->surface(context,surface);
}

//
//
//
