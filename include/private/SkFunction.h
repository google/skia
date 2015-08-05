/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFunction_DEFINED
#define SkFunction_DEFINED

// TODO: document, more pervasive move support in constructors, small-Fn optimization

#include "SkTemplates.h"
#include "SkTypes.h"

template <typename> class SkFunction;

template <typename R, typename... Args>
class SkFunction<R(Args...)> {
public:
    SkFunction() {}

    template <typename Fn>
    SkFunction(const Fn& fn) : fFunction(SkNEW_ARGS(LambdaImpl<Fn>, (fn))) {}

    SkFunction(R (*fn)(Args...)) : fFunction(SkNEW_ARGS(FnPtrImpl, (fn))) {}

    SkFunction(const SkFunction& other) { *this = other; }
    SkFunction& operator=(const SkFunction& other) {
        if (this != &other) {
            fFunction.reset(other.fFunction ? other.fFunction->clone() : nullptr);
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
        Interface* clone() const override { return SkNEW_ARGS(LambdaImpl<Fn>, (fFn)); }
    private:
        Fn fFn;
    };

    class FnPtrImpl final : public Interface {
    public:
        FnPtrImpl(R (*fn)(Args...)) : fFn(fn) {}

        R call(Args... args) const override { return fFn(skstd::forward<Args>(args)...); }
        Interface* clone() const override { return SkNEW_ARGS(FnPtrImpl, (fFn)); }
    private:
        R (*fFn)(Args...);
    };

    SkAutoTDelete<Interface> fFunction;
};

#endif//SkFunction_DEFINED
