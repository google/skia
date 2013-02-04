
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkTemplates_DEFINED
#define SkTemplates_DEFINED

#include "SkTypes.h"
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

/**
 *  SkTIsConst<T>::value is true if the type T is const.
 *  The type T is constrained not to be an array or reference type.
 */
template <typename T> struct SkTIsConst {
    static T* t;
    static uint16_t test(const volatile void*);
    static uint32_t test(volatile void *);
    static const bool value = (sizeof(uint16_t) == sizeof(test(t)));
};

///@{
/** SkTConstType<T, CONST>::type will be 'const T' if CONST is true, 'T' otherwise. */
template <typename T, bool CONST> struct SkTConstType {
    typedef T type;
};
template <typename T> struct SkTConstType<T, true> {
    typedef const T type;
};
///@}

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
    T* detach() { T* obj = fObj; fObj = NULL; return obj; }
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
    T* detach() { T* obj = fObj; fObj = NULL; return obj; }
private:
    T* fObj;
};

// See also SkTScopedPtr.
template <typename T> class SkAutoTDelete : SkNoncopyable {
public:
    SkAutoTDelete(T* obj) : fObj(obj) {}
    ~SkAutoTDelete() { delete fObj; }

    T* get() const { return fObj; }
    T& operator*() const { SkASSERT(fObj); return *fObj; }
    T* operator->() const { SkASSERT(fObj); return fObj; }

    /**
     *  Delete the owned object, setting the internal pointer to NULL.
     */
    void free() {
        delete fObj;
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
            fArray = new T[count];
        }
        SkDEBUGCODE(fCount = count;)
    }

    /** Reallocates given a new count. Reallocation occurs even if new count equals old count.
     */
    void reset(int count) {
        delete[] fArray;
        SkASSERT(count >= 0);
        fArray = NULL;
        if (count) {
            fArray = new T[count];
        }
        SkDEBUGCODE(fCount = count;)
    }

    ~SkAutoTArray() {
        delete[] fArray;
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

private:
    T*  fArray;
    SkDEBUGCODE(int fCount;)
};

/** Wraps SkAutoTArray, with room for up to N elements preallocated
 */
template <size_t N, typename T> class SkAutoSTArray : SkNoncopyable {
public:
    /** Allocate count number of T elements
     */
    SkAutoSTArray(size_t count) {
        if (count > N) {
            fArray = new T[count];
        } else if (count) {
            fArray = new (fStorage) T[count];
        } else {
            fArray = NULL;
        }
        fCount = count;
    }

    ~SkAutoSTArray() {
        if (fCount > N) {
            delete[] fArray;
        } else {
            T* start = fArray;
            T* iter = start + fCount;
            while (iter > start) {
                (--iter)->~T();
            }
        }
    }

    /** Return the number of T elements in the array
     */
    size_t count() const { return fCount; }

    /** Return the array of T elements. Will be NULL if count == 0
     */
    T* get() const { return fArray; }

    /** Return the nth element in the array
     */
    T&  operator[](int index) const {
        SkASSERT((unsigned)index < fCount);
        return fArray[index];
    }

private:
    size_t  fCount;
    T*      fArray;
    // since we come right after fArray, fStorage should be properly aligned
    char    fStorage[N * sizeof(T)];
};

/** Allocate a temp array on the stack/heap.
    Does NOT call any constructors/destructors on T (i.e. T must be POD)
*/
template <typename T> class SkAutoTMalloc : SkNoncopyable {
public:
    SkAutoTMalloc(size_t count) {
        fPtr = (T*)sk_malloc_flags(count * sizeof(T), SK_MALLOC_THROW | SK_MALLOC_TEMP);
    }

    ~SkAutoTMalloc() {
        sk_free(fPtr);
    }

    // doesn't preserve contents
    void reset (size_t count) {
        sk_free(fPtr);
        fPtr = fPtr = (T*)sk_malloc_flags(count * sizeof(T), SK_MALLOC_THROW | SK_MALLOC_TEMP);
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

private:
    T*  fPtr;
};

template <size_t N, typename T> class SK_API SkAutoSTMalloc : SkNoncopyable {
public:
    SkAutoSTMalloc(size_t count) {
        if (count <= N) {
            fPtr = fTStorage;
        } else {
            fPtr = (T*)sk_malloc_flags(count * sizeof(T), SK_MALLOC_THROW | SK_MALLOC_TEMP);
        }
    }

    ~SkAutoSTMalloc() {
        if (fPtr != fTStorage) {
            sk_free(fPtr);
        }
    }

    // doesn't preserve contents
    void reset(size_t count) {
        if (fPtr != fTStorage) {
            sk_free(fPtr);
        }
        if (count <= N) {
            fPtr = fTStorage;
        } else {
            fPtr = (T*)sk_malloc_flags(count * sizeof(T), SK_MALLOC_THROW | SK_MALLOC_TEMP);
        }
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

private:
    T*          fPtr;
    union {
        uint32_t    fStorage32[(N*sizeof(T) + 3) >> 2];
        T           fTStorage[1];   // do NOT want to invoke T::T()
    };
};

/**
 * Reserves memory that is aligned on double and pointer boundaries.
 * Hopefully this is sufficient for all practical purposes.
 */
template <size_t N> class SkAlignedSStorage : SkNoncopyable {
public:
    void* get() { return fData; }
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
