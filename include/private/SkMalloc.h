/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMalloc_DEFINED
#define SkMalloc_DEFINED

#include <cstddef>
#include <cstring>

#include "SkPreConfig.h"

/*
    memory wrappers to be implemented by the porting layer (platform)
*/

enum {
    SK_MALLOC_TEMP  = 0x01, //!< hint to sk_malloc that the requested memory will be freed in the scope of the stack frame
    SK_MALLOC_THROW = 0x02  //!< instructs sk_malloc to call sk_throw if the memory cannot be allocated.
};
/** Return a block of memory (at least 4-byte aligned) of at least the
    specified size. If the requested memory cannot be returned, either
    return null (if SK_MALLOC_TEMP bit is clear) or throw an exception
    (if SK_MALLOC_TEMP bit is set). To free the memory, call sk_free().
*/
SK_API extern void* sk_malloc_flags(size_t size, unsigned flags);
/** Same as sk_malloc(), but hard coded to pass SK_MALLOC_THROW as the flag
*/
SK_API extern void* sk_malloc_throw(size_t size);
/** Same as standard realloc(), but this one never returns null on failure. It will throw
    an exception if it fails.
*/
SK_API extern void* sk_realloc_throw(void* buffer, size_t size);
/** Free memory returned by sk_malloc(). It is safe to pass null.
*/
SK_API extern void sk_free(void*);

/** Much like calloc: returns a pointer to at least size zero bytes, or NULL on failure.
 */
SK_API extern void* sk_calloc(size_t size);

/** Same as sk_calloc, but throws an exception instead of returning NULL on failure.
 */
SK_API extern void* sk_calloc_throw(size_t size);

/** Called internally if we run out of memory. The platform implementation must
    not return, but should either throw an exception or otherwise exit.
*/
SK_API extern void sk_out_of_memory(void);

// bzero is safer than memset, but we can't rely on it, so... sk_bzero()
static inline void sk_bzero(void* buffer, size_t size) {
    // Please c.f. sk_careful_memcpy.  It's undefined behavior to call memset(null, 0, 0).
    if (size) {
        memset(buffer, 0, size);
    }
}

/**
 *  sk_careful_memcpy() is just like memcpy(), but guards against undefined behavior.
 *
 * It is undefined behavior to call memcpy() with null dst or src, even if len is 0.
 * If an optimizer is "smart" enough, it can exploit this to do unexpected things.
 *     memcpy(dst, src, 0);
 *     if (src) {
 *         printf("%x\n", *src);
 *     }
 * In this code the compiler can assume src is not null and omit the if (src) {...} check,
 * unconditionally running the printf, crashing the program if src really is null.
 * Of the compilers we pay attention to only GCC performs this optimization in practice.
 */
static inline void* sk_careful_memcpy(void* dst, const void* src, size_t len) {
    // When we pass >0 len we had better already be passing valid pointers.
    // So we just need to skip calling memcpy when len == 0.
    if (len) {
        memcpy(dst,src,len);
    }
    return dst;
}

#endif  // SkMalloc_DEFINED
