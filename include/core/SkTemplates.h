
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkTemplates_DEFINED
#define SkTemplates_DEFINED

#include "../private/SkTLogic.h"
#include "SkMath.h"
#include "SkTypes.h"
#include <limits.h>
#include <new>

/** \file SkTemplates.h

    This file contains light-weight template classes for type-safe and exception-safe
    resource management.
*/

/**
 *  Marks a local variable as known to be unused (to avoid warnings).
 *  Note that this does *not* prevent the local variable from being optimized away.
 */
template<typename T> inline void sk_ignore_unused_variable(const T&) { }

namespace skstd {

template <typename T> inline remove_reference_t<T>&& move(T&& t) {
  return static_cast<remove_reference_t<T>&&>(t);
}

template <typename T> inline T&& forward(remove_reference_t<T>& t) /*noexcept*/ {
    return static_cast<T&&>(t);
}
template <typename T> inline T&& forward(remove_reference_t<T>&& t) /*noexcept*/ {
    static_assert(!is_lvalue_reference<T>::value,
                  "Forwarding an rvalue reference as an lvalue reference is not allowed.");
    return static_cast<T&&>(t);
}

}  // namespace skstd

///@{
/** SkTConstType<T, CONST>::type will be 'const T' if CONST is true, 'T' otherwise. */
template <typename T, bool CONST> struct SkTConstType {
    typedef T type;
};
template <typename T> struct SkTConstType<T, true> {
    typedef const T type;
};
///@}

/**
 *  Returns a pointer to a D which comes immediately after S[count].
 */
template <typename D, typename S> static D* SkTAfter(S* ptr, size_t count = 1) {
    return reinterpret_cast<D*>(ptr + count);
}

/**
 *  Returns a pointer to a D which comes byteOffset bytes after S.
 */
template <typename D, typename S> static D* SkTAddOffset(S* ptr, size_t byteOffset) {
    // The intermediate char* has the same const-ness as D as this produces better error messages.
    // This relies on the fact that reinterpret_cast can add constness, but cannot remove it.
    return reinterpret_cast<D*>(
        reinterpret_cast<typename SkTConstType<char, SkTIsConst<D>::value>::type*>(ptr) + byteOffset
    );
}

/** \class SkAutoTCallVProc

    Call a function when this goes out of scope. The template uses two
    parameters, the object, and a function that is to be called in the destructor.
    If detach() is called, the object reference is set to null. If the object
    reference is null when the destructor is called, we do not call the
    function.
*/
template <typename T, void (*P)(T*)> class SkAutoTCallVProc : SkNoncopyable {
public:
    SkAutoTCallVProc(T* obj): fObj(obj) {}
    ~SkAutoTCallVProc() { if (fObj) P(fObj); }

    operator T*() const { return fObj; }
    T* operator->() const { SkASSERT(fObj); return fObj; }

    T* detach() { T* obj = fObj; fObj = NULL; return obj; }
    void reset(T* obj = NULL) {
        if (fObj != obj) {
            if (fObj) {
                P(fObj);
            }
            fObj = obj;
        }
    }
private:
    T* fObj;
};

/** \class SkAutoTCallIProc

Call a function when this goes out of scope. The template uses two
parameters, the object, and a function that is to be called in the destructor.
If detach() is called, the object reference is set to null. If the object
reference is null when the destructor is called, we do not call the
function.
*/
template <typename T, int (*P)(T*)> class SkAutoTCallIProc : SkNoncopyable {
public:
    SkAutoTCallIProc(T* obj): fObj(obj) {}
    ~SkAutoTCallIProc() { if (fObj) P(fObj); }

    operator T*() const { return fObj; }
    T* operator->() const { SkASSERT(fObj); return fObj; }

    T* detach() { T* obj = fObj; fObj = NULL; return obj; }
private:
    T* fObj;
};

