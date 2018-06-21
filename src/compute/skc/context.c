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
skc_context_create_cl(skc_context_t * context,
                      cl_context      context_cl,
                      cl_device_id    device_id_cl)
{
  (*context) = malloc(sizeof(**context));

  //
  // FIXME -- we'll clean up context creation by platform later.  For
  // now, just create a CL_12 context.
  //
  skc_err err;

  err = skc_runtime_cl_12_create(*context,context_cl,device_id_cl);

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
