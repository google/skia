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

#include <stdint.h>

//
// This structure packages all of the parameters and kernels for a
// target architecture.
//

struct hs_cl_target_config
{
  struct {
    uint8_t   threads_log2;
    uint8_t   width_log2;
    uint8_t   height;
  } slab;

  struct {
    uint8_t   key;
    uint8_t   val;
  } words;

  struct {
    uint8_t   slabs;
  } block;

  struct {
    struct {
      uint8_t scale_min;
      uint8_t scale_max;
    } fm;
    struct {
      uint8_t scale_min;
      uint8_t scale_max;
    } hm;
  } merge;
};

//
//
//

struct hs_cl_target
{
  struct hs_cl_target_config config;
  uint8_t                    program[];
};

//
//
//