/** \class SkAutoTDelete
  An SkAutoTDelete<T> is like a T*, except that the destructor of SkAutoTDelete<T>
  automatically deletes the pointer it holds (if any).  That is, SkAutoTDelete<T>
  owns the T object that it points to.  Like a T*, an SkAutoTDelete<T> may hold
  either NULL or a pointer to a T object.  Also like T*, SkAutoTDelete<T> is
  thread-compatible, and once you dereference it, you get the threadsafety
  guarantees of T.

  The size of a SkAutoTDelete is small: sizeof(SkAutoTDelete<T>) == sizeof(T*)
*/
template <typename T> class SkAutoTDelete : SkNoncopyable {
public:
    SkAutoTDelete(T* obj = NULL) : fObj(obj) {}
    ~SkAutoTDelete() { SkDELETE(fObj); }

    T* get() const { return fObj; }
    operator T*() const { return fObj; }
    T& operator*() const { SkASSERT(fObj); return *fObj; }
    T* operator->() const { SkASSERT(fObj); return fObj; }

    void reset(T* obj) {
        if (fObj != obj) {
            SkDELETE(fObj);
            fObj = obj;
        }
    }

    /**
     *  Delete the owned object, setting the internal pointer to NULL.
     */
    void free() {
        SkDELETE(fObj);
        fObj = NULL;
    }

    /**
     *  Transfer ownership of the object to the caller, setting the internal
     *  pointer to NULL. Note that this differs from get(), which also returns
     *  the pointer, but it does not transfer ownership.
     */
    T* detach() {
        T* obj = fObj;
        fObj = NULL;
        return obj;
    }

    void swap(SkAutoTDelete* that) {
        SkTSwap(fObj, that->fObj);
    }

private:
    T*  fObj;
};

// Calls ~T() in the destructor.
template <typename T> class SkAutoTDestroy : SkNoncopyable {
public:
    SkAutoTDestroy(T* obj = NULL) : fObj(obj) {}
    ~SkAutoTDestroy() {
        if (fObj) {
            fObj->~T();
        }
    }

    T* get() const { return fObj; }
    T& operator*() const { SkASSERT(fObj); return *fObj; }
    T* operator->() const { SkASSERT(fObj); return fObj; }

private:
    T*  fObj;
};

template <typename T> class SkAutoTDeleteArray : SkNoncopyable {
public:
    SkAutoTDeleteArray(T array[]) : fArray(array) {}
    ~SkAutoTDeleteArray() { SkDELETE_ARRAY(fArray); }

    T*      get() const { return fArray; }
    void    free() { SkDELETE_ARRAY(fArray); fArray = NULL; }
    T*      detach() { T* array = fArray; fArray = NULL; return array; }

    void reset(T array[]) {
        if (fArray != array) {
            SkDELETE_ARRAY(fArray);
            fArray = array;
        }
    }

private:
    T*  fArray;
};

/** Allocate an array of T elements, and free the array in the destructor
 */
template <typename T> class SkAutoTArray : SkNoncopyable {
public:
    SkAutoTArray() {
        fArray = NULL;
        SkDEBUGCODE(fCount = 0;)
    }
    /** Allocate count number of T elements
     */
    explicit SkAutoTArray(int count) {
        SkASSERT(count >= 0);
        fArray = NULL;
        if (count) {
            fArray = SkNEW_ARRAY(T, count);
        }
        SkDEBUGCODE(fCount = count;)
    }

    /** Reallocates given a new count. Reallocation occurs even if new count equals old count.
     */
    void reset(int count) {
        SkDELETE_ARRAY(fArray);
        SkASSERT(count >= 0);
        fArray = NULL;
        if (count) {
            fArray = SkNEW_ARRAY(T, count);
        }
        SkDEBUGCODE(fCount = count;)
    }

    ~SkAutoTArray() {
        SkDELETE_ARRAY(fArray);
    }

    /** Return the array of T elements. Will be NULL if count == 0
     */
    T* get() const { return fArray; }

    /** Return the nth element in the array
     */
    T&  operator[](int index) const {
        SkASSERT((unsigned)index < (unsigned)fCount);
        return fArray[index];
    }

    void swap(SkAutoTArray& other) {
        SkTSwap(fArray, other.fArray);
        SkDEBUGCODE(SkTSwap(fCount, other.fCount));
    }

private:
    T*  fArray;
    SkDEBUGCODE(int fCount;)
};

