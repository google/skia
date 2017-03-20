/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_string_DEFINED
#define sk_string_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

/**
    Returns a new empty sk_string_t.  This call must be balanced with a call to
    sk_string_destructor().
*/
SK_C_API sk_string_t* sk_string_new_empty(void);
/**
    Returns a new sk_string_t by copying the specified source string, encoded in UTF-8.
    This call must be balanced with a call to sk_string_destructor().
*/
SK_C_API sk_string_t* sk_string_new_with_copy(const char* src, size_t length);

/**
    Deletes the string.
*/
SK_C_API void sk_string_destructor(const sk_string_t*);

/**
    Returns the number of bytes stored in the UTF 8 string. Note that this is the number of bytes, not characters.
*/
SK_C_API size_t sk_string_get_size(const sk_string_t*);
/**
    Returns the pointer to the string.
 */
SK_C_API const char* sk_string_get_c_str(const sk_string_t*);

SK_C_PLUS_PLUS_END_GUARD

#endif
