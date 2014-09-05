/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTypes_DEFINED
#define SkTypes_DEFINED

#include "SkPreConfig.h"
#include "SkUserConfig.h"
#include "SkPostConfig.h"
#include <stdint.h>

/** \file SkTypes.h
*/

/** See SkGraphics::GetVersion() to retrieve these at runtime
 */
#define SKIA_VERSION_MAJOR  1
#define SKIA_VERSION_MINOR  0
#define SKIA_VERSION_PATCH  0

/*
    memory wrappers to be implemented by the porting layer (platform)
*/

/** Called internally if we run out of memory. The platform implementation must
    not return, but should either throw an exception or otherwise exit.
*/
SK_API extern void sk_out_of_memory(void);
/** Called internally if we hit an unrecoverable error.
    The platform implementation must not return, but should either throw
    an exception or otherwise exit.
*/
SK_API extern void sk_throw(void);

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

// bzero is safer than memset, but we can't rely on it, so... sk_bzero()
static inline void sk_bzero(void* buffer, size_t size) {
    memset(buffer, 0, size);
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_OVERRIDE_GLOBAL_NEW
#include <new>

inline void* operator new(size_t size) {
    return sk_malloc_throw(size);
}

inline void operator delete(void* p) {
    sk_free(p);
}
#endif

///////////////////////////////////////////////////////////////////////////////

#define SK_INIT_TO_AVOID_WARNING    = 0

#ifndef SkDebugf
    SK_API void SkDebugf(const char format[], ...);
#endif

#ifdef SK_DEBUG
    #define SkASSERT(cond)              SK_ALWAYSBREAK(cond)
    #define SkDEBUGFAIL(message)        SkASSERT(false && message)
    #define SkDEBUGCODE(code)           code
    #define SkDECLAREPARAM(type, var)   , type var
    #define SkPARAM(var)                , var
//  #define SkDEBUGF(args       )       SkDebugf##args
    #define SkDEBUGF(args       )       SkDebugf args
    #define SkAssertResult(cond)        SkASSERT(cond)
#else
    #define SkASSERT(cond)
    #define SkDEBUGFAIL(message)
    #define SkDEBUGCODE(code)
    #define SkDEBUGF(args)
    #define SkDECLAREPARAM(type, var)
    #define SkPARAM(var)

    // unlike SkASSERT, this guy executes its condition in the non-debug build
    #define SkAssertResult(cond)        cond
#endif

#define SkFAIL(message)                 SK_ALWAYSBREAK(false && message)

// We want to evaluate cond only once, and inside the SkASSERT somewhere so we see its string form.
// So we use the comma operator to make an SkDebugf that always returns false: we'll evaluate cond,
// and if it's true the assert passes; if it's false, we'll print the message and the assert fails.
#define SkASSERTF(cond, fmt, ...)       SkASSERT((cond) || (SkDebugf(fmt"\n", __VA_ARGS__), false))

#ifdef SK_DEVELOPER
    #define SkDEVCODE(code)             code
#else
    #define SkDEVCODE(code)
#endif

#ifdef SK_IGNORE_TO_STRING
    #define SK_TO_STRING_NONVIRT()
    #define SK_TO_STRING_VIRT()
    #define SK_TO_STRING_PUREVIRT()
    #define SK_TO_STRING_OVERRIDE()
#else
    // the 'toString' helper functions convert Sk* objects to human-readable
    // form in developer mode
    #define SK_TO_STRING_NONVIRT() void toString(SkString* str) const;
    #define SK_TO_STRING_VIRT() virtual void toString(SkString* str) const;
    #define SK_TO_STRING_PUREVIRT() virtual void toString(SkString* str) const = 0;
    #define SK_TO_STRING_OVERRIDE() virtual void toString(SkString* str) const SK_OVERRIDE;
#endif

template <bool>
struct SkCompileAssert {
};

// Uses static_cast<bool>(expr) instead of bool(expr) due to
// https://connect.microsoft.com/VisualStudio/feedback/details/832915

// The extra parentheses in SkCompileAssert<(...)> are a work around for
// http://gcc.gnu.org/bugzilla/show_bug.cgi?id=57771
// which was fixed in gcc 4.8.2.
#define SK_COMPILE_ASSERT(expr, msg) \
    typedef SkCompileAssert<(static_cast<bool>(expr))> \
            msg[static_cast<bool>(expr) ? 1 : -1] SK_UNUSED

/*
 *  Usage:  SK_MACRO_CONCAT(a, b)   to construct the symbol ab
 *
 *  SK_MACRO_CONCAT_IMPL_PRIV just exists to make this work. Do not use directly
 *
 */
#define SK_MACRO_CONCAT(X, Y)           SK_MACRO_CONCAT_IMPL_PRIV(X, Y)
#define SK_MACRO_CONCAT_IMPL_PRIV(X, Y)  X ## Y

/*
 *  Usage: SK_MACRO_APPEND_LINE(foo)    to make foo123, where 123 is the current
 *                                      line number. Easy way to construct
 *                                      unique names for local functions or
 *                                      variables.
 */
#define SK_MACRO_APPEND_LINE(name)  SK_MACRO_CONCAT(name, __LINE__)

/**
 * For some classes, it's almost always an error to instantiate one without a name, e.g.
 *   {
 *       SkAutoMutexAcquire(&mutex);
 *       <some code>
 *   }
 * In this case, the writer meant to hold mutex while the rest of the code in the block runs,
 * but instead the mutex is acquired and then immediately released.  The correct usage is
 *   {
 *       SkAutoMutexAcquire lock(&mutex);
 *       <some code>
 *   }
 *
 * To prevent callers from instantiating your class without a name, use SK_REQUIRE_LOCAL_VAR
 * like this:
 *   class classname {
 *       <your class>
 *   };
 *   #define classname(...) SK_REQUIRE_LOCAL_VAR(classname)
 *
 * This won't work with templates, and you must inline the class' constructors and destructors.
 * Take a look at SkAutoFree and SkAutoMalloc in this file for examples.
 */
#define SK_REQUIRE_LOCAL_VAR(classname) \
    SK_COMPILE_ASSERT(false, missing_name_for_##classname)

///////////////////////////////////////////////////////////////////////

/**
 *  Fast type for signed 8 bits. Use for parameter passing and local variables,
 *  not for storage.
 */
typedef int S8CPU;

/**
 *  Fast type for unsigned 8 bits. Use for parameter passing and local
 *  variables, not for storage
 */
typedef unsigned U8CPU;

/**
 *  Fast type for signed 16 bits. Use for parameter passing and local variables,
 *  not for storage
 */
typedef int S16CPU;

/**
 *  Fast type for unsigned 16 bits. Use for parameter passing and local
 *  variables, not for storage
 */
typedef unsigned U16CPU;

/**
 *  Meant to be faster than bool (doesn't promise to be 0 or 1,
 *  just 0 or non-zero
 */
typedef int SkBool;

/**
 *  Meant to be a small version of bool, for storage purposes. Will be 0 or 1
 */
typedef uint8_t SkBool8;

#ifdef SK_DEBUG
    SK_API int8_t      SkToS8(intmax_t);
    SK_API uint8_t     SkToU8(uintmax_t);
    SK_API int16_t     SkToS16(intmax_t);
    SK_API uint16_t    SkToU16(uintmax_t);
    SK_API int32_t     SkToS32(intmax_t);
    SK_API uint32_t    SkToU32(uintmax_t);
    SK_API int         SkToInt(intmax_t);
    SK_API unsigned    SkToUInt(uintmax_t);
    SK_API size_t      SkToSizeT(uintmax_t);
#else
    #define SkToS8(x)   ((int8_t)(x))
    #define SkToU8(x)   ((uint8_t)(x))
    #define SkToS16(x)  ((int16_t)(x))
    #define SkToU16(x)  ((uint16_t)(x))
    #define SkToS32(x)  ((int32_t)(x))
    #define SkToU32(x)  ((uint32_t)(x))
    #define SkToInt(x)  ((int)(x))
    #define SkToUInt(x) ((unsigned)(x))
    #define SkToSizeT(x) ((size_t)(x))
#endif

/** Returns 0 or 1 based on the condition
*/
#define SkToBool(cond)  ((cond) != 0)

#define SK_MaxS16   32767
#define SK_MinS16   -32767
#define SK_MaxU16   0xFFFF
#define SK_MinU16   0
#define SK_MaxS32   0x7FFFFFFF
#define SK_MinS32   -SK_MaxS32
#define SK_MaxU32   0xFFFFFFFF
#define SK_MinU32   0
#define SK_NaN32    (1 << 31)

/** Returns true if the value can be represented with signed 16bits
 */
static inline bool SkIsS16(long x) {
    return (int16_t)x == x;
}

/** Returns true if the value can be represented with unsigned 16bits
 */
static inline bool SkIsU16(long x) {
    return (uint16_t)x == x;
}

//////////////////////////////////////////////////////////////////////////////
#ifndef SK_OFFSETOF
    #define SK_OFFSETOF(type, field)    (size_t)((char*)&(((type*)1)->field) - (char*)1)
#endif

/** Returns the number of entries in an array (not a pointer)
*/
#define SK_ARRAY_COUNT(array)       (sizeof(array) / sizeof(array[0]))

#define SkAlign2(x)     (((x) + 1) >> 1 << 1)
#define SkIsAlign2(x)   (0 == ((x) & 1))

#define SkAlign4(x)     (((x) + 3) >> 2 << 2)
#define SkIsAlign4(x)   (0 == ((x) & 3))

#define SkAlign8(x)     (((x) + 7) >> 3 << 3)
#define SkIsAlign8(x)   (0 == ((x) & 7))

#define SkAlignPtr(x)   (sizeof(void*) == 8 ?   SkAlign8(x) :   SkAlign4(x))
#define SkIsAlignPtr(x) (sizeof(void*) == 8 ? SkIsAlign8(x) : SkIsAlign4(x))

typedef uint32_t SkFourByteTag;
#define SkSetFourByteTag(a, b, c, d)    (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

/** 32 bit integer to hold a unicode value
*/
typedef int32_t SkUnichar;
/** 32 bit value to hold a millisecond count
*/
typedef uint32_t SkMSec;
/** 1 second measured in milliseconds
*/
#define SK_MSec1 1000
/** maximum representable milliseconds
*/
#define SK_MSecMax 0x7FFFFFFF
/** Returns a < b for milliseconds, correctly handling wrap-around from 0xFFFFFFFF to 0
*/
#define SkMSec_LT(a, b)     ((int32_t)(a) - (int32_t)(b) < 0)
/** Returns a <= b for milliseconds, correctly handling wrap-around from 0xFFFFFFFF to 0
*/
#define SkMSec_LE(a, b)     ((int32_t)(a) - (int32_t)(b) <= 0)

/** The generation IDs in Skia reserve 0 has an invalid marker.
 */
#define SK_InvalidGenID     0
/** The unique IDs in Skia reserve 0 has an invalid marker.
 */
#define SK_InvalidUniqueID  0

/****************************************************************************
    The rest of these only build with C++
*/
#ifdef __cplusplus

/** Faster than SkToBool for integral conditions. Returns 0 or 1
*/
static inline int Sk32ToBool(uint32_t n) {
    return (n | (0-n)) >> 31;
}

/** Generic swap function. Classes with efficient swaps should specialize this function to take
    their fast path. This function is used by SkTSort. */
template <typename T> inline void SkTSwap(T& a, T& b) {
    T c(a);
    a = b;
    b = c;
}

static inline int32_t SkAbs32(int32_t value) {
    if (value < 0) {
        value = -value;
    }
    return value;
}

template <typename T> inline T SkTAbs(T value) {
    if (value < 0) {
        value = -value;
    }
    return value;
}

static inline int32_t SkMax32(int32_t a, int32_t b) {
    if (a < b)
        a = b;
    return a;
}

static inline int32_t SkMin32(int32_t a, int32_t b) {
    if (a > b)
        a = b;
    return a;
}

template <typename T> const T& SkTMin(const T& a, const T& b) {
    return (a < b) ? a : b;
}

template <typename T> const T& SkTMax(const T& a, const T& b) {
    return (b < a) ? a : b;
}

static inline int32_t SkSign32(int32_t a) {
    return (a >> 31) | ((unsigned) -a >> 31);
}

static inline int32_t SkFastMin32(int32_t value, int32_t max) {
    if (value > max) {
        value = max;
    }
    return value;
}

/** Returns signed 32 bit value pinned between min and max, inclusively
*/
static inline int32_t SkPin32(int32_t value, int32_t min, int32_t max) {
    if (value < min) {
        value = min;
    }
    if (value > max) {
        value = max;
    }
    return value;
}

static inline uint32_t SkSetClearShift(uint32_t bits, bool cond,
                                       unsigned shift) {
    SkASSERT((int)cond == 0 || (int)cond == 1);
    return (bits & ~(1 << shift)) | ((int)cond << shift);
}

static inline uint32_t SkSetClearMask(uint32_t bits, bool cond,
                                      uint32_t mask) {
    return cond ? bits | mask : bits & ~mask;
}

///////////////////////////////////////////////////////////////////////////////

/** Use to combine multiple bits in a bitmask in a type safe way.
 */
template <typename T>
T SkTBitOr(T a, T b) {
    return (T)(a | b);
}

/**
 *  Use to cast a pointer to a different type, and maintaining strict-aliasing
 */
template <typename Dst> Dst SkTCast(const void* ptr) {
    union {
        const void* src;
        Dst dst;
    } data;
    data.src = ptr;
    return data.dst;
}

//////////////////////////////////////////////////////////////////////////////

/** \class SkNoncopyable

SkNoncopyable is the base class for objects that may do not want to
be copied. It hides its copy-constructor and its assignment-operator.
*/
class SK_API SkNoncopyable {
public:
    SkNoncopyable() {}

private:
    SkNoncopyable(const SkNoncopyable&);
    SkNoncopyable& operator=(const SkNoncopyable&);
};

class SkAutoFree : SkNoncopyable {
public:
    SkAutoFree() : fPtr(NULL) {}
    explicit SkAutoFree(void* ptr) : fPtr(ptr) {}
    ~SkAutoFree() { sk_free(fPtr); }

    /** Return the currently allocate buffer, or null
    */
    void* get() const { return fPtr; }

    /** Assign a new ptr allocated with sk_malloc (or null), and return the
        previous ptr. Note it is the caller's responsibility to sk_free the
        returned ptr.
    */
    void* set(void* ptr) {
        void* prev = fPtr;
        fPtr = ptr;
        return prev;
    }

    /** Transfer ownership of the current ptr to the caller, setting the
        internal reference to null. Note the caller is reponsible for calling
        sk_free on the returned address.
    */
    void* detach() { return this->set(NULL); }

    /** Free the current buffer, and set the internal reference to NULL. Same
        as calling sk_free(detach())
    */
    void free() {
        sk_free(fPtr);
        fPtr = NULL;
    }

private:
    void* fPtr;
    // illegal
    SkAutoFree(const SkAutoFree&);
    SkAutoFree& operator=(const SkAutoFree&);
};
#define SkAutoFree(...) SK_REQUIRE_LOCAL_VAR(SkAutoFree)

/**
 *  Manage an allocated block of heap memory. This object is the sole manager of
 *  the lifetime of the block, so the caller must not call sk_free() or delete
 *  on the block, unless detach() was called.
 */
class SkAutoMalloc : SkNoncopyable {
public:
    explicit SkAutoMalloc(size_t size = 0) {
        fPtr = size ? sk_malloc_throw(size) : NULL;
        fSize = size;
    }

    ~SkAutoMalloc() {
        sk_free(fPtr);
    }

    /**
     *  Passed to reset to specify what happens if the requested size is smaller
     *  than the current size (and the current block was dynamically allocated).
     */
    enum OnShrink {
        /**
         *  If the requested size is smaller than the current size, and the
         *  current block is dynamically allocated, free the old block and
         *  malloc a new block of the smaller size.
         */
        kAlloc_OnShrink,

        /**
         *  If the requested size is smaller than the current size, and the
         *  current block is dynamically allocated, just return the old
         *  block.
         */
        kReuse_OnShrink
    };

    /**
     *  Reallocates the block to a new size. The ptr may or may not change.
     */
    void* reset(size_t size, OnShrink shrink = kAlloc_OnShrink,  bool* didChangeAlloc = NULL) {
        if (size == fSize || (kReuse_OnShrink == shrink && size < fSize)) {
            if (didChangeAlloc) {
                *didChangeAlloc = false;
            }
            return fPtr;
        }

        sk_free(fPtr);
        fPtr = size ? sk_malloc_throw(size) : NULL;
        fSize = size;
        if (didChangeAlloc) {
            *didChangeAlloc = true;
        }

        return fPtr;
    }

    /**
     *  Releases the block back to the heap
     */
    void free() {
        this->reset(0);
    }

    /**
     *  Return the allocated block.
     */
    void* get() { return fPtr; }
    const void* get() const { return fPtr; }

   /** Transfer ownership of the current ptr to the caller, setting the
       internal reference to null. Note the caller is reponsible for calling
       sk_free on the returned address.
    */
    void* detach() {
        void* ptr = fPtr;
        fPtr = NULL;
        fSize = 0;
        return ptr;
    }

private:
    void*   fPtr;
    size_t  fSize;  // can be larger than the requested size (see kReuse)
};
#define SkAutoMalloc(...) SK_REQUIRE_LOCAL_VAR(SkAutoMalloc)

/**
 *  Manage an allocated block of memory. If the requested size is <= kSize, then
 *  the allocation will come from the stack rather than the heap. This object
 *  is the sole manager of the lifetime of the block, so the caller must not
 *  call sk_free() or delete on the block.
 */
template <size_t kSize> class SkAutoSMalloc : SkNoncopyable {
public:
    /**
     *  Creates initially empty storage. get() returns a ptr, but it is to
     *  a zero-byte allocation. Must call reset(size) to return an allocated
     *  block.
     */
    SkAutoSMalloc() {
        fPtr = fStorage;
        fSize = kSize;
    }

    /**
     *  Allocate a block of the specified size. If size <= kSize, then the
     *  allocation will come from the stack, otherwise it will be dynamically
     *  allocated.
     */
    explicit SkAutoSMalloc(size_t size) {
        fPtr = fStorage;
        fSize = kSize;
        this->reset(size);
    }

    /**
     *  Free the allocated block (if any). If the block was small enought to
     *  have been allocated on the stack (size <= kSize) then this does nothing.
     */
    ~SkAutoSMalloc() {
        if (fPtr != (void*)fStorage) {
            sk_free(fPtr);
        }
    }

    /**
     *  Return the allocated block. May return non-null even if the block is
     *  of zero size. Since this may be on the stack or dynamically allocated,
     *  the caller must not call sk_free() on it, but must rely on SkAutoSMalloc
     *  to manage it.
     */
    void* get() const { return fPtr; }

    /**
     *  Return a new block of the requested size, freeing (as necessary) any
     *  previously allocated block. As with the constructor, if size <= kSize
     *  then the return block may be allocated locally, rather than from the
     *  heap.
     */
    void* reset(size_t size,
                SkAutoMalloc::OnShrink shrink = SkAutoMalloc::kAlloc_OnShrink,
                bool* didChangeAlloc = NULL) {
        size = (size < kSize) ? kSize : size;
        bool alloc = size != fSize && (SkAutoMalloc::kAlloc_OnShrink == shrink || size > fSize);
        if (didChangeAlloc) {
            *didChangeAlloc = alloc;
        }
        if (alloc) {
            if (fPtr != (void*)fStorage) {
                sk_free(fPtr);
            }

            if (size == kSize) {
                SkASSERT(fPtr != fStorage); // otherwise we lied when setting didChangeAlloc.
                fPtr = fStorage;
            } else {
                fPtr = sk_malloc_flags(size, SK_MALLOC_THROW | SK_MALLOC_TEMP);
            }

            fSize = size;
        }
        SkASSERT(fSize >= size && fSize >= kSize);
        SkASSERT((fPtr == fStorage) || fSize > kSize);
        return fPtr;
    }

private:
    void*       fPtr;
    size_t      fSize;  // can be larger than the requested size (see kReuse)
    uint32_t    fStorage[(kSize + 3) >> 2];
};
// Can't guard the constructor because it's a template class.

#endif /* C++ */

#endif
