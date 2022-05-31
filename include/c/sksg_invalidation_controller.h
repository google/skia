/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sksg_invalidationcontroller_DEFINED
#define sksg_invalidationcontroller_DEFINED

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_C_API sksg_invalidation_controller_t* sksg_invalidation_controller_new();
SK_C_API void sksg_invalidation_controller_delete(sksg_invalidation_controller_t* instance);

SK_C_API void sksg_invalidation_controller_inval(sksg_invalidation_controller_t* instance, sk_rect_t* rect, sk_matrix_t* matrix);
SK_C_API void sksg_invalidation_controller_get_bounds(sksg_invalidation_controller_t* instance, sk_rect_t* bounds);
SK_C_API void sksg_invalidation_controller_begin(sksg_invalidation_controller_t* instance);
SK_C_API void sksg_invalidation_controller_end(sksg_invalidation_controller_t* instance);
SK_C_API void sksg_invalidation_controller_reset(sksg_invalidation_controller_t* instance);

SK_C_PLUS_PLUS_END_GUARD

#endif
