/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#ifndef SKC_ONCE_SKC_TYPES
#define SKC_ONCE_SKC_TYPES

//
//
//

#include <stdint.h>
#include <stdbool.h>

//
//
//

typedef struct skc_context        * skc_context_t;
typedef struct skc_path_builder   * skc_path_builder_t;
typedef struct skc_raster_builder * skc_raster_builder_t;

typedef struct skc_composition    * skc_composition_t;
typedef struct skc_styling        * skc_styling_t;

typedef struct skc_surface        * skc_surface_t;

typedef        uint32_t             skc_path_t;
typedef        uint32_t             skc_raster_t;

typedef        uint32_t             skc_layer_id;
typedef        uint32_t             skc_group_id;

typedef        uint32_t             skc_styling_cmd_t;

typedef        uint64_t             skc_weakref_t;
typedef        skc_weakref_t        skc_transform_weakref_t;
typedef        skc_weakref_t        skc_raster_clip_weakref_t;

typedef        void               * skc_framebuffer_t;

//
//
//

#define SKC_PATH_INVALID     UINT32_MAX
#define SKC_RASTER_INVALID   UINT32_MAX
#define SKC_WEAKREF_INVALID  UINT64_MAX

//
// TRANSFORM LAYOUT: { sx shx tx shy sy ty w0 w1 }
//

//
// RASTER CLIP LAYOUT: { x0, y0, x1, y1 }
//

#endif

//
//
//
