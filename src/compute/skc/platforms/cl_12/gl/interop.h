/*
 * Copyright 2018 Google Inc.
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

struct skc_interop *
skc_interop_create();

void
skc_interop_destroy(struct skc_interop * interop);

cl_context_properties
skc_interop_get_wgl_context();

cl_context_properties
skc_interop_get_wgl_dc();

void
skc_interop_set_cl_context(struct skc_interop * interop,
                           cl_context           context);

void
skc_interop_poll(struct skc_interop         * interop,
                 struct skc_transform_stack * ts);

bool
skc_interop_should_exit(struct skc_interop * interop);

skc_framebuffer_t
skc_interop_get_framebuffer();

void
skc_interop_blit(struct skc_interop * interop);

//
// glClear() not working so explicitly clearing the cl_mem buffer
//

void
skc_interop_get_dim(struct skc_interop * interop,
                    uint32_t             dim[2]);

//
//
//
