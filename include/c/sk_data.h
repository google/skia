/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_data_DEFINED
#define sk_data_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_API sk_data_t* sk_data_new_empty();
SK_API sk_data_t* sk_data_new_with_copy(const void* src, size_t length);
SK_API sk_data_t* sk_data_new_from_malloc(const void* memory, size_t length);
SK_API sk_data_t* sk_data_new_subset(const sk_data_t* src, size_t offset, size_t length);

SK_API void sk_data_ref(const sk_data_t*);
SK_API void sk_data_unref(const sk_data_t*);

SK_API size_t sk_data_get_size(const sk_data_t*);
SK_API const void* sk_data_get_data(const sk_data_t*);

SK_C_PLUS_PLUS_END_GUARD

#endif
