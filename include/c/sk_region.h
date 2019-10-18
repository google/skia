/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2016 Bluebeam Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_region_DEFINED
#define sk_region_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_C_API sk_region_t* sk_region_new(void);
SK_C_API sk_region_t* sk_region_new2(const sk_region_t* region);
SK_C_API void sk_region_delete(sk_region_t* cpath); 
SK_C_API bool sk_region_contains(sk_region_t* r, const sk_region_t* region); 
SK_C_API bool sk_region_contains2(sk_region_t* r, int x, int y);
SK_C_API bool sk_region_intersects_rect(sk_region_t* r, const sk_irect_t* rect); 
SK_C_API bool sk_region_intersects(sk_region_t* r, const sk_region_t* src); 
SK_C_API bool sk_region_set_path(sk_region_t* dst, const sk_path_t* t, const sk_region_t* clip);
SK_C_API bool sk_region_set_rect(sk_region_t* dst, const sk_irect_t* rect);
SK_C_API bool sk_region_set_region(sk_region_t* r, const sk_region_t* region);
SK_C_API bool sk_region_op(sk_region_t* dst, int left, int top, int right, int bottom, sk_region_op_t op);
SK_C_API bool sk_region_op2(sk_region_t* dst, sk_region_t* src, sk_region_op_t op);
SK_C_API void sk_region_get_bounds(sk_region_t* r, sk_irect_t* rect);

SK_C_PLUS_PLUS_END_GUARD

#endif
