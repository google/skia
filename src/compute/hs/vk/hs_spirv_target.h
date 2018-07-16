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
// This structure packages all of the parameters and SPIR-V kernels
// for a target architecture.
//

struct hs_spirv_target_config
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

  uint8_t     pad[2];
};

static_assert(sizeof(struct hs_spirv_target_config) == 12,
              "modules.words[] must start on a 32-bit boundary");

//
// For now, kernels are appended end-to-end with a leading big-endian
// length followed by a SPIR-V binary.
//
// The entry point for each kernel is "main".
//
// When the tools support packaging multiple named compute shaders in
// one SPIR-V module then reevaluate this encoding.
//

struct hs_spirv_target
{
  struct hs_spirv_target_config config;
  union {
    uint8_t                     bytes[];
    uint32_t                    words[];
  } modules;
};

//
//
//
