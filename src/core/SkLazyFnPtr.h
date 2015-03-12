/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLazyFnPtr_DEFINED
#define SkLazyFnPtr_DEFINED

/** Declare a lazily-chosen static function pointer of type F.
 *
 *  Example usage:
 *
 *  typedef int (*FooImpl)(int, int);
 *
 *  static FooImpl choose_foo() { return ... };
 *
 *  int Foo(int a, int b) {
 *     SK_DECLARE_STATIC_LAZY_FN_PTR(FooImpl, foo, choose_foo);
 *     return foo.get()(a, b);
 *  }
 *
 *  You can think of SK_DECLARE_STATIC_LAZY_FN_PTR as a cheaper specialization of SkOnce.
 *  There is no mutex, and in the fast path, no memory barriers are issued.
 *
 *  This must be used in a global or function scope, not as a class member.
 */
#define SK_DECLARE_STATIC_LAZY_FN_PTR(F, name, Choose) static Private::SkLazyFnPtr<F, Choose> name


// Everything below here is private implementation details.  Don't touch, don't even look.

#include "SkAtomics.h"

namespace Private {

// This has no constructor and must be zero-initialized (the macro above does this).
template <typename F, F (*Choose)()>
class SkLazyFnPtr {
public:
    F get() {
        // First, try reading to see if it's already set.
        F fn = (F)sk_atomic_load(&fPtr, sk_memory_order_relaxed);
        if (fn != NULL) {
            return fn;
        }

        // We think it's not already set.
        fn = Choose();

        // No particular memory barriers needed; we're not guarding anything but the pointer itself.
        F prev = (F)sk_atomic_cas(&fPtr, NULL, (void*)fn);

        // If prev != NULL, someone snuck in and set fPtr concurrently.
        // If prev == NULL, we did write fn to fPtr.
        return prev != NULL ? prev : fn;
    }

private:
    void* fPtr;
};

}  // namespace Private

#endif//SkLazyFnPtr_DEFINED