/** Wraps SkAutoTArray, with room for up to N elements preallocated
 */
template <int N, typename T> class SkAutoSTArray : SkNoncopyable {
public:
    /** Initialize with no objects */
    SkAutoSTArray() {
        fArray = NULL;
        fCount = 0;
    }

    /** Allocate count number of T elements
     */
    SkAutoSTArray(int count) {
        fArray = NULL;
        fCount = 0;
        this->reset(count);
    }

    ~SkAutoSTArray() {
        this->reset(0);
    }

    /** Destroys previous objects in the array and default constructs count number of objects */
    void reset(int count) {
        T* start = fArray;
        T* iter = start + fCount;
        while (iter > start) {
            (--iter)->~T();
        }

        if (fCount != count) {
            if (fCount > N) {
                // 'fArray' was allocated last time so free it now
                SkASSERT((T*) fStorage != fArray);
                sk_free(fArray);
            }

            if (count > N) {
                const uint64_t size64 = sk_64_mul(count, sizeof(T));
                const size_t size = static_cast<size_t>(size64);
                if (size != size64) {
                    sk_out_of_memory();
                }
                fArray = (T*) sk_malloc_throw(size);
            } else if (count > 0) {
                fArray = (T*) fStorage;
            } else {
                fArray = NULL;
            }

            fCount = count;
        }

        iter = fArray;
        T* stop = fArray + count;
        while (iter < stop) {
            SkNEW_PLACEMENT(iter++, T);
        }
    }

    /** Return the number of T elements in the array
     */
    int count() const { return fCount; }

    /** Return the array of T elements. Will be NULL if count == 0
     */
    T* get() const { return fArray; }

    /** Return the nth element in the array
     */
    T&  operator[](int index) const {
        SkASSERT(index < fCount);
        return fArray[index];
    }

private:
    int     fCount;
    T*      fArray;
    // since we come right after fArray, fStorage should be properly aligned
    char    fStorage[N * sizeof(T)];
};

/** Manages an array of T elements, freeing the array in the destructor.
 *  Does NOT call any constructors/destructors on T (T must be POD).
 */
template <typename T> class SkAutoTMalloc : SkNoncopyable {
public:
    /** Takes ownership of the ptr. The ptr must be a value which can be passed to sk_free. */
    explicit SkAutoTMalloc(T* ptr = NULL) {
        fPtr = ptr;
    }

    /** Allocates space for 'count' Ts. */
    explicit SkAutoTMalloc(size_t count) {
        fPtr = (T*)sk_malloc_flags(count * sizeof(T), SK_MALLOC_THROW);
    }

    ~SkAutoTMalloc() {
        sk_free(fPtr);
    }

    /** Resize the memory area pointed to by the current ptr preserving contents. */
    void realloc(size_t count) {
        fPtr = reinterpret_cast<T*>(sk_realloc_throw(fPtr, count * sizeof(T)));
    }

    /** Resize the memory area pointed to by the current ptr without preserving contents. */
    void reset(size_t count) {
        sk_free(fPtr);
        fPtr = (T*)sk_malloc_flags(count * sizeof(T), SK_MALLOC_THROW);
    }

    T* get() const { return fPtr; }

    operator T*() {
        return fPtr;
    }

    operator const T*() const {
        return fPtr;
    }

    T& operator[](int index) {
        return fPtr[index];
    }

    const T& operator[](int index) const {
        return fPtr[index];
    }

    /**
     *  Transfer ownership of the ptr to the caller, setting the internal
     *  pointer to NULL. Note that this differs from get(), which also returns
     *  the pointer, but it does not transfer ownership.
     */
    T* detach() {
        T* ptr = fPtr;
        fPtr = NULL;
        return ptr;
    }

private:
    T* fPtr;
};

