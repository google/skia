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

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

// sk_region_t

SK_C_API sk_region_t* sk_region_new(void);
SK_C_API void sk_region_delete(sk_region_t* r);
SK_C_API bool sk_region_is_empty(const sk_region_t* r);
SK_C_API bool sk_region_is_rect(const sk_region_t* r);
SK_C_API bool sk_region_is_complex(const sk_region_t* r);
SK_C_API void sk_region_get_bounds(const sk_region_t* r, sk_irect_t* rect);
SK_C_API bool sk_region_get_boundary_path(const sk_region_t* r, sk_path_t* path);
SK_C_API bool sk_region_set_empty(sk_region_t* r);
SK_C_API bool sk_region_set_rect(sk_region_t* r, const sk_irect_t* rect);
SK_C_API bool sk_region_set_rects(sk_region_t* r, const sk_irect_t* rects, int count);
SK_C_API bool sk_region_set_region(sk_region_t* r, const sk_region_t* region);
SK_C_API bool sk_region_set_path(sk_region_t* r, const sk_path_t* t, const sk_region_t* clip);
SK_C_API bool sk_region_intersects_rect(const sk_region_t* r, const sk_irect_t* rect);
SK_C_API bool sk_region_intersects(const sk_region_t* r, const sk_region_t* src);
SK_C_API bool sk_region_contains_point(const sk_region_t* r, int x, int y);
SK_C_API bool sk_region_contains_rect(const sk_region_t* r, const sk_irect_t* rect);
SK_C_API bool sk_region_contains(const sk_region_t* r, const sk_region_t* region);
SK_C_API bool sk_region_quick_contains(const sk_region_t* r, const sk_irect_t* rect);
SK_C_API bool sk_region_quick_reject_rect(const sk_region_t* r, const sk_irect_t* rect);
SK_C_API bool sk_region_quick_reject(const sk_region_t* r, const sk_region_t* region);
SK_C_API void sk_region_translate(sk_region_t* r, int x, int y);
SK_C_API bool sk_region_op_rect(sk_region_t* r, const sk_irect_t* rect, sk_region_op_t op);
SK_C_API bool sk_region_op(sk_region_t* r, const sk_region_t* region, sk_region_op_t op);

// sk_region_iterator_t

SK_C_API sk_region_iterator_t* sk_region_iterator_new(const sk_region_t* region);
SK_C_API void sk_region_iterator_delete(sk_region_iterator_t* iter);
SK_C_API bool sk_region_iterator_rewind(sk_region_iterator_t* iter);
SK_C_API bool sk_region_iterator_done(const sk_region_iterator_t* iter);
SK_C_API void sk_region_iterator_next(sk_region_iterator_t* iter);
SK_C_API void sk_region_iterator_rect(const sk_region_iterator_t* iter, sk_irect_t* rect);

// sk_region_cliperator_t

SK_C_API sk_region_cliperator_t* sk_region_cliperator_new(const sk_region_t* region, const sk_irect_t* clip);
SK_C_API void sk_region_cliperator_delete(sk_region_cliperator_t* iter);
SK_C_API bool sk_region_cliperator_done(sk_region_cliperator_t* iter);
SK_C_API void sk_region_cliperator_next(sk_region_cliperator_t* iter);
SK_C_API void sk_region_cliperator_rect(const sk_region_cliperator_t* iter, sk_irect_t* rect);

// sk_region_spanerator_t

SK_C_API sk_region_spanerator_t* sk_region_spanerator_new(const sk_region_t* region, int y, int left, int right);
SK_C_API void sk_region_spanerator_delete(sk_region_spanerator_t* iter);
SK_C_API bool sk_region_spanerator_next(sk_region_spanerator_t* iter, int* left, int* right);

SK_C_PLUS_PLUS_END_GUARD

#endif
