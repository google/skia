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

#include "skc.h"

//
//
//

skc_err
skc_assert_skc(void const * const ptr, char const * const file, int const line, bool const abort);

//
//
//

#define skc(...)    skc_assert_skc((skc_##__VA_ARGS__), __FILE__, __LINE__, true);
#define skc_ok(err) skc_assert_skc((err              ), __FILE__, __LINE__, true);

//
//
//