template <size_t N, typename T> class SkAutoSTMalloc : SkNoncopyable {
public:
    SkAutoSTMalloc() : fPtr(fTStorage) {}

    SkAutoSTMalloc(size_t count) {
        if (count > N) {
            fPtr = (T*)sk_malloc_flags(count * sizeof(T), SK_MALLOC_THROW | SK_MALLOC_TEMP);
        } else {
            fPtr = fTStorage;
        }
    }

    ~SkAutoSTMalloc() {
        if (fPtr != fTStorage) {
            sk_free(fPtr);
        }
    }

    // doesn't preserve contents
    T* reset(size_t count) {
        if (fPtr != fTStorage) {
            sk_free(fPtr);
        }
        if (count > N) {
            fPtr = (T*)sk_malloc_throw(count * sizeof(T));
        } else {
            fPtr = fTStorage;
        }
        return fPtr;
    }

    T* get() const { return fPtr; }

    operator T*() {
        return fPtr;
    }

    operator const T*() const {
        return fPtr;
    }

    T& operator[](int index) {
        return fPtr[index];
    }

    const T& operator[](int index) const {
        return fPtr[index];
    }

    // Reallocs the array, can be used to shrink the allocation.  Makes no attempt to be intelligent
    void realloc(size_t count) {
        if (count > N) {
            if (fPtr == fTStorage) {
                fPtr = (T*)sk_malloc_throw(count * sizeof(T));
                memcpy(fPtr, fTStorage, N * sizeof(T));
            } else {
                fPtr = (T*)sk_realloc_throw(fPtr, count * sizeof(T));
            }
        } else if (fPtr != fTStorage) {
            fPtr = (T*)sk_realloc_throw(fPtr, count * sizeof(T));
        }
    }

private:
    T*          fPtr;
    union {
        uint32_t    fStorage32[(N*sizeof(T) + 3) >> 2];
        T           fTStorage[1];   // do NOT want to invoke T::T()
    };
};

//////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *  Pass the object and the storage that was offered during SkInPlaceNewCheck, and this will
 *  safely destroy (and free if it was dynamically allocated) the object.
 */
template <typename T> void SkInPlaceDeleteCheck(T* obj, void* storage) {
    if (storage == obj) {
        obj->~T();
    } else {
        SkDELETE(obj);
    }
}

/**
 *  Allocates T, using storage if it is large enough, and allocating on the heap (via new) if
 *  storage is not large enough.
 *
 *      obj = SkInPlaceNewCheck<Type>(storage, size);
 *      ...
 *      SkInPlaceDeleteCheck(obj, storage);
 */
template <typename T> T* SkInPlaceNewCheck(void* storage, size_t size) {
    return (sizeof(T) <= size) ? new (storage) T : SkNEW(T);
}

template <typename T, typename A1, typename A2, typename A3>
T* SkInPlaceNewCheck(void* storage, size_t size, const A1& a1, const A2& a2, const A3& a3) {
    return (sizeof(T) <= size) ? new (storage) T(a1, a2, a3) : SkNEW_ARGS(T, (a1, a2, a3));
}

/**
 * Reserves memory that is aligned on double and pointer boundaries.
 * Hopefully this is sufficient for all practical purposes.
 */
template <size_t N> class SkAlignedSStorage : SkNoncopyable {
public:
    size_t size() const { return N; }
    void* get() { return fData; }
    const void* get() const { return fData; }

private:
    union {
        void*   fPtr;
        double  fDouble;
        char    fData[N];
    };
};

/**
 * Reserves memory that is aligned on double and pointer boundaries.
 * Hopefully this is sufficient for all practical purposes. Otherwise,
 * we have to do some arcane trickery to determine alignment of non-POD
 * types. Lifetime of the memory is the lifetime of the object.
 */
template <int N, typename T> class SkAlignedSTStorage : SkNoncopyable {
public:
    /**
     * Returns void* because this object does not initialize the
     * memory. Use placement new for types that require a cons.
     */
    void* get() { return fStorage.get(); }
private:
    SkAlignedSStorage<sizeof(T)*N> fStorage;
};

#endif
