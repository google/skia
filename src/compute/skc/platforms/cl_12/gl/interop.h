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

void
skc_interop_init(GLFWwindow * * window);

void
skc_interop_register(skc_context_t context);

void
skc_interop_poll(GLFWwindow                 * window,
                 struct skc_transform_stack * ts);

void *
skc_interop_get_fb(GLFWwindow * window);

void
skc_interop_get_dim(uint32_t dim[2]);

void
skc_interop_blit(GLFWwindow * window);

//
//
//
