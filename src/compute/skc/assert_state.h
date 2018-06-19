/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
//
//

#pragma once

//
//
//

#include "macros.h"

//
//
//

#if 1

#include <assert.h>

#define SKC_ASSERT_STATE_DECLARE(st)            st state
#define SKC_ASSERT_STATE_MEMBER(sp)             SKC_CONCAT(sp,->state)
#define SKC_ASSERT_STATE_INIT(sp,to)            SKC_ASSERT_STATE_MEMBER(sp) = (to)
#define SKC_ASSERT_STATE_TRANSITION(from,to,sp) assert(SKC_ASSERT_STATE_MEMBER(sp) == (from)); SKC_ASSERT_STATE_INIT(sp,to)
#define SKC_ASSERT_STATE_ASSERT(at,sp)          assert(SKC_ASSERT_STATE_MEMBER(sp) == (at))

#else

#define SKC_ASSERT_STATE_DECLARE(st)
#define SKC_ASSERT_STATE_INIT(sp,to)
#define SKC_ASSERT_STATE_TRANSITION(from,to,sp)
#define SKC_ASSERT_STATE_ASSERT(at,sp)

#endif

//
//
//
