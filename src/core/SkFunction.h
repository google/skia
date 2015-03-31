/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFunction_DEFINED
#define SkFunction_DEFINED

// TODO: document

#include "SkTypes.h"

template <typename> class SkFunction;

template <typename R, typename... Args>
class SkFunction<R(Args...)> : SkNoncopyable {
public:
    explicit SkFunction(R (*fn)(Args...)) : fVTable(GetFunctionPointerVTable()) {
        // We've been passed a function pointer.  We'll just store it.
        fFunction = reinterpret_cast<void*>(fn);
    }

    template <typename Fn>
    explicit SkFunction(Fn fn) : fVTable(GetVTable<Fn>()) {
        // We've got a functor.  The basic thing we can always do is copy it onto the heap.
        fFunction = SkNEW_ARGS(Fn, (fn));
    }

    ~SkFunction() { fVTable.fDelete(fFunction); }

    R operator()(Args... args) { return fVTable.fCall(fFunction, args...); }

private:
    struct VTable {
        R (*fCall)(void*, Args...);
        void (*fDelete)(void*);
    };

    static const VTable& GetFunctionPointerVTable() {
        static const VTable vtable = {
            [](void* fn, Args... args) { return reinterpret_cast<R(*)(Args...)>(fn)(args...); },
            [](void*) { /* Don't delete function pointers. */ },
        };
        return vtable;
    }

    template <typename Fn>
    static const VTable& GetVTable() {
        static const VTable vtable = {
            [](void* fn, Args... args) { return (*static_cast<Fn*>(fn))(args...); },
            [](void* fn) { SkDELETE(static_cast<Fn*>(fn)); },
        };
        return vtable;
    }

    void* fFunction;        // Either a function pointer, or a pointer to a functor.
    const VTable& fVTable;  // How to call, delete (and one day copy, move) fFunction.
};

// TODO:
//   - is it worth moving fCall out of the VTable into SkFunction itself to avoid the indirection?
//   - should constructors be implicit?
//   - make SkFunction copyable
//   - emulate std::forward for moveable functors (e.g. lambdas)
//   - forward args too?
//   - implement small-object optimization to store functors inline

#endif//SkFunction_DEFINED
