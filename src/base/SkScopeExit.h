/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkScopeExit_DEFINED
#define SkScopeExit_DEFINED

#include "include/private/base/SkMacros.h"

#include <functional>
#include <utility>

/** SkScopeExit calls a std:::function<void()> in its destructor. */
class SkScopeExit {
public:
    SkScopeExit() = default;
    SkScopeExit(std::function<void()> f) : fFn(std::move(f)) {}
    SkScopeExit(SkScopeExit&& that) : fFn(std::move(that.fFn)) {}

    ~SkScopeExit() {
        if (fFn) {
            fFn();
        }
    }

    void clear() { fFn = {}; }

    SkScopeExit& operator=(SkScopeExit&& that) {
        fFn = std::move(that.fFn);
        return *this;
    }

private:
    std::function<void()> fFn;

    SkScopeExit(           const SkScopeExit& ) = delete;
    SkScopeExit& operator=(const SkScopeExit& ) = delete;
};

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
#define SK_AT_SCOPE_EXIT(stmt)                              \
    SkScopeExit SK_MACRO_APPEND_LINE(at_scope_exit_)([&]() { stmt; })

#endif  // SkScopeExit_DEFINED
