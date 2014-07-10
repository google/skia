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
 *  Foo* GetSingletonFoo() {
 *      SK_DECLARE_STATIC_LAZY_PTR(Foo, singleton);  // Created with SkNEW, destroyed with SkDELETE.
 *      return singleton.get();
 *  }
 *
 *  These macros take an optional T* (*Create)() and void (*Destroy)(T*) at the end.
 *  If not given, we'll use SkNEW and SkDELETE.
 *  These options are most useful when T doesn't have a public constructor or destructor.
 *  Create comes first, so you may use a custom Create with a default Destroy, but not vice versa.
 *
 *  Foo* CustomCreate() { return ...; }
 *  void CustomDestroy(Foo* ptr) { ... }
 *  Foo* GetSingletonFooWithCustomCleanup() {
 *      SK_DECLARE_STATIC_LAZY_PTR(Foo, singleton, CustomCreate, CustomDestroy);
 *      return singleton.get();
 *  }
 *
 *  If you have a bunch of related static pointers of the same type, you can
 *  declare an array of lazy pointers together, and we'll pass the index to Create().
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

#define SK_DECLARE_STATIC_LAZY_PTR(T, name, ...) \
    static Private::SkLazyPtr<T, ##__VA_ARGS__> name

#define SK_DECLARE_STATIC_LAZY_PTR_ARRAY(T, name, N, ...) \
    static Private::SkLazyPtrArray<T, N, ##__VA_ARGS__> name



// Everything below here is private implementation details.  Don't touch, don't even look.

#include "SkDynamicAnnotations.h"
#include "SkThread.h"
#include "SkThreadPriv.h"

// See FIXME below.
class SkFontConfigInterfaceDirect;

namespace Private {

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

template <typename T> T* sk_new() { return SkNEW(T); }
template <typename T> void sk_delete(T* ptr) { SkDELETE(ptr); }

// We're basing these implementations here on this article:
//   http://preshing.com/20140709/the-purpose-of-memory_order_consume-in-cpp11/
//
// Because the users of SkLazyPtr and SkLazyPtrArray will read the pointers
// _through_ our atomically set pointer, there is a data dependency between our
// atomic and the guarded data, and so we only need writer-releases /
// reader-consumes memory pairing rather than the more general write-releases /
// reader-acquires convention.
//
// This is nice, because a sk_consume_load is free on all our platforms: x86,
// ARM, MIPS.  In contrast, sk_acquire_load issues a memory barrier on non-x86.

// This has no constructor and must be zero-initalized (the macro above does this).
template <typename T, T* (*Create)() = sk_new<T>, void (*Destroy)(T*) = sk_delete<T> >
class SkLazyPtr {
public:
    T* get() {
        // If fPtr has already been filled, we need a consume barrier when loading it.
        // If not, we need a release barrier when setting it.  try_cas will do that.
        T* ptr = (T*)sk_consume_load(&fPtr);
        return ptr ? ptr : try_cas<T*, Destroy>(&fPtr, Create());
    }

#ifdef SK_DEVELOPER
    // FIXME: We know we leak refs on some classes.  For now, let them leak.
    void cleanup(SkFontConfigInterfaceDirect*) {}
    template <typename U> void cleanup(U* ptr) { Destroy(ptr); }

    ~SkLazyPtr() {
        this->cleanup((T*)fPtr);
        fPtr = NULL;
    }
#endif

private:
    void* fPtr;
};

template <typename T> T* sk_new_arg(int i) { return SkNEW_ARGS(T, (i)); }

// This has no constructor and must be zero-initalized (the macro above does this).
template <typename T, int N, T* (*Create)(int) = sk_new_arg<T>, void (*Destroy)(T*) = sk_delete<T> >
class SkLazyPtrArray {
public:
    T* operator[](int i) {
        SkASSERT(i >= 0 && i < N);
        // If fPtr has already been filled, we need an consume barrier when loading it.
        // If not, we need a release barrier when setting it.  try_cas will do that.
        T* ptr = (T*)sk_consume_load(&fArray[i]);
        return ptr ? ptr : try_cas<T*, Destroy>(&fArray[i], Create(i));
    }

#ifdef SK_DEVELOPER
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
