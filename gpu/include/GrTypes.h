/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#ifndef GrTypes_DEFINED
#define GrTypes_DEFINED

#include "GrConfig.h"

#include <memory.h>
#include <string.h>

/**
 *  Macro to round n up to the next multiple of 4, or return it unchanged if
 *  n is already a multiple of 4
 */
#define GrALIGN4(n)     (((n) + 3) >> 2 << 2)
#define GrIsALIGN4(n)   (((n) & 3) == 0)

template <typename T> const T& GrMin(const T& a, const T& b) {
	return (a < b) ? a : b;
}

template <typename T> const T& GrMax(const T& a, const T& b) {
	return (b < a) ? a : b;
}

// compile time versions of min/max
#define GR_CT_MAX(a, b) (((b) < (a)) ? (a) : (b))
#define GR_CT_MIN(a, b) (((b) < (a)) ? (b) : (a))

/**
 *  divide, rounding up
 */
static inline uint32_t GrUIDivRoundUp(uint32_t x, uint32_t y) {
    return (x + (y-1)) / y;
}
static inline size_t GrSizeDivRoundUp(size_t x, uint32_t y) {
    return (x + (y-1)) / y;
}

/**
 *  align up
 */
static inline uint32_t GrUIAlignUp(uint32_t x, uint32_t alignment) {
    return GrUIDivRoundUp(x, alignment) * alignment;
}
static inline uint32_t GrSizeAlignUp(size_t x, uint32_t alignment) {
    return GrSizeDivRoundUp(x, alignment) * alignment;
}

/**
 * amount of pad needed to align up
 */
static inline uint32_t GrUIAlignUpPad(uint32_t x, uint32_t alignment) {
    return (alignment - x % alignment) % alignment;
}
static inline size_t GrSizeAlignUpPad(size_t x, uint32_t alignment) {
    return (alignment - x % alignment) % alignment;
}

/**
 *  align down
 */
static inline uint32_t GrUIAlignDown(uint32_t x, uint32_t alignment) {
    return (x / alignment) * alignment;
}
static inline uint32_t GrSizeAlignDown(size_t x, uint32_t alignment) {
    return (x / alignment) * alignment;
}

/**
 *  Count elements in an array
 */
#define GR_ARRAY_COUNT(array)  (sizeof(array) / sizeof(array[0]))

//!< allocate a block of memory, will never return NULL
extern void* GrMalloc(size_t bytes);

//!< free block allocated by GrMalloc. ptr may be NULL
extern void GrFree(void* ptr);

static inline void Gr_bzero(void* dst, size_t size) {
    memset(dst, 0, size);
}

///////////////////////////////////////////////////////////////////////////////

/**
 *  Return the number of leading zeros in n
 */
extern int Gr_clz(uint32_t n);

/**
 *  Return true if n is a power of 2
 */
static inline bool GrIsPow2(unsigned n) {
    return n && 0 == (n & (n - 1));
}

/**
 *  Return the next power of 2 >= n.
 */
static inline uint32_t GrNextPow2(uint32_t n) {
    return n ? (1 << (32 - Gr_clz(n - 1))) : 1;
}

///////////////////////////////////////////////////////////////////////////////

/**
 *  16.16 fixed point type
 */
typedef int32_t GrFixed;

#if GR_DEBUG

static inline int16_t GrToS16(intptr_t x) {
    GrAssert((int16_t)x == x);
    return (int16_t)x;
}

#else

#define GrToS16(x)  x

#endif

///////////////////////////////////////////////////////////////////////////////

/**
 *  Use to cast a pointer to a different type, and maintaining strict-aliasing
 */
template <typename Dst> Dst GrTCast(const void* ptr) {
    union {
        const void* src;
        Dst dst;
    } data;
    data.src = ptr;
    return data.dst;
}

///////////////////////////////////////////////////////////////////////////////

/**
 * Type used to describe format of vertices in arrays
 * Values are defined in GrDrawTarget
 */
typedef uint16_t GrVertexLayout;

///////////////////////////////////////////////////////////////////////////////

// this is included only to make it easy to use this debugging facility
#include "GrInstanceCounter.h"

#endif
