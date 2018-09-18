/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

// sizeof(x) will return size_t, which is 32-bit on some machines and 64-bit on others.
// We have better testing on 64-bit machines, so force 32-bit machines to behave like 64-bit.
#define SAFE_SIZEOF(x) ((uint64_t)sizeof(x))

// Please do not use sizeof() directly, and size_t only when required.
// (We have no way of enforcing these requests...)

#define ARRAY_COUNT(arr) (int)(SAFE_SIZEOF((arr)) / SAFE_SIZEOF(*(arr)))
