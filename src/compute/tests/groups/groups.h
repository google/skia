/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
//
//

#pragma once

//
//
//

#include <skc/skc.h>

//
//
//

void
groups_toggle();

//
//
//

skc_path_t *
groups_paths_decode(skc_path_builder_t pb);

void
groups_paths_release(skc_context_t      context,
                     skc_path_t * const paths);

//
//
//

skc_raster_t *
groups_rasters_decode(struct ts_transform_stack * const ts,
                      skc_path_t const          * const paths,
                      skc_raster_builder_t              rb);

void
groups_rasters_release(skc_context_t        context,
                       skc_raster_t * const rasters);
//
//
//

void
groups_rewind();

//
//
//

uint32_t
groups_layer_count();

void
groups_layers_decode(skc_raster_t const * const rasters,
                     skc_composition_t          composition,
                     skc_styling_t              styling,
                     bool const                 is_srgb);

//
//
//
