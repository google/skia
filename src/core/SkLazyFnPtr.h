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
 *     SK_DECLARE_STATIC_LAZY_FN_PTR(FooImpl, choice);
 *     return choice.get(choose_foo)(a, b);
 *  }
 *
 *  You can think of SK_DECLARE_STATIC_LAZY_FN_PTR as a cheaper specialization of SkOnce.
 *  There is no mutex, and in the fast path, no memory barriers are issued.
 *
 *  This must be used in a global or function scope, not as a class member.
 */
#define SK_DECLARE_STATIC_LAZY_FN_PTR(F, name) static Private::SkLazyFnPtr<F> name = { NULL }


// Everything below here is private implementation details.  Don't touch, don't even look.

#include "SkDynamicAnnotations.h"
#include "SkThread.h"

namespace Private {

// This has no constructor and is link-time initialized, so its members must be public.
template <typename F>
struct SkLazyFnPtr {
    F get(F (*choose)()) {
        // First, try reading to see if it's already set.
        F fn = (F)SK_ANNOTATE_UNPROTECTED_READ(fPtr);
        if (fn != NULL) {
            return fn;
        }

        // We think it's not already set.
        fn = choose();

        // No particular memory barriers needed; we're not guarding anything but the pointer itself.
        F prev = (F)sk_atomic_cas(&fPtr, NULL, (void*)fn);

        // If prev != NULL, someone snuck in and set fPtr concurrently.
        // If prev == NULL, we did write fn to fPtr.
        return prev != NULL ? prev : fn;
    }

    void* fPtr;
};

}  // namespace Private

#endif//SkLazyFnPtr_DEFINED
