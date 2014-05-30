/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLazyPtr_DEFINED
#define SkLazyPtr_DEFINED

/** Declare a lazily-chosen static pointer (or array of pointers) of type F.
 *
 *  Example usage:
 *
 *  Foo* CreateFoo() { return SkNEW(Foo); }
 *  Foo* GetSingletonFoo() {
 *      SK_DECLARE_STATIC_LAZY_PTR(Foo, singleton, CreateFoo);  // Clean up with SkDELETE.
 *      return singleton.get();
 *  }
 *
 *  These macros take an optional void (*Destroy)(T*) at the end. If not given, we'll use SkDELETE.
 *  This option is most useful when T doesn't have a public destructor.
 *
 *  void CustomCleanup(Foo* ptr) { ... }
 *  Foo* GetSingletonFooWithCustomCleanup() {
 *      SK_DECLARE_STATIC_LAZY_PTR(Foo, singleton, CreateFoo, CustomCleanup);
 *      return singleton.get();
 *  }
 *
 *  If you have a bunch of related static pointers of the same type, you can
 *  declare an array of lazy pointers together:
 *
 *  Foo* CreateFoo(int i) { return ...; }
 *  Foo* GetCachedFoo(Foo::Enum enumVal) {
 *      SK_DECLARE_STATIC_LAZY_PTR_ARRAY(Foo, Foo::kEnumCount, cachedFoos, CreateFoo);
 *      return cachedFoos[enumVal];
 *  }
 *
 *
 *  You can think of SK_DECLARE_STATIC_LAZY_PTR as a cheaper specialization of
 *  SkOnce.  There is no mutex or extra storage used past the pointer itself.
 *  In debug mode, each lazy pointer will be cleaned up at process exit so we
 *  can check that we've not leaked or freed them early.
 *
 *  We may call Create more than once, but all threads will see the same pointer
 *  returned from get().  Any extra calls to Create will be cleaned up.
 *
 *  These macros must be used in a global or function scope, not as a class member.
 */

#define SK_DECLARE_STATIC_LAZY_PTR(T, name, Create, ...) \
    static Private::SkLazyPtr<T, Create, ##__VA_ARGS__> name

#define SK_DECLARE_STATIC_LAZY_PTR_ARRAY(T, name, N, Create, ...) \
    static Private::SkLazyPtrArray<T, N, Create, ##__VA_ARGS__> name



// Everything below here is private implementation details.  Don't touch, don't even look.

#include "SkDynamicAnnotations.h"
#include "SkThread.h"
#include "SkThreadPriv.h"

// See FIXME below.
class SkFontConfigInterface;
class SkTypeface;

namespace Private {

template <typename T> void sk_delete(T* ptr) { SkDELETE(ptr); }

// Set *dst to ptr if *dst is NULL.  Returns value of *dst, destroying ptr if not swapped in.
// Issues the same memory barriers as sk_atomic_cas: acquire on failure, release on success.
template <typename P, void (*Destroy)(P)>
static P try_cas(void** dst, P ptr) {
    P prev = (P)sk_atomic_cas(dst, NULL, ptr);

    if (prev) {
        // We need an acquire barrier before returning prev, which sk_atomic_cas provided.
        Destroy(ptr);
        return prev;
    } else {
        // We need a release barrier before returning ptr, which sk_atomic_cas provided.
        return ptr;
    }
}

// This has no constructor and must be zero-initalized (the macro above does this).
template <typename T, T* (*Create)(), void (*Destroy)(T*) = sk_delete<T> >
class SkLazyPtr {
public:
    T* get() {
        // If fPtr has already been filled, we need an acquire barrier when loading it.
        // If not, we need a release barrier when setting it.  try_cas will do that.
        T* ptr = (T*)sk_acquire_load(&fPtr);
        return ptr ? ptr : try_cas<T*, Destroy>(&fPtr, Create());
    }

#ifdef SK_DEBUG
    // FIXME: We know we leak refs on some classes.  For now, let them leak.
    void cleanup(SkFontConfigInterface*) {}
    void cleanup(SkTypeface*) {}
    template <typename U> void cleanup(U* ptr) { Destroy(ptr); }

    ~SkLazyPtr() {
        this->cleanup((T*)fPtr);
        fPtr = NULL;
    }
#endif

private:
    void* fPtr;
};

// This has no constructor and must be zero-initalized (the macro above does this).
template <typename T, int N, T* (*Create)(int), void (*Destroy)(T*) = sk_delete<T> >
class SkLazyPtrArray {
public:
    T* operator[](int i) {
        SkASSERT(i >= 0 && i < N);
        // If fPtr has already been filled, we need an acquire barrier when loading it.
        // If not, we need a release barrier when setting it.  try_cas will do that.
        T* ptr = (T*)sk_acquire_load(&fArray[i]);
        return ptr ? ptr : try_cas<T*, Destroy>(&fArray[i], Create(i));
    }

#ifdef SK_DEBUG
    ~SkLazyPtrArray() {
        for (int i = 0; i < N; i++) {
            Destroy((T*)fArray[i]);
            fArray[i] = NULL;
        }
    }
#endif

private:
    void* fArray[N];
};

}  // namespace Private

#endif//SkLazyPtr_DEFINED
