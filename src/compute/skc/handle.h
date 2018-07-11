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

#include "types.h"

//
// Add defensive high guard-bit flags to the opaque path and raster
// handles. This is tested once and stripped down to a handle.
//
//   union skc_typed_handle
//   {
//     skc_uint   u32;
//
//     struct {
//       skc_uint handle    : 30;
//       skc_uint is_path   :  1;
//       skc_uint is_raster :  1;
//     };
//     struct {
//       skc_uint na        : 30;
//       skc_uint type      :  2;
//     };
//   }
//

typedef enum skc_typed_handle_type_e
{
  SKC_TYPED_HANDLE_TYPE_IS_PATH   = 0x40000000,
  SKC_TYPED_HANDLE_TYPE_IS_RASTER = 0x80000000
} skc_typed_handle_type_e;

typedef skc_uint skc_typed_handle_t;
typedef skc_uint skc_handle_t;

//
//
//

#define SKC_TYPED_HANDLE_MASK_TYPE     (SKC_TYPED_HANDLE_TYPE_IS_PATH | SKC_TYPED_HANDLE_TYPE_IS_RASTER)

#define SKC_TYPED_HANDLE_TO_HANDLE(h)  ((h) & ~SKC_TYPED_HANDLE_MASK_TYPE)

#define SKC_TYPED_HANDLE_IS_TYPE(h,t)  ((h) & (t))
#define SKC_TYPED_HANDLE_IS_PATH(h)    ((h) & SKC_TYPED_HANDLE_TYPE_IS_PATH)
#define SKC_TYPED_HANDLE_IS_RASTER(h)  ((h) & SKC_TYPED_HANDLE_TYPE_IS_RASTER)

//
//
//
