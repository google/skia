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

#include "composition.h"
// #include "common.h"
// #include "context.h"

//
// high level composition object
//

skc_err
skc_composition_retain(skc_composition_t composition)
{
  composition->ref_count += 1;

  return SKC_ERR_SUCCESS;
}

skc_err
skc_composition_release(skc_composition_t composition)
{
  composition->release(composition->impl);

  return SKC_ERR_SUCCESS;
}

//
//
//

skc_err
skc_composition_seal(skc_composition_t composition)
{
  //
  // seal the composition
  //
  composition->seal(composition->impl);

  return SKC_ERR_SUCCESS;
}

//
//
//

skc_err
skc_composition_unseal(skc_composition_t composition, bool reset)
{
  //
  // unseal the composition
  //
  composition->unseal(composition->impl,reset);

  return SKC_ERR_SUCCESS;
}

//
//
//

skc_err
skc_composition_place(skc_composition_t    composition,
                      skc_raster_t const * rasters,
                      skc_layer_id const * layer_ids,
                      float        const * txs,
                      float        const * tys,
                      uint32_t             count) // NOTE: A PER-PLACE CLIP IS POSSIBLE
{
  return composition->place(composition->impl,rasters,layer_ids,txs,tys,count);
}

//
//
//

skc_err
skc_composition_get_bounds(skc_composition_t composition, int32_t bounds[4])
{
  //
  // not working yet -- need to think about the semantics
  //
  // Option 1: return tight bounds of entire composition
  // Option 2: ?
  //

  return SKC_ERR_SUCCESS;
}

//
//
//
