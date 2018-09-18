/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <stdint.h>

// 252 of a random shuffle of all possible bytes.
// 252 is evenly divisible by 3 and 4.  Only 192, 10, 241, and 43 are missing.
extern const uint8_t skcms_252_random_bytes[252];
