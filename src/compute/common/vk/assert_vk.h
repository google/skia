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

#include <vulkan/vulkan.h>
#include <stdbool.h>

//
//
//

char const *
vk_get_result_string(VkResult const result);

VkResult
assert_vk(VkResult     const result,
          char const * const file,
          int          const line,
          bool         const abort);

//
//
//

#define vk(...)    assert_vk((vk##__VA_ARGS__), __FILE__, __LINE__, true);
#define vk_ok(err) assert_vk(err,               __FILE__, __LINE__, true);

//
//
//
