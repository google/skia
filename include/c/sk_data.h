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

/**
    Returns a new empty sk_data_t.  This call must be balanced with a call to
    sk_data_unref().
*/
SK_C_API sk_data_t* sk_data_new_empty();
/**
    Returns a new sk_data_t by copying the specified source data.
    This call must be balanced with a call to sk_data_unref().
*/
SK_C_API sk_data_t* sk_data_new_with_copy(const void* src, size_t length);
/**
    Pass ownership of the given memory to a new sk_data_t, which will
    call free() when the refernce count of the data goes to zero.  For
    example:
        size_t length = 1024;
        void* buffer = malloc(length);
        memset(buffer, 'X', length);
        sk_data_t* data = sk_data_new_from_malloc(buffer, length);
    This call must be balanced with a call to sk_data_unref().
*/
SK_C_API sk_data_t* sk_data_new_from_malloc(const void* memory, size_t length);
/**
    Returns a new sk_data_t using a subset of the data in the
    specified source sk_data_t.  This call must be balanced with a
    call to sk_data_unref().
*/
SK_C_API sk_data_t* sk_data_new_subset(const sk_data_t* src, size_t offset, size_t length);
/**
    Increment the reference count on the given sk_data_t. Must be
    balanced by a call to sk_data_unref().
*/
SK_C_API void sk_data_ref(const sk_data_t*);
/**
    Decrement the reference count. If the reference count is 1 before
    the decrement, then release both the memory holding the sk_data_t
    and the memory it is managing.  New sk_data_t are created with a
    reference count of 1.
*/
SK_C_API void sk_data_unref(const sk_data_t*);
/**
    Returns the number of bytes stored.
*/
SK_C_API size_t sk_data_get_size(const sk_data_t*);
/**
    Returns the pointer to the data.
 */
SK_C_API const void* sk_data_get_data(const sk_data_t*);
/**
    Create a new dataref the file with the specified path.
    If the file cannot be opened, this returns NULL.
*/
SK_C_API sk_data_t* sk_data_new_from_file(const char* path);
/**
    Attempt to read size bytes into a SkData. If the read succeeds, return the data,
    else return NULL. Either way the stream's cursor may have been changed as a result
    of calling read().
*/
SK_C_API sk_data_t* sk_data_new_from_stream(sk_stream_t* stream, size_t length);
/**
    Like data(), returns a read-only ptr into the data, but in this case
    it is cast to uint8_t*, to make it easy to add an offset to it.
*/
SK_C_API const uint8_t* sk_data_get_bytes(const sk_data_t*);

SK_C_API sk_data_t* sk_data_new_with_proc(const void* ptr, size_t length, sk_data_release_proc proc, void* ctx);

SK_C_PLUS_PLUS_END_GUARD

#endif
