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
skc_surface_clear(skc_surface_t     surface, 
                  skc_framebuffer_t fb,
                  float       const rgba[4], 
                  uint32_t    const rect[4])
{
  surface->clear(surface->impl,fb,rgba,rect);

  return SKC_ERR_SUCCESS;
}

//
//
//

skc_err
skc_surface_render(skc_surface_t             surface,
                   skc_styling_t             styling,
                   skc_composition_t         composition,
                   skc_framebuffer_t         fb,
                   uint32_t            const clip[4],
                   skc_surface_render_notify notify,
                   void                    * data)
{
  skc_err err;

  // seal styling -- no dependencies so this will start immediately
  if ((err = skc_styling_seal(styling)) != SKC_ERR_SUCCESS)
    return err;

  // seal composition -- force started
  if ((err = skc_composition_seal(composition)) != SKC_ERR_SUCCESS)
    return err;

  //
  // NOTE: there is purposefully no guard against any of the following
  // use cases:
  //
  //   - Simultaneous renders to different frambuffers.
  //
  //   - Simultaneous renders with potentially overlapping clips to
  //     the same framebuffer.
  //
  surface->render(surface->impl,styling,composition,fb,clip,notify,data);

  return SKC_ERR_SUCCESS;
}

//
//
//
