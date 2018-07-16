/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#pragma once

//
//
//

#include "skc.h"

//
//
//

skc_err
skc_path_builder_cl_12_create(struct skc_context        * const context,
                              struct skc_path_builder * * const path_builder);

//
//
//

skc_err
skc_raster_builder_cl_12_create(struct skc_context          * const context,
                                struct skc_raster_builder * * const raster_builder);

//
//
//

skc_err
skc_composition_cl_12_create(struct skc_context       * const context,
                             struct skc_composition * * const composition);

//
//
//

skc_err
skc_styling_cl_12_create(struct skc_context   * const context,
                         struct skc_styling * * const styling,
                         uint32_t               const layers_count,
                         uint32_t               const groups_count,
                         uint32_t               const extras_count);

//
//
//

skc_err
skc_surface_cl_12_create(struct skc_context   * const context,
                         struct skc_surface * * const surface);

//
//
//
