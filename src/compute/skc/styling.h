/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#pragma once

//
//
//

#include "types.h"

//
// STYLING
//

struct skc_styling
{
  struct skc_context      * context;

  struct skc_styling_impl * impl;

  void                   (* seal   )(struct skc_styling_impl * const impl);
  void                   (* unseal )(struct skc_styling_impl * const impl, skc_bool const block);
  void                   (* release)(struct skc_styling_impl * const impl);

  skc_int                   ref_count;

  struct {
    union skc_layer_node  * extent;
    skc_uint                size;
    skc_uint                count;
  } layers;

  struct {
    struct skc_group_node * extent;
    skc_uint                size;
    skc_uint                count;
  } groups;

  struct {
    union skc_styling_cmd * extent;
    skc_uint                size;
    skc_uint                count;
  } extras;
};

//
//
//
