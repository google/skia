/*
 * Copyright 2018 Google Inc.
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

cl_int
clFindIdsByName(char const     * const target_platform_substring,
                char const     * const target_device_substring,
                cl_platform_id * const platform_id,
                cl_device_id   * const device_id,
                size_t           const matched_device_name_size,
                char           * const matched_device_name,
                size_t         * const matched_device_name_size_ret,
                bool             const is_verbose);

//
//
//
