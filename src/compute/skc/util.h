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
//
//

skc_bool skc_is_pow2_uint(skc_uint n);
skc_uint skc_msb_idx_uint(skc_uint n); // 0-based bit position
skc_uint skc_pow2_rd_uint(skc_uint n);
skc_uint skc_pow2_ru_uint(skc_uint n);

//
//
//

