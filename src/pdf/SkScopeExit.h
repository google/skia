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
    ~SkScopeExit() { fFn(); }

private:
    Fn fFn;

    SkScopeExit(           const SkScopeExit& ) = delete;
    SkScopeExit& operator=(const SkScopeExit& ) = delete;
    SkScopeExit(                 SkScopeExit&&) = delete;
    SkScopeExit& operator=(      SkScopeExit&&) = delete;
};

template <typename Fn>
inline SkScopeExit<Fn> SkMakeScopeExit(Fn&& fn) {
    return {std::move(fn)};
}

#define SK_AT_SCOPE_EXIT(stmt)                              \
    SK_UNUSED auto&& SK_MACRO_APPEND_LINE(at_scope_exit_) = \
        SkMakeScopeExit([&]() { stmt; });

#endif  // SkScopeExit_DEFINED
