/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#pragma once

//
//
//

#include <CL/opencl.h>
#include <stdbool.h>

//
//
//

char const *
cl_get_error_string(cl_int const err);

cl_int
assert_cl(cl_int       const code,
          char const * const file,
          int          const line,
          bool         const abort);

//
//
//

#define cl(...)    assert_cl((cl##__VA_ARGS__), __FILE__, __LINE__, true)
#define cl_ok(err) assert_cl(err,               __FILE__, __LINE__, true)

//
//
//

void
cl_get_event_info(cl_event                event,
                  cl_int          * const status,
                  cl_command_type * const type);

char const *
cl_get_event_command_status_string(cl_int const status);

char const *
cl_get_event_command_type_string(cl_command_type const type);

//
//
//
