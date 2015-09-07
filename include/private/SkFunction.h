/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFunction_DEFINED
#define SkFunction_DEFINED

// TODO: document, more pervasive move support in constructors, small-Fn optimization

#include "SkUtility.h"
#include "SkUniquePtr.h"
#include "SkTypes.h"

template <typename> class SkFunction;

template <typename R, typename... Args>
class SkFunction<R(Args...)> {
public:
    SkFunction() {}

    template <typename Fn>
    SkFunction(const Fn& fn)
        : fFunction(new LambdaImpl<Fn>(fn)) {}

    SkFunction(R (*fn)(Args...)) : fFunction(new FnPtrImpl(fn)) {}

    SkFunction(const SkFunction& other) { *this = other; }
    SkFunction& operator=(const SkFunction& other) {
        if (this != &other) {
            fFunction.reset(other.fFunction.get() ? other.fFunction->clone() : nullptr);
        }
        return *this;
    }

    R operator()(Args... args) const {
        SkASSERT(fFunction.get());
        return fFunction->call(skstd::forward<Args>(args)...);
    }

private:
    struct Interface {
        virtual ~Interface() {}
        virtual R call(Args...) const = 0;
        virtual Interface* clone() const = 0;
    };

    template <typename Fn>
    class LambdaImpl final : public Interface {
    public:
        LambdaImpl(const Fn& fn) : fFn(fn) {}

        R call(Args... args) const override { return fFn(skstd::forward<Args>(args)...); }
        Interface* clone() const override { return new LambdaImpl<Fn>(fFn); }

    private:
        Fn fFn;
    };

    class FnPtrImpl final : public Interface {
    public:
        FnPtrImpl(R (*fn)(Args...)) : fFn(fn) {}

        R call(Args... args) const override { return fFn(skstd::forward<Args>(args)...); }
        Interface* clone() const override { return new FnPtrImpl(fFn); }

    private:
        R (*fFn)(Args...);
    };

    skstd::unique_ptr<Interface> fFunction;
};

#endif//SkFunction_DEFINED
