/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkScopeExit_DEFINED
#define SkScopeExit_DEFINED

#include "SkTypes.h"

/**
 * SK_AT_SCOPE_EXIT(stmt) evaluates stmt when the current scope ends.
 *
 * E.g.
 *    {
 *        int x = 5;
 *        {
 *           SK_AT_SCOPE_EXIT(x--);
 *           SkASSERT(x == 5);
 *        }
 *        SkASSERT(x == 4);
 *    }
 */

template <typename Fn>
class SkScopeExit {
public:
    SkScopeExit(Fn f) : fFn(std::move(f)) {}
    SkScopeExit(SkScopeExit&& that) : fFn(std::move(that.fFn)) {}

    ~SkScopeExit() { Caller::Call(fFn); }

    SkScopeExit& operator=(SkScopeExit&& that) {
        fFn = std::move(that.fFn);
        return *this;
    }

private:
    template<bool Test> struct C;
    template<> struct C<true> {
        static void Call(Fn& fn) { return (fn == nullptr) ? void() : fn(); }
    };
    template<> struct C<false> {
        static void Call(Fn& fn) { fn(); }
    };
    using Caller = C<std::is_constructible<Fn, nullptr_t>::value>;

    Fn fFn;

    SkScopeExit(           const SkScopeExit& ) = delete;
    SkScopeExit& operator=(const SkScopeExit& ) = delete;
};

template <typename Fn>
inline SkScopeExit<Fn> SK_WARN_UNUSED_RESULT SkMakeScopeExit(Fn&& fn) {
    return {std::move(fn)};
}

#define SK_AT_SCOPE_EXIT(stmt)                              \
    SK_UNUSED auto&& SK_MACRO_APPEND_LINE(at_scope_exit_) = \
        SkMakeScopeExit([&]() { stmt; });

#endif  // SkScopeExit_DEFINED
