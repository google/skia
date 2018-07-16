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
#include "assert_state.h"

//
//
//

typedef enum skc_path_builder_state_e {

  SKC_PATH_BUILDER_STATE_READY,
  SKC_PATH_BUILDER_STATE_BUILDING

} skc_path_builder_state_e;

//
// FIXME -- we might be able to bury more of this in the impl
//

struct skc_coords_rem_count_line
{
  skc_uint    rem;
  skc_float * coords[4];
};

struct skc_coords_rem_count_quad
{
  skc_uint    rem;
  skc_float * coords[6];
};

struct skc_coords_rem_count_cubic
{
  skc_uint    rem;
  skc_float * coords[8];
};

//
//
//

struct skc_path_builder
{
  struct skc_context              * context;

  struct skc_path_builder_impl    * impl;

  void                           (* begin    )(struct skc_path_builder_impl * const impl);
  void                           (* end      )(struct skc_path_builder_impl * const impl, skc_path_t * const path);
  void                           (* new_line )(struct skc_path_builder_impl * const impl);
  void                           (* new_quad )(struct skc_path_builder_impl * const impl);
  void                           (* new_cubic)(struct skc_path_builder_impl * const impl);
  void                           (* release  )(struct skc_path_builder_impl * const impl);

  struct skc_coords_rem_count_line  line;
  struct skc_coords_rem_count_quad  quad;
  struct skc_coords_rem_count_cubic cubic;

  struct {
    float                           x;
    float                           y;
  } curr[2];

  skc_uint                          refcount;

  SKC_ASSERT_STATE_DECLARE(skc_path_builder_state_e);
};

//
//
//
