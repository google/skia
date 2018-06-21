/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#ifndef SKC_ONCE_SKC_ERR
#define SKC_ONCE_SKC_ERR

//
//
//

typedef enum skc_err {

  SKC_ERR_SUCCESS                           = 0,

  SKC_ERR_API_BASE                          = 10000,

  SKC_ERR_NOT_IMPLEMENTED                   = SKC_ERR_API_BASE,

  SKC_ERR_POOL_EMPTY,

  SKC_ERR_CONDVAR_WAIT,

  SKC_ERR_LAYER_ID_INVALID,
  SKC_ERR_LAYER_NOT_EMPTY,

  SKC_ERR_TRANSFORM_WEAKREF_INVALID,
  SKC_ERR_STROKE_STYLE_WEAKREF_INVALID,

  SKC_ERR_COMMAND_NOT_READY,
  SKC_ERR_COMMAND_NOT_COMPLETED,
  SKC_ERR_COMMAND_NOT_STARTED,

  SKC_ERR_COMMAND_NOT_READY_OR_COMPLETED,

  SKC_ERR_COMPOSITION_SEALED,
  SKC_ERR_STYLING_SEALED,

  SKC_ERR_HANDLE_INVALID,
  SKC_ERR_HANDLE_OVERFLOW,

  SKC_ERR_COUNT

} skc_err;

//
//
//

#endif

//
//
//
