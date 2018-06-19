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

#include "surface.h"
#include "composition.h"
#include "styling.h"

//
//
//

skc_err
skc_surface_retain(skc_surface_t surface)
{
  surface->ref_count += 1;

  return SKC_ERR_SUCCESS;
}

skc_err
skc_surface_release(skc_surface_t surface)
{
  surface->release(surface->impl);

  return SKC_ERR_SUCCESS;
}

skc_err
skc_surface_clear(skc_surface_t  surface, 
                  float    const rgba[4], 
                  uint32_t const rect[4],
                  void         * fb)
{
  surface->clear(surface->impl,rgba,rect,fb);

  return SKC_ERR_SUCCESS;
}

skc_err
skc_surface_blit(skc_surface_t  surface, 
                 uint32_t const rect[4], 
                 int32_t  const txty[2])
{
  surface->blit(surface->impl,rect,txty);

  return SKC_ERR_SUCCESS;
}

//
//
//

skc_err
skc_surface_render(skc_surface_t                 surface,
                   uint32_t                const clip[4],
                   skc_styling_t                 styling,
                   skc_composition_t             composition,
                   skc_surface_render_pfn_notify notify,
                   void                        * data,
                   void                        * fb)
{
  skc_err err;

  // seal styling -- no dependencies so this will start immediately
  if ((err = skc_styling_seal(styling)) != SKC_ERR_SUCCESS)
    return err;

  // seal composition -- force started
  if ((err = skc_composition_seal(composition)) != SKC_ERR_SUCCESS)
    return err;

  //
  // FIXME -- at some point, we will want non-overlapping clips to be
  // rendered simultaneously. There is plenty of compute for nominal
  // size render tasks so it might not make much a performance
  // improvement.
  //
  surface->render(surface->impl,clip,styling,composition,notify,data,fb);

  return SKC_ERR_SUCCESS;
}

//
//
//
